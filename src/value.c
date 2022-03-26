#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "value.h"
#include "map.h"
#include "common.h"

static value_t *create_value(value_type_t type, void *data, size_t size, int thrown);
static void copy_map(map_t *from, map_t *to);
static value_t *quote_string(string_t *body, char qualifier);
static void *copy_memory(void *memory, size_t size);
static int *integer_to_array(int integer);
static int compare_strings_ascending(const void *left, const void *right);
static void destroy_value_unsafe(void *value);
static int overflow_add(int left, int right);
static string_t **array_map_keys(map_t *map);
static void destroy_keys(string_t **keys, size_t length);

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
    map_t *data;
    size_t leftLength, rightLength;
    int sumLength;

    leftLength = length_value(left);
    rightLength = length_value(right);

    if (!number_add(leftLength, rightLength, &sumLength))
    {
        return throw_error(ERROR_BOUNDS);
    }

    data = empty_map(hash_string, destroy_value_unsafe, 8);
    copy_map(left->data, data);
    copy_map(right->data, data);

    return new_map(data);
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

value_t *new_string(string_t *string)
{
    return create_value(VALUE_TYPE_STRING, string, string->length, 0);
}

value_t *new_list(value_t **items, size_t length)
{
    return create_value(VALUE_TYPE_LIST, items, length, 0);
}

value_t *new_map(map_t *pairs)
{
    return create_value(VALUE_TYPE_MAP, pairs, pairs ? pairs->length : 0, 0);
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
        case VALUE_TYPE_NULL:
        case VALUE_TYPE_NUMBER:
        {
            void *data;

            data = this->size > 0 ? copy_memory(this->data, this->size) : NULL;

            return create_value(this->type, data, this->size, this->thrown);
        }
        case VALUE_TYPE_STRING:
            return create_value(this->type, copy_string(this->data), this->size, this->thrown);
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
            map_t *data;

            data = empty_map(hash_string, destroy_value_unsafe, 8);
            copy_map(this->data, data);

            return new_map(data);
        }
        default:
            crash_with_message("unsupported branch %s", "VALUE_COPY_TYPE");
            return NULL;
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
            crash_with_message("unsupported branch %s", "VALUE_HASH_TYPE");
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

int hash_string(string_t *string)
{
    int hash;
    size_t index;

    hash = 0;

    for (index = 0; index < string->length; index++)
    {
        hash = overflow_add(hash, string->bytes[index]);
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
            crash_with_message("unsupported branch %s", "VALUE_REPRESENT_TYPE");
            return NULL;
    }
}

value_t *represent_null(void)
{
    return new_string(cstring_to_string("?"));
}

value_t *represent_number(int number)
{
    return quote_string(integer_to_string(number), '#');
}

value_t *represent_string(string_t *string)
{
    value_t *unescaped;
    string_t *body;

    unescaped = unescape_string(string);

    if (unescaped->thrown)
    {
        return unescaped;
    }

    body = copy_string(view_string(unescaped));
    destroy_value(unescaped);

    return quote_string(body, '\"');
}

value_t *represent_list(value_t **items, size_t length)
{
    string_t *body, *swap, *delimiter, *end;
    size_t index;
    int fit;

    body = cstring_to_string("[");
    delimiter = cstring_to_string(" ");
    end = cstring_to_string("]");

    for (index = 0; index < length; index++)
    {
        value_t *item, *represent;

        if (index > 0)
        {
            fit = string_add(body, delimiter, &swap);
            destroy_string(body);

            if (!fit)
            {
                destroy_string(swap);
                destroy_string(delimiter);
                destroy_string(end);
                return throw_error(ERROR_BOUNDS);
            }

            body = swap;
        }

        item = items[index];
        represent = represent_value(item);

        if (represent->thrown)
        {
            destroy_string(swap);
            destroy_string(delimiter);
            destroy_string(end);
            return represent;
        }

        fit = string_add(body, view_string(represent), &swap);
        destroy_string(body);
        destroy_value(represent);

        if (!fit)
        {
            destroy_string(swap);
            destroy_string(delimiter);
            destroy_string(end);
            return throw_error(ERROR_BOUNDS);
        }

        body = swap;
    }

    fit = string_add(body, end, &swap);
    destroy_string(body);
    destroy_string(delimiter);
    destroy_string(end);

    if (!fit)
    {
        destroy_string(swap);
        return throw_error(ERROR_BOUNDS);
    }

    return new_string(swap);
}

value_t *represent_map(map_t *pairs)
{
    string_t *body, *swap, *delimiter, *end;
    int fit;

    body = cstring_to_string("{");
    delimiter = cstring_to_string(" ");
    end = cstring_to_string("}");

    if (pairs->length > 0)
    {
        string_t **keys;
        size_t index;

        keys = array_map_keys(pairs);

        for (index = 0; index < pairs->length; index++)
        {
            string_t *key;
            value_t *value, *represent;

            key = keys[index];
            value = get_map_item(pairs, key);

            if (index > 0)
            {
                fit = string_add(body, delimiter, &swap);
                destroy_string(body);

                if (!fit)
                {
                    destroy_string(swap);
                    destroy_string(delimiter);
                    destroy_string(end);
                    return throw_error(ERROR_BOUNDS);
                }

                body = swap;
            }

            represent = represent_string(key);

            if (represent->thrown)
            {
                destroy_string(swap);
                destroy_string(delimiter);
                destroy_string(end);
                return represent;
            }

            fit = string_add(body, view_string(represent), &swap);
            destroy_string(body);
            destroy_value(represent);

            if (!fit)
            {
                destroy_string(swap);
                destroy_string(delimiter);
                destroy_string(end);
                return throw_error(ERROR_BOUNDS);
            }

            body = swap;
            fit = string_add(body, delimiter, &swap);
            destroy_string(body);

            if (!fit)
            {
                destroy_string(swap);
                destroy_string(delimiter);
                destroy_string(end);
                return throw_error(ERROR_BOUNDS);
            }

            body = swap;
            represent = represent_value(value);

            if (represent->thrown)
            {
                destroy_string(swap);
                destroy_string(delimiter);
                destroy_string(end);
                return represent;
            }

            fit = string_add(body, view_string(represent), &swap);
            destroy_string(body);
            destroy_value(represent);

            if (!fit)
            {
                destroy_string(swap);
                destroy_string(delimiter);
                destroy_string(end);
                return throw_error(ERROR_BOUNDS);
            }

            body = swap;
        }

        destroy_keys(keys, pairs->length);
    }

    fit = string_add(body, end, &swap);
    destroy_string(body);
    destroy_string(delimiter);
    destroy_string(end);

    if (!fit)
    {
        destroy_string(swap);
        return throw_error(ERROR_BOUNDS);
    }

    return new_string(swap);
}

value_t *escape_string(string_t *string)
{
    char *bytes;
    size_t left, right;
    int escaping;

    if (string->length > 0)
    {
        bytes = allocate(sizeof(char) * string->length);
    }
    else
    {
        bytes = NULL;
    }

    escaping = 0;
    right = 0;

    for (left = 0; left < string->length; left++)
    {
        char current;

        current = string->bytes[left];

        if (escaping)
        {
            switch (current)
            {
                case '\\':
                    bytes[right++] = '\\';
                    break;
                case '"':
                    bytes[right++] = '"';
                    break;
                case 't':
                    bytes[right++] = '\t';
                    break;
                case 'n':
                    bytes[right++] = '\n';
                    break;
                case 'r':
                    bytes[right++] = '\r';
                    break;
                case 'a':
                    if (string->length > 2 && left < string->length - 3)
                    {
                        string_t *substring;
                        int code;

                        substring = slice_string(string, left + 1, left + 4);

                        if (string_to_integer(substring, 3, &code) && code >= 0 && code <= 255)
                        {
                            bytes[right++] = code;
                        }
                        else
                        {
                            destroy_string(substring);
                            free(bytes);
                            return throw_error(ERROR_TYPE);
                        }

                        destroy_string(substring);
                    }
                    else
                    {
                        free(bytes);
                        return throw_error(ERROR_TYPE);
                    }

                    left += 3;
                    break;
                default:
                    free(bytes);
                    return throw_error(ERROR_TYPE);
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
                bytes[right++] = current;
            }
        }
    }

    if (right > 0)
    {
        bytes = reallocate(bytes, right);
    }

    return new_string(create_string(bytes, right));
}

value_t *unescape_string(string_t *string)
{
    char *bytes;
    size_t length, left, right;

    length = 0;

    for (left = 0; left < string->length; left++)
    {
        unsigned char symbol;

        symbol = string->bytes[left];

        if (symbol == '\\' || symbol == '"' || symbol == '\t' || symbol == '\n' || symbol == '\r')
        {
            length += 2;
        }
        else if (symbol >= 32 && symbol <= 126)
        {
            length++;
        }
        else
        {
            length += 5;
        }
    }

    if (length > 0)
    {
        bytes = allocate(sizeof(char) * length);
    }
    else
    {
        bytes = NULL;
    }

    for (left = 0, right = 0; left < length; right++)
    {
        unsigned char symbol;

        symbol = string->bytes[right];

        switch (symbol)
        {
            case '\\':
                bytes[left++] = '\\';
                bytes[left++] = '\\';
                break;
            case '"':
                bytes[left++] = '\\';
                bytes[left++] = '"';
                break;
            case '\t':
                bytes[left++] = '\\';
                bytes[left++] = 't';
                break;
            case '\n':
                bytes[left++] = '\\';
                bytes[left++] = 'n';
                break;
            case '\r':
                bytes[left++] = '\\';
                bytes[left++] = 'r';
                break;
            default:
                if (symbol >= 32 && symbol <= 126)
                {
                    bytes[left++] = symbol;
                }
                else
                {
                    string_t *substring;

                    substring = integer_to_string(symbol);
                    bytes[left++] = '\\';
                    bytes[left++] = 'a';

                    if (substring->length == 1)
                    {
                        bytes[left++] = '0';
                        bytes[left++] = '0';
                        bytes[left++] = substring->bytes[0];
                    }
                    else if (substring->length == 2)
                    {
                        bytes[left++] = '0';
                        bytes[left++] = substring->bytes[0];
                        bytes[left++] = substring->bytes[1];
                    }
                    else if (substring->length == 3)
                    {
                        bytes[left++] = substring->bytes[0];
                        bytes[left++] = substring->bytes[1];
                        bytes[left++] = substring->bytes[2];
                    }
                    else
                    {
                        crash_with_message("unsupported branch %s", "VALUE_UNESCAPE_ASCII");
                    }

                    destroy_string(substring);
                }

                break;
        }
    }

    return new_string(create_string(bytes, length));
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
            return compare_strings(view_string(left), view_string(right));
        case VALUE_TYPE_LIST:
        {
            size_t index;

            for (index = 0; index < left->size; index++)
            {
                int different;

                if (index == right->size)
                {
                    return 1;
                }

                different = compare_values(((value_t **) left->data)[index], ((value_t **) right->data)[index]);

                if (different)
                {
                    return different;
                }
            }

            if (index < right->size)
            {
                return -1;
            }

            return 0;
        }
        case VALUE_TYPE_MAP:
        {
            size_t length, index;

            length = left->size;
            index = 0;

            if (length > 0)
            {
                string_t **leftKeys, **rightKeys;
                map_t *leftMap, *rightMap;

                leftMap = left->data;
                rightMap = right->data;

                if (rightMap->length == 0)
                {
                    return 1;
                }

                leftKeys = array_map_keys(leftMap);
                rightKeys = array_map_keys(rightMap);

                for (; index < length; index++)
                {
                    int different;
                    string_t *leftKey, *rightKey;
                    value_t *leftValue, *rightValue;

                    if (index == right->size)
                    {
                        destroy_keys(leftKeys, leftMap->length);
                        destroy_keys(rightKeys, rightMap->length);
                        return 1;
                    }

                    leftKey = leftKeys[index];
                    rightKey = rightKeys[index];
                    different = compare_strings(leftKey, rightKey);

                    if (different)
                    {
                        destroy_keys(leftKeys, leftMap->length);
                        destroy_keys(rightKeys, rightMap->length);
                        return different;
                    }

                    leftValue = get_map_item(leftMap, leftKey);
                    rightValue = get_map_item(rightMap, rightKey);

                    if (!rightValue)
                    {
                        destroy_keys(leftKeys, leftMap->length);
                        destroy_keys(rightKeys, rightMap->length);
                        return -1;
                    }

                    different = compare_values(leftValue, rightValue);

                    if (different)
                    {
                        destroy_keys(leftKeys, leftMap->length);
                        destroy_keys(rightKeys, rightMap->length);
                        return different;
                    }
                }

                destroy_keys(leftKeys, leftMap->length);
                destroy_keys(rightKeys, rightMap->length);
            }

            if (index < right->size)
            {
                return -1;
            }

            return 0;
        }
        default:
            crash_with_message("unsupported branch %s", "VALUE_COMPARE_TYPE");
            return 0;
    }
}

size_t length_value(value_t *value)
{
    switch (value->type)
    {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_LIST:
        case VALUE_TYPE_MAP:
            return value->size;
        default:
            crash_with_message("unsupported branch %s", "VALUE_LENGTH_TYPE");
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
            crash_with_message("unsupported branch %s", "VALUE_NUMBER_TYPE");
            return 0;
    }
}

string_t *view_string(value_t *value)
{
    switch (value->type)
    {
        case VALUE_TYPE_STRING:
            return (string_t *) value->data;
        default:
            crash_with_message("unsupported branch %s", "VALUE_STRING_TYPE");
            return NULL;
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

int string_add(string_t *left, string_t *right, string_t **out)
{
    char *bytes;
    int length;

    if (!number_add(left->length, right->length, &length))
    {
        return 0;
    }

    if (length > 0)
    {
        bytes = allocate(sizeof(char) * length);
        memcpy(bytes, left->bytes, left->length);
        memcpy(bytes + left->length, right->bytes, right->length);
    }
    else
    {
        bytes = NULL;
    }

    (*out) = create_string(bytes, length);

    return 1;
}

value_t *read_file(string_t *path)
{
    FILE *file;
    char *cPath;

    cPath = string_to_cstring(path);
    file = fopen(cPath, "rb");
    free(cPath);

    if (file)
    {
        char *bytes;
        long length;

        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (length > NUMBER_MAX)
        {
            fclose(file);
            return throw_error(ERROR_BOUNDS);
        }

        if (length > 0)
        {
            bytes = allocate(sizeof(char) * length);
            fread(bytes, 1, length, file);
        }
        else
        {
            bytes = NULL;
        }

        if (ferror(file))
        {
            if (bytes)
            {
                free(bytes);
            }

            fclose(file);
            return new_null();
        }
        else
        {
            fclose(file);
            return new_string(create_string(bytes, length));
        }
    }

    return new_null();
}

void destroy_value(value_t *value)
{
    if (value->data)
    {
        switch (value->type)
        {
            case VALUE_TYPE_NULL:
            case VALUE_TYPE_NUMBER:
                free(value->data);
                break;
            case VALUE_TYPE_STRING:
                destroy_string(value->data);
                break;
            case VALUE_TYPE_LIST:
                destroy_items(value->data, value->size);
                break;
            case VALUE_TYPE_MAP:
                destroy_map(value->data);
                break;
            default:
                crash_with_message("unsupported branch %s", "VALUE_DESTROY_TYPE");
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

static void copy_map(map_t *from, map_t *to)
{
    size_t index;

    for (index = 0; index < from->capacity; index++)
    {
        if (from->chains[index])
        {
            map_chain_t *chain;

            for (chain = from->chains[index]; chain != NULL; chain = chain->next)
            {
                string_t *key;
                value_t *value;

                key = copy_string(chain->key);
                value = copy_value(chain->value);
                set_map_item(to, key, value);
            }
        }
    }
}

static value_t *quote_string(string_t *body, char qualifier)
{
    size_t length, index;

    length = body->length;
    resize_string(body, length + 2);

    for (index = length; index > 0; index--)
    {
        body->bytes[index] = body->bytes[index - 1];
    }

    body->bytes[index] = qualifier;
    body->bytes[length + 1] = qualifier;

    return new_string(body);
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
    return compare_strings(*(string_t **) left, *(string_t **) right);
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

static string_t **array_map_keys(map_t *map)
{
    string_t **keys;
    size_t index, placement;

    keys = allocate(sizeof(string_t *) * map->length);

    for (index = 0, placement = 0; index < map->capacity; index++)
    {
        if (map->chains[index])
        {
            map_chain_t *chain;

            for (chain = map->chains[index]; chain != NULL; chain = chain->next)
            {
                keys[placement++] = copy_string(chain->key);
            }
        }
    }

    qsort(keys, map->length, sizeof(string_t *), compare_strings_ascending);

    return keys;
}

static void destroy_keys(string_t **keys, size_t length)
{
    size_t index;

    for (index = 0; index < length; index++)
    {
        destroy_string(keys[index]);
    }

    free(keys);
}
