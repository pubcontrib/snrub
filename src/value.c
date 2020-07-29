#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "common.h"

static value_t *create_value(type_t type, void *data, size_t size);

value_t *new_unset()
{
    return create_value(TYPE_UNSET, NULL, 0);
}

value_t *new_null()
{
    return create_value(TYPE_NULL, NULL, 0);
}

value_t *new_number(int number)
{
    int *data;

    data = integer_to_array(number);

    if (!data)
    {
        return NULL;
    }

    return create_value(TYPE_NUMBER, data, sizeof(int));
}

value_t *new_string(char *string)
{
    char *data;

    data = copy_string(string);

    if (!data)
    {
        return NULL;
    }

    return create_value(TYPE_STRING, data, sizeof(char) * (strlen(string) + 1));
}

value_t *new_list(value_t **items, size_t length)
{
    return create_value(TYPE_LIST, items, length);
}

value_t *new_error(error_t error)
{
    error_t *data;
    size_t size;

    size = sizeof(error_t);
    data = malloc(size);

    if (!data)
    {
        return NULL;
    }

    data[0] = error;

    return create_value(TYPE_ERROR, data, size);
}

value_t *new_call()
{
    return create_value(TYPE_CALL, NULL, 0);
}

value_t *steal_number(int *number, size_t size)
{
    return create_value(TYPE_NUMBER, number, size);
}

value_t *steal_string(char *string, size_t size)
{
    return create_value(TYPE_STRING, string, size);
}

value_t *steal_error(error_t *error, size_t size)
{
    return create_value(TYPE_ERROR, error, size);
}

value_t *copy_value(value_t *this)
{
    void *data;

    data = copy_memory(this->data, this->size);

    if (!data)
    {
        return NULL;
    }

    return create_value(this->type, data, this->size);
}

int view_number(value_t *value)
{
    switch (value->type)
    {
        case TYPE_NUMBER:
            return ((int *) value->data)[0];
        default:
            return 0;
    }
}

char *view_string(value_t *value)
{
    switch (value->type)
    {
        case TYPE_STRING:
            return (char *) value->data;
        default:
            return "";
    }
}

error_t view_error(value_t *value)
{
    switch (value->type)
    {
        case TYPE_ERROR:
            return ((error_t *) value->data)[0];
        default:
            return ERROR_UNSET;
    }
}

void destroy_value(value_t *value)
{
    if (value->data)
    {
        if (value->type == TYPE_LIST)
        {
            size_t index;
            value_t **items;

            items = value->data;

            for (index = 0; index < value->size; index++)
            {
                value_t *item;

                item = items[index];

                destroy_value(item);
            }
        }

        free(value->data);
    }

    free(value);
}

static value_t *create_value(type_t type, void *data, size_t size)
{
    value_t *value;

    value = malloc(sizeof(value_t));

    if (value)
    {
        value->type = type;
        value->data = data;
        value->size = size;
    }

    return value;
}
