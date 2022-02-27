#include <stdlib.h>
#include <string.h>
#include "string.h"
#include "common.h"

static int integer_digits(int integer);
static int shift_digit(int digit, int places);
static int is_digit(char symbol);

int compare_strings(string_t *left, string_t *right)
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

int string_to_integer(string_t *string, int digits, int *out)
{
    int integer;

    integer = 0;

    if (string->length > 0)
    {
        size_t index;
        int negative;

        negative = string->bytes[0] == '-';

        if (string->length > (negative ? digits + 1 : digits) || (string->length == 1 && negative))
        {
            return 0;
        }

        for (index = negative ? 1 : 0; index < string->length; index++)
        {
            unsigned char symbol;

            symbol = string->bytes[index];

            if (!is_digit(symbol))
            {
                return 0;
            }

            integer += shift_digit(symbol - '0', string->length - index - 1);
        }

        if (negative)
        {
            integer *= -1;
        }
    }

    (*out) = integer;

    return 1;
}

string_t *integer_to_string(int integer)
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

    return create_string(bytes, length);
}

char *string_to_cstring(string_t *string)
{
    char *cstring;

    cstring = allocate(sizeof(char) * (string->length + 1));

    if (string->length > 0)
    {
        memcpy(cstring, string->bytes, string->length);
    }

    cstring[string->length] = '\0';

    return cstring;
}

string_t *cstring_to_string(char *cstring)
{
    char *bytes;
    size_t length;

    length = strlen(cstring);

    if (length > 0)
    {
        bytes = allocate(sizeof(char) * length);
        memcpy(bytes, cstring, length);
    }
    else
    {
        bytes = NULL;
    }

    return create_string(bytes, length);
}

string_t *slice_string(string_t *string, size_t start, size_t end)
{
    char *bytes;
    size_t length;

    length = end - start;

    if (length > 0)
    {
        bytes = allocate(sizeof(char) * length);
        memcpy(bytes, string->bytes + start, length);
    }
    else
    {
        bytes = NULL;
    }

    return create_string(bytes, length);
}

string_t *copy_string(string_t *string)
{
    char *bytes;
    size_t length;

    length = string->length;

    if (length > 0)
    {
        bytes = allocate(sizeof(char) * length);
        memcpy(bytes, string->bytes, length);
    }
    else
    {
        bytes = NULL;
    }

    return create_string(bytes, length);
}

void resize_string(string_t *string, size_t length)
{
    if (string->length > 0)
    {
        if (length > 0)
        {
            string->bytes = reallocate(string->bytes, length);
        }
        else
        {
            free(string->bytes);
            string->bytes = NULL;
        }
    }
    else
    {
        if (length > 0)
        {
            string->bytes = allocate(length);
        }
        else
        {
            string->bytes = NULL;
        }
    }

    string->length = length;
}

string_t *create_string(char *bytes, size_t length)
{
    string_t *string;

    string = allocate(sizeof(string_t));
    string->bytes = bytes;
    string->length = length;

    return string;
}

void destroy_string(string_t *string)
{
    if (string->bytes)
    {
        free(string->bytes);
    }

    free(string);
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
