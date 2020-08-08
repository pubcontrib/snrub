#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "execute.h"
#include "parse.h"
#include "lex.h"
#include "value.h"
#include "common.h"

#define PROGRAM_NAME "snrub"
#define PROGRAM_VERSION "v0.35.1"

static int complete_script(char *document);
static int apply_script(char *document, variable_map_t *variables);
static char *read_file(char *path);
static void print_version();
static void print_usage();
static void print_error(error_t error);
static int print_value(value_t *value);

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
            print_error(ERROR_SHORTAGE);
            return 1;
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
            print_error(ERROR_SHORTAGE);
            return 1;
        }

        return complete_script(document);
    }

    if (get_flag(argc, argv, "--interactive") || get_flag(argc, argv, "-i"))
    {
        variable_map_t *variables;

        variables = empty_variable_map();

        if (!variables)
        {
            print_error(ERROR_SHORTAGE);
            return 1;
        }

        while (1)
        {
            line_t *line;
            char *document;

            printf("> ");
            line = next_line();

            if (!line)
            {
                destroy_variable_map(variables);
                print_error(ERROR_SHORTAGE);
                return 1;
            }

            if (line->exit)
            {
                destroy_variable_map(variables);
                destroy_line(line);
                return 0;
            }

            document = line->string;
            line->string = NULL;

            if (apply_script(document, variables))
            {
                destroy_variable_map(variables);
                destroy_line(line);
                return 1;
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
    variable_map_t *variables;
    int status;

    variables = empty_variable_map();

    if (!variables)
    {
        print_error(ERROR_SHORTAGE);
        return 1;
    }

    status = apply_script(document, variables);

    destroy_variable_map(variables);

    return status;
}

static int apply_script(char *document, variable_map_t *variables)
{
    scanner_t *scanner;
    expression_t *expressions;
    value_t *value;
    int status;

    scanner = start_scanner(document);

    if (!scanner)
    {
        print_error(ERROR_SHORTAGE);
        return 1;
    }

    expressions = parse_expressions(scanner);
    destroy_scanner(scanner);

    if (!expressions)
    {
        print_error(ERROR_SHORTAGE);
        return 1;
    }

    value = execute_expression(expressions, variables);
    destroy_expression(expressions);

    if (!value)
    {
        print_error(ERROR_SHORTAGE);
        return 1;
    }

    status = print_value(value);
    printf("\n");
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

static void print_version()
{
    printf("%s\n", PROGRAM_VERSION);
}

static void print_usage()
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

static void print_error(error_t error)
{
    printf("#%d#\n", error);
}

static int print_value(value_t *value)
{
    if (value->type == TYPE_NULL)
    {
        printf("%c", SYMBOL_NULL);
        return 0;
    }
    else if (value->type == TYPE_NUMBER)
    {
        printf("#%d#", view_number(value));
        return 0;
    }
    else if (value->type == TYPE_STRING)
    {
        char *escaped;
        escaped = unescape_string(view_string(value));

        if (escaped)
        {
            printf("\"%s\"", escaped);
            free(escaped);
            return 0;
        }
        else
        {
            print_error(ERROR_SHORTAGE);
            return 1;
        }
    }
    else if (value->type == TYPE_LIST)
    {
        value_t **items;
        size_t length, index;

        items = value->data;
        length = value->size;

        printf("[");

        for (index = 0; index < length; index++)
        {
            value_t *item;

            if (index > 0)
            {
                printf(" ");
            }

            item = items[index];

            if (print_value(item))
            {
                printf("]");
                return 1;
            }
        }

        printf("]");
        return 0;
    }
    else if (value->type == TYPE_ERROR)
    {
        printf("#%d#", view_error(value));
        return 1;
    }

    printf("#%d#", ERROR_UNSUPPORTED);
    return 1;
}
