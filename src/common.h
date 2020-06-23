#ifndef COMMON_H
#define COMMON_H

char *slice_string(char *string, size_t start, size_t end);
char *copy_string(char *string);
size_t characters_in_string(char *string, char character);
void *copy_memory(void *memory, size_t size);
int *integer_to_array(int integer);
char *integer_to_string(int integer);
int integer_digits(int integer);
int is_integer(char *string);

#endif
