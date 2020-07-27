#ifndef PARSE_H
#define PARSE_H

#include "lex.h"
#include "value.h"

typedef struct expression_t
{
    value_t *value;
    struct expression_t **arguments;
    size_t length;
    struct expression_t *next;
} expression_t;

expression_t *parse_expressions(scanner_t *scanner);
char *escape_string(char *string);
char *unescape_string(char *string);
void destroy_expression(expression_t *expression);

#endif
