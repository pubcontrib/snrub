#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "parse.h"
#include "lex.h"
#include "common.h"

#define PROGRAM_NAME "snrub"
#define PROGRAM_VERSION "v0.17.2"

static int run_script(char *document);
static char *read_file(char *path);
static void print_version();
static void print_usage();
static void print_error(execute_error_t error);
static void print_value(execute_type_t type, void *unsafe);
static int get_flag(int argc, char **argv, char *name);
static char *get_option(int argc, char **argv, char *name);
static char *unescape(char *value);

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
            print_error(EXECUTE_ERROR_SHORTAGE);
            return 1;
        }

        return run_script(document);
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
            print_error(EXECUTE_ERROR_SHORTAGE);
            return 1;
        }

        return run_script(document);
    }

    print_usage();
    return 0;
}

static int run_script(char *document)
{
    scanner_t *scanner;
    expression_t *expressions;
    execute_object_t *objects;
    execute_passback_t *last;

    scanner = start_scanner(document);

    if (!scanner)
    {
        print_error(EXECUTE_ERROR_SHORTAGE);
        return 1;
    }

    expressions = parse_expressions(scanner);
    destroy_scanner(scanner);

    if (!expressions)
    {
        print_error(EXECUTE_ERROR_SHORTAGE);
        return 1;
    }

    objects = execute_empty_objects();

    if (!objects)
    {
        destroy_expression(expressions);
        print_error(EXECUTE_ERROR_SHORTAGE);
        return 1;
    }

    last = execute_evaluate_expression(expressions, objects);
    destroy_expression(expressions);
    execute_destroy_object(objects);

    if (!last)
    {
        print_error(EXECUTE_ERROR_SHORTAGE);
        return 1;
    }

    if (last->error != EXECUTE_ERROR_UNKNOWN)
    {
        print_error(last->error);
        execute_destroy_passback(last);
        return 1;
    }
    else
    {
        print_value(last->type, last->unsafe);
        execute_destroy_passback(last);
        return 0;
    }
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
    printf("  -v --version  Show version.\n");
    printf("  -f --file     Execute a script file.\n");
    printf("  -t --text     Execute script text.\n");
}

static void print_error(execute_error_t error)
{
    printf("#%d#\n", error);
}

static void print_value(execute_type_t type, void *unsafe)
{
    if (type == EXECUTE_TYPE_UNKNOWN || type == EXECUTE_TYPE_NULL)
    {
        printf("?\n");
    }
    else if (type == EXECUTE_TYPE_NUMBER)
    {
        int number;
        number = ((int *) unsafe)[0];
        printf("#%d#\n", number);
    }
    else if (type == EXECUTE_TYPE_STRING)
    {
        char *string;
        string = unescape((char *) unsafe);

        if (string)
        {
            printf("\"%s\"\n", string);
            free(string);
        }
    }
}

static int get_flag(int argc, char **argv, char *name)
{
    int index;

    for (index = 0; index < argc; index++)
    {
        char *arg = argv[index];

        if (strcmp(arg, name) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static char *get_option(int argc, char **argv, char *name)
{
    int index;

    for (index = 0; index < argc; index++)
    {
        char *arg = argv[index];

        if (strcmp(arg, name) == 0)
        {
            if (index + 1 < argc)
            {
                return argv[index + 1];
            }
        }
    }

    return NULL;
}

static char *unescape(char *value)
{
    char *buffer;
    size_t length;

    length = strlen(value)
        + characters_in_string(value, '\\')
        + characters_in_string(value, '"')
        + characters_in_string(value, '\t')
        + characters_in_string(value, '\n')
        + characters_in_string(value, '\r');
    buffer = malloc(sizeof(char) * (length + 1));

    if (buffer)
    {
        size_t left, right;

        for (left = 0, right = 0; right < strlen(value); right++)
        {
            char symbol;

            symbol = value[right];

            if (symbol == '\\')
            {
                buffer[left++] = '\\';
                buffer[left++] = '\\';
            }
            else if (symbol == '"')
            {
                buffer[left++] = '\\';
                buffer[left++] = '"';
            }
            else if (symbol == '\t')
            {
                buffer[left++] = '\\';
                buffer[left++] = 't';
            }
            else if (symbol == '\n')
            {
                buffer[left++] = '\\';
                buffer[left++] = 'n';
            }
            else if (symbol == '\r')
            {
                buffer[left++] = '\\';
                buffer[left++] = 'r';
            }
            else
            {
                buffer[left++] = symbol;
            }
        }

        buffer[length] = '\0';
    }

    return buffer;
}
