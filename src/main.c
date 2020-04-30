#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "common.h"

#define PROGRAM_NAME "snrub"
#define PROGRAM_VERSION "v0.6.2"

static int run_script(char *document);
static char *read_file(char *path, size_t limit);
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

        document = read_file(file, limit);

        if (!document)
        {
            return 1;
        }

        if (strlen(document) >= limit - 1)
        {
            free(document);
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
            return 1;
        }

        document = malloc(sizeof(char) * limit);

        if (!document)
        {
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
            print_value(last->type, last->unsafe);
            execute_destroy_passback(last);
            return 0;
        }
    }
    else
    {
        print_error(EXECUTE_ERROR_SHORTAGE);
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

        for (left = 0, right = 0; right < length; right++)
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
