#ifndef CLI_H
#define CLI_H

typedef struct
{
    char *string;
    size_t length;
    size_t capacity;
    int exit;
} line_t;

int get_flag(int argc, char **argv, char *name);
char *get_option(int argc, char **argv, char *name);
char *read_file(char *path);
line_t *next_line(void);
void destroy_line(line_t *line);

#endif
