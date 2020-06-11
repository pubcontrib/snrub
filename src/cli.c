#include <stdio.h>
#include <stdlib.h>
#include "cli.h"

static line_t *create_line(char *document, size_t size, size_t capacity, int exit);
static line_t *empty_line();

line_t *next_line()
{
    line_t* line;

    line = empty_line();

    if (line)
    {
        while (1)
        {
            int key;

            key = getchar();

            if (key == EOF)
            {
                line->exit = 1;
            }
            else if (key == '\n')
            {
                break;
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
        }
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
