#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "execute.h"
#include "parse.h"
#include "lex.h"
#include "value.h"
#include "map.h"
#include "common.h"

#define PROGRAM_NAME "snrub"
#define PROGRAM_VERSION "v0.43.1"
#define PROGRAM_SUCCESS 0
#define PROGRAM_ERROR 1

static int run_help(void);
static int run_version(void);
static int run_file(char *file);
static int run_text(char *text);
static int run_interactive(void);
static int record_script(char *document, map_t *variables);
static value_t *apply_script(char *document, map_t *variables);
static map_t *empty_variables(void);
static void destroy_value_unsafe(void *value);
static void crash(void);

int main(int argc, char **argv)
{
    char **arguments;
    int help, version, text, file, interactive;
    int remaining, capacity;
    int modes;

    arguments = argv;
    help = version = text = file = interactive = 0;
    remaining = capacity = argc;

    while (--remaining > 0 && (*++arguments)[0] == '-')
    {
        int current;

        if (strcmp(arguments[0], "--") == 0)
        {
            remaining--;
            ++arguments;
            break;
        }

        while ((current = *++arguments[0]))
        {
            switch (current)
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
                    fprintf(stderr, "%s: illegal option %c\n", PROGRAM_NAME, current);
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
        if (remaining == 0)
        {
            fprintf(stderr, "%s: missing operand\n", PROGRAM_NAME);
            crash();
        }

        return run_file(*arguments++);
    }

    if (text)
    {
        if (remaining == 0)
        {
            fprintf(stderr, "%s: missing operand\n", PROGRAM_NAME);
            crash();
        }

        return run_text(*arguments++);
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
    printf("  %s -f script\n", PROGRAM_NAME);
    printf("  %s -t script\n", PROGRAM_NAME);
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

static int run_file(char *file)
{
    char *document;
    map_t *variables;
    int success;

    document = read_file(file);

    if (!document)
    {
        crash();
    }

    variables = empty_variables();

    if (!variables)
    {
        crash();
    }

    success = record_script(document, variables);
    destroy_map(variables);

    return success ? PROGRAM_SUCCESS : PROGRAM_ERROR;
}

static int run_text(char *text)
{
    char *document;
    map_t *variables;
    int success;

    document = copy_string(text);

    if (!document)
    {
        crash();
    }

    variables = empty_variables();

    if (!variables)
    {
        crash();
    }

    success = record_script(document, variables);
    destroy_map(variables);

    return success ? PROGRAM_SUCCESS : PROGRAM_ERROR;
}

static int run_interactive(void)
{
    map_t *variables;

    variables = empty_variables();

    if (!variables)
    {
        crash();
    }

    while (1)
    {
        line_t *line;
        char *document;
        int success;

        printf("> ");
        line = next_line();

        if (!line)
        {
            crash();
        }

        if (line->exit)
        {
            destroy_map(variables);
            destroy_line(line);
            return PROGRAM_SUCCESS;
        }

        document = line->string;
        line->string = NULL;
        destroy_line(line);

        success = record_script(document, variables);

        if (!success)
        {
            destroy_map(variables);
            return PROGRAM_ERROR;
        }

        fflush(stdout);
    }
}

static int record_script(char *document, map_t *variables)
{
    value_t *value;
    char *represent;
    int success;

    value = apply_script(document, variables);

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

static value_t *apply_script(char *document, map_t *variables)
{
    scanner_t *scanner;
    list_t *expressions;
    value_t *value;

    scanner = start_scanner(document);

    if (!scanner)
    {
        crash();
    }

    expressions = parse_expressions(scanner);
    destroy_scanner(scanner);

    if (!expressions)
    {
        crash();
    }

    value = execute_expression(expressions, variables);
    destroy_list(expressions);

    return value;
}

static map_t *empty_variables(void)
{
    return empty_map(hash_string, destroy_value_unsafe, 64);
}

static void destroy_value_unsafe(void *value)
{
    destroy_value((value_t *) value);
}

static void crash(void)
{
    exit(PROGRAM_ERROR);
}
