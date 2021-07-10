#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>
#include "map.h"

#define NUMBER_MAX 999999999
#define NUMBER_MIN -999999999
#define NUMBER_DIGIT_CAPACITY 9

typedef enum
{
    VALUE_TYPE_UNSET = 0x00,
    VALUE_TYPE_NULL = 0x01,
    VALUE_TYPE_NUMBER = 0x02,
    VALUE_TYPE_STRING = 0x04,
    VALUE_TYPE_LIST = 0x08,
    VALUE_TYPE_MAP = 0x10,
    VALUE_TYPE_CALL = 0x20
} value_type_t;

typedef struct
{
    value_type_t type;
    void *data;
    size_t size;
    int thrown;
} value_t;

typedef enum
{
    ERROR_UNSET,
    ERROR_SYNTAX,
    ERROR_BOUNDS,
    ERROR_TYPE,
    ERROR_ARGUMENT,
    ERROR_ARITHMETIC
} error_t;

int is_portable(void);
value_t *merge_lists(value_t *left, value_t *right);
value_t *new_unset(void);
value_t *new_null(void);
value_t *new_number(int number);
value_t *new_string(char *string);
value_t *new_list(value_t **items, size_t length);
value_t *new_map(map_t *pairs);
value_t *new_call(void);
value_t *steal_string(char *string, size_t size);
value_t *throw_error(error_t error);
value_t *copy_value(value_t *this);
int hash_value(value_t *this);
int hash_null(void);
int hash_number(int number);
int hash_string(char *string);
int hash_list(value_t **items, size_t length);
int hash_map(map_t *pairs);
value_t *represent_value(value_t *this);
value_t *represent_null(void);
value_t *represent_number(int number);
value_t *represent_string(char *string);
value_t *represent_list(value_t **items, size_t length);
value_t *represent_map(map_t *pairs);
char *escape_string(char *string);
char *unescape_string(char *string);
int compare_values(value_t *left, value_t *right);
size_t length_value(value_t *value);
int view_number(value_t *value);
char *view_string(value_t *value);
int number_add(int left, int right, int *out);
int number_subtract(int left, int right, int *out);
int number_multiply(int left, int right, int *out);
int number_divide(int left, int right, int *out);
int number_modulo(int left, int right, int *out);
int string_add(char *left, char *right, char **out);
void destroy_value(value_t *value);
void destroy_items(value_t **items, size_t length);

#endif
