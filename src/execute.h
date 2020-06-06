#ifndef EXECUTE_H
#define EXECUTE_H

#include "parse.h"

typedef enum
{
    EXECUTE_TYPE_UNKNOWN,
    EXECUTE_TYPE_NULL,
    EXECUTE_TYPE_NUMBER,
    EXECUTE_TYPE_STRING
} execute_type_t;

typedef enum
{
    EXECUTE_ERROR_UNKNOWN,
    EXECUTE_ERROR_SYNTAX,
    EXECUTE_ERROR_DEPTH,
    EXECUTE_ERROR_TYPE,
    EXECUTE_ERROR_ARGUMENT,
    EXECUTE_ERROR_ARITHMETIC,
    EXECUTE_ERROR_SHORTAGE
} execute_error_t;

typedef struct
{
    execute_type_t type;
    void *unsafe;
    size_t size;
    execute_error_t error;
} execute_passback_t;

typedef struct execute_object_t
{
    execute_type_t type;
    void *unsafe;
    size_t size;
    char *key;
    struct execute_object_t *next;
} execute_object_t;

execute_object_t *execute_empty_objects();
execute_passback_t *execute_evaluate_expression(parse_expression_t *expressions, execute_object_t *objects);
void execute_destroy_object(execute_object_t *object);
void execute_destroy_passback(execute_passback_t *passback);

#endif
