#ifndef COMMON_H
#define COMMON_H

char *slice_string(char *string, size_t start, size_t end);
char *copy_string(char *string);
char *integer_to_string(int integer);
int string_to_integer(char *string, int digits, int *out);
int integer_digits(int integer);

#endif
