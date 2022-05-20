#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "parse.h"
#include "value.h"
#include "map.h"
#include "string.h"
#include "common.h"

#define PROGRAM_VERSION "v0.83.0"

static int run_help(void);
static int run_version(void);
static int run_file(string_t *file, string_t *initial);
static int run_text(string_t *text, string_t *initial);
static int run_interactive(void);
static int record_script(string_t *document, stack_frame_t *frame);
static int print_value(value_t *value);
static value_t *initialize_arguments(string_t *document);
static void destroy_expression_unsafe(void *expression);

int main(int argc, char **argv)
{
    int help, version, text, file, interactive;
    int argument, modes;

    help = version = text = file = interactive = 0;

    for (argument = 1; argument < argc; argument++)
    {
        int character;

        if (strlen(argv[argument]) == 0 || argv[argument][0] != '-' || strcmp(argv[argument], "--") == 0)
        {
            break;
        }

        for (character = 1; character < strlen(argv[argument]); character++)
        {
            switch (argv[argument][character])
            {
                case 'h':
                    help = 1;
                    break;
                case 'v':
                    version = 1;
                    break;
                case 'f':
                    file = 1;
                    break;
                case 't':
                    text = 1;
                    break;
                case 'i':
                    interactive = 1;
                    break;
                default:
                    crash_with_message("illegal option %c", argv[argument][character]);
                    break;
            }
        }
    }

    modes = text + file + interactive;

    if (modes > 1)
    {
        crash_with_message("too many modes selected");
    }

    if (help)
    {
        return run_help();
    }

    if (version)
    {
        return run_version();
    }

    if (!is_portable())
    {
        crash_with_message("interpreter is not portable");
    }

    if (!modes || file)
    {
        if (argument == argc)
        {
            crash_with_message("missing operand");
        }

        if (argc - argument > 1)
        {
            return run_file(cstring_to_string(argv[argument]), cstring_to_string(argv[argument + 1]));
        }
        else
        {
            return run_file(cstring_to_string(argv[argument]), NULL);
        }
    }

    if (text)
    {
        if (argument == argc)
        {
            crash_with_message("missing operand");
        }

        if (argc - argument > 1)
        {
            return run_text(cstring_to_string(argv[argument]), cstring_to_string(argv[argument + 1]));
        }
        else
        {
            return run_text(cstring_to_string(argv[argument]), NULL);
        }
    }

    if (interactive)
    {
        return run_interactive();
    }

    return run_help();
}

static int run_help(void)
{
    printf("Usage:\n");
    printf("  %s -f script [arguments]\n", PROGRAM_NAME);
    printf("  %s -t script [arguments]\n", PROGRAM_NAME);
    printf("  %s -i\n", PROGRAM_NAME);
    printf("\n");
    printf("Options:\n");
    printf("  -h  Show help.\n");
    printf("  -v  Show version.\n");
    printf("  -f  Set program to file mode. Default mode.\n");
    printf("  -t  Set program to text mode.\n");
    printf("  -i  Set program to interactive mode.\n");
    return PROGRAM_SUCCESS;
}

static int run_version(void)
{
    printf("%s\n", PROGRAM_VERSION);
    return PROGRAM_SUCCESS;
}

static int run_file(string_t *file, string_t *initial)
{
    value_t *document;
    int exit;

    document = read_file(file);
    exit = 0;

    if (document->thrown || document->type == VALUE_TYPE_NULL)
    {
        exit = print_value(document);
    }
    else if (document->type == VALUE_TYPE_STRING)
    {
        exit = run_text(copy_string(view_string(document)), initial);
    }
    else
    {
        crash_with_message("unsupported branch %s", "CLI_FILE_TYPE");
    }

    destroy_value(document);

    return exit;
}

static int run_text(string_t *text, string_t *initial)
{
    expression_t *item;
    argument_iterator_t arguments;
    stack_frame_t frame;
    int success;

    item = allocate(sizeof(expression_t));
    item->type = EXPRESSION_TYPE_VALUE;
    item->value = initialize_arguments(initial);
    item->arguments = NULL;
    arguments.expressions = empty_list(destroy_expression_unsafe);
    add_list_item(arguments.expressions, item);
    arguments.index = 0;

    if (arguments.expressions && arguments.expressions->length > 0)
    {
        arguments.current = arguments.expressions->head;
        arguments.evaluated = allocate(sizeof(value_t *) * arguments.expressions->length);
    }
    else
    {
        arguments.evaluated = NULL;
    }

    frame.variables = empty_variables();
    frame.overloads = empty_overloads();
    frame.operators = default_operators();
    frame.depth = 0;
    frame.arguments = &arguments;
    frame.caller = NULL;
    success = record_script(text, &frame);
    destroy_map(frame.variables);
    destroy_map(frame.overloads);
    destroy_map(frame.operators);

    if (arguments.evaluated)
    {
        size_t index;

        for (index = 0; index < arguments.index; index++)
        {
            value_t *value;

            value = arguments.evaluated[index];

            if (value)
            {
                destroy_value(value);
            }
        }

        free(arguments.evaluated);
    }

    destroy_list(arguments.expressions);

    return success ? PROGRAM_SUCCESS : PROGRAM_ERROR;
}

static int run_interactive(void)
{
    expression_t *item;
    argument_iterator_t arguments;
    stack_frame_t frame;

    item = allocate(sizeof(expression_t));
    item->type = EXPRESSION_TYPE_VALUE;
    item->value = new_null();
    item->arguments = NULL;
    arguments.expressions = empty_list(destroy_expression_unsafe);
    add_list_item(arguments.expressions, item);
    arguments.index = 0;

    if (arguments.expressions && arguments.expressions->length > 0)
    {
        arguments.current = arguments.expressions->head;
        arguments.evaluated = allocate(sizeof(value_t *) * arguments.expressions->length);
    }
    else
    {
        arguments.evaluated = NULL;
    }

    frame.variables = empty_variables();
    frame.overloads = empty_overloads();
    frame.operators = default_operators();
    frame.depth = 0;
    frame.arguments = &arguments;
    frame.caller = NULL;

    while (1)
    {
        string_t *line;
        size_t capacity, fill;
        int key, success;

        printf("> ");
        capacity = 1024;
        fill = 0;
        line = create_string(allocate(sizeof(char) * capacity), capacity);

        do
        {
            key = getchar();

            if (key == EOF)
            {
                destroy_map(frame.variables);
                destroy_map(frame.overloads);
                destroy_map(frame.operators);
                destroy_string(line);

                if (arguments.evaluated)
                {
                    size_t index;

                    for (index = 0; index < arguments.index; index++)
                    {
                        value_t *value;

                        value = arguments.evaluated[index];

                        if (value)
                        {
                            destroy_value(value);
                        }
                    }

                    free(arguments.evaluated);
                }

                destroy_list(arguments.expressions);

                return PROGRAM_SUCCESS;
            }
            else
            {
                if (fill == line->length)
                {
                    resize_string(line, fill * 2);
                }

                line->bytes[fill++] = key;
            }
        } while (key != '\n');

        resize_string(line, fill);
        success = record_script(line, &frame);

        if (!success)
        {
            destroy_map(frame.variables);
            destroy_map(frame.overloads);
            destroy_map(frame.operators);

            if (arguments.evaluated)
            {
                size_t index;

                for (index = 0; index < arguments.index; index++)
                {
                    value_t *value;

                    value = arguments.evaluated[index];

                    if (value)
                    {
                        destroy_value(value);
                    }
                }

                free(arguments.evaluated);
            }

            destroy_list(arguments.expressions);

            return PROGRAM_ERROR;
        }

        fflush(stdout);
    }
}

static int record_script(string_t *document, stack_frame_t *frame)
{
    value_t *value;
    int success;

    value = execute_script(document, frame);
    success = print_value(value);
    destroy_value(value);

    return success;
}

static int print_value(value_t *value)
{
    value_t *represent;
    int success;

    represent = represent_value(value);
    success = 0;

    if (represent->thrown)
    {
        print_value(represent);
    }
    else
    {
        if (represent->type == VALUE_TYPE_STRING)
        {
            string_t *string;

            string = view_string(represent);
            fwrite(string->bytes, string->length, 1, stdout);
            fwrite("\n", 1, 1, stdout);
            success = !value->thrown;
        }
        else
        {
            crash_with_message("unsupported branch %s", "CLI_REPRESENT_TYPE");
        }
    }

    destroy_value(represent);

    return success;
}

static value_t *initialize_arguments(string_t *document)
{
    if (document)
    {
        argument_iterator_t arguments;
        value_t *result;
        stack_frame_t frame;
        int success;

        arguments.expressions = empty_list(destroy_expression_unsafe);
        arguments.index = 0;

        if (arguments.expressions && arguments.expressions->length > 0)
        {
            arguments.current = arguments.expressions->head;
            arguments.evaluated = allocate(sizeof(value_t *) * arguments.expressions->length);
        }
        else
        {
            arguments.evaluated = NULL;
        }

        frame.variables = empty_variables();
        frame.overloads = empty_overloads();
        frame.operators = default_operators();
        frame.depth = 0;
        frame.caller = NULL;
        result = execute_script(document, &frame);
        success = !result->thrown;
        destroy_map(frame.variables);
        destroy_map(frame.overloads);
        destroy_map(frame.operators);

        if (arguments.evaluated)
        {
            size_t index;

            for (index = 0; index < arguments.index; index++)
            {
                value_t *value;

                value = arguments.evaluated[index];

                if (value)
                {
                    destroy_value(value);
                }
            }

            free(arguments.evaluated);
        }

        destroy_list(arguments.expressions);

        if (!success)
        {
            print_value(result);
            crash();
        }

        return result;
    }

    return new_null();
}

static void destroy_expression_unsafe(void *expression)
{
    destroy_expression((expression_t *) expression);
}
