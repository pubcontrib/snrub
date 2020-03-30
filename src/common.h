#ifndef COMMON_H
#define COMMON_H

char *slice_string(char *string, size_t start, size_t end);
char *copy_string(char *string);
void *copy_memory(void *memory, size_t size);
int *integer_to_array(int integer);

#endif
