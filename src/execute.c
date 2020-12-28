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

#define TYPES_NONERROR (TYPE_NULL | TYPE_NUMBER | TYPE_STRING | TYPE_LIST)
#define TYPES_NONNULL (TYPE_NUMBER | TYPE_STRING | TYPE_LIST)
#define TYPES_ANY (TYPE_NULL | TYPE_NUMBER | TYPE_STRING | TYPE_LIST | TYPE_ERROR)

typedef struct
{
    list_t *expressions;
    list_node_t *current;
    value_t **evaluated;
    size_t index;
    value_t *value;
} argument_iterator_t;

typedef struct
{
    map_t *variables;
    map_t *operators;
    int depth;
} stack_frame_t;

typedef struct
{
    value_t *(*call)(argument_iterator_t *, stack_frame_t *);
} operator_t;

static value_t *evaluate_script(char *document, map_t *variables, int depth);
static value_t *evaluate_expressions(list_t *expressions, map_t *variables, int depth);
static value_t *apply_expression(expression_t *expression, stack_frame_t *frame);
static value_t *apply_list(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *apply_call(argument_iterator_t *arguments, stack_frame_t *frame);
static map_t *default_operators(void);
static int set_operator(map_t *operators, char *name, value_t *(*call)(argument_iterator_t *, stack_frame_t *));
static int set_variable(map_t *variables, char *identifier, value_t *variable);
static value_t *operator_evaluate(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_value(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_assign(argument_iterator_t *arguments, stack_frame_t *frame);
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
static value_t *operator_index(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_range(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_read(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *list_map_keys(map_t *map);
static int has_next_argument(argument_iterator_t *arguments);
static int next_argument(argument_iterator_t *arguments, stack_frame_t *frame, int types);
static void skip_argument(argument_iterator_t *arguments);
static void reset_arguments(argument_iterator_t *arguments);
static int compare_values_ascending(const void *left, const void *right);
static int compare_values_descending(const void *left, const void *right);

value_t *execute_script(char *document, map_t *variables)
{
    return evaluate_script(document, variables, 1);
}

static value_t *evaluate_script(char *document, map_t *variables, int depth)
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

    value = evaluate_expressions(expressions, variables, depth);
    destroy_list(expressions);

    return value;
}

static value_t *evaluate_expressions(list_t *expressions, map_t *variables, int depth)
{
    list_node_t *node;
    value_t *last;
    stack_frame_t frame;

    frame.variables = variables;
    frame.operators = default_operators();

    if (!frame.operators)
    {
        return NULL;
    }

    frame.depth = depth;

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
        value = apply_expression(expression, &frame);

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

    destroy_map(frame.operators);

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
        case TYPE_UNSET:
        case TYPE_NULL:
        case TYPE_NUMBER:
        case TYPE_STRING:
            result = copy_value(expression->value);
            break;
        case TYPE_LIST:
            result = apply_list(&arguments, frame);
            break;
        case TYPE_CALL:
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

            if (!next_argument(arguments, frame, TYPES_NONERROR))
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

    if (!next_argument(arguments, frame, TYPE_STRING))
    {
        return arguments->value;
    }

    name = arguments->value;

    operator = get_map_item(frame->operators, view_string(name));

    if (!operator)
    {
        return new_error(ERROR_ARGUMENT);
    }

    return operator->call(arguments, frame);
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
        || !set_operator(operators, "$", operator_represent)
        || !set_operator(operators, "| |", operator_length)
        || !set_operator(operators, "[#]", operator_index)
        || !set_operator(operators, "[# #]", operator_range)
        || !set_operator(operators, "^", operator_read))
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

static value_t *operator_evaluate(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *document, *value;
    char *copy;

    if (!next_argument(arguments, frame, TYPE_STRING))
    {
        return arguments->value;
    }

    document = arguments->value;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    value = arguments->value;

    if (!set_variable(frame->variables, "@", value))
    {
        return NULL;
    }

    copy = copy_string(view_string(document));

    if (!copy)
    {
        return NULL;
    }

    if (frame->depth >= LIMIT_DEPTH)
    {
        return new_error(ERROR_BOUNDS);
    }

    return evaluate_script(copy, frame->variables, frame->depth + 1);
}

static value_t *operator_value(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier, *value;

    if (!next_argument(arguments, frame, TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;

    value = get_map_item(frame->variables, view_string(identifier));

    return value ? copy_value(value) : new_null();
}

static value_t *operator_assign(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier, *value;

    if (!next_argument(arguments, frame, TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    value = arguments->value;

    if (value->type == TYPE_NULL)
    {
        remove_map_item(frame->variables, view_string(identifier));
    }
    else
    {
        if (!set_variable(frame->variables, view_string(identifier), value))
        {
            return NULL;
        }
    }

    return new_null();
}

static value_t *operator_variables(argument_iterator_t *arguments, stack_frame_t *frame)
{
    return list_map_keys(frame->variables);
}

static value_t *operator_operators(argument_iterator_t *arguments, stack_frame_t *frame)
{
    return list_map_keys(frame->operators);
}

static value_t *operator_catch(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, TYPES_ANY))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return solo->type == TYPE_ERROR ? new_number(view_error(solo)) : new_null();
}

static value_t *operator_throw(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return new_error(view_number(solo));
}

static value_t *operator_add(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPES_NONNULL))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPES_NONNULL))
    {
        return arguments->value;
    }

    right = arguments->value;

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

static value_t *operator_subtract(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(view_number(left) - view_number(right));
}

static value_t *operator_multiply(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(view_number(left) * view_number(right));
}

static value_t *operator_divide(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    if (view_number(right) == 0)
    {
        return new_error(ERROR_ARITHMETIC);
    }

    return new_number(div(view_number(left), view_number(right)).quot);
}

static value_t *operator_modulo(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    if (view_number(right) == 0)
    {
        return new_error(ERROR_ARITHMETIC);
    }

    return new_number(div(view_number(left), view_number(right)).rem);
}

static value_t *operator_and(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(view_number(left) && view_number(right));
}

static value_t *operator_or(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(view_number(left) || view_number(right));
}

static value_t *operator_not(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
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

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    condition = arguments->value;
    first = view_number(condition);

    if (!first)
    {
        skip_argument(arguments);
    }

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    if (first && !has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
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

        if (!next_argument(arguments, frame, TYPE_NUMBER))
        {
            return arguments->value;
        }

        condition = arguments->value;
        proceed = view_number(condition);

        if (proceed)
        {
            if (!next_argument(arguments, frame, TYPES_NONERROR))
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
        if (!next_argument(arguments, frame, TYPES_NONERROR))
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

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(compare_values(left, right) < 0);
}

static value_t *operator_greater(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(compare_values(left, right) > 0);
}

static value_t *operator_equal(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *left, *right;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    left = arguments->value;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    right = arguments->value;

    return new_number(compare_values(left, right) == 0);
}

static value_t *operator_sort(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection, *reversed;
    value_t **items;
    size_t length;

    if (!next_argument(arguments, frame, TYPE_LIST))
    {
        return arguments->value;
    }

    collection = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    reversed = arguments->value;

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
                destroy_items(items, index);
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

static value_t *operator_type(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    solo = arguments->value;

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

static value_t *operator_number(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    solo = arguments->value;

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

static value_t *operator_string(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    solo = arguments->value;

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

static value_t *operator_hash(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return new_number(hash_value(solo));
}

static value_t *operator_represent(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;
    char *represent;

    if (!next_argument(arguments, frame, TYPES_NONERROR))
    {
        return arguments->value;
    }

    solo = arguments->value;
    represent = represent_value(solo);

    if (!represent)
    {
        return NULL;
    }

    return steal_string(represent, strlen(represent) + 1);
}

static value_t *operator_length(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, TYPE_STRING | TYPE_LIST))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return new_number(length_value(solo));
}

static value_t *operator_index(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection, *index;
    int adjusted;

    if (!next_argument(arguments, frame, TYPE_STRING | TYPE_LIST))
    {
        return arguments->value;
    }

    collection = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    index = arguments->value;
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

static value_t *operator_range(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection, *start, *end;
    int adjustedstart, adjustedend;
    size_t limit, length;

    if (!next_argument(arguments, frame, TYPE_STRING | TYPE_LIST))
    {
        return arguments->value;
    }

    collection = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    start = arguments->value;

    if (!next_argument(arguments, frame, TYPE_NUMBER))
    {
        return arguments->value;
    }

    end = arguments->value;
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

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_read(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *path;
    char *file;
    size_t length, index;

    if (!next_argument(arguments, frame, TYPE_STRING))
    {
        return arguments->value;
    }

    path = arguments->value;
    file = read_file(view_string(path));

    if (!file)
    {
        return new_error(ERROR_IO);
    }

    length = strlen(file);

    for (index = 0; index < length; index++)
    {
        char symbol;

        symbol = file[index];

        if (!(isprint((unsigned char) symbol) || symbol == '\t' || symbol == '\n' || symbol == '\r'))
        {
            free(file);
            return new_error(ERROR_TYPE);
        }
    }

    return steal_string(file, sizeof(char) * (length + 1));
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

        qsort(items, length, sizeof(value_t *), compare_values_ascending);
    }
    else
    {
        items = NULL;
    }

    return new_list(items, length);
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
        arguments->value = new_error(ERROR_ARGUMENT);
        return 0;
    }

    result = apply_expression(arguments->current->value, frame);
    arguments->current = arguments->current->next;
    arguments->evaluated[arguments->index] = result;
    arguments->index += 1;

    if (!result)
    {
        arguments->value = NULL;
        return 0;
    }

    if (result->type == TYPE_ERROR && !(types & TYPE_ERROR))
    {
        arguments->value = copy_value(result);
        return 0;
    }

    if (!(types & result->type))
    {
        arguments->value = new_error(ERROR_ARGUMENT);
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
}

static int compare_values_ascending(const void *left, const void *right)
{
    return compare_values(*(value_t **) left, *(value_t **) right);
}

static int compare_values_descending(const void *left, const void *right)
{
    return compare_values(*(value_t **) left, *(value_t **) right) * -1;
}
