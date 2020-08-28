#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse.h"
#include "lex.h"
#include "common.h"

static const int LIMIT_DEPTH = 32;
static const int LIMIT_BREADTH = 1024;

typedef enum
{
    PARSER_STATE_SUCCESS,
    PARSER_STATE_ERROR,
    PARSER_STATE_START,
    PARSER_STATE_ARGUMENTS
} parser_state_t;

static expression_t *create_expression(value_t *value, expression_t **arguments, size_t length, expression_t *next);
static expression_t *next_expression(scanner_t *scanner, token_t *token, int depth);
static value_t *parse_null_literal(char *value);
static value_t *parse_number_literal(char *value);
static value_t *parse_string_literal(char *value);
static int is_printable(char *value);

expression_t *parse_expressions(scanner_t *scanner)
{
    expression_t *head, *tail;

    head = NULL;
    tail = NULL;

    do
    {
        expression_t *expression;

        expression = next_expression(scanner, NULL, 0);

        if (expression)
        {
            if (head)
            {
                tail->next = expression;
                tail = tail->next;
            }
            else
            {
                head = expression;
                tail = head;
            }

            if (expression->value->type == TYPE_ERROR)
            {
                return head;
            }
        }
        else
        {
            if (head)
            {
                destroy_expression(head);
            }

            return NULL;
        }
    } while (!scanner->closed);

    return head;
}

char *escape_string(char *string)
{
    char *escape;
    size_t length;

    length = strlen(string);
    escape = malloc(sizeof(char) * (length + 1));

    if (escape)
    {
        size_t left, right;
        int escaping;

        escaping = 0;
        right = 0;

        for (left = 0; left < length; left++)
        {
            char current;

            current = string[left];

            if (escaping)
            {
                switch (current)
                {
                    case '\\':
                        escape[right++] = '\\';
                        break;
                    case '"':
                        escape[right++] = '"';
                        break;
                    case 't':
                        escape[right++] = '\t';
                        break;
                    case 'n':
                        escape[right++] = '\n';
                        break;
                    case 'r':
                        escape[right++] = '\r';
                        break;
                }

                escaping = 0;
            }
            else
            {
                if (current == SYMBOL_ESCAPE)
                {
                    escaping = 1;
                }
                else
                {
                    escape[right++] = current;
                }
            }
        }

        escape[right] = '\0';
    }

    return escape;
}

char *unescape_string(char *string)
{
    char *unescape;
    size_t length;

    length = strlen(string)
        + characters_in_string(string, '\\')
        + characters_in_string(string, '"')
        + characters_in_string(string, '\t')
        + characters_in_string(string, '\n')
        + characters_in_string(string, '\r');
    unescape = malloc(sizeof(char) * (length + 1));

    if (unescape)
    {
        size_t left, right;

        for (left = 0, right = 0; left < length; right++)
        {
            char symbol;

            symbol = string[right];

            if (symbol == '\\')
            {
                unescape[left++] = SYMBOL_ESCAPE;
                unescape[left++] = '\\';
            }
            else if (symbol == '"')
            {
                unescape[left++] = SYMBOL_ESCAPE;
                unescape[left++] = '"';
            }
            else if (symbol == '\t')
            {
                unescape[left++] = SYMBOL_ESCAPE;
                unescape[left++] = 't';
            }
            else if (symbol == '\n')
            {
                unescape[left++] = SYMBOL_ESCAPE;
                unescape[left++] = 'n';
            }
            else if (symbol == '\r')
            {
                unescape[left++] = SYMBOL_ESCAPE;
                unescape[left++] = 'r';
            }
            else
            {
                unescape[left++] = symbol;
            }
        }

        unescape[length] = '\0';
    }

    return unescape;
}

void destroy_expression(expression_t *expression)
{
    if (expression->value)
    {
        destroy_value(expression->value);
    }

    if (expression->length > 0)
    {
        size_t index;

        for (index = 0; index < expression->length; index++)
        {
            destroy_expression(expression->arguments[index]);
        }

        free(expression->arguments);
    }

    if (expression->next)
    {
        destroy_expression(expression->next);
    }

    free(expression);
}

static expression_t *create_expression(value_t *value, expression_t **arguments, size_t length, expression_t *next)
{
    expression_t *expression;

    expression = malloc(sizeof(expression_t));

    if (expression)
    {
        expression->value = value;
        expression->arguments = arguments;
        expression->length = length;
        expression->next = next;
    }

    return expression;
}

static expression_t *next_expression(scanner_t *scanner, token_t *token, int depth)
{
    parser_state_t state;
    expression_t *expression;

    state = PARSER_STATE_START;
    expression = create_expression(NULL, NULL, 0, NULL);

    if (!expression)
    {
        return NULL;
    }

    if (depth > LIMIT_DEPTH)
    {
        state = PARSER_STATE_ERROR;
        expression->value = new_error(ERROR_BOUNDS);
    }

    while (!scanner->closed && state != PARSER_STATE_ERROR && state != PARSER_STATE_SUCCESS)
    {
        if (expression->length > LIMIT_BREADTH)
        {
            destroy_value(expression->value);

            state = PARSER_STATE_ERROR;
            expression->value = new_error(ERROR_BOUNDS);
            break;
        }

        token = token ? token : next_token(scanner);

        if (token)
        {
            if (token->name == TOKEN_NAME_UNKNOWN)
            {
                state = PARSER_STATE_ERROR;
                expression->value = new_error(ERROR_SYNTAX);
            }
            else if (token->name != TOKEN_NAME_WHITESPACE && token->name != TOKEN_NAME_COMMENT)
            {
                if (state == PARSER_STATE_START)
                {
                    state = PARSER_STATE_SUCCESS;

                    switch (token->name)
                    {
                        case TOKEN_NAME_NULL:
                            expression->value = parse_null_literal(token->value);
                            break;
                        case TOKEN_NAME_NUMBER:
                            expression->value = parse_number_literal(token->value);
                            break;
                        case TOKEN_NAME_STRING:
                            expression->value = parse_string_literal(token->value);
                            break;
                        case TOKEN_NAME_LIST_START:
                            state = PARSER_STATE_ARGUMENTS;
                            expression->value = new_list(NULL, 0);
                            break;
                        case TOKEN_NAME_CALL_START:
                            state = PARSER_STATE_ARGUMENTS;
                            expression->value = new_call();
                            break;
                        default:
                            expression->value = new_error(ERROR_SYNTAX);
                            break;
                    }

                    if (!expression->value)
                    {
                        destroy_token(token);
                        destroy_expression(expression);
                        return NULL;
                    }

                    if (expression->value->type == TYPE_ERROR)
                    {
                        state = PARSER_STATE_ERROR;
                    }
                }
                else if (state == PARSER_STATE_ARGUMENTS)
                {
                    if (expression->value->type == TYPE_LIST && token->name == TOKEN_NAME_LIST_END)
                    {
                        state = PARSER_STATE_SUCCESS;
                    }
                    else if (expression->value->type == TYPE_CALL && token->name == TOKEN_NAME_CALL_END)
                    {
                        if (expression->length > 0)
                        {
                            state = PARSER_STATE_SUCCESS;
                        }
                        else
                        {
                            destroy_value(expression->value);

                            state = PARSER_STATE_ERROR;
                            expression->value = new_error(ERROR_ARGUMENT);
                        }
                    }
                    else
                    {
                        expression_t *argument;

                        argument = next_expression(scanner, token, depth + 1);
                        token = NULL;

                        if (argument)
                        {
                            expression_t **existing;

                            if (argument->value->type == TYPE_ERROR)
                            {
                                destroy_value(expression->value);

                                state = PARSER_STATE_ERROR;
                                expression->value = copy_value(argument->value);
                            }

                            existing = expression->arguments;
                            expression->length += 1;
                            expression->arguments = malloc(sizeof(expression_t *) * expression->length);

                            if (expression->arguments)
                            {
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
                                destroy_expression(argument);
                                destroy_expression(expression);
                                return NULL;
                            }
                        }
                        else
                        {
                            destroy_expression(expression);
                            return NULL;
                        }
                    }
                }
            }

            if (token)
            {
                destroy_token(token);
                token = NULL;
            }
        }
        else
        {
            destroy_expression(expression);
            return NULL;
        }
    }

    if (state == PARSER_STATE_SUCCESS || state == PARSER_STATE_START)
    {
        if (!expression->value)
        {
            expression->value = new_unset();
        }
    }
    else if (state == PARSER_STATE_ARGUMENTS)
    {
        if (expression->value)
        {
            destroy_value(expression->value);
        }

        expression->value = new_error(ERROR_SYNTAX);
    }

    if (token)
    {
        destroy_token(token);
    }

    if (!expression->value)
    {
        destroy_expression(expression);
        return NULL;
    }

    return expression;
}

static value_t *parse_null_literal(char *value)
{
    if (strlen(value) != 1)
    {
        return new_error(ERROR_TYPE);
    }

    if (value[0] != SYMBOL_NULL)
    {
        return new_error(ERROR_TYPE);
    }

    return new_null();
}

static value_t *parse_number_literal(char *value)
{
    size_t length;
    char *trimmed;
    int numbered;

    length = strlen(value);

    if (length < 2)
    {
        return new_error(ERROR_TYPE);
    }

    if (value[0] != SYMBOL_NUMBER || value[length - 1] != SYMBOL_NUMBER)
    {
        return new_error(ERROR_TYPE);
    }

    trimmed = slice_string(value, 1, length - 1);

    if (!trimmed)
    {
        return NULL;
    }

    if (!is_integer(trimmed))
    {
        free(trimmed);
        return new_error(ERROR_TYPE);
    }

    numbered = atoi(trimmed);
    free(trimmed);

    return new_number(numbered);
}

static value_t *parse_string_literal(char *value)
{
    size_t length;
    char *trimmed, *escaped;

    length = strlen(value);

    if (length < 2)
    {
        return new_error(ERROR_TYPE);
    }

    if (value[0] != SYMBOL_STRING || value[length - 1] != SYMBOL_STRING)
    {
        return new_error(ERROR_TYPE);
    }

    trimmed = slice_string(value, 1, length - 1);

    if (!trimmed)
    {
        return NULL;
    }

    if (!is_printable(trimmed))
    {
        free(trimmed);
        return new_error(ERROR_TYPE);
    }

    escaped = escape_string(trimmed);
    free(trimmed);

    if (!escaped)
    {
        return NULL;
    }

    return steal_string(escaped, sizeof(char) * (strlen(escaped) + 1));
}

static int is_printable(char *value)
{
    size_t length, index;

    length = strlen(value);

    for (index = 0; index < length; index++)
    {
        char symbol;

        symbol = value[index];

        if (!isprint((unsigned char) symbol))
        {
            return 0;
        }
    }

    return 1;
}
