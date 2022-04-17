#ifndef EXECUTE_H
#define EXECUTE_H

#include "value.h"
#include "map.h"
#include "string.h"

typedef struct stack_frame_t
{
    map_t *variables;
    map_t *overloads;
    map_t *operators;
    int depth;
    struct stack_frame_t *caller;
} stack_frame_t;

map_t *empty_variables(void);
map_t *empty_overloads(void);
map_t *default_operators(void);
value_t *execute_script(string_t *document, value_t *arguments, stack_frame_t *frame);

#endif
