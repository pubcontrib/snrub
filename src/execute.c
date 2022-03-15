#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "execute.h"
#include "parse.h"
#include "lex.h"
#include "value.h"
#include "map.h"
#include "list.h"
#include "common.h"

#define VALUE_TYPES_ANY (VALUE_TYPE_NULL | VALUE_TYPE_NUMBER | VALUE_TYPE_STRING | VALUE_TYPE_LIST | VALUE_TYPE_MAP)
#define VALUE_TYPES_NONNULL (VALUE_TYPE_NUMBER | VALUE_TYPE_STRING | VALUE_TYPE_LIST | VALUE_TYPE_MAP)
#define VALUE_TYPES_COLLECTION (VALUE_TYPE_STRING | VALUE_TYPE_LIST | VALUE_TYPE_MAP)

typedef struct
{
    list_t *expressions;
    list_node_t *current;
    value_t **evaluated;
    size_t index;
    value_t *value;
    int interception;
} argument_iterator_t;

typedef struct stack_frame_t
{
    map_t *variables;
    map_t *overloads;
    map_t *operators;
    int depth;
    struct stack_frame_t *caller;
} stack_frame_t;

typedef struct
{
    value_t *(*call)(argument_iterator_t *, stack_frame_t *);
} operator_t;

static value_t *evaluate_script(string_t *document, value_t *arguments, stack_frame_t *caller);
static value_t *evaluate_expressions(list_t *expressions, value_t *arguments, stack_frame_t *caller);
static value_t *apply_expression(expression_t *expression, stack_frame_t *frame);
static value_t *apply_list(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *apply_map(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *apply_call(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_recall(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_memorize(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_forget(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_get(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_set(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_unset(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_read(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_write(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_remove(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_add(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_subtract(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_multiply(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_divide(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_modulo(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_and(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_or(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_not(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_less(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_greater(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_equal(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_conditional(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_loop(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_chain(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_catch(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_throw(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_type(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_number(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_string(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_length(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_overload(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_ripoff(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_mime(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_evaluate(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_variables(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_keys(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_operators(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_sort(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_slice(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_hash(argument_iterator_t *arguments, stack_frame_t *frame);
static value_t *operator_represent(argument_iterator_t *arguments, stack_frame_t *frame);
static map_t *default_operators(void);
static value_t *set_overload(map_t *overloads, string_t *identifier, list_t *overload);
static void set_operator(map_t *operators, char *name, value_t *(*call)(argument_iterator_t *, stack_frame_t *));
static value_t *set_variable(map_t *variables, string_t *identifier, value_t *variable);
static int has_next_argument(argument_iterator_t *arguments);
static int next_argument(argument_iterator_t *arguments, stack_frame_t *frame, int types);
static void skip_argument(argument_iterator_t *arguments);
static void reset_arguments(argument_iterator_t *arguments);
static value_t *list_map_keys(map_t *map);
static void sort_collection(value_t *collection, int reversed);
static list_t *copy_expressions(list_t *this);
static expression_t *copy_expression(expression_t *this);
static int compare_values_ascending(const void *left, const void *right);
static int compare_values_descending(const void *left, const void *right);
static void destroy_value_unsafe(void *value);
static void destroy_list_unsafe(void *value);
static void destroy_expression_unsafe(void *expression);

map_t *empty_overloads(void)
{
    return empty_map(hash_string, destroy_list_unsafe, 64);
}

map_t *empty_variables(void)
{
    return empty_map(hash_string, destroy_value_unsafe, 64);
}

value_t *execute_script(string_t *document, value_t *arguments, map_t *variables, map_t *overloads)
{
    stack_frame_t frame;
    value_t *value;

    frame.variables = variables;
    frame.overloads = overloads;
    frame.operators = default_operators();
    frame.depth = 0;
    frame.caller = NULL;

    value = evaluate_script(document, arguments, &frame);

    destroy_map(frame.operators);

    return value;
}

static value_t *evaluate_script(string_t *document, value_t *arguments, stack_frame_t *caller)
{
    scanner_t *scanner;
    list_t *expressions;
    value_t *value;

    scanner = start_scanner(document);
    expressions = parse_expressions(scanner);
    destroy_scanner(scanner);
    value = evaluate_expressions(expressions, arguments, caller);
    destroy_list(expressions);

    return value;
}

static value_t *evaluate_expressions(list_t *expressions, value_t *arguments, stack_frame_t *caller)
{
    list_node_t *node;
    stack_frame_t frame;
    value_t *last;

    if (caller->depth + 1 > LIMIT_DEPTH)
    {
        return throw_error(ERROR_BOUNDS);
    }

    for (node = expressions->head; node != NULL; node = node->next)
    {
        expression_t *expression;

        expression = node->value;

        if (expression->type == EXPRESSION_TYPE_VALUE && expression->value->thrown)
        {
            return copy_value(expression->value);
        }
    }

    frame.variables = empty_variables();
    frame.overloads = empty_overloads();
    frame.operators = caller->operators;
    frame.depth = caller->depth + 1;
    frame.caller = caller;

    last = set_variable(frame.variables, cstring_to_string("@"), arguments);

    if (last->type == VALUE_TYPE_NULL)
    {
        for (node = expressions->head; node != NULL; node = node->next)
        {
            expression_t *expression;

            expression = node->value;

            if (expression->type != EXPRESSION_TYPE_UNSET)
            {
                value_t *value;

                value = apply_expression(expression, &frame);
                destroy_value(last);
                last = value;

                if (last->thrown)
                {
                    break;
                }
            }
        }
    }

    destroy_map(frame.variables);
    destroy_map(frame.overloads);

    return last;
}

static value_t *apply_expression(expression_t *expression, stack_frame_t *frame)
{
    argument_iterator_t arguments;
    value_t *result;

    arguments.expressions = expression->arguments;
    arguments.index = 0;

    if (arguments.expressions && arguments.expressions->length > 0)
    {
        arguments.current = expression->arguments->head;
        arguments.evaluated = allocate(sizeof(value_t *) * arguments.expressions->length);
    }
    else
    {
        arguments.evaluated = NULL;
    }

    switch (expression->type)
    {
        case EXPRESSION_TYPE_VALUE:
            result = copy_value(expression->value);
            break;
        case EXPRESSION_TYPE_LIST:
            result = apply_list(&arguments, frame);
            break;
        case EXPRESSION_TYPE_MAP:
            result = apply_map(&arguments, frame);
            break;
        case EXPRESSION_TYPE_CALL:
            result = apply_call(&arguments, frame);
            break;
        default:
            crash_with_message("unsupported branch %s", "EXECUTE_EXPRESSION_TYPE");
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

        items = allocate(sizeof(value_t *) * length);

        for (index = 0; index < length; index++)
        {
            value_t *copy;

            if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
            {
                destroy_items(items, index);
                return arguments->value;
            }

            copy = copy_value(arguments->value);
            items[index] = copy;
        }
    }
    else
    {
        items = NULL;
    }

    return new_list(items, length);
}

static value_t *apply_map(argument_iterator_t *arguments, stack_frame_t *frame)
{
    map_t *pairs;
    size_t length, index;

    length = arguments->expressions->length;
    pairs = empty_map(hash_string, destroy_value_unsafe, 8);

    for (index = 0; index < length; index += 2)
    {
        value_t *key, *value;

        if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
        {
            destroy_map(pairs);
            return arguments->value;
        }

        key = arguments->value;

        if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
        {
            destroy_map(pairs);
            return arguments->value;
        }

        value = arguments->value;

        set_map_item(pairs, copy_string(view_string(key)), copy_value(value));
    }

    return new_map(pairs);
}

static value_t *apply_call(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *name;
    operator_t *operator;
    list_t *overload;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    name = arguments->value;
    overload = get_map_item(frame->overloads, view_string(name));

    if (overload)
    {
        value_t *initial;

        if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
        {
            return arguments->value;
        }

        initial = arguments->value;

        return evaluate_expressions(overload, initial, frame);
    }

    operator = get_map_item(frame->operators, view_string(name));

    if (!operator)
    {
        return throw_error(ERROR_ARGUMENT);
    }

    return operator->call(arguments, frame);
}

static value_t *operator_recall(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier, *value;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;
    value = get_map_item(frame->variables, view_string(identifier));

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

    return set_variable(frame->variables, copy_string(view_string(identifier)), value);
}

static value_t *operator_forget(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;
    remove_map_item(frame->variables, view_string(identifier));

    return new_null();
}

static value_t *operator_get(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection;

    if (!next_argument(arguments, frame, VALUE_TYPES_COLLECTION))
    {
        return arguments->value;
    }

    collection = arguments->value;

    switch (collection->type)
    {
        case VALUE_TYPE_STRING:
        {
            value_t *index;
            char *bytes;
            int adjusted;

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

            bytes = allocate(sizeof(char));
            bytes[0] = view_string(collection)->bytes[adjusted];

            return new_string(create_string(bytes, 1));
        }
        case VALUE_TYPE_LIST:
        {
            value_t *index;
            int adjusted;

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

            return copy_value(((value_t **) collection->data)[adjusted]);
        }
        case VALUE_TYPE_MAP:
        {
            value_t *key, *value;

            if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
            {
                return arguments->value;
            }

            key = arguments->value;
            value = get_map_item(collection->data, view_string(key));

            return value ? copy_value(value) : new_null();
        }
        default:
            return throw_error(ERROR_ARGUMENT);
    }
}

static value_t *operator_set(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection;

    if (!next_argument(arguments, frame, VALUE_TYPES_COLLECTION))
    {
        return arguments->value;
    }

    collection = arguments->value;

    switch (collection->type)
    {
        case VALUE_TYPE_STRING:
        {
            value_t *value, *index;
            char *bytes;
            int adjusted;
            size_t length, collectionLength, valueLength;

            if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
            {
                return arguments->value;
            }

            index = arguments->value;
            adjusted = view_number(index) - 1;

            if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
            {
                return arguments->value;
            }

            value = arguments->value;
            valueLength = length_value(value);
            collectionLength = length_value(collection);

            if (adjusted < 0 || adjusted >= collectionLength)
            {
                return copy_value(collection);
            }

            length = collectionLength + valueLength - 1;

            if (length > 0)
            {
                bytes = allocate(sizeof(char) * length);
                memcpy(bytes, view_string(collection)->bytes, adjusted);
                memcpy(bytes + adjusted, view_string(value)->bytes, valueLength);
                memcpy(bytes + adjusted + valueLength, view_string(collection)->bytes + adjusted + 1, collectionLength - adjusted - 1);
            }
            else
            {
                bytes = NULL;
            }

            return new_string(create_string(bytes, length));
        }
        case VALUE_TYPE_LIST:
        {
            value_t *index, *value, *item;
            value_t **items;
            int adjusted;
            size_t length, left, right;

            if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
            {
                return arguments->value;
            }

            index = arguments->value;
            adjusted = view_number(index) - 1;
            length = length_value(collection);

            if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
            {
                return arguments->value;
            }

            value = arguments->value;

            if (adjusted < 0 || adjusted >= length)
            {
                return copy_value(collection);
            }

            items = allocate(sizeof(value_t *) * length);

            for (left = 0, right = 0; right < adjusted; left++, right++)
            {
                item = copy_value(((value_t **) collection->data)[right]);
                items[left] = item;
            }

            item = copy_value(value);
            items[left++] = item;

            for (right = adjusted + 1; right < length; left++, right++)
            {
                item = copy_value(((value_t **) collection->data)[right]);
                items[left] = item;
            }

            return new_list(items, length);
        }
        case VALUE_TYPE_MAP:
        {
            value_t *key, *value, *copy;

            if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
            {
                return arguments->value;
            }

            key = arguments->value;

            if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
            {
                return arguments->value;
            }

            value = arguments->value;
            copy = copy_value(collection);

            remove_map_item(copy->data, view_string(key));
            set_map_item(copy->data, copy_string(view_string(key)), copy_value(value));

            return copy;
        }
        default:
            return throw_error(ERROR_ARGUMENT);
    }
}

static value_t *operator_unset(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *collection;

    if (!next_argument(arguments, frame, VALUE_TYPES_COLLECTION))
    {
        return arguments->value;
    }

    collection = arguments->value;

    switch (collection->type)
    {
        case VALUE_TYPE_STRING:
        {
            value_t *index;
            char *bytes;
            int adjusted;
            size_t length;

            if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
            {
                return arguments->value;
            }

            index = arguments->value;
            adjusted = view_number(index) - 1;
            length = length_value(collection);

            if (adjusted < 0 || adjusted >= length)
            {
                return copy_value(collection);
            }

            if (length > 1)
            {
                bytes = allocate(sizeof(char) * length - 1);
                memcpy(bytes, view_string(collection)->bytes, adjusted);
                memcpy(bytes + adjusted, view_string(collection)->bytes + adjusted + 1, length - adjusted - 1);
            }
            else
            {
                bytes = NULL;
            }

            return new_string(create_string(bytes, length - 1));
        }
        case VALUE_TYPE_LIST:
        {
            value_t *index, *item;
            value_t **items;
            int adjusted;
            size_t length, left, right;

            if (!next_argument(arguments, frame, VALUE_TYPE_NUMBER))
            {
                return arguments->value;
            }

            index = arguments->value;
            adjusted = view_number(index) - 1;
            length = length_value(collection);

            if (adjusted < 0 || adjusted >= length)
            {
                return copy_value(collection);
            }

            if (length > 1)
            {
                items = allocate(sizeof(value_t *) * (length - 1));

                for (left = 0, right = 0; right < length; right++)
                {
                    if (right != adjusted)
                    {
                        item = copy_value(((value_t **) collection->data)[right]);
                        items[left++] = item;
                    }
                }
            }
            else
            {
                items = NULL;
            }

            return new_list(items, length - 1);
        }
        case VALUE_TYPE_MAP:
        {
            value_t *key, *copy;

            if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
            {
                return arguments->value;
            }

            key = arguments->value;
            copy = copy_value(collection);

            remove_map_item(copy->data, view_string(key));

            return copy;
        }
        default:
            return throw_error(ERROR_ARGUMENT);
    }
}

static value_t *operator_read(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *path;
    string_t *file;

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

    return new_string(file);
}

static value_t *operator_write(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *path, *text;
    char *cPath;
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
    cPath = string_to_cstring(view_string(path));
    file = fopen(cPath, "wb");
    free(cPath);

    if (file)
    {
        string_t *string;

        string = view_string(text);
        fwrite(string->bytes, sizeof(char), string->length, file);
        fclose(file);
    }

    return new_null();
}

static value_t *operator_remove(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *path;
    char *cPath;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    path = arguments->value;
    cPath = string_to_cstring(view_string(path));
    remove(cPath);
    free(cPath);

    return new_null();
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

    switch (left->type)
    {
        case VALUE_TYPE_NUMBER:
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
        case VALUE_TYPE_STRING:
        {
            string_t *sum;

            if (string_add(view_string(left), view_string(right), &sum))
            {
                return new_string(sum);
            }
            else
            {
                return throw_error(ERROR_BOUNDS);
            }
        }
        case VALUE_TYPE_LIST:
            return merge_lists(left, right);
        case VALUE_TYPE_MAP:
            return merge_maps(left, right);
        default:
            return throw_error(ERROR_ARGUMENT);
    }
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
    size_t count;

    count = 0;

    do
    {
        if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
        {
            return arguments->value;
        }

        last = arguments->value;
        count++;
    } while (has_next_argument(arguments));

    if (count < 2)
    {
        return throw_error(ERROR_ARGUMENT);
    }

    return copy_value(last);
}

static value_t *operator_catch(argument_iterator_t *arguments, stack_frame_t *frame)
{
    if (!next_argument(arguments, frame, VALUE_TYPES_ANY))
    {
        value_t *solo;

        solo = arguments->value;

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
    copy->thrown = 1;

    return copy;
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
            return new_string(cstring_to_string("?"));
        case VALUE_TYPE_NUMBER:
            return new_string(cstring_to_string("##"));
        case VALUE_TYPE_STRING:
            return new_string(cstring_to_string("\"\""));
        case VALUE_TYPE_LIST:
            return new_string(cstring_to_string("[]"));
        case VALUE_TYPE_MAP:
            return new_string(cstring_to_string("{}"));
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

    switch (solo->type)
    {
        case VALUE_TYPE_NULL:
        case VALUE_TYPE_NUMBER:
            return copy_value(solo);
        case VALUE_TYPE_STRING:
        {
            int out;

            return string_to_integer(view_string(solo), NUMBER_DIGIT_CAPACITY, &out) ? new_number(out) : throw_error(ERROR_TYPE);
        }
        default:
            return throw_error(ERROR_ARGUMENT);
    }
}

static value_t *operator_string(argument_iterator_t *arguments, stack_frame_t *frame)
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
        case VALUE_TYPE_STRING:
            return copy_value(solo);
        case VALUE_TYPE_NUMBER:
            return new_string(integer_to_string(view_number(solo)));
        default:
            return throw_error(ERROR_ARGUMENT);
    }
}

static value_t *operator_length(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo;

    if (!next_argument(arguments, frame, VALUE_TYPES_COLLECTION))
    {
        return arguments->value;
    }

    solo = arguments->value;

    return new_number(length_value(solo));
}

static value_t *operator_overload(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier, *document, *result;
    scanner_t *scanner;
    list_t *expressions;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    document = arguments->value;

    scanner = start_scanner(copy_string(view_string(document)));
    expressions = parse_expressions(scanner);
    destroy_scanner(scanner);
    result = set_overload(frame->overloads, copy_string(view_string(identifier)), expressions);
    destroy_list(expressions);

    return result;
}

static value_t *operator_ripoff(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier;
    stack_frame_t *level;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;
    level = frame->caller;

    while (level != NULL)
    {
        value_t *variable;

        variable = get_map_item(level->variables, view_string(identifier));

        if (variable == NULL)
        {
            level = level->caller;
        }
        else
        {
            return set_variable(frame->variables, copy_string(view_string(identifier)), variable);
        }
    }

    return new_null();
}

static value_t *operator_mime(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *identifier;
    stack_frame_t *level;

    if (!next_argument(arguments, frame, VALUE_TYPE_STRING))
    {
        return arguments->value;
    }

    identifier = arguments->value;
    level = frame->caller;

    while (level != NULL)
    {
        list_t *overload;

        overload = get_map_item(level->overloads, view_string(identifier));

        if (overload == NULL)
        {
            level = level->caller;
        }
        else
        {
            return set_overload(frame->overloads, copy_string(view_string(identifier)), overload);
        }
    }

    return new_null();
}

static value_t *operator_evaluate(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *document, *initial;
    string_t *copy;

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

    return evaluate_script(copy, initial, frame);
}

static value_t *operator_variables(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *sorted;

    sorted = list_map_keys(frame->variables);
    sort_collection(sorted, 0);

    return sorted;
}

static value_t *operator_keys(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *solo, *sorted;

    if (!next_argument(arguments, frame, VALUE_TYPE_MAP))
    {
        return arguments->value;
    }

    solo = arguments->value;
    sorted = list_map_keys(solo->data);
    sort_collection(sorted, 0);

    return sorted;
}

static value_t *operator_operators(argument_iterator_t *arguments, stack_frame_t *frame)
{
    value_t *sorted;

    sorted = list_map_keys(frame->operators);
    sort_collection(sorted, 0);

    return sorted;
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
    sort_collection(sorted, view_number(reversed));

    return sorted;
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

    switch (collection->type)
    {
        case VALUE_TYPE_STRING:
        {
            string_t *slice;

            slice = slice_string(view_string(collection), adjustedStart, adjustedEnd);

            return new_string(slice);
        }
        case VALUE_TYPE_LIST:
        {
            value_t **items;

            if (length > 0)
            {
                size_t index, placement;

                items = allocate(sizeof(value_t *) * length);

                for (index = adjustedStart, placement = 0; index < adjustedEnd; index++, placement++)
                {
                    value_t *item;

                    item = copy_value(((value_t **) collection->data)[index]);
                    items[placement] = item;
                }
            }
            else
            {
                items = NULL;
            }

            return new_list(items, length);
        }
        default:
            return throw_error(ERROR_ARGUMENT);
    }
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

static map_t *default_operators(void)
{
    map_t *operators;

    operators = empty_map(hash_string, free, 64);

    set_operator(operators, "x->", operator_recall);
    set_operator(operators, "x<-", operator_memorize);
    set_operator(operators, "x--", operator_forget);
    set_operator(operators, "$->", operator_get);
    set_operator(operators, "$<-", operator_set);
    set_operator(operators, "$--", operator_unset);
    set_operator(operators, "[o]->", operator_read);
    set_operator(operators, "[o]<-", operator_write);
    set_operator(operators, "[o]--", operator_remove);
    set_operator(operators, "+", operator_add);
    set_operator(operators, "-", operator_subtract);
    set_operator(operators, "*", operator_multiply);
    set_operator(operators, "/", operator_divide);
    set_operator(operators, "%", operator_modulo);
    set_operator(operators, "&", operator_and);
    set_operator(operators, "|", operator_or);
    set_operator(operators, "!", operator_not);
    set_operator(operators, "<", operator_less);
    set_operator(operators, ">", operator_greater);
    set_operator(operators, "=", operator_equal);
    set_operator(operators, "?", operator_conditional);
    set_operator(operators, "o", operator_loop);
    set_operator(operators, "...", operator_chain);
    set_operator(operators, "><", operator_catch);
    set_operator(operators, "<>", operator_throw);
    set_operator(operators, "_", operator_type);
    set_operator(operators, "#", operator_number);
    set_operator(operators, "\"", operator_string);
    set_operator(operators, "| |", operator_length);
    set_operator(operators, "()<-", operator_overload);
    set_operator(operators, "x^", operator_ripoff);
    set_operator(operators, "()^", operator_mime);
    set_operator(operators, "~", operator_evaluate);
    set_operator(operators, "x[]", operator_variables);
    set_operator(operators, "$[]", operator_keys);
    set_operator(operators, "()[]", operator_operators);
    set_operator(operators, "<|>", operator_sort);
    set_operator(operators, "[# #]", operator_slice);
    set_operator(operators, "::", operator_hash);
    set_operator(operators, ";", operator_represent);

    return operators;
}

static value_t *set_overload(map_t *overloads, string_t *identifier, list_t *overload)
{
    int exists;

    exists = has_map_item(overloads, identifier);

    if (!exists && overloads->length >= NUMBER_MAX)
    {
        return throw_error(ERROR_BOUNDS);
    }

    set_map_item(overloads, identifier, copy_expressions(overload));

    return new_null();
}

static void set_operator(map_t *operators, char *name, value_t *(*call)(argument_iterator_t *, stack_frame_t *))
{
    string_t *key;
    operator_t *operator;

    key = cstring_to_string(name);
    operator = allocate(sizeof(operator_t *));
    operator->call = call;

    set_map_item(operators, key, operator);
}

static value_t *set_variable(map_t *variables, string_t *identifier, value_t *variable)
{
    int exists;

    exists = has_map_item(variables, identifier);

    if (!exists && variables->length >= NUMBER_MAX)
    {
        destroy_string(identifier);
        return throw_error(ERROR_BOUNDS);
    }

    set_map_item(variables, identifier, copy_value(variable));

    return new_null();
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

        items = allocate(sizeof(value_t *) * length);

        for (index = 0, placement = 0; index < map->capacity; index++)
        {
            if (map->chains[index])
            {
                map_chain_t *chain;

                for (chain = map->chains[index]; chain != NULL; chain = chain->next)
                {
                    value_t *item;

                    item = new_string(copy_string(chain->key));
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

static list_t *copy_expressions(list_t *this)
{
    list_t *copy;
    list_node_t *node;

    copy = empty_list(destroy_expression_unsafe);

    for (node = this->head; node != NULL; node = node->next)
    {
        add_list_item(copy, copy_expression(node->value));
    }

    return copy;
}

static expression_t *copy_expression(expression_t *this)
{
    expression_t *copy;

    copy = allocate(sizeof(expression_t));
    copy->type = this->type;

    if (this->value)
    {
        copy->value = copy_value(this->value);
    }
    else
    {
        copy->value = NULL;
    }

    if (this->arguments)
    {
        list_node_t *node;

        copy->arguments = empty_list(destroy_expression_unsafe);

        for (node = this->arguments->head; node != NULL; node = node->next)
        {
            add_list_item(copy->arguments, copy_expression(node->value));
        }
    }
    else
    {
        copy->arguments = NULL;
    }

    return copy;
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

static void destroy_list_unsafe(void *value)
{
    destroy_list((list_t *) value);
}

static void destroy_expression_unsafe(void *expression)
{
    destroy_expression((expression_t *) expression);
}
