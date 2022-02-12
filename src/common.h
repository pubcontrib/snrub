#ifndef COMMON_H
#define COMMON_H

#define PROGRAM_NAME "snrub"
#define PROGRAM_SUCCESS 0
#define PROGRAM_ERROR 1

void *allocate(size_t size);
void *callocate(size_t number, size_t size);
void *reallocate(void *memory, size_t size);
void crash_with_message(char *format, ...);
void crash(void);

#endif
