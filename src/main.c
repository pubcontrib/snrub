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
#define PROGRAM_VERSION "v0.41.0"

static int complete_script(char *document);
static int apply_script(char *document, map_t *variables);
static char *read_file(char *path);
static void print_version(void);
static void print_usage(void);
static void print_value(value_t *value);
static map_t *empty_variables(void);
static void destroy_value_unsafe(void *value);
static void crash();

int main(int argc, char **argv)
{
    char *file, *text;

    if (get_flag(argc, argv, "--version") || get_flag(argc, argv, "-v"))
    {
        print_version();
        return 0;
    }

    file = get_option(argc, argv, "--file");

    if (!file)
    {
        file = get_option(argc, argv, "-f");
    }

    if (file)
    {
        char *document;

        document = read_file(file);

        if (!document)
        {
            crash();
        }

        return complete_script(document);
    }

    text = get_option(argc, argv, "--text");

    if (!text)
    {
        text = get_option(argc, argv, "-t");
    }

    if (text)
    {
        char *document;

        document = copy_string(text);

        if (!document)
        {
            crash();
        }

        return complete_script(document);
    }

    if (get_flag(argc, argv, "--interactive") || get_flag(argc, argv, "-i"))
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
                return 0;
            }

            document = line->string;
            line->string = NULL;

            if (apply_script(document, variables))
            {
                crash();
            }

            fflush(stdout);
            destroy_line(line);
        }
    }

    print_usage();
    return 0;
}

static int complete_script(char *document)
{
    map_t *variables;
    int status;

    variables = empty_variables();

    if (!variables)
    {
        crash();
    }

    status = apply_script(document, variables);

    destroy_map(variables);

    return status;
}

static int apply_script(char *document, map_t *variables)
{
    scanner_t *scanner;
    list_t *expressions;
    value_t *value;
    int status;

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

    if (!value)
    {
        crash();
    }

    status = value->type == TYPE_ERROR;
    print_value(value);
    destroy_value(value);

    return status;
}

static char *read_file(char *path)
{
    FILE *file;

    file = fopen(path, "rb");

    if (file)
    {
        char *buffer;
        long int length;

        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = malloc(sizeof(char) * (length + 1));

        if (buffer)
        {
            fread(buffer, 1, length, file);
            buffer[length] = '\0';
        }

        fclose(file);

        return buffer;
    }

    return NULL;
}

static void print_version(void)
{
    printf("%s\n", PROGRAM_VERSION);
}

static void print_usage(void)
{
    printf("Usage:\n");
    printf("  %s [options]\n", PROGRAM_NAME);
    printf("\n");
    printf("Options:\n");
    printf("  -v --version      Show version.\n");
    printf("  -f --file         Execute a script file.\n");
    printf("  -i --interactive  Start an interactive scripting shell.\n");
    printf("  -t --text         Execute script text.\n");
}

static void print_value(value_t *value)
{
    char *represent;

    represent = represent_value(value);

    if (represent)
    {
        printf("%s\n", represent);
        free(represent);
    }
    else
    {
        crash();
    }
}

static map_t *empty_variables(void)
{
    return empty_map(hash_string, destroy_value_unsafe, 64);
}

static void destroy_value_unsafe(void *value)
{
    destroy_value((value_t *) value);
}

static void crash()
{
    exit(1);
}
