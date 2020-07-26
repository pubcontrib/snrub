#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "parse.h"
#include "common.h"

typedef struct
{
    expression_t **candidates;
    handoff_t **evaluated;
    size_t length;
    size_t index;
} argument_iterator_t;

static object_t *create_object(char *identifier, type_t type, void *unsafe, size_t size, object_t *next);
static handoff_t *create_handoff(error_t error, type_t type, void *unsafe, size_t size);
static handoff_t *create_error(error_t error);
static handoff_t *create_unset();
static handoff_t *create_null();
static handoff_t *create_number(int number);
static handoff_t *create_string(char *string);
static handoff_t *create_copy(handoff_t *this);
static handoff_t *apply_expression(expression_t *expression, object_t *objects);
static handoff_t *apply_call(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_value(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_assign(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_catch(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_add(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_subtract(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_multiply(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_divide(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_modulo(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_and(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_or(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_not(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_conditional(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_loop(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_chain(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_less(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_greater(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_equal(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_type(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_number(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_string(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_hash(argument_iterator_t *arguments, object_t *objects);
static handoff_t *operator_length(argument_iterator_t *arguments, object_t *objects);
static int has_next_argument(argument_iterator_t *iterator);
static handoff_t *next_argument(argument_iterator_t *iterator, object_t *objects);
static void skip_argument(argument_iterator_t *iterator);
static void rewind_argument(argument_iterator_t *iterator);

object_t *empty_object()
{
    return create_object(NULL, TYPE_UNSET, NULL, 0, NULL);
}

handoff_t *execute_expression(expression_t *expressions, object_t *objects)
{
    expression_t *expression;
    handoff_t *last;

    for (expression = expressions; expression != NULL; expression = expression->next)
    {
        if (expression->error != ERROR_UNSET)
        {
            return create_error(expression->error);
        }
    }

    last = create_null();

    for (expression = expressions; expression != NULL; expression = expression->next)
    {
        handoff_t *handoff;

        handoff = apply_expression(expression, objects);

        if (!handoff)
        {
            destroy_handoff(last);
            return NULL;
        }

        if (handoff->type == TYPE_UNSET)
        {
            destroy_handoff(handoff);
        }
        else
        {
            destroy_handoff(last);
            last = handoff;
        }

        if (last->error != ERROR_UNSET)
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

    if (object->unsafe)
    {
        free(object->unsafe);
    }

    if (object->next)
    {
        destroy_object(object->next);
    }

    free(object);
}

void destroy_handoff(handoff_t *handoff)
{
    if (handoff->unsafe)
    {
        free(handoff->unsafe);
    }

    free(handoff);
}

static object_t *create_object(char *identifier, type_t type, void *unsafe, size_t size, object_t *next)
{
    object_t *object;

    object = malloc(sizeof(object_t));

    if (object)
    {
        object->identifier = identifier;
        object->type = type;
        object->unsafe = unsafe;
        object->size = size;
        object->next = next;
    }

    return object;
}

static handoff_t *create_handoff(error_t error, type_t type, void *unsafe, size_t size)
{
    handoff_t *handoff;

    handoff = malloc(sizeof(handoff_t));

    if (handoff)
    {
        handoff->error = error;
        handoff->type = type;
        handoff->unsafe = unsafe;
        handoff->size = size;
    }

    return handoff;
}

static handoff_t *create_error(error_t error)
{
    return create_handoff(error, TYPE_NULL, NULL, 0);
}

static handoff_t *create_unset()
{
    return create_handoff(ERROR_UNSET, TYPE_UNSET, NULL, 0);
}

static handoff_t *create_null()
{
    return create_handoff(ERROR_UNSET, TYPE_NULL, NULL, 0);
}

static handoff_t *create_number(int number)
{
    int *unsafe;
    size_t size;

    unsafe = integer_to_array(number);

    if (!unsafe)
    {
        return NULL;
    }

    size = sizeof(int);

    return create_handoff(ERROR_UNSET, TYPE_NUMBER, unsafe, size);
}

static handoff_t *create_string(char *string)
{
    char *unsafe;
    size_t size;

    unsafe = copy_string(string);

    if (!unsafe)
    {
        return NULL;
    }

    size = sizeof(char) * (strlen(unsafe) + 1);

    return create_handoff(ERROR_UNSET, TYPE_STRING, unsafe, size);
}

static handoff_t *create_copy(handoff_t *this)
{
    void *unsafe;

    unsafe = copy_memory(this->unsafe, this->size);

    if (!unsafe)
    {
        return NULL;
    }

    return create_handoff(ERROR_UNSET, this->type, unsafe, this->size);
}

static handoff_t *apply_expression(expression_t *expression, object_t *objects)
{
    argument_iterator_t *arguments;
    handoff_t *result;
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
        arguments->evaluated = malloc(sizeof(handoff_t *) * expression->length);

        if (!arguments->evaluated)
        {
            return NULL;
        }
    }

    switch (expression->type)
    {
        case TYPE_UNSET:
            result = create_unset();
            break;
        case TYPE_NULL:
            result = create_null();
            break;
        case TYPE_NUMBER:
            result = create_number(((int *) expression->segment)[0]);
            break;
        case TYPE_STRING:
            result = create_string(expression->segment);
            break;
        case TYPE_CALL:
            result = apply_call(arguments, objects);
            break;
        default:
            result = create_error(ERROR_UNSUPPORTED);
            break;
    }

    if (arguments->length > 0)
    {
        for (index = 0; index < arguments->index; index++)
        {
            handoff_t *handoff;

            handoff = arguments->evaluated[index];

            if (handoff)
            {
                destroy_handoff(handoff);
            }
        }

        free(arguments->evaluated);
    }

    free(arguments);

    return result;
}

static handoff_t *apply_call(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *operator;
    char *name;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    operator = next_argument(arguments, objects);

    if (!operator)
    {
        return NULL;
    }

    if (operator->type != TYPE_STRING)
    {
        return create_error(ERROR_ARGUMENT);
    }

    name = operator->unsafe;

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

    return create_error(ERROR_ARGUMENT);
}

static handoff_t *operator_value(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *identifier;
    object_t *object;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    identifier = next_argument(arguments, objects);

    if (!identifier)
    {
        return NULL;
    }

    if (identifier->error != ERROR_UNSET)
    {
        return create_error(identifier->error);
    }

    if (identifier->type != TYPE_STRING)
    {
        return create_error(ERROR_ARGUMENT);
    }

    for (object = objects; object != NULL; object = object->next)
    {
        if (object->identifier && strcmp(object->identifier, identifier->unsafe) == 0)
        {
            void *unsafe;

            unsafe = copy_memory(object->unsafe, object->size);

            if (!unsafe)
            {
                return NULL;
            }

            return create_handoff(ERROR_UNSET, object->type, unsafe, object->size);
        }
    }

    return create_null();
}

static handoff_t *operator_assign(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *identifier, *handoff;
    object_t *object, *last;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    identifier = next_argument(arguments, objects);

    if (!identifier)
    {
        return NULL;
    }

    if (identifier->error != ERROR_UNSET)
    {
        return create_error(identifier->error);
    }

    if (identifier->type != TYPE_STRING)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->error != ERROR_UNSET)
    {
        return create_error(handoff->error);
    }

    last = NULL;

    for (object = objects; object != NULL; object = object->next)
    {
        if (object->identifier && strcmp(object->identifier, identifier->unsafe) == 0)
        {
            if (handoff->type != TYPE_NULL)
            {
                if (object->unsafe)
                {
                    free(object->unsafe);
                }

                object->type = handoff->type;
                object->unsafe = handoff->unsafe;
                object->size = handoff->size;

                handoff->unsafe = NULL;
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

            return create_null();
        }

        last = object;
    }

    if (handoff->type != TYPE_NULL)
    {
        last->next = create_object(identifier->unsafe, handoff->type, handoff->unsafe, handoff->size, NULL);

        if (!last->next)
        {
            return NULL;
        }

        identifier->unsafe = NULL;
        handoff->unsafe = NULL;
    }

    return create_null();
}

static handoff_t *operator_catch(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *handoff;

    if (!has_next_argument(arguments))
    {
        return create_null();
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->error != ERROR_UNSET)
    {
        return create_number(handoff->error);
    }

    return create_null();
}

static handoff_t *operator_add(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_NUMBER && left->type != TYPE_STRING)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (left->type != right->type)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type == TYPE_NUMBER)
    {
        int x, y;

        x = ((int *) left->unsafe)[0];
        y = ((int *) right->unsafe)[0];

        return create_number(x + y);
    }

    if (left->type == TYPE_STRING)
    {
        char *unsafe;
        size_t size;

        unsafe = merge_strings(left->unsafe, right->unsafe);

        if (!unsafe)
        {
            return NULL;
        }

        size = sizeof(char) * (strlen(unsafe) + 1);

        return create_handoff(ERROR_UNSET, TYPE_STRING, unsafe, size);
    }

    return create_error(ERROR_UNSUPPORTED);
}

static handoff_t *operator_subtract(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x - y);
}

static handoff_t *operator_multiply(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x * y);
}

static handoff_t *operator_divide(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    if (y == 0)
    {
        return create_error(ERROR_ARITHMETIC);
    }

    return create_number(div(x, y).quot);
}

static handoff_t *operator_modulo(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    if (y == 0)
    {
        return create_error(ERROR_ARITHMETIC);
    }

    return create_number(div(x, y).rem);
}

static handoff_t *operator_and(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x && y);
}

static handoff_t *operator_or(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x || y);
}

static handoff_t *operator_not(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left;
    int x;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];

    return create_number(!x);
}

static handoff_t *operator_conditional(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *condition;
    int x;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    condition = next_argument(arguments, objects);

    if (!condition)
    {
        return NULL;
    }

    if (condition->error != ERROR_UNSET)
    {
        return create_error(condition->error);
    }

    if (condition->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) condition->unsafe)[0];

    if (x)
    {
        handoff_t *pass;

        if (!has_next_argument(arguments))
        {
            return create_error(ERROR_ARGUMENT);
        }

        pass = next_argument(arguments, objects);

        if (!pass)
        {
            return NULL;
        }

        if (pass->error != ERROR_UNSET)
        {
            return create_error(pass->error);
        }

        return create_copy(pass);
    }
    else
    {
        handoff_t *fail;

        skip_argument(arguments);

        if (!has_next_argument(arguments))
        {
            return create_error(ERROR_ARGUMENT);
        }

        fail = next_argument(arguments, objects);

        if (!fail)
        {
            return NULL;
        }

        if (fail->error != ERROR_UNSET)
        {
            return create_error(fail->error);
        }

        return create_copy(fail);
    }
}

static handoff_t *operator_loop(argument_iterator_t *arguments, object_t *objects)
{
    int proceed;

    proceed = 1;

    while (proceed)
    {
        handoff_t *condition;

        if (!has_next_argument(arguments))
        {
            return create_error(ERROR_ARGUMENT);
        }

        condition = next_argument(arguments, objects);

        if (!condition)
        {
            return NULL;
        }

        if (condition->error != ERROR_UNSET)
        {
            return create_error(condition->error);
        }

        if (condition->type != TYPE_NUMBER)
        {
            return create_error(ERROR_ARGUMENT);
        }

        proceed = ((int *) condition->unsafe)[0];

        if (proceed)
        {
            handoff_t *pass;

            if (!has_next_argument(arguments))
            {
                return create_error(ERROR_ARGUMENT);
            }

            pass = next_argument(arguments, objects);

            if (!pass)
            {
                return NULL;
            }

            if (pass->error != ERROR_UNSET)
            {
                return create_error(pass->error);
            }

            rewind_argument(arguments);
            rewind_argument(arguments);
        }
    }

    return create_null();
}

static handoff_t *operator_chain(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *last;

    if (!has_next_argument(arguments))
    {
        return create_null();
    }

    while (has_next_argument(arguments))
    {
        last = next_argument(arguments, objects);

        if (!last)
        {
            return NULL;
        }

        if (last->error != ERROR_UNSET)
        {
            return create_error(last->error);
        }
    }

    return create_copy(last);
}

static handoff_t *operator_less(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (left->type != right->type)
    {
        return create_number(0);
    }

    if (left->type == TYPE_NULL)
    {
        return create_number(0);
    }

    if (left->type == TYPE_NUMBER)
    {
        int x, y;

        x = ((int *) left->unsafe)[0];
        y = ((int *) right->unsafe)[0];

        return create_number(x < y);
    }

    if (left->type == TYPE_STRING)
    {
        return create_number(strcmp(left->unsafe, right->unsafe) < 0);
    }

    return create_error(ERROR_UNSUPPORTED);
}

static handoff_t *operator_greater(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (left->type != right->type)
    {
        return create_number(0);
    }

    if (left->type == TYPE_NULL)
    {
        return create_number(0);
    }

    if (left->type == TYPE_NUMBER)
    {
        int x, y;

        x = ((int *) left->unsafe)[0];
        y = ((int *) right->unsafe)[0];

        return create_number(x > y);
    }

    if (left->type == TYPE_STRING)
    {
        return create_number(strcmp(left->unsafe, right->unsafe) > 0);
    }

    return create_error(ERROR_UNSUPPORTED);
}

static handoff_t *operator_equal(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left, *right;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    right = next_argument(arguments, objects);

    if (!right)
    {
        return NULL;
    }

    if (right->error != ERROR_UNSET)
    {
        return create_error(right->error);
    }

    if (left->type != right->type)
    {
        return create_number(0);
    }

    if (left->type == TYPE_NULL)
    {
        return create_number(1);
    }

    if (left->type == TYPE_NUMBER)
    {
        int x, y;

        x = ((int *) left->unsafe)[0];
        y = ((int *) right->unsafe)[0];

        return create_number(x == y);
    }

    if (left->type == TYPE_STRING)
    {
        return create_number(strcmp(left->unsafe, right->unsafe) == 0);
    }

    return create_error(ERROR_UNSUPPORTED);
}

static handoff_t *operator_type(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *handoff;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->error != ERROR_UNSET)
    {
        return create_error(handoff->error);
    }

    if (handoff->type == TYPE_NULL)
    {
        return create_null();
    }

    if (handoff->type == TYPE_NUMBER)
    {
        return create_string("#");
    }

    if (handoff->type == TYPE_STRING)
    {
        return create_string("\"");
    }

    return create_error(ERROR_UNSUPPORTED);
}

static handoff_t *operator_number(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *handoff;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->error != ERROR_UNSET)
    {
        return create_error(handoff->error);
    }

    if (handoff->type == TYPE_NULL)
    {
        return create_null();
    }

    if (handoff->type == TYPE_NUMBER)
    {
        return create_number(((int *) handoff->unsafe)[0]);
    }

    if (handoff->type == TYPE_STRING)
    {
        if (is_integer(handoff->unsafe))
        {
            return create_number(atoi(handoff->unsafe));
        }
        else
        {
            return create_error(ERROR_TYPE);
        }
    }

    return create_error(ERROR_UNSUPPORTED);
}

static handoff_t *operator_string(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *handoff;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    handoff = next_argument(arguments, objects);

    if (!handoff)
    {
        return NULL;
    }

    if (handoff->error != ERROR_UNSET)
    {
        return create_error(handoff->error);
    }

    if (handoff->type == TYPE_NULL)
    {
        return create_null();
    }

    if (handoff->type == TYPE_STRING)
    {
        return create_string(handoff->unsafe);
    }

    if (handoff->type == TYPE_NUMBER)
    {
        void *unsafe;
        size_t size;

        unsafe = integer_to_string(((int *) handoff->unsafe)[0]);

        if (!unsafe)
        {
            return NULL;
        }

        size = sizeof(char) * (strlen(unsafe) + 1);

        return create_handoff(ERROR_UNSET, TYPE_STRING, unsafe, size);
    }

    return create_error(ERROR_UNSUPPORTED);
}

static handoff_t *operator_hash(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type == TYPE_NULL)
    {
        return create_number(hash_null());
    }

    if (left->type == TYPE_NUMBER)
    {
        return create_number(hash_integer(((int *) left->unsafe)[0]));
    }

    if (left->type == TYPE_STRING)
    {
        return create_number(hash_string(left->unsafe));
    }

    return create_error(ERROR_UNSUPPORTED);
}

static handoff_t *operator_length(argument_iterator_t *arguments, object_t *objects)
{
    handoff_t *left;
    int x;

    if (!has_next_argument(arguments))
    {
        return create_error(ERROR_ARGUMENT);
    }

    left = next_argument(arguments, objects);

    if (!left)
    {
        return NULL;
    }

    if (left->error != ERROR_UNSET)
    {
        return create_error(left->error);
    }

    if (left->type != TYPE_STRING)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = strlen(left->unsafe);

    return create_number(x);
}

static int has_next_argument(argument_iterator_t *iterator)
{
    return iterator->index < iterator->length;
}

static handoff_t *next_argument(argument_iterator_t *iterator, object_t *objects)
{
    handoff_t *result;

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
    destroy_handoff(iterator->evaluated[iterator->index]);
    iterator->evaluated[iterator->index] = NULL;
}
