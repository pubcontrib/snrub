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

static object_t *create_object(char *identifier, value_t *value, object_t *next);
static value_t *apply_expression(expression_t *expression, object_t *objects);
static value_t *apply_call(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_value(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_assign(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_catch(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_add(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_subtract(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_multiply(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_divide(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_modulo(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_and(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_or(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_not(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_conditional(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_loop(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_chain(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_less(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_greater(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_equal(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_type(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_number(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_string(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_hash(argument_iterator_t *arguments, object_t *objects);
static value_t *operator_length(argument_iterator_t *arguments, object_t *objects);
static int has_next_argument(argument_iterator_t *iterator);
static value_t *next_argument(argument_iterator_t *iterator, object_t *objects);
static void skip_argument(argument_iterator_t *iterator);
static void rewind_argument(argument_iterator_t *iterator);

object_t *empty_object()
{
    return create_object(NULL, NULL, NULL);
}

value_t *execute_expression(expression_t *expressions, object_t *objects)
{
    expression_t *expression;
    value_t *last;

    for (expression = expressions; expression != NULL; expression = expression->next)
    {
        if (expression->error != ERROR_UNSET)
        {
            return new_error(expression->error);
        }
    }

    last = new_null();

    for (expression = expressions; expression != NULL; expression = expression->next)
    {
        value_t *handoff;

        handoff = apply_expression(expression, objects);

        if (!handoff)
        {
            destroy_value(last);
            return NULL;
        }

        if (handoff->type == TYPE_UNSET)
        {
            destroy_value(handoff);
        }
        else
        {
            destroy_value(last);
            last = handoff;
        }

        if (last->type == TYPE_ERROR)
        {
            break;
        }
    }

    return last;
}

void destroy_object(object_t *object)
{
    if (object->identifier)
    {
        free(object->identifier);
    }

    if (object->value)
    {
        destroy_value(object->value);
    }

    if (object->next)
    {
        destroy_object(object->next);
    }

    free(object);
}

static object_t *create_object(char *identifier, value_t *value, object_t *next)
{
    object_t *object;

    object = malloc(sizeof(object_t));

    if (object)
    {
        object->identifier = identifier;
        object->value = value;
        object->next = next;
    }

    return object;
}

static value_t *apply_expression(expression_t *expression, object_t *objects)
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

    switch (expression->type)
    {
        case TYPE_UNSET:
            result = new_unset();
            break;
        case TYPE_NULL:
            result = new_null();
            break;
        case TYPE_NUMBER:
            result = new_number(((int *) expression->segment)[0]);
            break;
        case TYPE_STRING:
            result = new_string(expression->segment);
            break;
        case TYPE_CALL:
            result = apply_call(arguments, objects);
            break;
        default:
            result = new_error(ERROR_UNSUPPORTED);
            break;
    }

    if (arguments->length > 0)
    {
        for (index = 0; index < arguments->index; index++)
        {
            value_t *handoff;

            handoff = arguments->evaluated[index];

            if (handoff)
            {
                destroy_value(handoff);
            }
        }

        free(arguments->evaluated);
    }

    free(arguments);

    return result;
}

static value_t *apply_call(argument_iterator_t *arguments, object_t *objects)
{
    value_t *operator;
    char *name;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    operator = next_argument(arguments, objects);

    if (!operator)
    {
        return NULL;
    }

    if (operator->type != TYPE_STRING)
    {
        return new_error(ERROR_ARGUMENT);
    }

    name = view_string(operator);

    if (strcmp(name, "<--") == 0)
    {
        return operator_value(arguments, objects);
    }
    else if (strcmp(name, "-->") == 0)
    {
        return operator_assign(arguments, objects);
    }
    else if (strcmp(name, "><") == 0)
    {
        return operator_catch(arguments, objects);
    }
    else if (strcmp(name, "+") == 0)
    {
        return operator_add(arguments, objects);
    }
    else if (strcmp(name, "-") == 0)
    {
        return operator_subtract(arguments, objects);
    }
    else if (strcmp(name, "*") == 0)
    {
        return operator_multiply(arguments, objects);
    }
    else if (strcmp(name, "/") == 0)
    {
        return operator_divide(arguments, objects);
    }
    else if (strcmp(name, "%") == 0)
    {
        return operator_modulo(arguments, objects);
    }
    else if (strcmp(name, "&") == 0)
    {
        return operator_and(arguments, objects);
    }
    else if (strcmp(name, "|") == 0)
    {
        return operator_or(arguments, objects);
    }
    else if (strcmp(name, "!") == 0)
    {
        return operator_not(arguments, objects);
    }
    else if (strcmp(name, "?") == 0)
    {
        return operator_conditional(arguments, objects);
    }
    else if (strcmp(name, "o") == 0)
    {
        return operator_loop(arguments, objects);
    }
    else if (strcmp(name, "...") == 0)
    {
        return operator_chain(arguments, objects);
    }
    else if (strcmp(name, "<") == 0)
    {
        return operator_less(arguments, objects);
    }
    else if (strcmp(name, ">") == 0)
    {
        return operator_greater(arguments, objects);
    }
    else if (strcmp(name, "=") == 0)
    {
        return operator_equal(arguments, objects);
    }
    else if (strcmp(name, "_") == 0)
    {
        return operator_type(arguments, objects);
    }
    else if (strcmp(name, "#") == 0)
    {
        return operator_number(arguments, objects);
    }
    else if (strcmp(name, "\"") == 0)
    {
        return operator_string(arguments, objects);
    }
    else if (strcmp(name, "::") == 0)
    {
        return operator_hash(arguments, objects);
    }
    else if (strcmp(name, "| |") == 0)
    {
        return operator_length(arguments, objects);
    }

    return new_error(ERROR_ARGUMENT);
}

static value_t *operator_value(argument_iterator_t *arguments, object_t *objects)
{
    value_t *identifier;
    object_t *object;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    identifier = next_argument(arguments, objects);

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

    for (object = objects; object != NULL; object = object->next)
    {
        if (object->identifier && strcmp(object->identifier, view_string(identifier)) == 0)
        {
            return copy_value(object->value);
        }
    }

    return new_null();
}

static value_t *operator_assign(argument_iterator_t *arguments, object_t *objects)
{
    value_t *identifier, *handoff;
    object_t *object, *last;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    identifier = next_argument(arguments, objects);

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

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->type == TYPE_ERROR)
    {
        return copy_value(handoff);
    }

    last = NULL;

    for (object = objects; object != NULL; object = object->next)
    {
        if (object->identifier && strcmp(object->identifier, view_string(identifier)) == 0)
        {
            if (handoff->type != TYPE_NULL)
            {
                value_t *value;

                value = copy_value(handoff);

                if (!value)
                {
                    return NULL;
                }

                destroy_value(object->value);

                object->value = value;
            }
            else
            {
                if (last)
                {
                    last->next = object->next;
                }

                object->next = NULL;
                destroy_object(object);
            }

            return new_null();
        }

        last = object;
    }

    if (handoff->type != TYPE_NULL)
    {
        char *name;
        value_t *value;

        name = copy_string(view_string(identifier));

        if (!name)
        {
            return NULL;
        }

        value = copy_value(handoff);

        if (!value)
        {
            free(name);
            return NULL;
        }

        last->next = create_object(name, value, NULL);

        if (!last->next)
        {
            return NULL;
        }
    }

    return new_null();
}

static value_t *operator_catch(argument_iterator_t *arguments, object_t *objects)
{
    value_t *handoff;

    if (!has_next_argument(arguments))
    {
        return new_null();
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->type == TYPE_ERROR)
    {
        return new_number(view_error(handoff));
    }

    return new_null();
}

static value_t *operator_add(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_NUMBER && left->type != TYPE_STRING)
    {
        return new_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

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
        int x, y;

        x = view_number(left);
        y = view_number(right);

        return new_number(x + y);
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

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_subtract(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    x = view_number(left);
    y = view_number(right);

    return new_number(x - y);
}

static value_t *operator_multiply(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    x = view_number(left);
    y = view_number(right);

    return new_number(x * y);
}

static value_t *operator_divide(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    x = view_number(left);
    y = view_number(right);

    if (y == 0)
    {
        return new_error(ERROR_ARITHMETIC);
    }

    return new_number(div(x, y).quot);
}

static value_t *operator_modulo(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    x = view_number(left);
    y = view_number(right);

    if (y == 0)
    {
        return new_error(ERROR_ARITHMETIC);
    }

    return new_number(div(x, y).rem);
}

static value_t *operator_and(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    x = view_number(left);
    y = view_number(right);

    return new_number(x && y);
}

static value_t *operator_or(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    x = view_number(left);
    y = view_number(right);

    return new_number(x || y);
}

static value_t *operator_not(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left;
    int x;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    x = view_number(left);

    return new_number(!x);
}

static value_t *operator_conditional(argument_iterator_t *arguments, object_t *objects)
{
    value_t *condition;
    int x;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    condition = next_argument(arguments, objects);

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

    x = view_number(condition);

    if (x)
    {
        value_t *pass;

        if (!has_next_argument(arguments))
        {
            return new_error(ERROR_ARGUMENT);
        }

        pass = next_argument(arguments, objects);

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

        fail = next_argument(arguments, objects);

        if (!fail)
        {
            return NULL;
        }

        return copy_value(fail);
    }
}

static value_t *operator_loop(argument_iterator_t *arguments, object_t *objects)
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

        condition = next_argument(arguments, objects);

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

            pass = next_argument(arguments, objects);

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

static value_t *operator_chain(argument_iterator_t *arguments, object_t *objects)
{
    value_t *last;

    if (!has_next_argument(arguments))
    {
        return new_null();
    }

    while (has_next_argument(arguments))
    {
        last = next_argument(arguments, objects);

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

static value_t *operator_less(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    if (left->type == TYPE_NULL)
    {
        return new_number(0);
    }

    if (left->type == TYPE_NUMBER)
    {
        int x, y;

        x = view_number(left);
        y = view_number(right);

        return new_number(x < y);
    }

    if (left->type == TYPE_STRING)
    {
        return new_number(strcmp(view_string(left), view_string(right)) < 0);
    }

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_greater(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    if (left->type == TYPE_NULL)
    {
        return new_number(0);
    }

    if (left->type == TYPE_NUMBER)
    {
        int x, y;

        x = view_number(left);
        y = view_number(right);

        return new_number(x > y);
    }

    if (left->type == TYPE_STRING)
    {
        return new_number(strcmp(view_string(left), view_string(right)) > 0);
    }

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_equal(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

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

    right = next_argument(arguments, objects);

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

    if (left->type == TYPE_NULL)
    {
        return new_number(1);
    }

    if (left->type == TYPE_NUMBER)
    {
        int x, y;

        x = view_number(left);
        y = view_number(right);

        return new_number(x == y);
    }

    if (left->type == TYPE_STRING)
    {
        return new_number(strcmp(view_string(left), view_string(right)) == 0);
    }

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_type(argument_iterator_t *arguments, object_t *objects)
{
    value_t *handoff;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->type == TYPE_ERROR)
    {
        return copy_value(handoff);
    }

    if (handoff->type == TYPE_NULL)
    {
        return copy_value(handoff);
    }

    if (handoff->type == TYPE_NUMBER)
    {
        return new_string("#");
    }

    if (handoff->type == TYPE_STRING)
    {
        return new_string("\"");
    }

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_number(argument_iterator_t *arguments, object_t *objects)
{
    value_t *handoff;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->type == TYPE_ERROR)
    {
        return copy_value(handoff);
    }

    if (handoff->type == TYPE_NULL)
    {
        return copy_value(handoff);
    }

    if (handoff->type == TYPE_NUMBER)
    {
        return copy_value(handoff);
    }

    if (handoff->type == TYPE_STRING)
    {
        if (is_integer(view_string(handoff)))
        {
            return new_number(atoi(view_string(handoff)));
        }
        else
        {
            return new_error(ERROR_TYPE);
        }
    }

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_string(argument_iterator_t *arguments, object_t *objects)
{
    value_t *handoff;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->type == TYPE_ERROR)
    {
        return copy_value(handoff);
    }

    if (handoff->type == TYPE_NULL)
    {
        return copy_value(handoff);
    }

    if (handoff->type == TYPE_STRING)
    {
        return copy_value(handoff);
    }

    if (handoff->type == TYPE_NUMBER)
    {
        char *string;
        size_t size;

        string = integer_to_string(view_number(handoff));

        if (!string)
        {
            return NULL;
        }

        size = sizeof(char) * (strlen(string) + 1);

        return steal_string(string, size);
    }

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_hash(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type == TYPE_NULL)
    {
        return new_number(hash_null());
    }

    if (left->type == TYPE_NUMBER)
    {
        return new_number(hash_integer(view_number(left)));
    }

    if (left->type == TYPE_STRING)
    {
        return new_number(hash_string(view_string(left)));
    }

    return new_error(ERROR_UNSUPPORTED);
}

static value_t *operator_length(argument_iterator_t *arguments, object_t *objects)
{
    value_t *left;
    int x;

    if (!has_next_argument(arguments))
    {
        return new_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->type == TYPE_ERROR)
    {
        return copy_value(left);
    }

    if (left->type != TYPE_STRING)
    {
        return new_error(ERROR_ARGUMENT);
    }

    x = strlen(view_string(left));

    return new_number(x);
}

static int has_next_argument(argument_iterator_t *iterator)
{
    return iterator->index < iterator->length;
}

static value_t *next_argument(argument_iterator_t *iterator, object_t *objects)
{
    value_t *result;

    result = apply_expression(iterator->candidates[iterator->index], objects);
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
