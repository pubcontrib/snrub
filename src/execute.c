#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cli.h"
#include "execute.h"
#include "parse.h"
#include "lex.h"
#include "value.h"
#include "map.h"
#include "list.h"
#include "common.h"

#define VALUE_TYPES_NONNULL (VALUE_TYPE_NUMBER | VALUE_TYPE_STRING | VALUE_TYPE_LIST)
#define VALUE_TYPES_ANY (VALUE_TYPE_NULL | VALUE_TYPE_NUMBER | VALUE_TYPE_STRING | VALUE_TYPE_LIST)

typedef struct
{
    list_t *expressions;
    list_node_t *current;
    value_t **evaluated;
    size_t index;
    value_t *value;
    int interception;
} argument_iterator_t;

typedef struct
{
    map_t *globals;
    map_t *locals;
    map_t *operators;
    int depth;
} stack_frame_t;

typedef struct
{
    value_t *(*call)(argument_iterator_t *, stack_frame_t *);
} operator_t;

static value_t *evaluate_script(char *document, map_t *globals, value_t *arguments, int depth);
static value_t *evaluate_expressions(list_t *expressions, map_t *globals, value_t *arguments, int depth);
static value_t *apply_expression(expression_t *expression, stack_frame_t *frame);
static value_t *apply_list(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *apply_call(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_evaluate(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_recall(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_memorize(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_forget(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_promote(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_demote(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_variables(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_operators(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_catch(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_throw(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_add(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_subtract(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_multiply(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_divide(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_modulo(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_and(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_or(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_not(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_conditional(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_loop(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_chain(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_less(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_greater(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_equal(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_sort(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_type(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_number(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_string(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_hash(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_represent(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_length(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_get(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_set(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_unset(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_slice(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_read(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_write(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_remove(argument_iterator_t *arguments, stack_frame_t *frame);
static map_t *default_operators(void);
static int set_operator(map_t *operators, char *name, value_t *(*call)(argument_iterator_t *, stack_frame_t *));
static map_t *empty_variables(void);
static value_t *set_scoped_variable(stack_frame_t *frame, char *identifier, value_t *variable);
static value_t *swap_variable_scope(map_t *before, map_t *after, char *identifier);
static int set_variable(map_t *variables, char *identifier, value_t *variable);
static int has_next_argument(argument_iterator_t *arguments);
static int next_argument(argument_iterator_t *arguments, stack_frame_t *frame, int types);
static void skip_argument(argument_iterator_t *arguments);
static void reset_arguments(argument_iterator_t *arguments);
static value_t *list_map_keys(map_t *map);
static void sort_collection(value_t *collection, int reversed);
static int compare_values_ascending(const void *left, const void *right);
static int compare_values_descending(const void *left, const void *right);
static void destroy_value_unsafe(void *value);

value_t *execute_script(char *document, map_t *globals, value_t *arguments)
{
    document = copy_string(document);

    if (!document)
    {
        return NULL;
    }

    return evaluate_script(document, globals, arguments, 1);
}

static value_t *evaluate_script(char *document, map_t *globals, value_t *arguments, int depth)
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

    value = evaluate_expressions(expressions, globals, arguments, depth);
    destroy_list(expressions);

    return value;
}

static value_t *evaluate_expressions(list_t *expressions, map_t *globals, value_t *arguments, int depth)
{
    list_node_t *node;
    stack_frame_t frame;
    value_t *last;

    if (depth > LIMIT_DEPTH)
    {
        return throw_error(ERROR_BOUNDS);
    }

    for (node = expressions->head; node != NULL; node = node->next)
    {
        expression_t *expression;

        expression = node->value;

        if (expression->value->thrown)
        {
            return copy_value(expression->value);
        }
    }

    frame.globals = globals;
    frame.operators = default_operators();
    frame.depth = depth;

    if (!frame.operators)
    {
        return NULL;
    }

    frame.locals = empty_variables();

    if (!frame.locals)
    {
        destroy_map(frame.operators);
        return NULL;
    }

    last = set_scoped_variable(&frame, "@", arguments);

    if (last && last->type == VALUE_TYPE_NULL)
    {
        for (node = expressions->head; node != NULL; node = node->next)
        {
            expression_t *expression;
            value_t *value;

            expression = node->value;
            value = apply_expression(expression, &frame);

            if (!value)
            {
                destroy_value(last);
                last = NULL;
                break;
            }

            if (value->type == VALUE_TYPE_UNSET)
            {
                destroy_value(value);
            }
            else
            {
                destroy_value(last);
                last = value;
            }

            if (last->thrown)
            {
                break;
            }
        }
    }

    destroy_map(frame.operators);
    destroy_map(frame.locals);

    return last;
}

static value_t *apply_expression(expression_t *expression, stack_frame_t *frame)
{
    argument_iterator_t arguments;
    value_t *result;

    arguments.expressions = expression->arguments;
    arguments.current = expression->arguments->head;
    arguments.index = 0;

    if (arguments.expressions->length > 0)
    {
        arguments.evaluated = malloc(sizeof(value_t *) * arguments.expressions->length);

        if (!arguments.evaluated)
        {
            return NULL;
        }
    }
    else
    {
        arguments.evaluated = NULL;
    }

    switch (expression->value->type)
    {
        case VALUE_TYPE_UNSET:
        case VALUE_TYPE_NULL:
        case VALUE_TYPE_NUMBER:
        case VALUE_TYPE_STRING:
            result = copy_value(expression->value);
            break;
        case VALUE_TYPE_LIST:
            result = apply_list(&arguments, frame);
            break;
        case VALUE_TYPE_CALL:
            result = apply_call(&arguments, frame);
            break;
        default:
            result = NULL;
            break;
    }

    if (arguments.evaluated)
    {
        size_t index;

        for (index = 0; index < arguments.index; index++)
        {
            value_t *value;

            value = arguments.evaluated[index];

            if (value)
            {
                destroy_value(value);
            }
        }

        free(arguments.evaluated);
    }

    return result;
}

static value_t *apply_list(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t **items;
    size_t length;

    length = arguments->expressions->length;

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

            if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
            {
                destroy_items(items, index);
                return arguments->value;
            }

            copy = copy_value(arguments->value);

            if (!copy)
            {
                destroy_items(items, index);
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

static value_t *apply_call(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *name;
    operator_t *operator;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    name = arguments->value;

    operator = get_map_item(frame->operators, view_string(name));

    if (!operator)
    {
        return throw_error(ERROR_ARGUMENT);
    }

    return operator->call(arguments, frame);
}

static value_t *operator_evaluate(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *document, *initial;
    char *copy;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    document = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    initial = arguments->value;

    copy = copy_string(view_string(document));

    if (!copy)
    {
        return NULL;
    }

    return evaluate_script(copy, frame->globals, initial, frame->depth + 1);
}

static value_t *operator_recall(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier, *value;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;

    value = get_map_item(frame->globals, view_string(identifier));

    if (!value)
    {
        value = get_map_item(frame->locals, view_string(identifier));
    }

    return value ? copy_value(value) : new_null();
}

static value_t *operator_memorize(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier, *value;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    value = arguments->value;

    return set_scoped_variable(frame, view_string(identifier), value);
}

static value_t *operator_forget(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier;
    int global;
    map_t *variables;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;
    global = has_map_item(frame->globals, view_string(identifier));
    variables = global ? frame->globals : frame->locals;
    remove_map_item(variables, view_string(identifier));

    return new_null();
}

static value_t *operator_promote(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;

    return swap_variable_scope(frame->locals, frame->globals, view_string(identifier));
}

static value_t *operator_demote(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;

    return swap_variable_scope(frame->globals, frame->locals, view_string(identifier));
}

static value_t *operator_variables(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right, *sorted;

    left = list_map_keys(frame->globals);

    if (!left)
    {
        return NULL;
    }

    right = list_map_keys(frame->locals);

    if (!right)
    {
        destroy_value(left);
        return NULL;
    }

    sorted = merge_lists(left, right);
    destroy_value(left);
    destroy_value(right);

    if (!sorted)
    {
        return NULL;
    }

    sort_collection(sorted, 0);

    return sorted;
}

static value_t *operator_operators(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *sorted;

    sorted = list_map_keys(frame->operators);

    if (!sorted)
    {
        return NULL;
    }

    sort_collection(sorted, 0);

    return sorted;
}

static value_t *operator_catch(argument_iterator_t *arguments, stack_frame_t *frame)
{
    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        value_t *solo;

        solo = arguments->value;

        if (!solo)
        {
            return NULL;
        }

        if (arguments->interception)
        {
            solo->thrown = 0;
        }

        return solo;
    }

    return new_null();
}

static value_t *operator_throw(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo, *copy;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    solo = arguments->value;
    copy = copy_value(solo);

    if (!copy)
    {
        return NULL;
    }

    copy->thrown = 1;

    return copy;
}

static value_t *operator_add(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, VALUE_TYPES_NONNULL))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPES_NONNULL))
    {
        return arguments->value;
    }

    right = arguments->value;

    if (left->type != right->type)
    {
        return throw_error(ERROR_ARGUMENT);
    }

    if (left->type == VALUE_TYPE_NUMBER)
    {
        int sum;

        if (number_add(view_number(left), view_number(right), &sum))
        {
            return new_number(sum);
        }
        else
        {
            return throw_error(ERROR_BOUNDS);
        }
    }

    if (left->type == VALUE_TYPE_STRING)
    {
        char *sum;

        if (string_add(view_string(left), view_string(right), &sum))
        {
            size_t size;

            if (!sum)
            {
                return NULL;
            }

            size = sizeof(char) * (strlen(sum) + 1);

            return steal_string(sum, size);
        }
        else
        {
            return throw_error(ERROR_BOUNDS);
        }
    }

    if (left->type == VALUE_TYPE_LIST)
    {
        return merge_lists(left, right);
    }

    return throw_error(ERROR_ARGUMENT);
}

static value_t *operator_subtract(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;
    int difference;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    if (number_subtract(view_number(left), view_number(right), &difference))
    {
        return new_number(difference);
    }
    else
    {
        return throw_error(ERROR_BOUNDS);
    }
}

static value_t *operator_multiply(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;
    int product;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    if (number_multiply(view_number(left), view_number(right), &product))
    {
        return new_number(product);
    }
    else
    {
        return throw_error(ERROR_BOUNDS);
    }
}

static value_t *operator_divide(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;
    int quotient;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    if (view_number(right) == 0)
    {
        return throw_error(ERROR_ARITHMETIC);
    }

    if (number_divide(view_number(left), view_number(right), &quotient))
    {
        return new_number(quotient);
    }
    else
    {
        return throw_error(ERROR_BOUNDS);
    }
}

static value_t *operator_modulo(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;
    int remainder;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    if (view_number(right) == 0)
    {
        return throw_error(ERROR_ARITHMETIC);
    }

    if (number_modulo(view_number(left), view_number(right), &remainder))
    {
        return new_number(remainder);
    }
    else
    {
        return throw_error(ERROR_BOUNDS);
    }
}

static value_t *operator_and(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(view_number(left) && view_number(right));
}

static value_t *operator_or(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(view_number(left) || view_number(right));
}

static value_t *operator_not(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return new_number(!view_number(solo));
}

static value_t *operator_conditional(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *condition;
    int first;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    condition = arguments->value;
    first = view_number(condition);

    if (!first)
    {
        skip_argument(arguments);
    }

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    if (first && !has_next_argument(arguments))
    {
        return throw_error(ERROR_ARGUMENT);
    }

    return copy_value(arguments->value);
}

static value_t *operator_loop(argument_iterator_t *arguments, stack_frame_t *frame)
{
    int proceed;

    proceed = 1;

    while (proceed)
    {
        value_t *condition;

        if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
        {
            return arguments->value;
        }

        condition = arguments->value;
        proceed = view_number(condition);

        if (proceed)
        {
            if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
            {
                return arguments->value;
            }

            reset_arguments(arguments);
            skip_argument(arguments);
        }
    }

    return new_null();
}

static value_t *operator_chain(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *last;

    do
    {
        if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
        {
            return arguments->value;
        }

        last = arguments->value;
    } while (has_next_argument(arguments));

    return copy_value(last);
}

static value_t *operator_less(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(compare_values(left, right) < 0);
}

static value_t *operator_greater(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(compare_values(left, right) > 0);
}

static value_t *operator_equal(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(compare_values(left, right) == 0);
}

static value_t *operator_sort(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection, *reversed, *sorted;

    if (!next_argument(arguments, frame, VALUE_TYPE_LIST))
    {
        return arguments->value;
    }

    collection = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    reversed = arguments->value;
    sorted = copy_value(collection);

    if (sorted)
    {
        sort_collection(sorted, view_number(reversed));
    }

    return sorted;
}

static value_t *operator_type(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    solo = arguments->value;

    switch (solo->type)
    {
        case VALUE_TYPE_NULL:
            return new_string("?");
        case VALUE_TYPE_NUMBER:
            return new_string("##");
        case VALUE_TYPE_STRING:
            return new_string("\"\"");
        case VALUE_TYPE_LIST:
            return new_string("[]");
        default:
            return throw_error(ERROR_ARGUMENT);
    }
}

static value_t *operator_number(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    solo = arguments->value;

    if (solo->type == VALUE_TYPE_NULL)
    {
        return copy_value(solo);
    }

    if (solo->type == VALUE_TYPE_NUMBER)
    {
        return copy_value(solo);
    }

    if (solo->type == VALUE_TYPE_STRING)
    {
        int out;

        return string_to_integer(view_string(solo), NUMBER_DIGIT_CAPACITY, &out) ? new_number(out) : throw_error(ERROR_TYPE);
    }

    return throw_error(ERROR_ARGUMENT);
}

static value_t *operator_string(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    solo = arguments->value;

    if (solo->type == VALUE_TYPE_NULL)
    {
        return copy_value(solo);
    }

    if (solo->type == VALUE_TYPE_STRING)
    {
        return copy_value(solo);
    }

    if (solo->type == VALUE_TYPE_NUMBER)
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

    return throw_error(ERROR_ARGUMENT);
}

static value_t *operator_hash(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return new_number(hash_value(solo));
}

static value_t *operator_represent(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return represent_value(solo);
}

static value_t *operator_length(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING | VALUE_TYPE_LIST))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return new_number(length_value(solo));
}

static value_t *operator_get(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection, *index;
    int adjusted;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING | VALUE_TYPE_LIST))
    {
        return arguments->value;
    }

    collection = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    index = arguments->value;
    adjusted = view_number(index) - 1;

    if (adjusted < 0 || adjusted >= length_value(collection))
    {
        return new_null();
    }

    if (collection->type == VALUE_TYPE_STRING)
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

    if (collection->type == VALUE_TYPE_LIST)
    {
        return copy_value(((value_t **) collection->data)[adjusted]);
    }

    return throw_error(ERROR_ARGUMENT);
}

static value_t *operator_set(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection, *index;
    int adjusted;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING | VALUE_TYPE_LIST))
    {
        return arguments->value;
    }

    collection = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    index = arguments->value;
    adjusted = view_number(index) - 1;

    if (collection->type == VALUE_TYPE_STRING)
    {
        value_t *value;
        char *string;
        size_t size, left, right;

        if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
        {
            return arguments->value;
        }

        value = arguments->value;

        if (adjusted < 0 || adjusted >= length_value(collection))
        {
            return copy_value(collection);
        }

        size = sizeof(char) * length_value(collection) + length_value(value);
        string = malloc(size);

        if (!string)
        {
            return NULL;
        }

        for (left = 0, right = 0; right < adjusted; left++, right++)
        {
            string[left] = view_string(collection)[right];
        }

        for (right = 0; right < length_value(value); left++, right++)
        {
            string[left] = view_string(value)[right];
        }

        for (right = adjusted + 1; right < length_value(collection); left++, right++)
        {
            string[left] = view_string(collection)[right];
        }

        string[size - 1] = '\0';

        return steal_string(string, size);
    }

    if (collection->type == VALUE_TYPE_LIST)
    {
        value_t *value, *item;
        value_t **items;
        size_t length, left, right;

        if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
        {
            return arguments->value;
        }

        value = arguments->value;

        if (adjusted < 0 || adjusted >= length_value(collection))
        {
            return copy_value(collection);
        }

        length = length_value(collection);
        items = malloc(sizeof(value_t *) * length);

        if (!items)
        {
            return NULL;
        }

        for (left = 0, right = 0; right < adjusted; left++, right++)
        {
            item = copy_value(((value_t **) collection->data)[right]);

            if (!item)
            {
                destroy_items(items, left);
                return NULL;
            }

            items[left] = item;
        }

        item = copy_value(value);

        if (!item)
        {
            destroy_items(items, left);
            return NULL;
        }

        items[left++] = item;

        for (right = adjusted + 1; right < length_value(collection); left++, right++)
        {
            item = copy_value(((value_t **) collection->data)[right]);

            if (!item)
            {
                destroy_items(items, left);
                return NULL;
            }

            items[left] = item;
        }

        return new_list(items, length);
    }

    return throw_error(ERROR_ARGUMENT);
}

static value_t *operator_unset(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection, *index;
    int adjusted;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING | VALUE_TYPE_LIST))
    {
        return arguments->value;
    }

    collection = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    index = arguments->value;
    adjusted = view_number(index) - 1;

    if (adjusted < 0 || adjusted >= length_value(collection))
    {
        return copy_value(collection);
    }

    if (collection->type == VALUE_TYPE_STRING)
    {
        char *string;
        size_t size, left, right;

        size = sizeof(char) * length_value(collection);
        string = malloc(size);

        if (!string)
        {
            return NULL;
        }

        for (left = 0, right = 0; right < length_value(collection); right++)
        {
            if (right != adjusted)
            {
                string[left++] = view_string(collection)[right];
            }
        }

        string[size - 1] = '\0';

        return steal_string(string, size);
    }

    if (collection->type == VALUE_TYPE_LIST)
    {
        value_t *item;
        value_t **items;
        size_t length, left, right;

        length = length_value(collection) - 1;
        items = malloc(sizeof(value_t *) * length);

        if (!items)
        {
            return NULL;
        }

        for (left = 0, right = 0; right < length_value(collection); right++)
        {
            if (right != adjusted)
            {
                item = copy_value(((value_t **) collection->data)[right]);

                if (!item)
                {
                    destroy_items(items, left);
                    return NULL;
                }

                items[left++] = item;
            }
        }

        return new_list(items, length);
    }

    return throw_error(ERROR_ARGUMENT);
}

static value_t *operator_slice(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection, *start, *end;
    int adjustedStart, adjustedEnd;
    size_t limit, length;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING | VALUE_TYPE_LIST))
    {
        return arguments->value;
    }

    collection = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    start = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
    {
        return arguments->value;
    }

    end = arguments->value;
    adjustedStart = view_number(start) - 1;
    adjustedEnd = view_number(end) - 1;
    limit = length_value(collection);

    if (adjustedStart > adjustedEnd)
    {
        int swap;

        swap = adjustedStart;
        adjustedStart = adjustedEnd;
        adjustedEnd = swap;
    }

    if (adjustedStart < 0)
    {
        adjustedStart = 0;
    }

    if (adjustedEnd >= limit)
    {
        adjustedEnd = limit - 1;
    }

    adjustedEnd += 1;
    length = adjustedEnd - adjustedStart;

    if (collection->type == VALUE_TYPE_STRING)
    {
        char *slice;

        slice = slice_string(view_string(collection), adjustedStart, adjustedEnd);

        if (!slice)
        {
            return NULL;
        }

        return steal_string(slice, length + 1);
    }

    if (collection->type == VALUE_TYPE_LIST)
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

            for (index = adjustedStart, placement = 0; index < adjustedEnd; index++, placement++)
            {
                value_t *item;

                item = copy_value(((value_t **) collection->data)[index]);

                if (!item)
                {
                    destroy_items(items, index);
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

    return throw_error(ERROR_ARGUMENT);
}

static value_t *operator_read(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *path;
    char *file;
    size_t length, index;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    path = arguments->value;
    file = read_file(view_string(path));

    if (!file)
    {
        return new_null();
    }

    length = strlen(file);

    if (length > NUMBER_MAX)
    {
        return throw_error(ERROR_BOUNDS);
    }

    for (index = 0; index < length; index++)
    {
        unsigned char symbol;

        symbol = file[index];

        if (!(isprint(symbol) || symbol == '\t' || symbol == '\n' || symbol == '\r'))
        {
            free(file);
            return throw_error(ERROR_TYPE);
        }
    }

    return steal_string(file, sizeof(char) * (length + 1));
}

static value_t *operator_write(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *path, *text;
    FILE *file;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    path = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    text = arguments->value;
    file = fopen(view_string(path), "wb");

    if (file)
    {
        fwrite(view_string(text), sizeof(char), length_value(text), file);
        fclose(file);
    }

    return new_null();
}

static value_t *operator_remove(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *path;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    path = arguments->value;
    remove(view_string(path));

    return new_null();
}

static map_t *default_operators(void)
{
    map_t *operators;

    operators = empty_map(hash_string, free, 64);

    if (!operators)
    {
        return NULL;
    }

    if (!set_operator(operators, "~", operator_evaluate)
        || !set_operator(operators, "x->", operator_recall)
        || !set_operator(operators, "x<-", operator_memorize)
        || !set_operator(operators, "x--", operator_forget)
        || !set_operator(operators, "<3", operator_promote)
        || !set_operator(operators, "</3", operator_demote)
        || !set_operator(operators, "x[]", operator_variables)
        || !set_operator(operators, "()[]", operator_operators)
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
        || !set_operator(operators, ";", operator_represent)
        || !set_operator(operators, "| |", operator_length)
        || !set_operator(operators, "$->", operator_get)
        || !set_operator(operators, "$<-", operator_set)
        || !set_operator(operators, "$--", operator_unset)
        || !set_operator(operators, "[# #]", operator_slice)
        || !set_operator(operators, "[o]->", operator_read)
        || !set_operator(operators, "[o]<-", operator_write)
        || !set_operator(operators, "[o]--", operator_remove))
    {
        destroy_map(operators);
        return NULL;
    }

    return operators;
}

static int set_operator(map_t *operators, char *name, value_t *(*call)(argument_iterator_t *, stack_frame_t *))
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

static map_t *empty_variables(void)
{
    return empty_map(hash_string, destroy_value_unsafe, 64);
}

static value_t *set_scoped_variable(stack_frame_t *frame, char *identifier, value_t *variable)
{
    int global, exists;
    map_t *variables;

    global = has_map_item(frame->globals, identifier);
    variables = global ? frame->globals : frame->locals;
    exists = global || has_map_item(variables, identifier);

    if (!exists && variables->length >= NUMBER_MAX)
    {
        return throw_error(ERROR_BOUNDS);
    }

    if (!set_variable(variables, identifier, variable))
    {
        return NULL;
    }

    return new_null();
}

static value_t *swap_variable_scope(map_t *before, map_t *after, char *identifier)
{
    value_t *value;

    value = get_map_item(before, identifier);

    if (value)
    {
        int exists;

        exists = has_map_item(after, identifier);

        if (!exists && after->length >= NUMBER_MAX)
        {
            return throw_error(ERROR_BOUNDS);
        }

        if (!set_variable(after, identifier, value))
        {
            return NULL;
        }

        remove_map_item(before, identifier);
    }

    return new_null();
}

static int set_variable(map_t *variables, char *identifier, value_t *variable)
{
    char *key;
    value_t *value;

    key = copy_string(identifier);

    if (!key)
    {
        return 0;
    }

    value = copy_value(variable);

    if (!value)
    {
        free(key);
        return 0;
    }

    return set_map_item(variables, key, value);
}

static int has_next_argument(argument_iterator_t *arguments)
{
    return arguments->index < arguments->expressions->length;
}

static int next_argument(argument_iterator_t *arguments, stack_frame_t *frame, int types)
{
    value_t *result;

    if (!has_next_argument(arguments))
    {
        arguments->value = throw_error(ERROR_ARGUMENT);
        return 0;
    }

    result = apply_expression(arguments->current->value, frame);
    arguments->current = arguments->current->next;
    arguments->evaluated[arguments->index] = result;
    arguments->index += 1;
    arguments->interception = 0;

    if (!result)
    {
        arguments->value = NULL;
        return 0;
    }

    if (result->thrown)
    {
        arguments->value = copy_value(result);
        arguments->interception = 1;
        return 0;
    }

    if (!(types & result->type))
    {
        arguments->value = throw_error(ERROR_ARGUMENT);
        return 0;
    }

    arguments->value = result;
    return 1;
}

static void skip_argument(argument_iterator_t *arguments)
{
    if (has_next_argument(arguments))
    {
        arguments->current = arguments->current->next;
        arguments->evaluated[arguments->index] = NULL;
        arguments->index += 1;
        arguments->value = NULL;
        arguments->interception = 0;
    }
}

static void reset_arguments(argument_iterator_t *arguments)
{
    size_t index;

    for (index = 0; index < arguments->index; index++)
    {
        if (arguments->evaluated[index])
        {
            destroy_value(arguments->evaluated[index]);
            arguments->evaluated[index] = NULL;
        }
    }

    arguments->current = arguments->expressions->head;
    arguments->index = 0;
    arguments->value = NULL;
    arguments->interception = 0;
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
                        destroy_items(items, index);
                        return NULL;
                    }

                    items[placement++] = item;
                }
            }
        }
    }
    else
    {
        items = NULL;
    }

    return new_list(items, length);
}

static void sort_collection(value_t *collection, int reversed)
{
    size_t length;

    length = length_value(collection);

    if (length > 0 && collection->type == VALUE_TYPE_LIST)
    {
        qsort(collection->data, length, sizeof(value_t *), reversed ? compare_values_descending : compare_values_ascending);
    }
}

static int compare_values_ascending(const void *left, const void *right)
{
    return compare_values(*(value_t **) left, *(value_t **) right);
}

static int compare_values_descending(const void *left, const void *right)
{
    return compare_values(*(value_t **) left, *(value_t **) right) * -1;
}

static void destroy_value_unsafe(void *value)
{
    destroy_value((value_t *) value);
}
