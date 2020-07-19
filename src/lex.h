#ifndef LEX_H
#define LEX_H

#include <stdlib.h>

static const char SYMBOL_COMMENT = '`';
static const char SYMBOL_NULL = '?';
static const char SYMBOL_NUMBER = '#';
static const char SYMBOL_STRING = '"';
static const char SYMBOL_ESCAPE = '\\';

typedef enum
{
    SCANNER_STATE_ROAMING,
    SCANNER_STATE_COMMENT,
    SCANNER_STATE_NUMBER,
    SCANNER_STATE_STRING,
    SCANNER_STATE_CLOSED
} scanner_state_t;

typedef struct
{
    char *document;
    size_t start;
    size_t end;
    size_t length;
    int escaping;
    scanner_state_t state;
} scanner_t;

typedef enum
{
    TOKEN_NAME_UNKNOWN,
    TOKEN_NAME_START,
    TOKEN_NAME_END,
    TOKEN_NAME_WHITESPACE,
    TOKEN_NAME_COMMENT,
    TOKEN_NAME_NULL,
    TOKEN_NAME_NUMBER,
    TOKEN_NAME_STRING
} token_name_t;

typedef struct
{
    token_name_t name;
    char *value;
} token_t;

scanner_t *start_scanner(char *document);
token_t *next_token(scanner_t *scanner);
void destroy_scanner(scanner_t *scanner);
void destroy_token(token_t *token);

#endif
