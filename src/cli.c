#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "common.h"

static line_t *create_line(string_t *string, size_t fill, int exit);
static line_t *empty_line(void);

line_t *next_line(void)
{
    line_t *line;
    int key;

    line = empty_line();

    do
    {
        key = getchar();

        if (key == EOF)
        {
            line->exit = 1;
        }
        else
        {
            if (line->fill == line->string->length)
            {
                resize_string(line->string, line->fill * 2);
            }

            line->string->bytes[line->fill++] = key;
        }
    } while (key != EOF && key != '\n');

    resize_string(line->string, line->fill);

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

static line_t *create_line(string_t *string, size_t fill, int exit)
{
    line_t *line;

    line = allocate(sizeof(line_t));
    line->string = string;
    line->fill = fill;
    line->exit = exit;

    return line;
}

static line_t *empty_line(void)
{
    string_t *string;
    size_t capacity;

    capacity = 1024;
    string = create_string(allocate(sizeof(char) * capacity), capacity);

    return create_line(string, 0, 0);
}
