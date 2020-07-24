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
    PARSER_STATE_ARGUMENTS,
    PARSER_STATE_END
} parser_state_t;

static expression_t *create_expression(error_t error, type_t type, void *segment, expression_t **arguments, size_t length, expression_t *next);
static expression_t *next_expression(scanner_t *scanner, token_t *token, int depth);
static void set_null(expression_t *expression, char *value);
static void set_number(expression_t *expression, char *value);
static void set_string(expression_t *expression, char *value);
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

            if (expression->error != ERROR_UNKNOWN)
            {
                return head;
            }
        }
        else
        {
            destroy_expression(head);
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
    if (expression->segment)
    {
        free(expression->segment);
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

static expression_t *create_expression(error_t error, type_t type, void *segment, expression_t **arguments, size_t length, expression_t *next)
{
    expression_t *expression;

    expression = malloc(sizeof(expression_t));

    if (expression)
    {
        expression->error = error;
        expression->type = type;
        expression->segment = segment;
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
    expression = create_expression(ERROR_UNKNOWN, TYPE_UNSET, NULL, NULL, 0, NULL);

    if (depth > LIMIT_DEPTH)
    {
        state = PARSER_STATE_ERROR;
        expression->error = ERROR_BOUNDS;
    }

    while (!scanner->closed && state != PARSER_STATE_ERROR && state != PARSER_STATE_SUCCESS)
    {
        if (expression->length > LIMIT_BREADTH)
        {
            state = PARSER_STATE_ERROR;
            expression->error = ERROR_BOUNDS;
            break;
        }

        token = token ? token : next_token(scanner);

        if (token)
        {
            if (token->name == TOKEN_NAME_UNKNOWN)
            {
                state = PARSER_STATE_ERROR;
            }
            else if (token->name != TOKEN_NAME_WHITESPACE && token->name != TOKEN_NAME_COMMENT)
            {
                if (state == PARSER_STATE_START)
                {
                    state = PARSER_STATE_SUCCESS;

                    switch (token->name)
                    {
                        case TOKEN_NAME_NULL:
                            set_null(expression, token->value);
                            break;
                        case TOKEN_NAME_NUMBER:
                            set_number(expression, token->value);
                            break;
                        case TOKEN_NAME_STRING:
                            set_string(expression, token->value);
                            break;
                        case TOKEN_NAME_CALL_START:
                            state = PARSER_STATE_ARGUMENTS;
                            expression->type = TYPE_CALL;
                            break;
                        default:
                            expression->error = ERROR_SYNTAX;
                            break;
                    }

                    if (expression->error == ERROR_SHORTAGE)
                    {
                        destroy_token(token);
                        destroy_expression(expression);
                        return NULL;
                    }

                    if (expression->error != ERROR_UNKNOWN)
                    {
                        state = PARSER_STATE_ERROR;
                    }
                }
                else if (state == PARSER_STATE_ARGUMENTS)
                {
                    if (token->name == TOKEN_NAME_CALL_END)
                    {
                        if (expression->length > 0)
                        {
                            state = PARSER_STATE_SUCCESS;
                        }
                        else
                        {
                            expression->error = ERROR_ARGUMENT;
                            state = PARSER_STATE_ERROR;
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

                            expression->error = argument->error;
                            state = expression->error == ERROR_UNKNOWN ? PARSER_STATE_ARGUMENTS : PARSER_STATE_ERROR;

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

    if (state != PARSER_STATE_SUCCESS && state != PARSER_STATE_START)
    {
        if (expression->error == ERROR_UNKNOWN)
        {
            expression->error = ERROR_SYNTAX;
        }
    }

    if (token)
    {
        destroy_token(token);
    }

    return expression;
}

static void set_null(expression_t *expression, char *value)
{
    if (strlen(value) != 1)
    {
        expression->error = ERROR_TYPE;
        return;
    }

    if (value[0] != SYMBOL_NULL)
    {
        expression->error = ERROR_TYPE;
        return;
    }

    expression->type = TYPE_NULL;
    expression->segment = NULL;
}

static void set_number(expression_t *expression, char *value)
{
    size_t length;
    char *trimmed;
    int *numbered;

    length = strlen(value);

    if (length < 2)
    {
        expression->error = ERROR_TYPE;
        return;
    }

    if (value[0] != SYMBOL_NUMBER || value[length - 1] != SYMBOL_NUMBER)
    {
        expression->error = ERROR_TYPE;
        return;
    }

    trimmed = slice_string(value, 1, length - 1);

    if (!trimmed)
    {
        expression->error = ERROR_SHORTAGE;
        return;
    }

    if (!is_integer(trimmed))
    {
        free(trimmed);
        expression->error = ERROR_TYPE;
        return;
    }

    numbered = integer_to_array(atoi(trimmed));
    free(trimmed);

    if (!numbered)
    {
        expression->error = ERROR_SHORTAGE;
        return;
    }

    expression->type = TYPE_NUMBER;
    expression->segment = numbered;
}

static void set_string(expression_t *expression, char *value)
{
    size_t length;
    char *trimmed, *escaped;

    length = strlen(value);

    if (length < 2)
    {
        expression->error = ERROR_TYPE;
        return;
    }

    if (value[0] != SYMBOL_STRING || value[length - 1] != SYMBOL_STRING)
    {
        expression->error = ERROR_TYPE;
        return;
    }

    trimmed = slice_string(value, 1, length - 1);

    if (!trimmed)
    {
        expression->error = ERROR_SHORTAGE;
        return;
    }

    if (!is_printable(trimmed))
    {
        free(trimmed);
        expression->error = ERROR_TYPE;
        return;
    }

    escaped = escape_string(trimmed);
    free(trimmed);

    if (!escaped)
    {
        expression->error = ERROR_SHORTAGE;
        return;
    }

    expression->type = TYPE_STRING;
    expression->segment = escaped;
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
