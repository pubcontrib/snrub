#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

typedef enum
{
    TYPE_UNKNOWN,
    TYPE_NULL,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_CALL
} type_t;

typedef struct
{
    type_t type;
    void *unsafe;
} literal_t;

typedef enum
{
    ERROR_UNKNOWN,
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
void destroy_literal(literal_t *literal);

#endif
