#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse.h"
#include "lex.h"
#include "value.h"
#include "list.h"
#include "common.h"

typedef enum
{
    PARSER_STATE_SUCCESS,
    PARSER_STATE_ERROR,
    PARSER_STATE_START,
    PARSER_STATE_ARGUMENTS
} parser_state_t;

static void destroy_expression_unsafe(void *expression);
static expression_t *create_expression(value_t *value, list_t *arguments);
static expression_t *next_expression(scanner_t *scanner, token_t *token, int depth);
static value_t *parse_null_literal(char *value);
static value_t *parse_number_literal(char *value);
static value_t *parse_string_literal(char *value);
static int is_printable(char *value);

list_t *parse_expressions(scanner_t *scanner)
{
    list_t *expressions;

    expressions = empty_list(destroy_expression_unsafe);

    do
    {
        expression_t *expression;

        expression = next_expression(scanner, NULL, 0);
        add_list_item(expressions, expression);

        if (expression->value->thrown)
        {
            return expressions;
        }
    } while (!scanner->closed);

    return expressions;
}

void destroy_expression(expression_t *expression)
{
    if (expression->value)
    {
        destroy_value(expression->value);
    }

    if (expression->arguments)
    {
        destroy_list(expression->arguments);
    }

    free(expression);
}

static void destroy_expression_unsafe(void *expression)
{
    destroy_expression((expression_t *) expression);
}

static expression_t *create_expression(value_t *value, list_t *arguments)
{
    expression_t *expression;

    expression = allocate(sizeof(expression_t));
    expression->value = value;
    expression->arguments = arguments;

    return expression;
}

static expression_t *next_expression(scanner_t *scanner, token_t *token, int depth)
{
    parser_state_t state;
    list_t *arguments;
    expression_t *expression;

    state = PARSER_STATE_START;
    arguments = empty_list(destroy_expression_unsafe);
    expression = create_expression(NULL, arguments);

    if (depth > LIMIT_DEPTH || scanner->length > NUMBER_MAX)
    {
        state = PARSER_STATE_ERROR;
        expression->value = throw_error(ERROR_BOUNDS);
    }

    while (!scanner->closed && state != PARSER_STATE_ERROR && state != PARSER_STATE_SUCCESS)
    {
        token = token ? token : next_token(scanner);

        if (token)
        {
            if (token->name == TOKEN_NAME_UNKNOWN)
            {
                state = PARSER_STATE_ERROR;
                expression->value = throw_error(ERROR_SYNTAX);
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
                        case TOKEN_NAME_MAP_START:
                            state = PARSER_STATE_ARGUMENTS;
                            expression->value = new_map(NULL);
                            break;
                        case TOKEN_NAME_CALL_START:
                            state = PARSER_STATE_ARGUMENTS;
                            expression->value = new_call();
                            break;
                        default:
                            expression->value = throw_error(ERROR_SYNTAX);
                            break;
                    }

                    if (expression->value->thrown)
                    {
                        state = PARSER_STATE_ERROR;
                    }
                }
                else if (state == PARSER_STATE_ARGUMENTS)
                {
                    if (expression->value->type == VALUE_TYPE_LIST && token->name == TOKEN_NAME_LIST_END)
                    {
                        state = PARSER_STATE_SUCCESS;
                    }
                    else if (expression->value->type == VALUE_TYPE_MAP && token->name == TOKEN_NAME_MAP_END)
                    {
                        state = PARSER_STATE_SUCCESS;
                    }
                    else if (expression->value->type == VALUE_TYPE_CALL && token->name == TOKEN_NAME_CALL_END)
                    {
                        if (arguments->length > 0)
                        {
                            state = PARSER_STATE_SUCCESS;
                        }
                        else
                        {
                            destroy_value(expression->value);

                            state = PARSER_STATE_ERROR;
                            expression->value = throw_error(ERROR_ARGUMENT);
                        }
                    }
                    else
                    {
                        expression_t *argument;

                        argument = next_expression(scanner, token, depth + 1);
                        token = NULL;

                        if (argument->value->thrown)
                        {
                            destroy_value(expression->value);

                            state = PARSER_STATE_ERROR;
                            expression->value = copy_value(argument->value);
                        }

                        add_list_item(arguments, argument);
                    }
                }
            }

            if (token)
            {
                destroy_token(token);
                token = NULL;
            }
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

        expression->value = throw_error(ERROR_SYNTAX);
    }

    if (token)
    {
        destroy_token(token);
    }

    return expression;
}

static value_t *parse_null_literal(char *value)
{
    if (strlen(value) != 1)
    {
        return throw_error(ERROR_TYPE);
    }

    if (value[0] != SYMBOL_NULL)
    {
        return throw_error(ERROR_TYPE);
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
        return throw_error(ERROR_TYPE);
    }

    if (value[0] != SYMBOL_NUMBER || value[length - 1] != SYMBOL_NUMBER)
    {
        return throw_error(ERROR_TYPE);
    }

    trimmed = slice_string(value, 1, length - 1);

    if (!string_to_integer(trimmed, NUMBER_DIGIT_CAPACITY, &numbered))
    {
        free(trimmed);
        return throw_error(ERROR_TYPE);
    }

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
        return throw_error(ERROR_TYPE);
    }

    if (value[0] != SYMBOL_STRING || value[length - 1] != SYMBOL_STRING)
    {
        return throw_error(ERROR_TYPE);
    }

    trimmed = slice_string(value, 1, length - 1);

    if (!is_printable(trimmed))
    {
        free(trimmed);
        return throw_error(ERROR_TYPE);
    }

    escaped = escape_string(trimmed);
    free(trimmed);

    return new_string(escaped);
}

static int is_printable(char *value)
{
    size_t length, index;

    length = strlen(value);

    for (index = 0; index < length; index++)
    {
        unsigned char symbol;

        symbol = value[index];

        if (!isprint(symbol))
        {
            return 0;
        }
    }

    return 1;
}
