#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "common.h"

void *allocate(size_t size)
{
    void *memory;

    if (size == 0)
    {
        crash_with_message("zero memory requested");
    }

    memory = malloc(size);

    if (!memory)
    {
        crash_with_message("memory allocation failed");
    }

    return memory;
}

void *callocate(size_t number, size_t size)
{
    void *memory;

    if (number == 0 || size == 0)
    {
        crash_with_message("zero memory requested");
    }

    memory = calloc(number, size);

    if (!memory)
    {
        crash_with_message("memory allocation failed");
    }

    return memory;
}

void *reallocate(void *memory, size_t size)
{
    if (size == 0)
    {
        crash_with_message("zero memory requested");
    }

    memory = realloc(memory, size);

    if (!memory)
    {
        crash_with_message("memory allocation failed");
    }

    return memory;
}

void crash_with_message(char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    fprintf(stderr, "%s: ", PROGRAM_NAME);
    vfprintf(stderr, format, arguments);
    fprintf(stderr, "\n");
    va_end(arguments);

    crash();
}

void crash(void)
{
    exit(PROGRAM_ERROR);
}
