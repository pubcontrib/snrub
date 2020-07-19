#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "common.h"

static scanner_t *create_scanner(char *document, size_t position, size_t length, int escaping, scanner_state_t state);
static token_t *create_token(token_name_t name, char *value);
static token_t *slice_token(scanner_t *scanner, size_t start, size_t end, token_name_t name);
static token_name_t match_name(char symbol);

scanner_t *start_scanner(char *document)
{
    size_t length;
    scanner_state_t state;

    length = strlen(document);
    state = length ? SCANNER_STATE_ROAMING : SCANNER_STATE_CLOSED;

    return create_scanner(document, 0, length, 0, state);
}

token_t *next_token(scanner_t *scanner)
{
    size_t start, end;

    start = 0;
    end = 0;

    while (scanner->position < scanner->length)
    {
        char symbol;

        symbol = scanner->document[scanner->position];
        scanner->position += 1;
        end = scanner->position;

        if (scanner->state == SCANNER_STATE_ROAMING)
        {
            if (symbol == SYMBOL_COMMENT)
            {
                start = scanner->position - 1;
                scanner->state = SCANNER_STATE_COMMENT;
            }
            else if (symbol == SYMBOL_NUMBER)
            {
                start = scanner->position - 1;
                scanner->state = SCANNER_STATE_NUMBER;
            }
            else if (symbol == SYMBOL_STRING)
            {
                start = scanner->position - 1;
                scanner->state = SCANNER_STATE_STRING;
            }
            else
            {
                token_name_t name;

                name = match_name(symbol);

                return slice_token(scanner, scanner->position - 1, scanner->position, name);
            }
        }
        else if (scanner->state == SCANNER_STATE_COMMENT)
        {
            if (symbol == SYMBOL_COMMENT)
            {
                if (scanner->escaping)
                {
                    scanner->escaping = 0;
                }
                else
                {
                    return slice_token(scanner, start, end, TOKEN_NAME_COMMENT);
                }
            }
            else if (symbol == SYMBOL_ESCAPE)
            {
                scanner->escaping = scanner->escaping ? 0 : 1;
            }
            else
            {
                if (scanner->escaping)
                {
                    scanner->escaping = 0;
                }
            }
        }
        else if (scanner->state == SCANNER_STATE_NUMBER)
        {
            if (symbol == SYMBOL_NUMBER)
            {
                return slice_token(scanner, start, end, TOKEN_NAME_NUMBER);
            }
        }
        else if (scanner->state == SCANNER_STATE_STRING)
        {
            if (symbol == SYMBOL_STRING)
            {
                if (scanner->escaping)
                {
                    scanner->escaping = 0;
                }
                else
                {
                    return slice_token(scanner, start, end, TOKEN_NAME_STRING);
                }
            }
            else if (symbol == SYMBOL_ESCAPE)
            {
                scanner->escaping = scanner->escaping ? 0 : 1;
            }
            else
            {
                if (scanner->escaping)
                {
                    scanner->escaping = 0;
                }
            }
        }
    }

    if (scanner->state == SCANNER_STATE_COMMENT || scanner->state == SCANNER_STATE_NUMBER || scanner->state == SCANNER_STATE_STRING)
    {
        return slice_token(scanner, start, end, TOKEN_NAME_UNKNOWN);
    }
    else
    {
        scanner->state = SCANNER_STATE_CLOSED;

        return NULL;
    }
}

void destroy_scanner(scanner_t *scanner)
{
    if (scanner->document)
    {
        free(scanner->document);
    }

    free(scanner);
}

void destroy_token(token_t *token)
{
    if (token->value)
    {
        free(token->value);
    }

    free(token);
}

static scanner_t *create_scanner(char *document, size_t position, size_t length, int escaping, scanner_state_t state)
{
    scanner_t *scanner;

    scanner = malloc(sizeof(scanner_t));

    if (scanner)
    {
        scanner->document = document;
        scanner->position = position;
        scanner->length = length;
        scanner->escaping = escaping;
        scanner->state = state;
    }

    return scanner;
}

static token_t *create_token(token_name_t name, char *value)
{
    token_t *token;

    token = malloc(sizeof(token_t));

    if (token)
    {
        token->name = name;
        token->value = value;
    }

    return token;
}

static token_t *slice_token(scanner_t *scanner, size_t start, size_t end, token_name_t name)
{
    char *value;

    scanner->state = scanner->position < scanner->length ? SCANNER_STATE_ROAMING : SCANNER_STATE_CLOSED;
    value = slice_string(scanner->document, start, end);

    if (!value)
    {
        return NULL;
    }

    return create_token(name, value);
}

static token_name_t match_name(char symbol)
{
    if (symbol == SYMBOL_NULL)
    {
        return TOKEN_NAME_NULL;
    }

    switch (symbol)
    {
        case '(':
            return TOKEN_NAME_START;
        case ')':
            return TOKEN_NAME_END;
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return TOKEN_NAME_WHITESPACE;
        default:
            return TOKEN_NAME_UNKNOWN;
    }
}
