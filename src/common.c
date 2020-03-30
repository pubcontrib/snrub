#include <stdlib.h>
#include <string.h>
#include "common.h"

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

void *copy_memory(void *memory, size_t size)
{
    void *copy;

    copy = malloc(size);

    if (copy)
    {
        memcpy(copy, memory, size);
    }

    return copy;
}

int *integer_to_array(int integer)
{
    int *array;

    array = malloc(sizeof(int));

    if (array)
    {
        array[0] = integer;
    }

    return array;
}
