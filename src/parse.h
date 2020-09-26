#ifndef PARSE_H
#define PARSE_H

#include "lex.h"
#include "value.h"
#include "list.h"

typedef struct
{
    value_t *value;
    list_t *arguments;
} expression_t;

list_t *parse_expressions(scanner_t *scanner);
void destroy_expression(expression_t *expression);

#endif
