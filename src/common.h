#ifndef COMMON_H
#define COMMON_H

#define PROGRAM_NAME "snrub"
#define PROGRAM_SUCCESS 0
#define PROGRAM_ERROR 1

char *slice_string(char *string, size_t start, size_t end);
char *copy_string(char *string);
char *integer_to_string(int integer);
int string_to_integer(char *string, int digits, int *out);
void *allocate(size_t size);
void *callocate(int number, size_t size);
void *reallocate(void *memory, size_t size);
void crash_with_message(char *format, ...);
void crash(void);

#endif
