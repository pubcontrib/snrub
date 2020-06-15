#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "execute.h"
#include "parse.h"
#include "lex.h"
#include "common.h"

#define PROGRAM_NAME "snrub"
#define PROGRAM_VERSION "v0.18.0"

static int complete_script(char *document);
static int apply_script(char *document, object_t *objects);
static char *read_file(char *path);
static void print_version();
static void print_usage();
static void print_error(error_t error);
static void print_value(type_t type, void *unsafe);
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
        object_t *objects;

        objects = empty_object();

        if (!objects)
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
                destroy_object(objects);
                print_error(ERROR_SHORTAGE);
                return 1;
            }

            if (line->exit)
            {
                destroy_object(objects);
                destroy_line(line);
                return 0;
            }

            document = line->string;
            line->string = NULL;

            if (apply_script(document, objects))
            {
                destroy_object(objects);
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
    object_t *objects;
    int status;

    objects = empty_object();

    if (!objects)
    {
        print_error(ERROR_SHORTAGE);
        return 1;
    }

    status = apply_script(document, objects);

    destroy_object(objects);

    return status;
}

static int apply_script(char *document, object_t *objects)
{
    scanner_t *scanner;
    expression_t *expressions;
    handoff_t *handoff;

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

    handoff = execute_expression(expressions, objects);
    destroy_expression(expressions);

    if (!handoff)
    {
        print_error(ERROR_SHORTAGE);
        return 1;
    }

    if (handoff->error != ERROR_UNKNOWN)
    {
        print_error(handoff->error);
        destroy_handoff(handoff);
        return 1;
    }
    else
    {
        print_value(handoff->type, handoff->unsafe);
        destroy_handoff(handoff);
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
    printf("  -v --version      Show version.\n");
    printf("  -f --file         Execute a script file.\n");
    printf("  -i --interactive  Start an interactive scripting shell.\n");
    printf("  -t --text         Execute script text.\n");
}

static void print_error(error_t error)
{
    printf("#%d#\n", error);
}

static void print_value(type_t type, void *unsafe)
{
    if (type == TYPE_UNKNOWN || type == TYPE_NULL)
    {
        printf("?\n");
    }
    else if (type == TYPE_NUMBER)
    {
        int number;
        number = ((int *) unsafe)[0];
        printf("#%d#\n", number);
    }
    else if (type == TYPE_STRING)
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
