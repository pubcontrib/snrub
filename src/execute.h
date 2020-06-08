#ifndef EXECUTE_H
#define EXECUTE_H

#include "parse.h"

typedef struct
{
    error_t error;
    type_t type;
    void *unsafe;
    size_t size;
} handoff_t;

typedef struct object_t
{
    char *identifier;
    type_t type;
    void *unsafe;
    size_t size;
    struct object_t *next;
} object_t;

object_t *empty_object();
handoff_t *execute_expression(expression_t *expressions, object_t *objects);
void destroy_object(object_t *object);
void destroy_handoff(handoff_t *handoff);

#endif
