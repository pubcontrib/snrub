#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "common.h"

static int integer_digits(int integer);
static int shift_digit(int digit, int places);
static int is_digit(char symbol);

int compare_buffers(buffer_t *left, buffer_t *right)
{
    size_t index;

    for (index = 0; index < left->length; index++)
    {
        int different;

        if (index == right->length)
        {
            return 1;
        }

        different = left->bytes[index] - right->bytes[index];

        if (different)
        {
            return different;
        }
    }

    if (index < right->length)
    {
        return -1;
    }

    return 0;
}

int buffer_to_integer(buffer_t *buffer, int digits, int *out)
{
    int integer;

    integer = 0;

    if (buffer->length > 0)
    {
        size_t index;
        int negative;

        negative = buffer->bytes[0] == '-';

        if (buffer->length > (negative ? digits + 1 : digits) || (buffer->length == 1 && negative))
        {
            return 0;
        }

        for (index = negative ? 1 : 0; index < buffer->length; index++)
        {
            unsigned char symbol;

            symbol = buffer->bytes[index];

            if (!is_digit(symbol))
            {
                return 0;
            }

            integer += shift_digit(symbol - '0', buffer->length - index - 1);
        }

        if (negative)
        {
            integer *= -1;
        }
    }

    (*out) = integer;

    return 1;
}

buffer_t *integer_to_buffer(int integer)
{
    char *bytes;
    size_t length, index;

    length = integer_digits(integer) + (integer < 0 ? 1 : 0);
    bytes = allocate(sizeof(char) * length);
    index = length - 1;

    if (integer < 0)
    {
        bytes[0] = '-';
        integer *= -1;
    }

    if (integer == 0)
    {
        bytes[0] = '0';
    }

    while (integer > 0)
    {
        int next, digit;

        next = integer / 10;
        digit = integer - (next * 10);
        bytes[index--] = '0' + digit;
        integer = next;
    }

    return create_buffer(bytes, length);
}

char *buffer_to_cstring(buffer_t *buffer)
{
    char *string;

    string = allocate(sizeof(char) * (buffer->length + 1));

    if (buffer->length > 0)
    {
        memcpy(string, buffer->bytes, buffer->length);
    }

    string[buffer->length] = '\0';

    return string;
}

buffer_t *cstring_to_buffer(char *string)
{
    char *bytes;
    size_t length;

    length = strlen(string);

    if (length > 0)
    {
        bytes = allocate(sizeof(char) * length);
        memcpy(bytes, string, length);
    }
    else
    {
        bytes = NULL;
    }

    return create_buffer(bytes, length);
}

buffer_t *slice_buffer(buffer_t *buffer, size_t start, size_t end)
{
    char *bytes;
    size_t length;

    length = end - start;

    if (length > 0)
    {
        bytes = allocate(sizeof(char) * length);
        memcpy(bytes, buffer->bytes + start, length);
    }
    else
    {
        bytes = NULL;
    }

    return create_buffer(bytes, length);
}

buffer_t *copy_buffer(buffer_t *buffer)
{
    char *bytes;
    size_t length;

    length = buffer->length;

    if (length > 0)
    {
        bytes = allocate(sizeof(char) * length);
        memcpy(bytes, buffer->bytes, length);
    }
    else
    {
        bytes = NULL;
    }

    return create_buffer(bytes, length);
}

void resize_buffer(buffer_t *buffer, size_t length)
{
    if (buffer->length > 0)
    {
        if (length > 0)
        {
            buffer->bytes = reallocate(buffer->bytes, length);
        }
        else
        {
            free(buffer->bytes);
            buffer->bytes = NULL;
        }
    }
    else
    {
        if (length > 0)
        {
            buffer->bytes = allocate(length);
        }
        else
        {
            buffer->bytes = NULL;
        }
    }

    buffer->length = length;
}

buffer_t *create_buffer(char *bytes, size_t length)
{
    buffer_t *buffer;

    buffer = allocate(sizeof(buffer_t));
    buffer->bytes = bytes;
    buffer->length = length;

    return buffer;
}

void destroy_buffer(buffer_t *buffer)
{
    if (buffer->bytes)
    {
        free(buffer->bytes);
    }

    free(buffer);
}

static int integer_digits(int integer)
{
    int digits;

    if (integer < 0)
    {
        integer *= -1;
    }

    for (digits = 0; integer > 0 || digits == 0; digits++)
    {
        integer /= 10;
    }

    return digits;
}

static int shift_digit(int digit, int places)
{
    for (; places > 0; places--)
    {
        digit *= 10;
    }

    return digit;
}

static int is_digit(char symbol)
{
    return symbol >= 48 && symbol <= 57;
}
