#ifndef EXECUTE_H
#define EXECUTE_H

#include "value.h"
#include "map.h"
#include "list.h"
#include "string.h"

typedef struct
{
    list_t *expressions;
    list_node_t *current;
    value_t **evaluated;
    size_t index;
    value_t *value;
    int interception;
} argument_iterator_t;

typedef struct stack_frame_t
{
    map_t *variables;
    map_t *overloads;
    map_t *operators;
    int depth;
    argument_iterator_t *arguments;
    struct stack_frame_t *caller;
} stack_frame_t;

map_t *empty_variables(void);
map_t *empty_overloads(void);
map_t *default_operators(void);
int has_next_argument(argument_iterator_t *arguments);
int next_argument(argument_iterator_t *arguments, stack_frame_t *frame, int types);
void skip_argument(argument_iterator_t *arguments);
void reset_arguments(argument_iterator_t *arguments);
value_t *execute_script(string_t *document, stack_frame_t *frame);

#endif
