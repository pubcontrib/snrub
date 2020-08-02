#ifndef EXECUTE_H
#define EXECUTE_H

#include "parse.h"
#include "value.h"

typedef struct variable_t
{
    char *identifier;
    value_t *value;
    struct variable_t *next;
} variable_t;

variable_t *empty_variable();
value_t *execute_expression(expression_t *expressions, variable_t *variables);
void destroy_variable(variable_t *variable);

#endif
