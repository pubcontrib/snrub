#ifndef PARSE_H
#define PARSE_H

#include "lex.h"
#include "value.h"
#include "list.h"

#define LIMIT_DEPTH 32

typedef enum
{
    EXPRESSION_TYPE_UNSET,
    EXPRESSION_TYPE_VALUE,
    EXPRESSION_TYPE_LIST,
    EXPRESSION_TYPE_MAP,
    EXPRESSION_TYPE_CALL
} expression_type_t;

typedef struct
{
    expression_type_t type;
    value_t *value;
    list_t *arguments;
} expression_t;

list_t *parse_expressions(scanner_t *scanner);
void destroy_expression(expression_t *expression);

#endif
