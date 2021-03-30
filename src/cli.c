#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

static line_t *create_line(char *document, size_t size, size_t capacity, int exit);
static line_t *empty_line(void);

char *read_file(char *path)
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

void write_file(char *path, char *text)
{
    FILE *file;

    file = fopen(path, "wb");

    if (file)
    {
        fwrite(text, sizeof(char), strlen(text), file);

        fclose(file);
    }
}

void remove_file(char *path)
{
    remove(path);
}

line_t *next_line(void)
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

static line_t *empty_line(void)
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
