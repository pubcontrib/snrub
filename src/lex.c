#include <stdlib.h>
#include "lex.h"
#include "common.h"

static scanner_t *create_scanner(string_t *document, size_t start, size_t end, int closed);
static token_t *create_token(token_name_t name, string_t *value);
static token_t *escape_token(scanner_t *scanner, char qualifier, token_name_t name);
static token_t *slice_token(scanner_t *scanner, token_name_t name);
static token_name_t match_name(char symbol);

scanner_t *start_scanner(string_t *document)
{
    return create_scanner(document, 0, 0, document->length <= 0);
}

token_t *next_token(scanner_t *scanner)
{
    char symbol;

    if (scanner->closed)
    {
        return NULL;
    }

    symbol = scanner->document->bytes[scanner->end++];

    switch (symbol)
    {
        case SYMBOL_COMMENT:
            return escape_token(scanner, SYMBOL_COMMENT, TOKEN_NAME_COMMENT);
        case SYMBOL_NUMBER:
            return escape_token(scanner, SYMBOL_NUMBER, TOKEN_NAME_NUMBER);
        case SYMBOL_STRING:
            return escape_token(scanner, SYMBOL_STRING, TOKEN_NAME_STRING);
        default:
            return slice_token(scanner, match_name(symbol));
    }
}

void destroy_scanner(scanner_t *scanner)
{
    if (scanner->document)
    {
        destroy_string(scanner->document);
    }

    free(scanner);
}

void destroy_token(token_t *token)
{
    if (token->value)
    {
        destroy_string(token->value);
    }

    free(token);
}

static scanner_t *create_scanner(string_t *document, size_t start, size_t end, int closed)
{
    scanner_t *scanner;

    scanner = allocate(sizeof(scanner_t));
    scanner->document = document;
    scanner->start = start;
    scanner->end = end;
    scanner->closed = closed;

    return scanner;
}

static token_t *create_token(token_name_t name, string_t *value)
{
    token_t *token;

    token = allocate(sizeof(token_t));
    token->name = name;
    token->value = value;

    return token;
}

static token_t *escape_token(scanner_t *scanner, char qualifier, token_name_t name)
{
    int escaping;

    escaping = 0;

    while (scanner->end < scanner->document->length)
    {
        char symbol;

        symbol = scanner->document->bytes[scanner->end++];

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
    string_t *value;

    value = slice_string(scanner->document, scanner->start, scanner->end);
    scanner->closed = scanner->end >= scanner->document->length;
    scanner->start = scanner->end;

    return create_token(name, value);
}

static token_name_t match_name(char symbol)
{
    switch (symbol)
    {
        case SYMBOL_NULL:
            return TOKEN_NAME_NULL;
        case '[':
            return TOKEN_NAME_LIST_START;
        case ']':
            return TOKEN_NAME_LIST_END;
        case '{':
            return TOKEN_NAME_MAP_START;
        case '}':
            return TOKEN_NAME_MAP_END;
        case '(':
            return TOKEN_NAME_CALL_START;
        case ')':
            return TOKEN_NAME_CALL_END;
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return TOKEN_NAME_WHITESPACE;
        default:
            return TOKEN_NAME_UNKNOWN;
    }
}
