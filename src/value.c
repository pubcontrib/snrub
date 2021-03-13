#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "common.h"

static value_t *create_value(type_t type, void *data, size_t size);
static char *quote_string(char *body, char qualifier);

value_t *merge_lists(value_t *left, value_t *right)
{
    value_t **items;
    size_t length, index;

    length = left->size + right->size;

    if (length > 0)
    {
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
                destroy_items(items, index);
                return NULL;
            }

            items[index] = copy;
        }
    }
    else
    {
        items = NULL;
    }

    return new_list(items, length);
}

value_t *new_unset(void)
{
    return create_value(TYPE_UNSET, NULL, 0);
}

value_t *new_null(void)
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

value_t *new_call(void)
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
        value_t **data;
        size_t length;

        length = this->size;

        if (length > 0)
        {
            value_t **items;
            size_t index;

            items = this->data;
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
                    destroy_items(data, index);
                    return NULL;
                }

                data[index] = copy;
            }
        }
        else
        {
            data = NULL;
        }

        return new_list(data, length);
    }
    else
    {
        void *data;

        if (this->size > 0)
        {
            data = copy_memory(this->data, this->size);

            if (!data)
            {
                return NULL;
            }
        }
        else
        {
            data = NULL;
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

int hash_null(void)
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

char *represent_value(value_t *this)
{
    switch (this->type)
    {
        case TYPE_NULL:
            return represent_null();
        case TYPE_NUMBER:
            return represent_number(view_number(this));
        case TYPE_STRING:
            return represent_string(view_string(this));
        case TYPE_LIST:
            return represent_list(this->data, this->size);
        case TYPE_ERROR:
            return represent_error(view_error(this));
        default:
            return NULL;
    }
}

char *represent_null(void)
{
    return copy_string("?");
}

char *represent_number(int number)
{
    char *body;

    body = integer_to_string(number);

    if (!body)
    {
        return NULL;
    }

    return quote_string(body, '#');
}

char *represent_string(char *string)
{
    char *body;

    body = unescape_string(string);

    if (!body)
    {
        return NULL;
    }

    return quote_string(body, '\"');
}

char *represent_list(value_t **items, size_t length)
{
    char *body, *swap;
    size_t index;

    body = copy_string("[");

    if (!body)
    {
        return NULL;
    }

    for (index = 0; index < length; index++)
    {
        value_t *item;
        char *represent;

        if (index > 0)
        {
            swap = merge_strings(body, " ");
            free(body);

            if (!swap)
            {
                return NULL;
            }

            body = swap;
        }

        item = items[index];
        represent = represent_value(item);

        if (!represent)
        {
            free(body);
            return NULL;
        }

        swap = merge_strings(body, represent);
        free(body);
        free(represent);

        if (!swap)
        {
            return NULL;
        }

        body = swap;
    }

    swap = merge_strings(body, "]");
    free(body);

    return swap;
}

char *represent_error(error_t error)
{
    return represent_number(error);
}

char *escape_string(char *string)
{
    char *escape;
    size_t length;

    length = strlen(string);
    escape = malloc(sizeof(char) * (length + 1));

    if (escape)
    {
        size_t left, right;
        int escaping;

        escaping = 0;
        right = 0;

        for (left = 0; left < length; left++)
        {
            char current;

            current = string[left];

            if (escaping)
            {
                switch (current)
                {
                    case '\\':
                        escape[right++] = '\\';
                        break;
                    case '"':
                        escape[right++] = '"';
                        break;
                    case 't':
                        escape[right++] = '\t';
                        break;
                    case 'n':
                        escape[right++] = '\n';
                        break;
                    case 'r':
                        escape[right++] = '\r';
                        break;
                }

                escaping = 0;
            }
            else
            {
                if (current == '\\')
                {
                    escaping = 1;
                }
                else
                {
                    escape[right++] = current;
                }
            }
        }

        escape[right] = '\0';
    }

    return escape;
}

char *unescape_string(char *string)
{
    char *unescape;
    size_t length;

    length = strlen(string)
        + characters_in_string(string, '\\')
        + characters_in_string(string, '"')
        + characters_in_string(string, '\t')
        + characters_in_string(string, '\n')
        + characters_in_string(string, '\r');
    unescape = malloc(sizeof(char) * (length + 1));

    if (unescape)
    {
        size_t left, right;

        for (left = 0, right = 0; left < length; right++)
        {
            char symbol;

            symbol = string[right];

            if (symbol == '\\')
            {
                unescape[left++] = '\\';
                unescape[left++] = '\\';
            }
            else if (symbol == '"')
            {
                unescape[left++] = '\\';
                unescape[left++] = '"';
            }
            else if (symbol == '\t')
            {
                unescape[left++] = '\\';
                unescape[left++] = 't';
            }
            else if (symbol == '\n')
            {
                unescape[left++] = '\\';
                unescape[left++] = 'n';
            }
            else if (symbol == '\r')
            {
                unescape[left++] = '\\';
                unescape[left++] = 'r';
            }
            else
            {
                unescape[left++] = symbol;
            }
        }

        unescape[length] = '\0';
    }

    return unescape;
}

int compare_values(value_t *left, value_t *right)
{
    if (left->type != right->type)
    {
        return left->type - right->type;
    }

    if (left->type == TYPE_LIST)
    {
        size_t index;

        if (left->size != right->size)
        {
            return left->size - right->size;
        }

        for (index = 0; index < left->size; index++)
        {
            int equal;

            equal = compare_values(((value_t **) left->data)[index], ((value_t **) right->data)[index]);

            if (equal != 0)
            {
                return equal;
            }
        }

        return 0;
    }

    switch (left->type)
    {
        case TYPE_NULL:
            return 0;
        case TYPE_NUMBER:
            return view_number(left) - view_number(right);
        case TYPE_STRING:
            return strcmp(view_string(left), view_string(right));
        default:
            return 0;
    }
}

size_t length_value(value_t *value)
{
    switch (value->type)
    {
        case TYPE_STRING:
            return value->size - 1;
        case TYPE_LIST:
            return value->size;
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

int number_add(int left, int right, int *out)
{
    int sum;

    if (left < NUMBER_MIN || left > NUMBER_MAX || right < NUMBER_MIN || right > NUMBER_MAX)
    {
        return 0;
    }

    sum = left + right;

    if (sum < NUMBER_MIN || sum > NUMBER_MAX)
    {
        return 0;
    }

    (*out) = sum;

    return 1;
}

int number_subtract(int left, int right, int *out)
{
    int difference;

    if (left < NUMBER_MIN || left > NUMBER_MAX || right < NUMBER_MIN || right > NUMBER_MAX)
    {
        return 0;
    }

    difference = left - right;

    if (difference < NUMBER_MIN || difference > NUMBER_MAX)
    {
        return 0;
    }

    (*out) = difference;

    return 1;
}

int number_multiply(int left, int right, int *out)
{
    int product, goal, increment;

    if (left < NUMBER_MIN || left > NUMBER_MAX || right < NUMBER_MIN || right > NUMBER_MAX)
    {
        return 0;
    }

    product = 0;

    if (abs(left) < abs(right))
    {
        goal = left;
        increment = right;
    }
    else
    {
        goal = right;
        increment = left;
    }

    if (goal < 0)
    {
        goal *= -1;
        increment *= -1;
    }

    if (goal == 1)
    {
        product = increment;
    }
    else
    {
        while (goal--)
        {
            if (!number_add(product, increment, &product))
            {
                return 0;
            }
        }
    }

    (*out) = product;

    return 1;
}

int number_divide(int left, int right, int *out)
{
    int quotient;

    if (left < NUMBER_MIN || left > NUMBER_MAX || right < NUMBER_MIN || right > NUMBER_MAX)
    {
        return 0;
    }

    if (right == 0)
    {
        return 0;
    }

    quotient = div(left, right).quot;

    (*out) = quotient;

    return 1;
}

void destroy_value(value_t *value)
{
    if (value->data)
    {
        if (value->type == TYPE_LIST)
        {
            destroy_items(value->data, value->size);
        }
        else
        {
            free(value->data);
        }
    }

    free(value);
}

void destroy_items(value_t **items, size_t length)
{
    size_t index;

    for (index = 0; index < length; index++)
    {
        destroy_value(items[index]);
    }

    free(items);
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

static char *quote_string(char *body, char qualifier)
{
    char *represent;
    size_t length, index;

    length = strlen(body);
    represent = realloc(body, sizeof(char) * (length + 2 + 1));

    if (!represent)
    {
        free(body);
        return NULL;
    }

    for (index = length; index > 0; index--)
    {
        represent[index] = represent[index - 1];
    }

    represent[index] = qualifier;
    represent[length + 1] = qualifier;
    represent[length + 2] = '\0';

    return represent;
}
