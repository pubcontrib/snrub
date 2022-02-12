#ifndef CLI_H
#define CLI_H

#include "buffer.h"

typedef struct
{
    buffer_t *string;
    size_t fill;
    int exit;
} line_t;

line_t *next_line(void);
void destroy_line(line_t *line);

#endif
