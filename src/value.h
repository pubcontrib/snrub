#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>

typedef enum
{
    TYPE_UNSET,
    TYPE_NULL,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_LIST,
    TYPE_ERROR,
    TYPE_CALL
} type_t;

typedef struct
{
    type_t type;
    void *data;
    size_t size;
} value_t;

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

value_t *new_unset();
value_t *new_null();
value_t *new_number(int number);
value_t *new_string(char *string);
value_t *new_list(value_t **items, size_t length);
value_t *new_error(error_t error);
value_t *new_call();
value_t *steal_number(int *number, size_t size);
value_t *steal_string(char *string, size_t size);
value_t *steal_error(error_t *error, size_t size);
value_t *copy_value(value_t *this);
int view_number(value_t *value);
char *view_string(value_t *value);
error_t view_error(value_t *value);
void destroy_value(value_t *value);

#endif
