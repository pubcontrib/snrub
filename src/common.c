#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

static int integer_digits(int integer);
static int shift_digit(int digit, int places);

char *slice_string(char *string, size_t start, size_t end)
{
    char *slice;
    size_t length;

    length = end - start;
    slice = malloc(sizeof(char) * (length + 1));

    if (slice)
    {
        strncpy(slice, string + start, length);
        slice[length] = '\0';
    }

    return slice;
}

char *copy_string(char *string)
{
    char *copy;
    size_t length;

    length = strlen(string);
    copy = malloc(sizeof(char) * (length + 1));

    if (copy)
    {
        strncpy(copy, string, length + 1);
    }

    return copy;
}

char *integer_to_string(int integer)
{
    char *string;
    size_t length;

    length = integer_digits(integer) + (integer < 0 ? 1 : 0);
    string = malloc(sizeof(char) * (length + 1));

    if (string)
    {
        size_t index;

        index = length - 1;

        if (integer < 0)
        {
            string[0] = '-';
            integer *= -1;
        }

        if (integer == 0)
        {
            string[0] = '0';
        }

        while (integer > 0)
        {
            int next, digit;

            next = integer / 10;
            digit = integer - (next * 10);
            string[index--] = '0' + digit;
            integer = next;
        }

        string[length] = '\0';
    }

    return string;
}

int string_to_integer(char *string, int digits, int *out)
{
    size_t length;
    int integer;

    length = strlen(string);
    integer = 0;

    if (length > 0)
    {
        size_t index;
        int negative;

        negative = string[0] == '-';

        if (length > (negative ? digits + 1 : digits) || (length == 1 && negative))
        {
            return 0;
        }

        for (index = negative ? 1 : 0; index < length; index++)
        {
            unsigned char symbol;

            symbol = string[index];

            if (!isdigit(symbol))
            {
                return 0;
            }

            integer += shift_digit(symbol - '0', length - index - 1);
        }

        if (negative)
        {
            integer *= -1;
        }
    }

    (*out) = integer;

    return 1;
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
