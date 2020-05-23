#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse.h"
#include "lex.h"
#include "common.h"

static parse_link_t *create_link(parse_expression_t *expression, parse_link_t *next);
static parse_expression_t *create_expression(parse_error_t error, parse_value_t *value, parse_expression_t **arguments, size_t length);
static parse_value_t *create_value(parse_type_t type, void *unsafe);
static parse_expression_t *next_expression(lex_cursor_t *cursor, lex_token_t *token, int depth);
static parse_value_t *token_to_value(lex_token_t *token);
static parse_value_t *null_to_value(char *value);
static parse_value_t *number_to_value(char *value);
static parse_value_t *string_to_value(char *value);
static int is_value(lex_identifier_t identifier);
static char *escape(char *value);
static char is_printable(char *value);

parse_link_t *parse_list_document(char *document)
{
    lex_cursor_t *cursor;
    parse_link_t *head, *tail;

    cursor = lex_iterate_document(document);

    if (!cursor)
    {
        return NULL;
    }

    head = NULL;
    tail = NULL;

    do
    {
        parse_expression_t *expression;

        expression = next_expression(cursor, NULL, 1);

        if (expression)
        {
            if (head)
            {
                tail->next = create_link(expression, NULL);

                if (!tail->next)
                {
                    parse_destroy_link(head);
                    lex_destroy_cursor(cursor);
                    return NULL;
                }

                tail = tail->next;
            }
            else
            {
                head = create_link(expression, NULL);

                if (!head)
                {
                    lex_destroy_cursor(cursor);
                    return NULL;
                }

                tail = head;
            }

            if (tail->expression->error != PARSE_ERROR_UNKNOWN)
            {
                lex_destroy_cursor(cursor);
                return head;
            }
        }
        else
        {
            parse_destroy_link(head);
            lex_destroy_cursor(cursor);
            return NULL;
        }
    } while (cursor->status != LEX_STATUS_CLOSED);

    lex_destroy_cursor(cursor);

    return head;
}

void parse_destroy_link(parse_link_t *link)
{
    if (link->next)
    {
        parse_destroy_link(link->next);
    }

    if (link->expression)
    {
        parse_destroy_expression(link->expression);
    }

    free(link);
}

void parse_destroy_expression(parse_expression_t *expression)
{
    if (expression->value)
    {
        parse_destroy_value(expression->value);
    }

    if (expression->length > 0)
    {
        size_t index;

        for (index = 0; index < expression->length; index++)
        {
            parse_destroy_expression(expression->arguments[index]);
        }

        free(expression->arguments);
    }

    free(expression);
}

void parse_destroy_value(parse_value_t *value)
{
    if (value->unsafe)
    {
        free(value->unsafe);
    }

    free(value);
}

static parse_link_t *create_link(parse_expression_t *expression, parse_link_t *next)
{
    parse_link_t *link;

    link = malloc(sizeof(parse_link_t));

    if (link)
    {
        link->expression = expression;
        link->next = next;
    }

    return link;
}

static parse_expression_t *create_expression(parse_error_t error, parse_value_t *value, parse_expression_t **arguments, size_t length)
{
    parse_expression_t *expression;

    expression = malloc(sizeof(parse_expression_t));

    if (expression)
    {
        expression->error = error;
        expression->value = value;
        expression->arguments = arguments;
        expression->length = length;
    }

    return expression;
}

static parse_value_t *create_value(parse_type_t type, void *unsafe)
{
    parse_value_t *value;

    value = malloc(sizeof(parse_value_t));

    if (value)
    {
        value->type = type;
        value->unsafe = unsafe;
    }

    return value;
}

static parse_expression_t *next_expression(lex_cursor_t *cursor, lex_token_t *token, int depth)
{
    parse_status_t status;
    parse_expression_t *expression;

    status = PARSE_STATUS_START;
    expression = create_expression(PARSE_ERROR_UNKNOWN, NULL, NULL, 0);

    if (depth > 32)
    {
        status = PARSE_STATUS_ERROR;
        expression->error = PARSE_ERROR_DEPTH;
    }

    while (cursor->status != LEX_STATUS_CLOSED && status != PARSE_STATUS_ERROR && status != PARSE_STATUS_SUCCESS)
    {
        if (expression->length > 32)
        {
            status = PARSE_STATUS_ERROR;
            expression->error = PARSE_ERROR_DEPTH;
            break;
        }

        token = token ? token : lex_next_token(cursor);

        if (token)
        {
            if (token->identifier == LEX_IDENTIFIER_UNKNOWN)
            {
                status = PARSE_STATUS_ERROR;
            }
            else if (token->identifier != LEX_IDENTIFIER_WHITESPACE)
            {
                if (status == PARSE_STATUS_START)
                {
                    if (is_value(token->identifier))
                    {
                        expression->value = token_to_value(token);

                        if (expression->value)
                        {
                            status = expression->value->type == PARSE_TYPE_UNKNOWN ? PARSE_STATUS_ERROR : PARSE_STATUS_SUCCESS;
                            expression->error = status == PARSE_STATUS_ERROR ? PARSE_ERROR_TYPE : PARSE_ERROR_UNKNOWN;
                        }
                        else
                        {
                            lex_destroy_token(token);
                            parse_destroy_expression(expression);
                            return NULL;
                        }
                    }
                    else
                    {
                        status = token->identifier == LEX_IDENTIFIER_START ? PARSE_STATUS_ARGUMENTS : PARSE_STATUS_ERROR;
                    }
                }
                else if (status == PARSE_STATUS_ARGUMENTS)
                {
                    if (token->identifier == LEX_IDENTIFIER_END)
                    {
                        if (expression->length > 0)
                        {
                            status = PARSE_STATUS_SUCCESS;
                        }
                        else
                        {
                            expression->error = PARSE_ERROR_ARGUMENT;
                            status = PARSE_STATUS_ERROR;
                        }
                    }
                    else
                    {
                        parse_expression_t *argument;

                        argument = next_expression(cursor, token, depth + 1);
                        token = NULL;

                        if (argument)
                        {
                            parse_expression_t **existing;

                            expression->error = argument->error;
                            status = expression->error == PARSE_ERROR_UNKNOWN ? PARSE_STATUS_ARGUMENTS : PARSE_STATUS_ERROR;

                            existing = expression->arguments;
                            expression->length += 1;
                            expression->arguments = malloc(sizeof(parse_expression_t *) * expression->length);

                            if (expression->length > 1)
                            {
                                size_t index;

                                for (index = 0; index < expression->length - 1; index++)
                                {
                                    expression->arguments[index] = existing[index];
                                }

                                free(existing);
                            }

                            expression->arguments[expression->length - 1] = argument;
                        }
                        else
                        {
                            parse_destroy_expression(expression);
                            return NULL;
                        }
                    }
                }
            }

            if (token)
            {
                lex_destroy_token(token);
                token = NULL;
            }
        }
        else
        {
            parse_destroy_expression(expression);
            return NULL;
        }
    }

    if (status != PARSE_STATUS_SUCCESS && status != PARSE_STATUS_START)
    {
        if (expression->error == PARSE_ERROR_UNKNOWN)
        {
            expression->error = PARSE_ERROR_SYNTAX;
        }
    }

    if (token)
    {
        lex_destroy_token(token);
    }

    return expression;
}

static parse_value_t *token_to_value(lex_token_t *token)
{
    switch (token->identifier)
    {
        case LEX_IDENTIFIER_NULL:
            return null_to_value(token->value);
        case LEX_IDENTIFIER_NUMBER:
            return number_to_value(token->value);
        case LEX_IDENTIFIER_STRING:
            return string_to_value(token->value);
        default:
            return NULL;
    }
}

static parse_value_t *null_to_value(char *value)
{
    return strcmp(value, "?") ? create_value(PARSE_TYPE_UNKNOWN, NULL) : create_value(PARSE_TYPE_NULL, NULL);
}

static parse_value_t *number_to_value(char *value)
{
    size_t length, start, end, index;
    char *trimmed;
    int *unsafe;

    length = strlen(value);

    if (length < 2)
    {
        return create_value(PARSE_TYPE_UNKNOWN, NULL);
    }

    if (value[0] != lex_number_symbol() || value[length - 1] != lex_number_symbol())
    {
        return create_value(PARSE_TYPE_UNKNOWN, NULL);
    }

    start = value[1] == '-' ? 2 : 1;
    end = length - 1;

    if (start == end && value[1] == '-')
    {
        return create_value(PARSE_TYPE_UNKNOWN, NULL);
    }

    for (index = start; index < end; index++)
    {
        if (!isdigit(value[index]))
        {
            return create_value(PARSE_TYPE_UNKNOWN, NULL);
        }
    }

    trimmed = slice_string(value, 1, end);

    if (!trimmed)
    {
        return NULL;
    }

    unsafe = integer_to_array(atoi(trimmed));
    free(trimmed);

    if (!unsafe)
    {
        return NULL;
    }

    return create_value(PARSE_TYPE_NUMBER, unsafe);
}

static parse_value_t *string_to_value(char *value)
{
    size_t length;
    char *trimmed, *unsafe;

    if (!is_printable(value))
    {
        return create_value(PARSE_TYPE_UNKNOWN, NULL);
    }

    length = strlen(value);

    if (length < 2)
    {
        return create_value(PARSE_TYPE_UNKNOWN, NULL);
    }

    if (value[0] != lex_string_symbol() || value[length - 1] != lex_string_symbol())
    {
        return create_value(PARSE_TYPE_UNKNOWN, NULL);
    }

    trimmed = slice_string(value, 1, length - 1);

    if (!trimmed)
    {
        return NULL;
    }

    unsafe = escape(trimmed);
    free(trimmed);

    if (!unsafe)
    {
        return NULL;
    }

    return create_value(PARSE_TYPE_STRING, unsafe);
}

static int is_value(lex_identifier_t identifier)
{
    switch (identifier)
    {
        case LEX_IDENTIFIER_NULL:
        case LEX_IDENTIFIER_NUMBER:
        case LEX_IDENTIFIER_STRING:
            return 1;
        default:
            return 0;
    }
}

static char *escape(char *value)
{
    size_t length, scan, fill;
    char *loose, *tight;
    int escaping;

    length = strlen(value);
    loose = malloc(sizeof(char) * (length + 1));

    if (!loose)
    {
        return NULL;
    }

    escaping = 0;
    fill = 0;

    for (scan = 0; scan < length; scan++)
    {
        char current;

        current = value[scan];

        if (escaping)
        {
            switch (current)
            {
                case '\\':
                    loose[fill++] = '\\';
                    break;
                case '"':
                    loose[fill++] = '"';
                    break;
                case 't':
                    loose[fill++] = '\t';
                    break;
                case 'n':
                    loose[fill++] = '\n';
                    break;
                case 'r':
                    loose[fill++] = '\r';
                    break;
            }

            escaping = 0;
        }
        else
        {
            if (current == '\\')
            {
                escaping = 1;
            }
            else
            {
                loose[fill++] = current;
            }
        }
    }

    loose[fill] = '\0';

    tight = copy_string(loose);
    free(loose);

    return tight;
}

static char is_printable(char *value)
{
    size_t index;

    for (index = 0; index < strlen(value); index++)
    {
        char symbol;

        symbol = value[index];

        if (!isprint(symbol))
        {
            return 0;
        }
    }

    return 1;
}
