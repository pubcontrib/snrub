#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "common.h"

#define PROGRAM_NAME "snrub"
#define PROGRAM_VERSION "v0.4.0"

static int run_script(char *document);
static char *read_file(char *path, size_t limit);
static void print_version();
static void print_usage();
static void print_error(execute_error_t error);
static void print_value(execute_passback_t *passback);
static int get_flag(int argc, char **argv, char *name);
static char *get_option(int argc, char **argv, char *name);
static char *unescape(char *value);

int main(int argc, char **argv)
{
    char *file, *text;
    int limit;

    limit = 1024 * 4;

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
        int size;

        printf("Checking %s for a script...\n", file);

        document = read_file(file, limit);

        if (!document)
        {
            fprintf(stderr, "No script found.\n");
            return 1;
        }

        size = strlen(document);

        if (size >= limit - 1)
        {
            free(document);
            fprintf(stderr, "Script exceeds the size limit of %d bytes.\n", limit);
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

        if (strlen(text) >= limit - 1)
        {
            fprintf(stderr, "Script exceeds the size limit of %d bytes.\n", limit);
            return 1;
        }

        document = malloc(sizeof(char) * limit);

        if (!document)
        {
            fprintf(stderr, "No script found.\n");
            return 1;
        }

        strcpy(document, text);

        return run_script(document);
    }

    print_usage();
    return 0;
}

static int run_script(char *document)
{
    execute_passback_t *last;

    last = execute_do_document(document);

    if (last)
    {
        if (last->error != EXECUTE_ERROR_UNKNOWN)
        {
            print_error(last->error);
            execute_destroy_passback(last);
            return 1;
        }
        else
        {
            print_value(last);
            execute_destroy_passback(last);
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "Ran out of memory.\n");
        return 1;
    }
}

static char *read_file(char *path, size_t limit)
{
    FILE *file;

    file = fopen(path, "r");

    if (file)
    {
        char *buffer;

        buffer = malloc(sizeof(char) * limit);

        if (buffer)
        {
            size_t index;

            index = 0;

            while (1)
            {
                int current;

                current = fgetc(file);

                if (current == EOF || index >= limit - 1)
                {
                    buffer[index] = '\0';
                    break;
                }
                else
                {
                    buffer[index] = (char) current;
                }

                index += 1;
            }
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
    switch (error)
    {
        case EXECUTE_ERROR_SYNTAX:
            printf("A language syntax error was found!\n");
            break;
        case EXECUTE_ERROR_DEPTH:
            printf("An expression violated the depth limit!\n");
            break;
        case EXECUTE_ERROR_TYPE:
            printf("An invalid literal value was found!\n");
            break;
        case EXECUTE_ERROR_ARGUMENT:
            printf("An expression was found with the wrong arguments!\n");
            break;
        case EXECUTE_ERROR_ARITHMETIC:
            printf("An expression found an arithmetic error!\n");
            break;
        case EXECUTE_ERROR_UNSUPPORTED:
            printf("An expression found unsupported functionality!\n");
            break;
        default:
            break;
    }
}

static void print_value(execute_passback_t *passback)
{
    if (passback->type == EXECUTE_TYPE_NULL)
    {
        printf("?");
    }
    else if (passback->type == EXECUTE_TYPE_NUMBER)
    {
        int number;
        number = ((int *) passback->unsafe)[0];
        printf("#%d#", number);
    }
    else if (passback->type == EXECUTE_TYPE_STRING)
    {
        char *string;
        string = unescape((char *) passback->unsafe);

        if (string)
        {
            printf("\"%s\"", string);
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
    size_t length, index;

    length = strlen(value)
        + characters_in_string(value, '\\')
        + characters_in_string(value, '"')
        + characters_in_string(value, '\t')
        + characters_in_string(value, '\n')
        + characters_in_string(value, '\r');
    buffer = malloc(sizeof(char) * (length + 1));

    if (buffer)
    {
        for (index = 0; index < length; index++)
        {
            char symbol;

            symbol = value[index];

            if (symbol == '\\')
            {
                buffer[index++] = '\\';
                buffer[index] = '\\';
            }
            else if (symbol == '"')
            {
                buffer[index++] = '\\';
                buffer[index] = '"';
            }
            else if (symbol == '\t')
            {
                buffer[index++] = '\\';
                buffer[index] = 't';
            }
            else if (symbol == '\n')
            {
                buffer[index++] = '\\';
                buffer[index] = 'n';
            }
            else if (symbol == '\r')
            {
                buffer[index++] = '\\';
                buffer[index] = 'r';
            }
            else
            {
                buffer[index] = symbol;
            }
        }

        buffer[length] = '\0';
    }

    return buffer;
}
