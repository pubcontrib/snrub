#ifndef EXECUTE_H
#define EXECUTE_H

#include "parse.h"

typedef struct
{
    type_t type;
    void *unsafe;
    size_t size;
    error_t error;
} handoff_t;

typedef struct object_t
{
    type_t type;
    void *unsafe;
    size_t size;
    char *key;
    struct object_t *next;
} object_t;

object_t *empty_object();
handoff_t *execute_expression(expression_t *expressions, object_t *objects);
void destroy_object(object_t *object);
void destroy_handoff(handoff_t *handoff);

#endif
