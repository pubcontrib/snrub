#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

typedef enum
{
    PARSE_TYPE_UNKNOWN,
    PARSE_TYPE_NULL,
    PARSE_TYPE_NUMBER,
    PARSE_TYPE_STRING
} parse_type_t;

typedef enum
{
    PARSE_ERROR_UNKNOWN,
    PARSE_ERROR_SYNTAX,
    PARSE_ERROR_DEPTH,
    PARSE_ERROR_TYPE,
    PARSE_ERROR_ARGUMENT
} parse_error_t;

typedef enum
{
    PARSE_STATUS_SUCCESS,
    PARSE_STATUS_ERROR,
    PARSE_STATUS_START,
    PARSE_STATUS_OPERATOR,
    PARSE_STATUS_LEFT,
    PARSE_STATUS_RIGHT,
    PARSE_STATUS_END
} parse_status_t;

typedef struct
{
    parse_type_t type;
    void *unsafe;
} parse_value_t;

typedef struct parse_expression_t
{
    parse_error_t error;
    parse_value_t *value;
    struct parse_expression_t *operator;
    struct parse_expression_t *left;
    struct parse_expression_t *right;
} parse_expression_t;

typedef struct parse_link_t
{
    parse_expression_t *expression;
    struct parse_link_t *next;
} parse_link_t;

parse_link_t *parse_list_document(char *document);
void parse_destroy_link(parse_link_t *link);
void parse_destroy_expression(parse_expression_t *expression);
void parse_destroy_value(parse_value_t *value);

#endif
