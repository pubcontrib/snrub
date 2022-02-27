#ifndef LEX_H
#define LEX_H

#include <stddef.h>
#include "string.h"

#define SYMBOL_COMMENT '`'
#define SYMBOL_NULL '?'
#define SYMBOL_NUMBER '#'
#define SYMBOL_STRING '"'
#define SYMBOL_ESCAPE '\\'

typedef struct
{
    string_t *document;
    size_t start;
    size_t end;
    int closed;
} scanner_t;

typedef enum
{
    TOKEN_NAME_UNKNOWN,
    TOKEN_NAME_WHITESPACE,
    TOKEN_NAME_COMMENT,
    TOKEN_NAME_NULL,
    TOKEN_NAME_NUMBER,
    TOKEN_NAME_STRING,
    TOKEN_NAME_LIST_START,
    TOKEN_NAME_LIST_END,
    TOKEN_NAME_MAP_START,
    TOKEN_NAME_MAP_END,
    TOKEN_NAME_CALL_START,
    TOKEN_NAME_CALL_END
} token_name_t;

typedef struct
{
    token_name_t name;
    string_t *value;
} token_t;

scanner_t *start_scanner(string_t *document);
token_t *next_token(scanner_t *scanner);
void destroy_scanner(scanner_t *scanner);
void destroy_token(token_t *token);

#endif
