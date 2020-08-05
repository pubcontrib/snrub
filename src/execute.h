#ifndef EXECUTE_H
#define EXECUTE_H

#include "parse.h"
#include "value.h"

typedef struct variable_list_t
{
    char *identifier;
    value_t *value;
    struct variable_list_t *next;
} variable_list_t;

typedef struct
{
    variable_list_t **lists;
    size_t length;
    size_t capacity;
} variable_map_t;

variable_map_t *empty_variable_map();
value_t *execute_expression(expression_t *expressions, variable_map_t *variables);
void destroy_variable_map(variable_map_t *map);
void destroy_variable_list(variable_list_t *list);

#endif
