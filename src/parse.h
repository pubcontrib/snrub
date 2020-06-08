#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

typedef enum
{
    ERROR_UNKNOWN,
    ERROR_SYNTAX,
    ERROR_DEPTH,
    ERROR_TYPE,
    ERROR_ARGUMENT
} error_t;

typedef enum
{
    TYPE_UNKNOWN,
    TYPE_NULL,
    TYPE_NUMBER,
    TYPE_STRING
} type_t;

typedef struct
{
    type_t type;
    void *unsafe;
} literal_t;

typedef struct expression_t
{
    error_t error;
    literal_t *literal;
    struct expression_t **arguments;
    size_t length;
    struct expression_t *next;
} expression_t;

expression_t *parse_expressions(scanner_t *scanner);
void destroy_expression(expression_t *expression);
void destroy_literal(literal_t *literal);

#endif
