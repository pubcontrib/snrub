#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "common.h"

static value_t *create_value(type_t type, void *data, size_t size);

value_t *merge_lists(value_t *left, value_t *right)
{
    value_t **items;
    size_t length, index;

    length = left->size + right->size;
    items = malloc(sizeof(value_t *) * length);

    if (!items)
    {
        return NULL;
    }

    for (index = 0; index < length; index++)
    {
        value_t *copy;

        copy = copy_value(index < left->size ? ((value_t **) left->data)[index] : ((value_t **) right->data)[index - left->size]);

        if (!copy)
        {
            free(items);
            return NULL;
        }

        items[index] = copy;
    }

    return new_list(items, length);
}

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
    if (this->type == TYPE_LIST)
    {
        value_t **items, **data;
        size_t length, index;

        items = this->data;
        length = this->size;
        data = malloc(sizeof(value_t *) * length);

        if (!data)
        {
            return NULL;
        }

        for (index = 0; index < length; index++)
        {
            value_t *copy;

            copy = copy_value(items[index]);

            if (!copy)
            {
                free(data);
                return NULL;
            }

            data[index] = copy;
        }

        return new_list(data, length);
    }
    else
    {
        void *data;

        data = copy_memory(this->data, this->size);

        if (!data)
        {
            return NULL;
        }

        return create_value(this->type, data, this->size);
    }
}

int hash_value(value_t *this)
{
    switch (this->type)
    {
        case TYPE_NULL:
            return hash_null();
        case TYPE_NUMBER:
            return hash_number(view_number(this));
        case TYPE_STRING:
            return hash_string(view_string(this));
        case TYPE_LIST:
            return hash_list(this->data, this->size);
        default:
            return 0;
    }
}

int hash_null()
{
    return 0;
}

int hash_number(int number)
{
    return number;
}

int hash_string(char *string)
{
    int hash;
    size_t length, index;

    hash = 0;
    length = strlen(string);

    for (index = 0; index < length; index++)
    {
        hash += string[index];
    }

    return hash;
}

int hash_list(value_t **items, size_t length)
{
    int hash;
    size_t index;

    hash = 0;

    for (index = 0; index < length; index++)
    {
        hash += hash_value(items[index]);
    }

    return hash;
}

int equal_values(value_t *left, value_t *right)
{
    if (left->type != right->type)
    {
        return 0;
    }

    if (left->type == TYPE_LIST)
    {
        size_t index;

        if (left->size != right->size)
        {
            return 0;
        }

        for (index = 0; index < left->size; index++)
        {
            if (!equal_values(((value_t **) left->data)[index], ((value_t **) right->data)[index]))
            {
                return 0;
            }
        }

        return 1;
    }

    switch (left->type)
    {
        case TYPE_NULL:
            return 1;
        case TYPE_NUMBER:
            return view_number(left) == view_number(right);
        case TYPE_STRING:
            return strcmp(view_string(left), view_string(right)) == 0;
        default:
            return 0;
    }
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
