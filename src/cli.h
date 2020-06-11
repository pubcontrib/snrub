#ifndef CLI_H
#define CLI_H

typedef struct
{
    char *string;
    size_t length;
    size_t capacity;
    int exit;
} line_t;

line_t *next_line();
void destroy_line(line_t *line);

#endif
