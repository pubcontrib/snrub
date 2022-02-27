#ifndef STRING_H
#define STRING_H

typedef struct
{
    char *bytes;
    size_t length;
} string_t;

int compare_strings(string_t *left, string_t *right);
int string_to_integer(string_t *string, int digits, int *out);
string_t *integer_to_string(int integer);
char *string_to_cstring(string_t *string);
string_t *cstring_to_string(char *cstring);
string_t *slice_string(string_t *string, size_t start, size_t end);
string_t *copy_string(string_t *string);
void resize_string(string_t *string, size_t length);
string_t *create_string(char *bytes, size_t length);
void destroy_string(string_t *string);

#endif
