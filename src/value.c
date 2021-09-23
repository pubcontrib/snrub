#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "value.h"
#include "map.h"
#include "common.h"

static value_t *create_value(value_type_t type, void *data, size_t size, int thrown);
static value_t *quote_string(char *body, char qualifier);
static size_t characters_in_string(char *string, char character);
static void *copy_memory(void *memory, size_t size);
static int *integer_to_array(int integer);
static int compare_strings_ascending(const void *left, const void *right);
static void destroy_value_unsafe(void *value);
static int overflow_add(int left, int right);
static char **array_map_keys(map_t *map);

int is_portable(void)
{
    if (INT_MIN > -2147483647L || INT_MAX < 2147483647L)
    {
        return 0;
    }

    if (((size_t) -1) < 2147483647L)
    {
        return 0;
    }

    return 1;
}

value_t *merge_lists(value_t *left, value_t *right)
{
    value_t **items;
    size_t leftLength, rightLength, index;
    int sumLength;

    leftLength = length_value(left);
    rightLength = length_value(right);

    if (!number_add(leftLength, rightLength, &sumLength))
    {
        return throw_error(ERROR_BOUNDS);
    }

    if (sumLength > 0)
    {
        items = allocate(sizeof(value_t *) * sumLength);

        for (index = 0; index < sumLength; index++)
        {
            value_t *copy;

            copy = copy_value(index < leftLength ? ((value_t **) left->data)[index] : ((value_t **) right->data)[index - leftLength]);
            items[index] = copy;
        }
    }
    else
    {
        items = NULL;
    }

    return new_list(items, sumLength);
}

value_t *merge_maps(value_t *left, value_t *right)
{
    map_t *pairs, *data;
    size_t leftLength, rightLength, index;
    int sumLength;

    leftLength = length_value(left);
    rightLength = length_value(right);

    if (!number_add(leftLength, rightLength, &sumLength))
    {
        return throw_error(ERROR_BOUNDS);
    }

    data = empty_map(hash_string, destroy_value_unsafe, 8);
    pairs = left->data;

    for (index = 0; index < pairs->capacity; index++)
    {
        if (pairs->chains[index])
        {
            map_chain_t *chain;

            for (chain = pairs->chains[index]; chain != NULL; chain = chain->next)
            {
                char *key;
                value_t *value;

                key = copy_string(chain->key);
                value = copy_value(chain->value);
                set_map_item(data, key, value);
            }
        }
    }

    pairs = right->data;

    for (index = 0; index < pairs->capacity; index++)
    {
        if (pairs->chains[index])
        {
            map_chain_t *chain;

            for (chain = pairs->chains[index]; chain != NULL; chain = chain->next)
            {
                char *key;
                value_t *value;

                key = copy_string(chain->key);
                value = copy_value(chain->value);
                set_map_item(data, key, value);
            }
        }
    }

    return new_map(data);
}

value_t *new_unset(void)
{
    return create_value(VALUE_TYPE_UNSET, NULL, 0, 0);
}

value_t *new_null(void)
{
    return create_value(VALUE_TYPE_NULL, NULL, 0, 0);
}

value_t *new_number(int number)
{
    int *data;

    data = integer_to_array(number);

    return create_value(VALUE_TYPE_NUMBER, data, sizeof(int), 0);
}

value_t *new_string(char *string)
{
    char *data;

    data = copy_string(string);

    return create_value(VALUE_TYPE_STRING, data, sizeof(char) * (strlen(string) + 1), 0);
}

value_t *new_list(value_t **items, size_t length)
{
    return create_value(VALUE_TYPE_LIST, items, length, 0);
}

value_t *new_map(map_t *pairs)
{
    return create_value(VALUE_TYPE_MAP, pairs, pairs ? pairs->length : 0, 0);
}

value_t *new_call(void)
{
    return create_value(VALUE_TYPE_CALL, NULL, 0, 0);
}

value_t *steal_string(char *string, size_t size)
{
    return create_value(VALUE_TYPE_STRING, string, size, 0);
}

value_t *throw_error(error_t error)
{
    value_t *number;

    number = new_number(error);
    number->thrown = 1;

    return number;
}

value_t *copy_value(value_t *this)
{
    switch (this->type)
    {
        case VALUE_TYPE_LIST:
        {
            value_t **data;
            size_t length;

            length = this->size;

            if (length > 0)
            {
                value_t **items;
                size_t index;

                items = this->data;
                data = allocate(sizeof(value_t *) * length);

                for (index = 0; index < length; index++)
                {
                    value_t *copy;

                    copy = copy_value(items[index]);
                    data[index] = copy;
                }
            }
            else
            {
                data = NULL;
            }

            return new_list(data, length);
        }
        case VALUE_TYPE_MAP:
        {
            map_t *pairs, *data;
            size_t index;

            pairs = this->data;
            data = empty_map(hash_string, destroy_value_unsafe, 8);

            for (index = 0; index < pairs->capacity; index++)
            {
                if (pairs->chains[index])
                {
                    map_chain_t *chain;

                    for (chain = pairs->chains[index]; chain != NULL; chain = chain->next)
                    {
                        char *key;
                        value_t *value;

                        key = copy_string(chain->key);
                        value = copy_value(chain->value);
                        set_map_item(data, key, value);
                    }
                }
            }

            return new_map(data);
        }
        default:
        {
            void *data;

            data = this->size > 0 ? copy_memory(this->data, this->size) : NULL;

            return create_value(this->type, data, this->size, this->thrown);
        }
    }
}

int hash_value(value_t *this)
{
    switch (this->type)
    {
        case VALUE_TYPE_NULL:
            return hash_null();
        case VALUE_TYPE_NUMBER:
            return hash_number(view_number(this));
        case VALUE_TYPE_STRING:
            return hash_string(view_string(this));
        case VALUE_TYPE_LIST:
            return hash_list(this->data, this->size);
        case VALUE_TYPE_MAP:
            return hash_map(this->data);
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
        hash = overflow_add(hash, string[index]);
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
        hash = overflow_add(hash, hash_value(items[index]));
    }

    return hash;
}

int hash_map(map_t *pairs)
{
    int hash;

    hash = 0;

    if (pairs->length > 0)
    {
        size_t index;

        for (index = 0; index < pairs->capacity; index++)
        {
            if (pairs->chains[index])
            {
                map_chain_t *chain;

                for (chain = pairs->chains[index]; chain != NULL; chain = chain->next)
                {
                    hash = overflow_add(hash, hash_string(chain->key));
                    hash = overflow_add(hash, hash_value(chain->value));
                }
            }
        }
    }

    return hash;
}

value_t *represent_value(value_t *this)
{
    switch (this->type)
    {
        case VALUE_TYPE_NULL:
            return represent_null();
        case VALUE_TYPE_NUMBER:
            return represent_number(view_number(this));
        case VALUE_TYPE_STRING:
            return represent_string(view_string(this));
        case VALUE_TYPE_LIST:
            return represent_list(this->data, this->size);
        case VALUE_TYPE_MAP:
            return represent_map(this->data);
        default:
            return NULL;
    }
}

value_t *represent_null(void)
{
    return new_string("?");
}

value_t *represent_number(int number)
{
    return quote_string(integer_to_string(number), '#');
}

value_t *represent_string(char *string)
{
    return quote_string(unescape_string(string), '\"');
}

value_t *represent_list(value_t **items, size_t length)
{
    char *body, *swap;
    size_t index;
    int fit;

    body = copy_string("[");

    for (index = 0; index < length; index++)
    {
        value_t *item, *represent;

        if (index > 0)
        {
            fit = string_add(body, " ", &swap);
            free(body);

            if (!fit)
            {
                free(swap);
                return throw_error(ERROR_BOUNDS);
            }

            body = swap;
        }

        item = items[index];
        represent = represent_value(item);

        if (represent->thrown)
        {
            free(swap);
            return represent;
        }

        fit = string_add(body, view_string(represent), &swap);
        free(body);
        destroy_value(represent);

        if (!fit)
        {
            free(swap);
            return throw_error(ERROR_BOUNDS);
        }

        body = swap;
    }

    fit = string_add(body, "]", &swap);
    free(body);

    if (!fit)
    {
        free(swap);
        return throw_error(ERROR_BOUNDS);
    }

    return steal_string(swap, sizeof(char) * (strlen(swap) + 1));
}

value_t *represent_map(map_t *pairs)
{
    char *body, *swap;
    int fit;

    body = copy_string("{");

    if (pairs->length > 0)
    {
        char **keys;
        size_t index;

        keys = array_map_keys(pairs);

        for (index = 0; index < pairs->length; index++)
        {
            char *key;
            value_t *value, *represent;

            key = keys[index];
            value = get_map_item(pairs, key);

            if (index > 0)
            {
                fit = string_add(body, " ", &swap);
                free(body);

                if (!fit)
                {
                    free(swap);
                    return throw_error(ERROR_BOUNDS);
                }

                body = swap;
            }

            represent = represent_string(key);

            if (represent->thrown)
            {
                free(swap);
                return represent;
            }

            fit = string_add(body, view_string(represent), &swap);
            free(body);
            destroy_value(represent);

            if (!fit)
            {
                free(swap);
                return throw_error(ERROR_BOUNDS);
            }

            body = swap;
            fit = string_add(body, " ", &swap);
            free(body);

            if (!fit)
            {
                free(swap);
                return throw_error(ERROR_BOUNDS);
            }

            body = swap;
            represent = represent_value(value);

            if (represent->thrown)
            {
                free(swap);
                return represent;
            }

            fit = string_add(body, view_string(represent), &swap);
            free(body);
            destroy_value(represent);

            if (!fit)
            {
                free(swap);
                return throw_error(ERROR_BOUNDS);
            }

            body = swap;
        }

        free(keys);
    }

    fit = string_add(body, "}", &swap);

    if (!fit)
    {
        free(swap);
        return throw_error(ERROR_BOUNDS);
    }

    free(body);

    return steal_string(swap, sizeof(char) * (strlen(swap) + 1));
}

char *escape_string(char *string)
{
    char *escape;
    size_t length, left, right;
    int escaping;

    length = strlen(string);
    escape = allocate(sizeof(char) * (length + 1));
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

    return escape;
}

char *unescape_string(char *string)
{
    char *unescape;
    size_t length, left, right;

    length = strlen(string)
        + characters_in_string(string, '\\')
        + characters_in_string(string, '"')
        + characters_in_string(string, '\t')
        + characters_in_string(string, '\n')
        + characters_in_string(string, '\r');
    unescape = allocate(sizeof(char) * (length + 1));

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

    return unescape;
}

int compare_values(value_t *left, value_t *right)
{
    if (left->type != right->type)
    {
        return left->type - right->type;
    }

    switch (left->type)
    {
        case VALUE_TYPE_NULL:
            return 0;
        case VALUE_TYPE_NUMBER:
            return view_number(left) - view_number(right);
        case VALUE_TYPE_STRING:
            return strcmp(view_string(left), view_string(right));
        case VALUE_TYPE_LIST:
        {
            size_t index;

            if (left->size != right->size)
            {
                return left->size - right->size;
            }

            for (index = 0; index < left->size; index++)
            {
                int different;

                different = compare_values(((value_t **) left->data)[index], ((value_t **) right->data)[index]);

                if (different)
                {
                    return different;
                }
            }

            return 0;
        }
        case VALUE_TYPE_MAP:
        {
            size_t length;

            if (left->size != right->size)
            {
                return left->size - right->size;
            }

            length = left->size;

            if (length > 0)
            {
                char **leftKeys, **rightKeys;
                map_t *leftMap, *rightMap;
                size_t index;

                leftMap = left->data;
                rightMap = right->data;
                leftKeys = array_map_keys(leftMap);
                rightKeys = array_map_keys(rightMap);

                for (index = 0; index < length; index++)
                {
                    int different;
                    char *leftKey, *rightKey;
                    value_t *leftValue, *rightValue;

                    leftKey = leftKeys[index];
                    rightKey = rightKeys[index];
                    different = strcmp(leftKey, rightKey);

                    if (different)
                    {
                        free(leftKeys);
                        free(rightKeys);
                        return different;
                    }

                    leftValue = get_map_item(leftMap, leftKey);
                    rightValue = get_map_item(rightMap, rightKey);

                    if (!rightValue)
                    {
                        free(leftKeys);
                        free(rightKeys);
                        return -1;
                    }

                    different = compare_values(leftValue, rightValue);

                    if (different)
                    {
                        free(leftKeys);
                        free(rightKeys);
                        return different;
                    }
                }

                free(leftKeys);
                free(rightKeys);
            }

            return 0;
        }
        default:
            return 0;
    }
}

size_t length_value(value_t *value)
{
    switch (value->type)
    {
        case VALUE_TYPE_STRING:
            return value->size - 1;
        case VALUE_TYPE_LIST:
        case VALUE_TYPE_MAP:
            return value->size;
        default:
            return 0;
    }
}

int view_number(value_t *value)
{
    switch (value->type)
    {
        case VALUE_TYPE_NUMBER:
            return ((int *) value->data)[0];
        default:
            return 0;
    }
}

char *view_string(value_t *value)
{
    switch (value->type)
    {
        case VALUE_TYPE_STRING:
            return (char *) value->data;
        default:
            return "";
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

int number_modulo(int left, int right, int *out)
{
    int remainder;

    if (left < NUMBER_MIN || left > NUMBER_MAX || right < NUMBER_MIN || right > NUMBER_MAX)
    {
        return 0;
    }

    if (right == 0)
    {
        return 0;
    }

    remainder = div(left, right).rem;

    (*out) = remainder;

    return 1;
}

int string_add(char *left, char *right, char **out)
{
    char *sum;
    size_t leftLength, rightLength;
    int sumLength;

    leftLength = strlen(left);
    rightLength = strlen(right);

    if (!number_add(leftLength, rightLength, &sumLength))
    {
        return 0;
    }

    sum = allocate(sizeof(char) * (sumLength + 1));

    strncpy(sum, left, leftLength);
    strncpy(sum + leftLength, right, rightLength + 1);

    (*out) = sum;

    return 1;
}

void destroy_value(value_t *value)
{
    if (value->data)
    {
        switch (value->type)
        {
            case VALUE_TYPE_LIST:
                destroy_items(value->data, value->size);
                break;
            case VALUE_TYPE_MAP:
                destroy_map(value->data);
                break;
            default:
                free(value->data);
                break;
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

static value_t *create_value(value_type_t type, void *data, size_t size, int thrown)
{
    value_t *value;

    value = allocate(sizeof(value_t));
    value->type = type;
    value->data = data;
    value->size = size;
    value->thrown = thrown;

    return value;
}

static value_t *quote_string(char *body, char qualifier)
{
    char *represent;
    size_t length, size, index;

    length = strlen(body);
    size = sizeof(char) * (length + 2 + 1);
    represent = reallocate(body, size);

    for (index = length; index > 0; index--)
    {
        represent[index] = represent[index - 1];
    }

    represent[index] = qualifier;
    represent[length + 1] = qualifier;
    represent[length + 2] = '\0';

    return steal_string(represent, size);
}

static size_t characters_in_string(char *string, char character)
{
    size_t count, length, index;

    count = 0;
    length = strlen(string);

    for (index = 0; index < length; index++)
    {
        if (string[index] == character)
        {
            count += 1;
        }
    }

    return count;
}

static void *copy_memory(void *memory, size_t size)
{
    void *copy;

    copy = allocate(size);
    memcpy(copy, memory, size);

    return copy;
}

static int *integer_to_array(int integer)
{
    int *array;

    array = allocate(sizeof(int));
    array[0] = integer;

    return array;
}

static int compare_strings_ascending(const void *left, const void *right)
{
    return strcmp(*(char **) left, *(char **) right);
}

static void destroy_value_unsafe(void *value)
{
    destroy_value((value_t *) value);
}

static int overflow_add(int left, int right)
{
    int sum;

    sum = left + right;

    if (sum > NUMBER_MAX)
    {
        return sum - NUMBER_MAX + NUMBER_MIN - 1;
    }
    else if (sum < NUMBER_MIN)
    {
        return sum - NUMBER_MIN + NUMBER_MAX + 1;
    }
    else
    {
        return sum;
    }
}

static char **array_map_keys(map_t *map)
{
    char **keys;
    size_t index, placement;

    keys = allocate(sizeof(char *) * map->length);

    for (index = 0, placement = 0; index < map->capacity; index++)
    {
        if (map->chains[index])
        {
            map_chain_t *chain;

            for (chain = map->chains[index]; chain != NULL; chain = chain->next)
            {
                keys[placement++] = chain->key;
            }
        }
    }

    qsort(keys, map->length, sizeof(char *), compare_strings_ascending);

    return keys;
}
