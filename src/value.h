#ifndef VALUE_H
#define VALUE_H

#include <stdlib.h>
#include <limits.h>

#if INT_MAX < 2147483647 || INT_MIN > -2147483647
#error int needs to be able to hold values between -2147483647 and 2147483647.
#endif

#define NUMBER_MAX 999999999
#define NUMBER_MIN -999999999
#define NUMBER_DIGIT_CAPACITY 9

typedef enum
{
    TYPE_UNSET = 0x00,
    TYPE_NULL = 0x01,
    TYPE_NUMBER = 0x02,
    TYPE_STRING = 0x04,
    TYPE_LIST = 0x08,
    TYPE_ERROR = 0x10,
    TYPE_CALL = 0x20
} type_t;

typedef struct
{
    type_t type;
    void *data;
    size_t size;
} value_t;

typedef enum
{
    ERROR_UNSET,
    ERROR_SYNTAX,
    ERROR_BOUNDS,
    ERROR_TYPE,
    ERROR_ARGUMENT,
    ERROR_ARITHMETIC,
    ERROR_IO
} error_t;

value_t *merge_lists(value_t *left, value_t *right);
value_t *new_unset(void);
value_t *new_null(void);
value_t *new_number(int number);
value_t *new_string(char *string);
value_t *new_list(value_t **items, size_t length);
value_t *new_error(error_t error);
value_t *new_call(void);
value_t *steal_number(int *number, size_t size);
value_t *steal_string(char *string, size_t size);
value_t *steal_error(error_t *error, size_t size);
value_t *copy_value(value_t *this);
int hash_value(value_t *this);
int hash_null(void);
int hash_number(int number);
int hash_string(char *string);
int hash_list(value_t **items, size_t length);
char *represent_value(value_t *this);
char *represent_null(void);
char *represent_number(int number);
char *represent_string(char *string);
char *represent_list(value_t **items, size_t length);
char *represent_error(error_t error);
char *escape_string(char *string);
char *unescape_string(char *string);
int compare_values(value_t *left, value_t *right);
size_t length_value(value_t *value);
int view_number(value_t *value);
char *view_string(value_t *value);
error_t view_error(value_t *value);
int number_add(int left, int right, int *out);
int number_subtract(int left, int right, int *out);
int number_multiply(int left, int right, int *out);
int number_divide(int left, int right, int *out);
int number_modulo(int left, int right, int *out);
void destroy_value(value_t *value);
void destroy_items(value_t **items, size_t length);

#endif
