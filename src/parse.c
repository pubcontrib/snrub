#include <stdlib.h>
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
static expression_t *create_expression(expression_type_t type, value_t *value, list_t *arguments);
static expression_t *next_expression(scanner_t *scanner, token_t *token, int depth);
static value_t *parse_null_literal(string_t *value);
static value_t *parse_number_literal(string_t *value);
static value_t *parse_string_literal(string_t *value);
static int is_printable(string_t *value);

list_t *parse_expressions(scanner_t *scanner)
{
    list_t *expressions;

    expressions = empty_list(destroy_expression_unsafe);

    do
    {
        expression_t *expression;

        expression = next_expression(scanner, NULL, 0);
        add_list_item(expressions, expression);

        if (expression->type == EXPRESSION_TYPE_VALUE && expression->value->thrown)
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

static expression_t *create_expression(expression_type_t type, value_t *value, list_t *arguments)
{
    expression_t *expression;

    expression = allocate(sizeof(expression_t));
    expression->type = type;
    expression->value = value;
    expression->arguments = arguments;

    return expression;
}

static expression_t *next_expression(scanner_t *scanner, token_t *token, int depth)
{
    parser_state_t state;
    expression_t *expression;

    state = PARSER_STATE_START;
    expression = NULL;

    if (depth > LIMIT_DEPTH || scanner->document->length > NUMBER_MAX)
    {
        state = PARSER_STATE_ERROR;
        expression = create_expression(EXPRESSION_TYPE_VALUE, throw_error(ERROR_BOUNDS), NULL);
    }

    while (!scanner->closed && state != PARSER_STATE_ERROR && state != PARSER_STATE_SUCCESS)
    {
        token = token ? token : next_token(scanner);

        if (token)
        {
            if (token->name == TOKEN_NAME_UNKNOWN)
            {
                state = PARSER_STATE_ERROR;
                expression = create_expression(EXPRESSION_TYPE_VALUE, throw_error(ERROR_SYNTAX), NULL);
            }
            else if (token->name != TOKEN_NAME_WHITESPACE && token->name != TOKEN_NAME_COMMENT)
            {
                if (state == PARSER_STATE_START)
                {
                    state = PARSER_STATE_SUCCESS;

                    switch (token->name)
                    {
                        case TOKEN_NAME_NULL:
                            expression = create_expression(EXPRESSION_TYPE_VALUE, parse_null_literal(token->value), NULL);
                            break;
                        case TOKEN_NAME_NUMBER:
                            expression = create_expression(EXPRESSION_TYPE_VALUE, parse_number_literal(token->value), NULL);
                            break;
                        case TOKEN_NAME_STRING:
                            expression = create_expression(EXPRESSION_TYPE_VALUE, parse_string_literal(token->value), NULL);
                            break;
                        case TOKEN_NAME_LIST_START:
                            state = PARSER_STATE_ARGUMENTS;
                            expression = create_expression(EXPRESSION_TYPE_LIST, NULL, empty_list(destroy_expression_unsafe));
                            break;
                        case TOKEN_NAME_MAP_START:
                            state = PARSER_STATE_ARGUMENTS;
                            expression = create_expression(EXPRESSION_TYPE_MAP, NULL, empty_list(destroy_expression_unsafe));
                            break;
                        case TOKEN_NAME_CALL_START:
                            state = PARSER_STATE_ARGUMENTS;
                            expression = create_expression(EXPRESSION_TYPE_CALL, NULL, empty_list(destroy_expression_unsafe));
                            break;
                        default:
                            expression = create_expression(EXPRESSION_TYPE_VALUE, throw_error(ERROR_SYNTAX), NULL);
                            break;
                    }

                    if (expression->type == EXPRESSION_TYPE_VALUE && expression->value->thrown)
                    {
                        state = PARSER_STATE_ERROR;
                    }
                }
                else if (state == PARSER_STATE_ARGUMENTS)
                {
                    if (expression->type == EXPRESSION_TYPE_LIST && token->name == TOKEN_NAME_LIST_END)
                    {
                        state = PARSER_STATE_SUCCESS;
                    }
                    else if (expression->type == EXPRESSION_TYPE_MAP && token->name == TOKEN_NAME_MAP_END)
                    {
                        state = PARSER_STATE_SUCCESS;
                    }
                    else if (expression->type == EXPRESSION_TYPE_CALL && token->name == TOKEN_NAME_CALL_END)
                    {
                        if (expression->arguments->length > 0)
                        {
                            state = PARSER_STATE_SUCCESS;
                        }
                        else
                        {
                            state = PARSER_STATE_ERROR;
                            destroy_expression(expression);
                            expression = create_expression(EXPRESSION_TYPE_VALUE, throw_error(ERROR_ARGUMENT), NULL);
                        }
                    }
                    else
                    {
                        expression_t *argument;

                        argument = next_expression(scanner, token, depth + 1);
                        token = NULL;

                        if (argument->type == EXPRESSION_TYPE_VALUE && argument->value->thrown)
                        {
                            state = PARSER_STATE_ERROR;
                            destroy_expression(expression);
                            expression = argument;
                        }
                        else
                        {
                            add_list_item(expression->arguments, argument);
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
    }

    if (state == PARSER_STATE_START)
    {
        expression = create_expression(EXPRESSION_TYPE_UNSET, NULL, NULL);
    }
    else if (state == PARSER_STATE_ARGUMENTS)
    {
        destroy_expression(expression);
        expression = create_expression(EXPRESSION_TYPE_VALUE, throw_error(ERROR_SYNTAX), NULL);
    }

    if (token)
    {
        destroy_token(token);
    }

    return expression;
}

static value_t *parse_null_literal(string_t *value)
{
    if (value->length != 1)
    {
        return throw_error(ERROR_TYPE);
    }

    if (value->bytes[0] != SYMBOL_NULL)
    {
        return throw_error(ERROR_TYPE);
    }

    return new_null();
}

static value_t *parse_number_literal(string_t *value)
{
    string_t *trimmed;
    int numbered;

    if (value->length < 2)
    {
        return throw_error(ERROR_TYPE);
    }

    if (value->bytes[0] != SYMBOL_NUMBER || value->bytes[value->length - 1] != SYMBOL_NUMBER)
    {
        return throw_error(ERROR_TYPE);
    }

    trimmed = slice_string(value, 1, value->length - 1);

    if (!string_to_integer(trimmed, NUMBER_DIGIT_CAPACITY, &numbered))
    {
        destroy_string(trimmed);
        return throw_error(ERROR_TYPE);
    }

    destroy_string(trimmed);

    return new_number(numbered);
}

static value_t *parse_string_literal(string_t *value)
{
    value_t *escaped;
    string_t *trimmed;

    if (value->length < 2)
    {
        return throw_error(ERROR_TYPE);
    }

    if (value->bytes[0] != SYMBOL_STRING || value->bytes[value->length - 1] != SYMBOL_STRING)
    {
        return throw_error(ERROR_TYPE);
    }

    trimmed = slice_string(value, 1, value->length - 1);

    if (!is_printable(trimmed))
    {
        destroy_string(trimmed);
        return throw_error(ERROR_TYPE);
    }

    escaped = escape_string(trimmed);
    destroy_string(trimmed);

    return escaped;
}

static int is_printable(string_t *value)
{
    size_t index;

    for (index = 0; index < value->length; index++)
    {
        unsigned char symbol;

        symbol = value->bytes[index];

        if (symbol < 32 || symbol > 126)
        {
            return 0;
        }
    }

    return 1;
}
