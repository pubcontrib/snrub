#ifndef CLI_H
#define CLI_H

#include <stddef.h>
#include "string.h"

typedef struct
{
    string_t *string;
    size_t fill;
    int exit;
} line_t;

line_t *next_line(void);
void destroy_line(line_t *line);

#endif
