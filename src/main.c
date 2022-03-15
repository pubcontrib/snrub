#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "execute.h"
#include "value.h"
#include "map.h"
#include "string.h"
#include "common.h"

#define PROGRAM_VERSION "v0.78.0"

static int run_help(void);
static int run_version(void);
static int run_file(string_t *file, string_t *initial);
static int run_text(string_t *text, string_t *initial);
static int run_interactive(void);
static int record_script(string_t *document, value_t *arguments, map_t *variables, map_t *overloads);
static int print_value(value_t *value);
static value_t *initialize_arguments(string_t *document);

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
    string_t *document;
    int exit;

    document = read_file(file);

    if (!document)
    {
        document = create_string(NULL, 0);
    }

    exit = run_text(document, initial);

    return exit;
}

static int run_text(string_t *text, string_t *initial)
{
    value_t *arguments;
    map_t *variables, *overloads;
    int success;

    arguments = initialize_arguments(initial);
    variables = empty_variables();
    overloads = empty_overloads();
    success = record_script(text, arguments, variables, overloads);
    destroy_value(arguments);
    destroy_map(variables);
    destroy_map(overloads);

    return success ? PROGRAM_SUCCESS : PROGRAM_ERROR;
}

static int run_interactive(void)
{
    value_t *arguments;
    map_t *variables, *overloads;

    arguments = new_null();
    variables = empty_variables();
    overloads = empty_overloads();

    while (1)
    {
        line_t *line;
        int success;

        printf("> ");
        line = next_line();

        if (line->exit)
        {
            destroy_value(arguments);
            destroy_map(variables);
            destroy_map(overloads);
            destroy_line(line);
            return PROGRAM_SUCCESS;
        }

        success = record_script(line->string, arguments, variables, overloads);
        line->string = NULL;
        destroy_line(line);

        if (!success)
        {
            destroy_value(arguments);
            destroy_map(variables);
            destroy_map(overloads);
            return PROGRAM_ERROR;
        }

        fflush(stdout);
    }
}

static int record_script(string_t *document, value_t *arguments, map_t *variables, map_t *overloads)
{
    value_t *value;
    int success;

    value = execute_script(document, arguments, variables, overloads);
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
        value_t *null, *arguments;
        map_t *variables, *overloads;
        int success;

        null = new_null();
        variables = empty_variables();
        overloads = empty_overloads();
        arguments = execute_script(document, null, variables, overloads);
        success = !arguments->thrown;
        destroy_value(null);
        destroy_map(variables);
        destroy_map(overloads);

        if (!success)
        {
            print_value(arguments);
            crash();
        }

        return arguments;
    }

    return new_null();
}
