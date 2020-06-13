#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

static line_t *create_line(char *document, size_t size, size_t capacity, int exit);
static line_t *empty_line();

int get_flag(int argc, char **argv, char *name)
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

char *get_option(int argc, char **argv, char *name)
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

line_t *next_line()
{
    line_t* line;

    line = empty_line();

    if (line)
    {
        int key;

        do
        {
            key = getchar();

            if (key == EOF)
            {
                line->exit = 1;
            }
            else
            {
                if (line->length == line->capacity)
                {
                    char *string;

                    line->capacity *= 2;
                    string = realloc(line->string, sizeof(char) * (line->capacity + 1));

                    if (!string)
                    {
                        destroy_line(line);
                        return NULL;
                    }

                    line->string = string;
                }

                line->string[line->length++] = key;
                line->string[line->length] = '\0';
            }
        } while (key != EOF && key != '\n');
    }

    return line;
}

void destroy_line(line_t *line)
{
    if (line->string)
    {
        free(line->string);
    }

    free(line);
}

static line_t *create_line(char *string, size_t length, size_t capacity, int exit)
{
    line_t *line;

    line = malloc(sizeof(line_t));

    if (line)
    {
        line->string = string;
        line->length = length;
        line->capacity = capacity;
        line->exit = exit;
    }

    return line;
}

static line_t *empty_line()
{
    char *string;
    size_t capacity;

    capacity = 1024;
    string = malloc(sizeof(char) * (capacity + 1));

    if (!string)
    {
        return NULL;
    }

    string[0] = '\0';
    return create_line(string, 0, capacity, 0);
}
