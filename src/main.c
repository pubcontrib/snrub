#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "execute.h"
#include "value.h"
#include "map.h"
#include "common.h"

#define PROGRAM_NAME "snrub"
#define PROGRAM_VERSION "v0.54.2"
#define PROGRAM_SUCCESS 0
#define PROGRAM_ERROR 1

static int run_help(void);
static int run_version(void);
static int run_file(char *file, char *initial);
static int run_text(char *text, char *initial);
static int run_interactive(void);
static int record_script(char *document, map_t *globals, value_t *arguments);
static value_t *initialize_arguments(char *document);
static map_t *empty_variables(void);
static void destroy_value_unsafe(void *value);
static void print_error(error_t error);
static void crash(void);

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
                    fprintf(stderr, "%s: illegal option %c\n", PROGRAM_NAME, argv[argument][character]);
                    crash();
            }
        }
    }

    modes = text + file + interactive;

    if (modes > 1)
    {
        fprintf(stderr, "%s: modes are mutually exclusive\n", PROGRAM_NAME);
        crash();
    }

    if (help)
    {
        return run_help();
    }

    if (version)
    {
        return run_version();
    }

    if (!modes || file)
    {
        if (argument == argc)
        {
            fprintf(stderr, "%s: missing operand\n", PROGRAM_NAME);
            crash();
        }

        if (argc - argument > 1)
        {
            return run_file(argv[argument], argv[argument + 1]);
        }
        else
        {
            return run_file(argv[argument], NULL);
        }
    }

    if (text)
    {
        if (argument == argc)
        {
            fprintf(stderr, "%s: missing operand\n", PROGRAM_NAME);
            crash();
        }

        if (argc - argument > 1)
        {
            return run_text(argv[argument], argv[argument + 1]);
        }
        else
        {
            return run_text(argv[argument], NULL);
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

static int run_file(char *file, char *initial)
{
    char *document;
    int exit;

    document = read_file(file);

    if (!document)
    {
        print_error(ERROR_IO);
        crash();
    }

    exit = run_text(document, initial);
    free(document);

    return exit;
}

static int run_text(char *text, char *initial)
{
    map_t *globals;
    value_t *arguments;
    int success;

    globals = empty_variables();

    if (!globals)
    {
        crash();
    }

    arguments = initialize_arguments(initial);

    if (!arguments)
    {
        crash();
    }

    success = record_script(text, globals, arguments);
    destroy_map(globals);
    destroy_value(arguments);

    return success ? PROGRAM_SUCCESS : PROGRAM_ERROR;
}

static int run_interactive(void)
{
    map_t *globals;
    value_t *arguments;

    globals = empty_variables();

    if (!globals)
    {
        crash();
    }

    arguments = new_null();

    if (!arguments)
    {
        crash();
    }

    while (1)
    {
        line_t *line;
        int success;

        printf("> ");
        line = next_line();

        if (!line)
        {
            crash();
        }

        if (line->exit)
        {
            destroy_map(globals);
            destroy_value(arguments);
            destroy_line(line);
            return PROGRAM_SUCCESS;
        }

        success = record_script(line->string, globals, arguments);
        destroy_line(line);

        if (!success)
        {
            destroy_map(globals);
            destroy_value(arguments);
            return PROGRAM_ERROR;
        }

        fflush(stdout);
    }
}

static int record_script(char *document, map_t *globals, value_t *arguments)
{
    value_t *value;
    char *represent;
    int success;

    value = execute_script(document, globals, arguments);

    if (!value)
    {
        crash();
    }

    represent = represent_value(value);

    if (!represent)
    {
        crash();
    }

    success = value->type != TYPE_ERROR;
    destroy_value(value);
    printf("%s\n", represent);
    free(represent);

    return success;
}

static value_t *initialize_arguments(char *document)
{
    if (document)
    {
        map_t *globals;
        value_t *null, *arguments;
        int success;

        globals = empty_variables();

        if (!globals)
        {
            crash();
        }

        null = new_null();

        if (!null)
        {
            crash();
        }

        arguments = execute_script(document, globals, null);

        if (!arguments)
        {
            crash();
        }

        success = arguments->type != TYPE_ERROR;
        destroy_map(globals);
        destroy_value(null);

        if (!success)
        {
            char *represent;

            represent = represent_value(arguments);

            if (!represent)
            {
                crash();
            }

            printf("%s\n", represent);
            crash();
        }

        return arguments;
    }

    return new_null();
}

static map_t *empty_variables(void)
{
    return empty_map(hash_string, destroy_value_unsafe, 64);
}

static void destroy_value_unsafe(void *value)
{
    destroy_value((value_t *) value);
}

static void print_error(error_t error)
{
    char *represent;

    represent = represent_error(error);
    printf("%s\n", represent);
    free(represent);
}

static void crash(void)
{
    exit(PROGRAM_ERROR);
}
