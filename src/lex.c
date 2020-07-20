#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "common.h"

static scanner_t *create_scanner(char *document, size_t start, size_t end, size_t length, int closed);
static token_t *create_token(token_name_t name, char *value);
static token_t *escape_token(scanner_t *scanner, char qualifier, token_name_t name);
static token_t *slice_token(scanner_t *scanner, token_name_t name);
static token_name_t match_name(char symbol);

scanner_t *start_scanner(char *document)
{
    size_t length;
    int closed;

    length = strlen(document);
    closed = length <= 0;

    return create_scanner(document, 0, 0, length, closed);
}

token_t *next_token(scanner_t *scanner)
{
    char symbol;

    if (scanner->closed)
    {
        return NULL;
    }

    symbol = scanner->document[scanner->end];
    scanner->end += 1;

    if (symbol == SYMBOL_COMMENT)
    {
        return escape_token(scanner, SYMBOL_COMMENT, TOKEN_NAME_COMMENT);
    }
    else if (symbol == SYMBOL_NUMBER)
    {
        return escape_token(scanner, SYMBOL_NUMBER, TOKEN_NAME_NUMBER);
    }
    else if (symbol == SYMBOL_STRING)
    {
        return escape_token(scanner, SYMBOL_STRING, TOKEN_NAME_STRING);
    }
    else
    {
        token_name_t name;

        name = match_name(symbol);

        return slice_token(scanner, name);
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

static scanner_t *create_scanner(char *document, size_t start, size_t end, size_t length, int closed)
{
    scanner_t *scanner;

    scanner = malloc(sizeof(scanner_t));

    if (scanner)
    {
        scanner->document = document;
        scanner->start = start;
        scanner->end = end;
        scanner->length = length;
        scanner->closed = closed;
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

static token_t *escape_token(scanner_t *scanner, char qualifier, token_name_t name)
{
    int escaping;

    while (scanner->end < scanner->length)
    {
        char symbol;

        symbol = scanner->document[scanner->end];
        scanner->end += 1;

        if (symbol == qualifier)
        {
            if (escaping)
            {
                escaping = 0;
            }
            else
            {
                return slice_token(scanner, name);
            }
        }
        else if (symbol == SYMBOL_ESCAPE)
        {
            escaping = escaping ? 0 : 1;
        }
        else
        {
            if (escaping)
            {
                escaping = 0;
            }
        }
    }

    return slice_token(scanner, TOKEN_NAME_UNKNOWN);
}

static token_t *slice_token(scanner_t *scanner, token_name_t name)
{
    char *value;

    value = slice_string(scanner->document, scanner->start, scanner->end);

    if (!value)
    {
        return NULL;
    }

    scanner->closed = scanner->end >= scanner->length;
    scanner->start = scanner->end;

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
