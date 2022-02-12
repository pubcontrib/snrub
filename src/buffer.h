#ifndef BUFFER_H
#define BUFFER_H

typedef struct
{
    char *bytes;
    size_t length;
} buffer_t;

int compare_buffers(buffer_t *left, buffer_t *right);
int buffer_to_integer(buffer_t *buffer, int digits, int *out);
buffer_t *integer_to_buffer(int integer);
char *buffer_to_cstring(buffer_t *buffer);
buffer_t *cstring_to_buffer(char *string);
buffer_t *slice_buffer(buffer_t *buffer, size_t start, size_t end);
buffer_t *copy_buffer(buffer_t *buffer);
void resize_buffer(buffer_t *buffer, size_t length);
buffer_t *create_buffer(char *bytes, size_t length);
void destroy_buffer(buffer_t *buffer);

#endif
