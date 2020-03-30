#ifndef LEX_H
#define LEX_H

#include <stdlib.h>

typedef enum
{
    LEX_IDENTIFIER_UNKNOWN,
    LEX_IDENTIFIER_COMMENT,
    LEX_IDENTIFIER_VALUE,
    LEX_IDENTIFIER_ASSIGN,
    LEX_IDENTIFIER_ADD,
    LEX_IDENTIFIER_SUBTRACT,
    LEX_IDENTIFIER_MULTIPLY,
    LEX_IDENTIFIER_DIVIDE,
    LEX_IDENTIFIER_START,
    LEX_IDENTIFIER_END,
    LEX_IDENTIFIER_SPACE,
    LEX_IDENTIFIER_TAB,
    LEX_IDENTIFIER_NEWLINE,
    LEX_IDENTIFIER_NULL,
    LEX_IDENTIFIER_NUMBER,
    LEX_IDENTIFIER_STRING
} lex_identifier_t;

typedef enum
{
    LEX_STATUS_ROAMING,
    LEX_STATUS_NUMBER,
    LEX_STATUS_STRING,
    LEX_STATUS_CLOSED
} lex_status_t;

typedef struct
{
    lex_identifier_t identifier;
    char *value;
} lex_token_t;

typedef struct
{
    char *document;
    size_t position;
    lex_status_t status;
} lex_cursor_t;

lex_cursor_t *lex_iterate_document(char *document);
lex_token_t *lex_next_token(lex_cursor_t *cursor);
char lex_number_symbol();
char lex_string_symbol();
char lex_escape_symbol();
void lex_destroy_cursor(lex_cursor_t *cursor);
void lex_destroy_token(lex_token_t *token);

#endif
