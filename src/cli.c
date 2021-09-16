#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "common.h"

static line_t *create_line(char *document, size_t size, size_t capacity, int exit);
static line_t *empty_line(void);

char *read_file(char *path)
{
    FILE *file;

    file = fopen(path, "rb");

    if (file)
    {
        char *buffer;
        long length;

        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = allocate(sizeof(char) * (length + 1));
        fread(buffer, 1, length, file);
        buffer[length] = '\0';

        fclose(file);

        return buffer;
    }

    return NULL;
}

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
            if (line->length == line->capacity)
            {
                char *string;

                line->capacity *= 2;
                string = reallocate(line->string, sizeof(char) * (line->capacity + 1));
                line->string = string;
            }

            line->string[line->length++] = key;
            line->string[line->length] = '\0';
        }
    } while (key != EOF && key != '\n');

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

    line = allocate(sizeof(line_t));
    line->string = string;
    line->length = length;
    line->capacity = capacity;
    line->exit = exit;

    return line;
}

static line_t *empty_line(void)
{
    char *string;
    size_t capacity;

    capacity = 1024;
    string = allocate(sizeof(char) * (capacity + 1));
    string[0] = '\0';

    return create_line(string, 0, capacity, 0);
}
