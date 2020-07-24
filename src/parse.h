#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

typedef enum
{
    TYPE_UNSET,
    TYPE_NULL,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_CALL
} type_t;

typedef enum
{
    ERROR_UNSET,
    ERROR_SYNTAX,
    ERROR_BOUNDS,
    ERROR_TYPE,
    ERROR_ARGUMENT,
    ERROR_ARITHMETIC,
    ERROR_SHORTAGE,
    ERROR_UNSUPPORTED
} error_t;

typedef struct expression_t
{
    error_t error;
    type_t type;
    void *segment;
    struct expression_t **arguments;
    size_t length;
    struct expression_t *next;
} expression_t;

expression_t *parse_expressions(scanner_t *scanner);
char *escape_string(char *string);
char *unescape_string(char *string);
void destroy_expression(expression_t *expression);

#endif
