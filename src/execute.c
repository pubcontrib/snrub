#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "parse.h"
#include "value.h"
#include "common.h"

typedef struct
{
    expression_t **candidates;
    value_t **evaluated;
    size_t length;
    size_t index;
} argument_iterator_t;

static variable_list_t *create_variable_list(char *identifier, value_t *value, variable_list_t *next);
static variable_map_t *create_variable_map(variable_list_t **lists, size_t length, size_t capacity);
static value_t *apply_expression(expression_t *expression, variable_map_t *variables);
static value_t *apply_list(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *apply_call(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_value(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_assign(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_roster(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_catch(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_add(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_subtract(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_multiply(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_divide(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_modulo(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_and(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_or(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_not(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_conditional(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_loop(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_chain(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_less(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_greater(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_equal(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_sort(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_type(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_number(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_string(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_hash(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_length(argument_iterator_t *arguments, variable_map_t *variables);
static value_t *operator_index(argument_iterator_t *arguments, variable_map_t *variables);
static int has_next_argument(argument_iterator_t *iterator);
static value_t *next_argument(argument_iterator_t *iterator, variable_map_t *variables);
static void skip_argument(argument_iterator_t *iterator);
static void rewind_argument(argument_iterator_t *iterator);
static int set_variable(variable_map_t *map, char *identifier, value_t *value);
static value_t *get_variable(variable_map_t *map, char *identifier);
static int remove_variable(variable_map_t *map, char *identifier);
static int resize_variable_map(variable_map_t *map);
static int compare_values_unsafe(const void *left, const void *right);

variable_map_t *empty_variable_map()
{
    variable_list_t **lists;
    size_t capacity;

    capacity = 64;
    lists = calloc(capacity, sizeof(variable_map_t *));

    return create_variable_map(lists, 0, capacity);
}

value_t *execute_expression(expression_t *expressions, variable_map_t *variables)
{
    expression_t *expression;
    value_t *last;

    for (expression = expressions; expression != NULL; expression = expression->next)
    {
        if (expression->value->type == TYPE_ERROR)
        {
            return copy_value(expression->value);
        }
    }

    last = new_null();

    for (expression = expressions; expression != NULL; expression = expression->next)
    {
        value_t *value;

        value = apply_expression(expression, variables);

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

    return last;
}

void destroy_variable_map(variable_map_t *map)
{
    size_t index;

    for (index = 0; index < map->capacity; index++)
    {
        variable_list_t *list;

        list = map->lists[index];

        if (list)
        {
            destroy_variable_list(list);
        }
    }

    free(map->lists);
    free(map);
}

void destroy_variable_list(variable_list_t *list)
{
    if (list->identifier)
    {
        free(list->identifier);
    }

    if (list->value)
    {
        destroy_value(list->value);
    }

    if (list->next)
    {
        destroy_variable_list(list->next);
    }

    free(list);
}

static variable_list_t *create_variable_list(char *identifier, value_t *value, variable_list_t *next)
{
    variable_list_t *list;

    list = malloc(sizeof(variable_list_t));

    if (list)
    {
        list->identifier = identifier;
        list->value = value;
        list->next = next;
    }

    return list;
}

static variable_map_t *create_variable_map(variable_list_t **lists, size_t length, size_t capacity)
{
    variable_map_t *map;

    map = malloc(sizeof(variable_map_t));

    if (map)
    {
        map->lists = lists;
        map->length = length;
        map->capacity = capacity;
    }

    return map;
}

static value_t *apply_expression(expression_t *expression, variable_map_t *variables)
{
    argument_iterator_t *arguments;
    value_t *result;
    size_t index;

    arguments = malloc(sizeof(argument_iterator_t));

    if (!arguments)
    {
        return NULL;
    }

    arguments->candidates = expression->arguments;
    arguments->length = expression->length;
    arguments->index = 0;

    if (expression->length > 0)
    {
        arguments->evaluated = malloc(sizeof(value_t *) * expression->length);

        if (!arguments->evaluated)
        {
            return NULL;
        }
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
            result = apply_list(arguments, variables);
            break;
        case TYPE_CALL:
            result = apply_call(arguments, variables);
            break;
        default:
            result = new_error(ERROR_UNSUPPORTED);
            break;
    }

    if (arguments->length > 0)
    {
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

static value_t *apply_list(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *item, **items;
    size_t length, index;

    length = arguments->length;
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

        item = next_argument(arguments, variables);

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

    return new_list(items, length);
}

static value_t *apply_call(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *operator;
    char *name;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    operator = next_argument(arguments, variables);

    if (!operator)
    {
        return NULL;
    }

    if (operator->type == TYPE_ERROR)
    {
        return copy_value(operator);
    }

    if (operator->type != TYPE_STRING)
    {
        return new_error(ERROR_ARGUMENT);
    }

    name = view_string(operator);

    if (strcmp(name, "<--") == 0)
    {
        return operator_value(arguments, variables);
    }
    else if (strcmp(name, "-->") == 0)
    {
        return operator_assign(arguments, variables);
    }
    else if (strcmp(name, "---") == 0)
    {
        return operator_roster(arguments, variables);
    }
    else if (strcmp(name, "><") == 0)
    {
        return operator_catch(arguments, variables);
    }
    else if (strcmp(name, "+") == 0)
    {
        return operator_add(arguments, variables);
    }
    else if (strcmp(name, "-") == 0)
    {
        return operator_subtract(arguments, variables);
    }
    else if (strcmp(name, "*") == 0)
    {
        return operator_multiply(arguments, variables);
    }
    else if (strcmp(name, "/") == 0)
    {
        return operator_divide(arguments, variables);
    }
    else if (strcmp(name, "%") == 0)
    {
        return operator_modulo(arguments, variables);
    }
    else if (strcmp(name, "&") == 0)
    {
        return operator_and(arguments, variables);
    }
    else if (strcmp(name, "|") == 0)
    {
        return operator_or(arguments, variables);
    }
    else if (strcmp(name, "!") == 0)
    {
        return operator_not(arguments, variables);
    }
    else if (strcmp(name, "?") == 0)
    {
        return operator_conditional(arguments, variables);
    }
    else if (strcmp(name, "o") == 0)
    {
        return operator_loop(arguments, variables);
    }
    else if (strcmp(name, "...") == 0)
    {
        return operator_chain(arguments, variables);
    }
    else if (strcmp(name, "<") == 0)
    {
        return operator_less(arguments, variables);
    }
    else if (strcmp(name, ">") == 0)
    {
        return operator_greater(arguments, variables);
    }
    else if (strcmp(name, "=") == 0)
    {
        return operator_equal(arguments, variables);
    }
    else if (strcmp(name, "<|>") == 0)
    {
        return operator_sort(arguments, variables);
    }
    else if (strcmp(name, "_") == 0)
    {
        return operator_type(arguments, variables);
    }
    else if (strcmp(name, "#") == 0)
    {
        return operator_number(arguments, variables);
    }
    else if (strcmp(name, "\"") == 0)
    {
        return operator_string(arguments, variables);
    }
    else if (strcmp(name, "::") == 0)
    {
        return operator_hash(arguments, variables);
    }
    else if (strcmp(name, "| |") == 0)
    {
        return operator_length(arguments, variables);
    }
    else if (strcmp(name, "[#]") == 0)
    {
        return operator_index(arguments, variables);
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_value(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *identifier, *value;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    identifier = next_argument(arguments, variables);

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

    value = get_variable(variables, view_string(identifier));

    return value ? copy_value(value) : new_null();
}

static value_t *operator_assign(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *identifier, *value;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    identifier = next_argument(arguments, variables);

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

    value = next_argument(arguments, variables);

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
        remove_variable(variables, view_string(identifier));
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

        if (set_variable(variables, name, copy) == -1)
        {
            return NULL;
        }

        if (resize_variable_map(variables) == -1)
        {
            return NULL;
        }
    }

    return new_null();
}

static value_t *operator_roster(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t **items;
    size_t length, index, placement;

    length = variables->length;
    items = malloc(sizeof(value_t *) * length);

    if (!items)
    {
        return NULL;
    }

    for (index = 0, placement = 0; index < variables->capacity; index++)
    {
        if (variables->lists[index])
        {
            variable_list_t *list;

            for (list = variables->lists[index]; list != NULL; list = list->next)
            {
                value_t *item;

                item = new_string(list->identifier);

                if (!item)
                {
                    free(items);
                    return NULL;
                }

                items[placement++] = item;
            }
        }
    }

    qsort(items, length, sizeof(value_t *), compare_values_unsafe);

    return new_list(items, length);
}

static value_t *operator_catch(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_null();
    }

    solo = next_argument(arguments, variables);

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

static value_t *operator_add(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_subtract(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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

static value_t *operator_multiply(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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

static value_t *operator_divide(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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

static value_t *operator_modulo(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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

static value_t *operator_and(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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

static value_t *operator_or(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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

static value_t *operator_not(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables);

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

static value_t *operator_conditional(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *condition;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    condition = next_argument(arguments, variables);

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

        pass = next_argument(arguments, variables);

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

        fail = next_argument(arguments, variables);

        if (!fail)
        {
            return NULL;
        }

        return copy_value(fail);
    }
}

static value_t *operator_loop(argument_iterator_t *arguments, variable_map_t *variables)
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

        condition = next_argument(arguments, variables);

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

            pass = next_argument(arguments, variables);

            if (!pass)
            {
                return NULL;
            }

            if (pass->type == TYPE_ERROR)
            {
                return copy_value(pass);
            }

            rewind_argument(arguments);
            rewind_argument(arguments);
        }
    }

    return new_null();
}

static value_t *operator_chain(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *last;

    if (!has_next_argument(arguments))
    {
        return new_null();
    }

    while (has_next_argument(arguments))
    {
        last = next_argument(arguments, variables);

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

static value_t *operator_less(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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
        return new_number(0);
    }

    if (left->type == TYPE_LIST)
    {
        return new_error(ERROR_UNSUPPORTED);
    }

    return new_number(compare_values(left, right) < 0);
}

static value_t *operator_greater(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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
        return new_number(0);
    }

    if (left->type == TYPE_LIST)
    {
        return new_error(ERROR_UNSUPPORTED);
    }

    return new_number(compare_values(left, right) > 0);
}

static value_t *operator_equal(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, variables);

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

    right = next_argument(arguments, variables);

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

static value_t *operator_sort(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *solo;
    value_t **items;
    size_t length, index;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    if (solo->type != TYPE_LIST)
    {
        return new_error(ERROR_ARGUMENT);
    }

    length = solo->size;
    items = malloc(sizeof(value_t *) * length);

    if (!items)
    {
        return NULL;
    }

    for (index = 0; index < length; index++)
    {
        value_t *copy;

        copy = copy_value(((value_t **) solo->data)[index]);

        if (!copy)
        {
            free(items);
            return NULL;
        }

        items[index] = copy;
    }

    qsort(items, length, sizeof(value_t *), compare_values_unsafe);

    return new_list(items, length);
}

static value_t *operator_type(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables);

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

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_number(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables);

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

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_string(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables);

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

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_hash(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables);

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

static value_t *operator_length(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *solo;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    solo = next_argument(arguments, variables);

    if (!solo)
    {
        return NULL;
    }

    if (solo->type == TYPE_ERROR)
    {
        return copy_value(solo);
    }

    if (solo->type == TYPE_STRING)
    {
        return new_number(strlen(view_string(solo)));
    }

    if (solo->type == TYPE_LIST)
    {
        return new_number(solo->size);
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_index(argument_iterator_t *arguments, variable_map_t *variables)
{
    value_t *collection, *index;
    int adjusted;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    collection = next_argument(arguments, variables);

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

    index = next_argument(arguments, variables);

    if (!index)
    {
        return NULL;
    }

    if (index->type != TYPE_NUMBER)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (index->type == TYPE_ERROR)
    {
        return copy_value(collection);
    }

    adjusted = view_number(index) - 1;

    if (collection->type == TYPE_STRING)
    {
        char *string;
        size_t size;

        if (adjusted < 0 || adjusted >= strlen(view_string(collection)))
        {
            return new_null();
        }

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
        if (adjusted < 0 || adjusted >= collection->size)
        {
            return new_null();
        }

        return copy_value(((value_t **) collection->data)[adjusted]);
    }

    return new_error(ERROR_UNSUPPORTED);
}

static int has_next_argument(argument_iterator_t *iterator)
{
    return iterator->index < iterator->length;
}

static value_t *next_argument(argument_iterator_t *iterator, variable_map_t *variables)
{
    value_t *result;

    result = apply_expression(iterator->candidates[iterator->index], variables);
    iterator->evaluated[iterator->index] = result;
    iterator->index += 1;

    return result;
}

static void skip_argument(argument_iterator_t *iterator)
{
    iterator->evaluated[iterator->index] = NULL;
    iterator->index += 1;
}

static void rewind_argument(argument_iterator_t *iterator)
{
    iterator->index -= 1;
    destroy_value(iterator->evaluated[iterator->index]);
    iterator->evaluated[iterator->index] = NULL;
}

static int set_variable(variable_map_t *map, char *identifier, value_t *value)
{
    variable_list_t *list, *last, *created;
    int hash, index;

    hash = abs(hash_string(identifier));
    index = div(hash, map->capacity).rem;

    for (list = map->lists[index], last = NULL; list != NULL; list = list->next)
    {
        if (strcmp(identifier, list->identifier) == 0)
        {
            free(identifier);
            destroy_value(list->value);
            list->value = value;
            return 2;
        }

        last = list;
    }

    created = create_variable_list(identifier, value, NULL);

    if (!created)
    {
        return -1;
    }

    if (last)
    {
        last->next = created;
    }
    else
    {
        map->lists[index] = created;
    }

    map->length += 1;

    return 1;
}

static value_t *get_variable(variable_map_t *map, char *identifier)
{
    variable_list_t *list;
    int hash, index;

    hash = abs(hash_string(identifier));
    index = div(hash, map->capacity).rem;

    for (list = map->lists[index]; list != NULL; list = list->next)
    {
        if (strcmp(identifier, list->identifier) == 0)
        {
            return list->value;
        }
    }

    return NULL;
}

static int remove_variable(variable_map_t *map, char *identifier)
{
    variable_list_t *list, *previous;
    int hash, index;

    hash = abs(hash_string(identifier));
    index = div(hash, map->capacity).rem;

    for (list = map->lists[index], previous = NULL; list != NULL; list = list->next)
    {
        if (strcmp(identifier, list->identifier) == 0)
        {
            if (previous)
            {
                previous->next = list->next;
            }
            else
            {
                map->lists[index] = list->next;
            }

            list->next = NULL;
            destroy_variable_list(list);
            map->length -= 1;

            return 1;
        }
    }

    return 0;
}

static int resize_variable_map(variable_map_t *map)
{
    if (map->length == map->capacity)
    {
        variable_list_t **existing, **lists;
        variable_list_t *list;
        size_t fill, index;

        existing = map->lists;
        fill = map->capacity;
        map->capacity *= 2;
        lists = calloc(map->capacity, sizeof(variable_list_t *));

        if (!lists)
        {
            return -1;
        }

        map->length = 0;
        map->lists = lists;

        for (index = 0; index < fill; index++)
        {
            for (list = existing[index]; list != NULL; list = list->next)
            {
                if (set_variable(map, list->identifier, list->value) == -1)
                {
                    free(map->lists);
                    return -1;
                }

                list->identifier = NULL;
                list->value = NULL;
            }
        }

        for (index = 0; index < fill; index++)
        {
            list = existing[index];

            if (list)
            {
                destroy_variable_list(list);
            }
        }

        free(existing);

        return 1;
    }

    return 0;
}

static int compare_values_unsafe(const void *left, const void *right)
{
    return compare_values(*(value_t **) left, *(value_t **) right);
}
