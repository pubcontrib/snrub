#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "common.h"

static lex_cursor_t *create_cursor(char *document, size_t position, lex_status_t status);
static lex_token_t *create_token(lex_identifier_t identifier, char *value);
static lex_token_t *slice_token(lex_cursor_t *cursor, size_t start, size_t end, size_t length, lex_identifier_t identifier);
static lex_identifier_t match_identifier(char symbol);

lex_cursor_t *lex_iterate_document(char *document)
{
    return create_cursor(document, 0, LEX_STATUS_ROAMING);
}

lex_token_t *lex_next_token(lex_cursor_t *cursor)
{
    size_t length, start, end;
    int escaping;

    length = strlen(cursor->document);
    start = 0;
    end = 0;
    escaping = 0;

    while (cursor->position < length)
    {
        char symbol;

        symbol = cursor->document[cursor->position];
        cursor->position += 1;

        if (cursor->status == LEX_STATUS_ROAMING)
        {
            if (symbol == lex_number_symbol())
            {
                start = cursor->position - 1;
                end = cursor->position;
                cursor->status = LEX_STATUS_NUMBER;
            }
            else if (symbol == lex_string_symbol())
            {
                start = cursor->position - 1;
                end = cursor->position;
                cursor->status = LEX_STATUS_STRING;
            }
            else
            {
                lex_identifier_t identifier;

                identifier = match_identifier(symbol);

                return slice_token(cursor, cursor->position - 1, cursor->position, length, identifier);
            }
        }
        else if (cursor->status == LEX_STATUS_NUMBER)
        {
            if (symbol == lex_number_symbol())
            {
                return slice_token(cursor, start, end + 1, length, LEX_IDENTIFIER_NUMBER);
            }
            else
            {
                end = cursor->position;
            }
        }
        else if (cursor->status == LEX_STATUS_STRING)
        {
            if (symbol == lex_string_symbol())
            {
                if (escaping)
                {
                    escaping = 0;
                }
                else
                {
                    return slice_token(cursor, start, end + 1, length, LEX_IDENTIFIER_STRING);
                }
            }
            else if (symbol == lex_escape_symbol())
            {
                escaping = escaping ? 0 : 1;
            }
            else
            {
                if (escaping)
                {
                    escaping = 0;
                }
                else
                {
                    end = cursor->position;
                }
            }
        }
    }

    if (cursor->status == LEX_STATUS_NUMBER || cursor->status == LEX_STATUS_STRING)
    {
        return slice_token(cursor, start, end, length, LEX_IDENTIFIER_UNKNOWN);
    }
    else
    {
        cursor->status = LEX_STATUS_CLOSED;

        return NULL;
    }
}

char lex_number_symbol()
{
    return '#';
}

char lex_string_symbol()
{
    return '"';
}

char lex_escape_symbol()
{
    return '\\';
}

void lex_destroy_cursor(lex_cursor_t *cursor)
{
    if (cursor->document)
    {
        free(cursor->document);
    }

    free(cursor);
}

void lex_destroy_token(lex_token_t *token)
{
    if (token->value)
    {
        free(token->value);
    }

    free(token);
}

static lex_cursor_t *create_cursor(char *document, size_t position, lex_status_t status)
{
    lex_cursor_t *cursor;

    cursor = malloc(sizeof(lex_cursor_t));

    if (cursor)
    {
        cursor->document = document;
        cursor->position = position;
        cursor->status = status;
    }

    return cursor;
}

static lex_token_t *create_token(lex_identifier_t identifier, char *value)
{
    lex_token_t *token;

    token = NULL;

    if (value)
    {
        token = malloc(sizeof(lex_token_t));

        if (token)
        {
            token->identifier = identifier;
            token->value = value;
        }
    }

    return token;
}

static lex_token_t *slice_token(lex_cursor_t *cursor, size_t start, size_t end, size_t length, lex_identifier_t identifier)
{
    char *value;

    cursor->status = cursor->position < length ? LEX_STATUS_ROAMING : LEX_STATUS_CLOSED;
    value = slice_string(cursor->document, start, end);

    return create_token(identifier, value);
}

static lex_identifier_t match_identifier(char symbol)
{
    switch (symbol)
    {
        case '~':
            return LEX_IDENTIFIER_COMMENT;
        case '<':
            return LEX_IDENTIFIER_VALUE;
        case '>':
            return LEX_IDENTIFIER_ASSIGN;
        case '+':
            return LEX_IDENTIFIER_ADD;
        case '-':
            return LEX_IDENTIFIER_SUBTRACT;
        case '*':
            return LEX_IDENTIFIER_MULTIPLY;
        case '/':
            return LEX_IDENTIFIER_DIVIDE;
        case '(':
            return LEX_IDENTIFIER_START;
        case ')':
            return LEX_IDENTIFIER_END;
        case ' ':
            return LEX_IDENTIFIER_SPACE;
        case '\t':
            return LEX_IDENTIFIER_TAB;
        case '\n':
            return LEX_IDENTIFIER_NEWLINE;
        case '\r':
            return LEX_IDENTIFIER_NEWLINE;
        case '?':
            return LEX_IDENTIFIER_NULL;
        default:
            return LEX_IDENTIFIER_UNKNOWN;
    }
}
