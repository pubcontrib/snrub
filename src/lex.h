#ifndef LEX_H
#define LEX_H

#include <stdlib.h>

typedef enum
{
    SCANNER_STATE_ROAMING,
    SCANNER_STATE_NUMBER,
    SCANNER_STATE_STRING,
    SCANNER_STATE_CLOSED
} scanner_state_t;

typedef struct
{
    char *document;
    size_t position;
    scanner_state_t state;
} scanner_t;

typedef enum
{
    TOKEN_NAME_UNKNOWN,
    TOKEN_NAME_START,
    TOKEN_NAME_END,
    TOKEN_NAME_WHITESPACE,
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
char number_symbol();
char string_symbol();
char escape_symbol();
void destroy_scanner(scanner_t *scanner);
void destroy_token(token_t *token);

#endif
