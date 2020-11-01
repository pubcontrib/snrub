#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "parse.h"
#include "lex.h"
#include "value.h"
#include "map.h"
#include "list.h"
#include "common.h"

typedef struct
{
    list_t *expressions;
    list_node_t *current;
    value_t **evaluated;
    size_t index;
} argument_iterator_t;

typedef struct
{
    value_t *(*call)(argument_iterator_t *, map_t *, map_t *);
} operator_t;

static value_t *execute_expression(list_t *expressions, map_t *variables);
static value_t *apply_expression(expression_t *expression, map_t *variables, map_t *operators);
static value_t *apply_list(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *apply_call(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static map_t *default_operators(void);
static int set_operator(map_t *operators, char *name, value_t *(*call)(argument_iterator_t *, map_t *, map_t *));
static value_t *operator_evaluate(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_value(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_assign(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_variables(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_operators(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_catch(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_throw(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_add(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_subtract(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_multiply(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_divide(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_modulo(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_and(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_or(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_not(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_conditional(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_loop(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_chain(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_less(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_greater(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_equal(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_sort(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_type(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_number(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_string(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_hash(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_length(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_index(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *operator_range(argument_iterator_t *arguments, map_t *variables, map_t *operators);
static value_t *list_map_keys(map_t *map);
static int has_next_argument(argument_iterator_t *iterator);
static value_t *next_argument(argument_iterator_t *iterator, map_t *variables, map_t *operators);
static void skip_argument(argument_iterator_t *iterator);
static void reset_arguments(argument_iterator_t *iterator);
static int compare_values_ascending(const void *left, const void *right);
static int compare_values_descending(const void *left, const void *right);

value_t *execute_script(char *document, map_t *variables)
{
    scanner_t *scanner;
    list_t *expressions;
    value_t *value;

    scanner = start_scanner(document);

    if (!scanner)
    {
        return NULL;
    }

    expressions = parse_expressions(scanner);
    destroy_scanner(scanner);

    if (!expressions)
    {
        return NULL;
    }

    value = execute_expression(expressions, variables);
    destroy_list(expressions);

    return value;
}

static value_t *execute_expression(list_t *expressions, map_t *variables)
{
    list_node_t *node;
    value_t *last;
    map_t *operators;

    operators = default_operators();

    if (!operators)
    {
        return NULL;
    }

    for (node = expressions->head; node != NULL; node = node->next)
    {
        expression_t *expression;

        expression = node->value;

        if (expression->value->type == TYPE_ERROR)
        {
            return copy_value(expression->value);
        }
    }

    last = new_null();

    for (node = expressions->head; node != NULL; node = node->next)
    {
        expression_t *expression;
        value_t *value;

        expression = node->value;
        value = apply_expression(expression, variables, operators);

        if (!value)
        {
            destroy_value(last);
            return NULL;
        }

        if (value->type == TYPE_UNSET)
        {
            destroy_value(value);
        }
        else
        {
            destroy_value(last);
            last = value;
        }

        if (last->type == TYPE_ERROR)
        {
            break;
        }
    }

    destroy_map(operators);

    return last;
}

static value_t *apply_expression(expression_t *expression, map_t *variables, map_t *operators)
{
    argument_iterator_t *arguments;
    value_t *result;

    arguments = malloc(sizeof(argument_iterator_t));

    if (!arguments)
    {
        return NULL;
    }

    arguments->expressions = expression->arguments;
    arguments->current = expression->arguments->head;
    arguments->index = 0;

    if (arguments->expressions->length > 0)
    {
        arguments->evaluated = malloc(sizeof(value_t *) * arguments->expressions->length);

        if (!arguments->evaluated)
        {
            return NULL;
        }
    }
    else
    {
        arguments->evaluated = NULL;
    }

    switch (expression->value->type)
    {
        case TYPE_UNSET:
        case TYPE_NULL:
        case TYPE_NUMBER:
        case TYPE_STRING:
            result = copy_value(expression->value);
            break;
        case TYPE_LIST:
            result = apply_list(arguments, variables, operators);
            break;
        case TYPE_CALL:
            result = apply_call(arguments, variables, operators);
            break;
        default:
            result = NULL;
            break;
    }

    if (arguments->evaluated)
    {
        size_t index;

        for (index = 0; index < arguments->index; index++)
        {
            value_t *value;

            value = arguments->evaluated[index];

            if (value)
            {
                destroy_value(value);
            }
        }

        free(arguments->evaluated);
    }

    free(arguments);

    return result;
}

static value_t *apply_list(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t **items;
    size_t length;

    length = arguments->expressions->length;

    if (length > 0)
    {
        value_t *item;
        size_t index;

        items = malloc(sizeof(value_t *) * length);

        if (!items)
        {
            return NULL;
        }

        for (index = 0; index < length; index++)
        {
            value_t *copy;

            if (!has_next_argument(arguments))
            {
                return new_error(ERROR_ARGUMENT);
            }

            item = next_argument(arguments, variables, operators);

            if (!item)
            {
                free(items);
                return NULL;
            }

            if (item->type == TYPE_ERROR)
            {
                size_t reindex;

                for (reindex = 0; reindex < index; reindex++)
                {
                    destroy_value(items[reindex]);
                }

                free(items);
                return copy_value(item);
            }

            copy = copy_value(item);

            if (!copy)
            {
                free(items);
                return NULL;
            }

            items[index] = copy;
        }
    }
    else
    {
        items = NULL;
    }

    return new_list(items, length);
}

static value_t *apply_call(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *name;
    operator_t *operator;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    name = next_argument(arguments, variables, operators);

    if (!name)
    {
        return NULL;
    }

    if (name->type == TYPE_ERROR)
    {
        return copy_value(name);
    }

    if (name->type != TYPE_STRING)
    {
        return new_error(ERROR_ARGUMENT);
    }

    operator = get_map_item(operators, view_string(name));

    if (!operator)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return operator->call(arguments, variables, operators);
}

static map_t *default_operators(void)
{
    map_t *operators;

    operators = empty_map(hash_string, free, 32);

    if (!operators)
    {
        return NULL;
    }

    if (!set_operator(operators, "~", operator_evaluate)
        || !set_operator(operators, "<--", operator_value)
        || !set_operator(operators, "-->", operator_assign)
        || !set_operator(operators, "---", operator_variables)
        || !set_operator(operators, "(-)", operator_operators)
        || !set_operator(operators, "><", operator_catch)
        || !set_operator(operators, "<>", operator_throw)
        || !set_operator(operators, "+", operator_add)
        || !set_operator(operators, "-", operator_subtract)
        || !set_operator(operators, "*", operator_multiply)
        || !set_operator(operators, "/", operator_divide)
        || !set_operator(operators, "%", operator_modulo)
        || !set_operator(operators, "&", operator_and)
        || !set_operator(operators, "|", operator_or)
        || !set_operator(operators, "!", operator_not)
        || !set_operator(operators, "?", operator_conditional)
        || !set_operator(operators, "o", operator_loop)
        || !set_operator(operators, "...", operator_chain)
        || !set_operator(operators, "<", operator_less)
        || !set_operator(operators, ">", operator_greater)
        || !set_operator(operators, "=", operator_equal)
        || !set_operator(operators, "<|>", operator_sort)
        || !set_operator(operators, "_", operator_type)
        || !set_operator(operators, "#", operator_number)
        || !set_operator(operators, "\"", operator_string)
        || !set_operator(operators, "::", operator_hash)
        || !set_operator(operators, "| |", operator_length)
        || !set_operator(operators, "[#]", operator_index)
        || !set_operator(operators, "[# #]", operator_range))
    {
        destroy_map(operators);
        return NULL;
    }

    return operators;
}

static int set_operator(map_t *operators, char *name, value_t *(*call)(argument_iterator_t *, map_t *, map_t *))
{
    char *key;
    operator_t *operator;

    key = copy_string(name);

    if (!key)
    {
        return 0;
    }

    operator = malloc(sizeof(operator_t *));

    if (!operator)
    {
        free(key);
        return 0;
    }

    operator->call = call;

    return set_map_item(operators, key, operator);
}

static value_t *operator_evaluate(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *document;
    char *copy;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    document = next_argument(arguments, variables, operators);

    if (!document)
    {
        return NULL;
    }

    if (document->type == TYPE_ERROR)
    {
        return copy_value(document);
    }

    if (document->type != TYPE_STRING)
    {
        return new_error(ERROR_ARGUMENT);
    }

    copy = copy_string(view_string(document));

    if (!copy)
    {
        return NULL;
    }

    return execute_script(copy, variables);
}

static value_t *operator_value(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *identifier, *value;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    identifier = next_argument(arguments, variables, operators);

    if (!identifier)
    {
        return NULL;
    }

    if (identifier->type == TYPE_ERROR)
    {
        return copy_value(identifier);
    }

    if (identifier->type != TYPE_STRING)
    {
        return new_error(ERROR_ARGUMENT);
    }

    value = get_map_item(variables, view_string(identifier));

    return value ? copy_value(value) : new_null();
}

static value_t *operator_assign(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *identifier, *value;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    identifier = next_argument(arguments, variables, operators);

    if (!identifier)
    {
        return NULL;
    }

    if (identifier->type == TYPE_ERROR)
    {
        return copy_value(identifier);
    }

    if (identifier->type != TYPE_STRING)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    value = next_argument(arguments, variables, operators);

    if (!value)
    {
        return NULL;
    }

    if (value->type == TYPE_ERROR)
    {
        return copy_value(value);
    }

    if (value->type == TYPE_NULL)
    {
        remove_map_item(variables, view_string(identifier));
    }
    else
    {
        char *name;
        value_t *copy;

        name = copy_string(view_string(identifier));

        if (!name)
        {
            return NULL;
        }

        copy = copy_value(value);

        if (!copy)
        {
            free(name);
            return NULL;
        }

        if (!set_map_item(variables, name, copy))
        {
            return NULL;
        }
    }

    return new_null();
}

static value_t *operator_variables(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    return list_map_keys(variables);
}

static value_t *operator_operators(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    return list_map_keys(operators);
}

static value_t *operator_catch(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_null();
    }

    solo = next_argument(arguments, variables, operators);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return new_number(view_error(solo));
    }

    return new_null();
}

static value_t *operator_throw(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables, operators);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    if (solo->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return new_error(view_number(solo));
}

static value_t *operator_add(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_NUMBER && left->type != TYPE_STRING && left->type != TYPE_LIST)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    if (left->type != right->type)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (left->type == TYPE_NUMBER)
    {
        return new_number(view_number(left) + view_number(right));
    }

    if (left->type == TYPE_STRING)
    {
        char *string;
        size_t size;

        string = merge_strings(view_string(left), view_string(right));

        if (!string)
        {
            return NULL;
        }

        size = sizeof(char) * (strlen(string) + 1);

        return steal_string(string, size);
    }

    if (left->type == TYPE_LIST)
    {
        return merge_lists(left, right);
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_subtract(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    if (right->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return new_number(view_number(left) - view_number(right));
}

static value_t *operator_multiply(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    if (right->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return new_number(view_number(left) * view_number(right));
}

static value_t *operator_divide(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    if (right->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (view_number(right) == 0)
    {
        return new_error(ERROR_ARITHMETIC);
    }

    return new_number(div(view_number(left), view_number(right)).quot);
}

static value_t *operator_modulo(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    if (right->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (view_number(right) == 0)
    {
        return new_error(ERROR_ARITHMETIC);
    }

    return new_number(div(view_number(left), view_number(right)).rem);
}

static value_t *operator_and(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    if (right->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return new_number(view_number(left) && view_number(right));
}

static value_t *operator_or(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    if (right->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return new_number(view_number(left) || view_number(right));
}

static value_t *operator_not(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables, operators);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    if (solo->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return new_number(!view_number(solo));
}

static value_t *operator_conditional(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *condition;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    condition = next_argument(arguments, variables, operators);

    if (!condition)
    {
        return NULL;
    }

    if (condition->type == TYPE_ERROR)
    {
        return copy_value(condition);
    }

    if (condition->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (view_number(condition))
    {
        value_t *pass;

        if (!has_next_argument(arguments))
        {
            return new_error(ERROR_ARGUMENT);
        }

        pass = next_argument(arguments, variables, operators);

        if (!pass)
        {
            return NULL;
        }

        return copy_value(pass);
    }
    else
    {
        value_t *fail;

        skip_argument(arguments);

        if (!has_next_argument(arguments))
        {
            return new_error(ERROR_ARGUMENT);
        }

        fail = next_argument(arguments, variables, operators);

        if (!fail)
        {
            return NULL;
        }

        return copy_value(fail);
    }
}

static value_t *operator_loop(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    int proceed;

    proceed = 1;

    while (proceed)
    {
        value_t *condition;

        if (!has_next_argument(arguments))
        {
            return new_error(ERROR_ARGUMENT);
        }

        condition = next_argument(arguments, variables, operators);

        if (!condition)
        {
            return NULL;
        }

        if (condition->type == TYPE_ERROR)
        {
            return copy_value(condition);
        }

        if (condition->type != TYPE_NUMBER)
        {
            return new_error(ERROR_ARGUMENT);
        }

        proceed = view_number(condition);

        if (proceed)
        {
            value_t *pass;

            if (!has_next_argument(arguments))
            {
                return new_error(ERROR_ARGUMENT);
            }

            pass = next_argument(arguments, variables, operators);

            if (!pass)
            {
                return NULL;
            }

            if (pass->type == TYPE_ERROR)
            {
                return copy_value(pass);
            }

            reset_arguments(arguments);
            skip_argument(arguments);
        }
    }

    return new_null();
}

static value_t *operator_chain(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *last;

    if (!has_next_argument(arguments))
    {
        return new_null();
    }

    while (has_next_argument(arguments))
    {
        last = next_argument(arguments, variables, operators);

        if (!last)
        {
            return NULL;
        }

        if (last->type == TYPE_ERROR)
        {
            return copy_value(last);
        }
    }

    return copy_value(last);
}

static value_t *operator_less(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    return new_number(compare_values(left, right) < 0);
}

static value_t *operator_greater(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    return new_number(compare_values(left, right) > 0);
}

static value_t *operator_equal(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables, operators);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, variables, operators);

    if (!right)
    {
        return NULL;
    }

    if (right->type == TYPE_ERROR)
    {
        return copy_value(right);
    }

    return new_number(compare_values(left, right) == 0);
}

static value_t *operator_sort(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *collection, *reversed;
    value_t **items;
    size_t length;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    collection = next_argument(arguments, variables, operators);

    if (!collection)
    {
        return NULL;
    }

    if (collection->type == TYPE_ERROR)
    {
        return copy_value(collection);
    }

    if (collection->type != TYPE_LIST)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    reversed = next_argument(arguments, variables, operators);

    if (!reversed)
    {
        return NULL;
    }

    if (reversed->type == TYPE_ERROR)
    {
        return copy_value(reversed);
    }

    if (reversed->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    length = length_value(collection);

    if (length > 0)
    {
        size_t index;

        items = malloc(sizeof(value_t *) * length);

        if (!items)
        {
            return NULL;
        }

        for (index = 0; index < length; index++)
        {
            value_t *copy;

            copy = copy_value(((value_t **) collection->data)[index]);

            if (!copy)
            {
                free(items);
                return NULL;
            }

            items[index] = copy;
        }

        qsort(items, length, sizeof(value_t *), view_number(reversed) ? compare_values_descending : compare_values_ascending);
    }
    else
    {
        items = NULL;
    }

    return new_list(items, length);
}

static value_t *operator_type(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables, operators);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_NULL)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_NUMBER)
    {
        return new_string("##");
    }

    if (solo->type == TYPE_STRING)
    {
        return new_string("\"\"");
    }

    if (solo->type == TYPE_LIST)
    {
        return new_string("[]");
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_number(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables, operators);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_NULL)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_NUMBER)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_STRING)
    {
        if (is_integer(view_string(solo)))
        {
            return new_number(atoi(view_string(solo)));
        }
        else
        {
            return new_error(ERROR_TYPE);
        }
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_string(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables, operators);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_NULL)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_STRING)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_NUMBER)
    {
        char *string;
        size_t size;

        string = integer_to_string(view_number(solo));

        if (!string)
        {
            return NULL;
        }

        size = sizeof(char) * (strlen(string) + 1);

        return steal_string(string, size);
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_hash(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables, operators);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    return new_number(hash_value(solo));
}

static value_t *operator_length(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables, operators);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    if (solo->type != TYPE_STRING && solo->type != TYPE_LIST)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return new_number(length_value(solo));
}

static value_t *operator_index(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *collection, *index;
    int adjusted;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    collection = next_argument(arguments, variables, operators);

    if (!collection)
    {
        return NULL;
    }

    if (collection->type == TYPE_ERROR)
    {
        return copy_value(collection);
    }

    if (collection->type != TYPE_STRING && collection->type != TYPE_LIST)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    index = next_argument(arguments, variables, operators);

    if (!index)
    {
        return NULL;
    }

    if (index->type == TYPE_ERROR)
    {
        return copy_value(index);
    }

    if (index->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    adjusted = view_number(index) - 1;

    if (adjusted < 0 || adjusted >= length_value(collection))
    {
        return new_null();
    }

    if (collection->type == TYPE_STRING)
    {
        char *string;
        size_t size;

        size = sizeof(char) * 2;
        string = malloc(size);

        if (!string)
        {
            return NULL;
        }

        string[0] = view_string(collection)[adjusted];
        string[1] = '\0';

        return steal_string(string, size);
    }

    if (collection->type == TYPE_LIST)
    {
        return copy_value(((value_t **) collection->data)[adjusted]);
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_range(argument_iterator_t *arguments, map_t *variables, map_t *operators)
{
    value_t *collection, *start, *end;
    int adjustedstart, adjustedend;
    size_t limit, length;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    collection = next_argument(arguments, variables, operators);

    if (!collection)
    {
        return NULL;
    }

    if (collection->type == TYPE_ERROR)
    {
        return copy_value(collection);
    }

    if (collection->type != TYPE_STRING && collection->type != TYPE_LIST)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    start = next_argument(arguments, variables, operators);

    if (!start)
    {
        return NULL;
    }

    if (start->type == TYPE_ERROR)
    {
        return copy_value(start);
    }

    if (start->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    end = next_argument(arguments, variables, operators);

    if (!end)
    {
        return NULL;
    }

    if (end->type == TYPE_ERROR)
    {
        return copy_value(end);
    }

    if (end->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    adjustedstart = view_number(start) - 1;
    adjustedend = view_number(end) - 1;
    limit = length_value(collection);

    if (adjustedstart > adjustedend)
    {
        int swap;

        swap = adjustedstart;
        adjustedstart = adjustedend;
        adjustedend = swap;
    }

    if (adjustedstart < 0)
    {
        adjustedstart = 0;
    }

    if (adjustedend >= limit)
    {
        adjustedend = limit - 1;
    }

    adjustedend += 1;
    length = adjustedend - adjustedstart;

    if (collection->type == TYPE_STRING)
    {
        char *slice;

        slice = slice_string(view_string(collection), adjustedstart, adjustedend);

        if (!slice)
        {
            return NULL;
        }

        return steal_string(slice, length + 1);
    }

    if (collection->type == TYPE_LIST)
    {
        value_t **items;

        if (length > 0)
        {
            size_t index, placement;

            items = malloc(sizeof(value_t *) * length);

            if (!items)
            {
                return NULL;
            }

            for (index = adjustedstart, placement = 0; index < adjustedend; index++, placement++)
            {
                value_t *item;

                item = copy_value(((value_t **) collection->data)[index]);

                if (!item)
                {
                    free(items);
                    return NULL;
                }

                items[placement] = item;
            }
        }
        else
        {
            items = NULL;
        }

        return new_list(items, length);
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *list_map_keys(map_t *map)
{
    value_t **items;
    size_t length;

    length = map->length;

    if (length > 0)
    {
        size_t index, placement;

        items = malloc(sizeof(value_t *) * length);

        if (!items)
        {
            return NULL;
        }

        for (index = 0, placement = 0; index < map->capacity; index++)
        {
            if (map->chains[index])
            {
                map_chain_t *chain;

                for (chain = map->chains[index]; chain != NULL; chain = chain->next)
                {
                    value_t *item;

                    item = new_string(chain->key);

                    if (!item)
                    {
                        free(items);
                        return NULL;
                    }

                    items[placement++] = item;
                }
            }
        }

        qsort(items, length, sizeof(value_t *), compare_values_ascending);
    }
    else
    {
        items = NULL;
    }

    return new_list(items, length);
}

static int has_next_argument(argument_iterator_t *iterator)
{
    return iterator->index < iterator->expressions->length;
}

static value_t *next_argument(argument_iterator_t *iterator, map_t *variables, map_t *operators)
{
    value_t *result;

    result = apply_expression(iterator->current->value, variables, operators);
    iterator->current = iterator->current->next;
    iterator->evaluated[iterator->index] = result;
    iterator->index += 1;

    return result;
}

static void skip_argument(argument_iterator_t *iterator)
{
    iterator->current = iterator->current->next;
    iterator->evaluated[iterator->index] = NULL;
    iterator->index += 1;
}

static void reset_arguments(argument_iterator_t *iterator)
{
    size_t index;

    for (index = 0; index < iterator->index; index++)
    {
        if (iterator->evaluated[index])
        {
            destroy_value(iterator->evaluated[index]);
            iterator->evaluated[index] = NULL;
        }
    }

    iterator->current = iterator->expressions->head;
    iterator->index = 0;
}

static int compare_values_ascending(const void *left, const void *right)
{
    return compare_values(*(value_t **) left, *(value_t **) right);
}

static int compare_values_descending(const void *left, const void *right)
{
    return compare_values(*(value_t **) left, *(value_t **) right) * -1;
}
