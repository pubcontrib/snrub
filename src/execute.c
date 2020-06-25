#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "parse.h"
#include "common.h"

static object_t *create_object(char *identifier, type_t type, void *unsafe, size_t size, object_t *next);
static handoff_t *create_handoff(error_t error, type_t type, void *unsafe, size_t size);
static handoff_t *create_error(error_t error);
static handoff_t *create_unknown();
static handoff_t *create_null();
static handoff_t *create_number(int number);
static handoff_t *create_string(char *string);
static handoff_t *create_copy(handoff_t *this);
static handoff_t *apply_expression(expression_t *expression, object_t *objects);
static handoff_t *apply_operator(literal_t *literal, handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_comment(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_value(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_assign(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_add(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_subtract(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_multiply(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_divide(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_and(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_or(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_not(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_conditional(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_less(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_greater(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_equal(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_number(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_string(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *operator_length(handoff_t **arguments, size_t length, object_t *objects);
static handoff_t *arguments_get(handoff_t **arguments, size_t length, size_t index);
static void arguments_free(handoff_t **arguments, size_t length);

object_t *empty_object()
{
    return create_object(NULL, TYPE_UNKNOWN, NULL, 0, NULL);
}

handoff_t *execute_expression(expression_t *expressions, object_t *objects)
{
    expression_t *expression;
    handoff_t *last;

    last = NULL;

    for (expression = expressions; expression != NULL; expression = expression->next)
    {
        handoff_t *handoff;

        handoff = apply_expression(expression, objects);

        if (!handoff)
        {
            if (last)
            {
                destroy_handoff(last);
                last = NULL;
            }

            break;
        }

        if (last)
        {
            if (handoff->type == TYPE_UNKNOWN)
            {
                destroy_handoff(handoff);
            }
            else
            {
                destroy_handoff(last);
                last = handoff;
            }
        }
        else
        {
            last = handoff;
        }

        if (last->error != ERROR_UNKNOWN)
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

static handoff_t *create_unknown()
{
    return create_handoff(ERROR_UNKNOWN, TYPE_UNKNOWN, NULL, 0);
}

static handoff_t *create_null()
{
    return create_handoff(ERROR_UNKNOWN, TYPE_NULL, NULL, 0);
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

    return create_handoff(ERROR_UNKNOWN, TYPE_NUMBER, unsafe, size);
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

    return create_handoff(ERROR_UNKNOWN, TYPE_STRING, unsafe, size);
}

static handoff_t *create_copy(handoff_t *this)
{
    switch (this->type)
    {
        case TYPE_NULL:
            return create_null();
        case TYPE_NUMBER:
            return create_number(((int *) this->unsafe)[0]);
        case TYPE_STRING:
            return create_string(this->unsafe);
        default:
            return create_error(ERROR_TYPE);
    }
}

static handoff_t *apply_expression(expression_t *expression, object_t *objects)
{
    handoff_t **arguments;
    handoff_t *result;
    size_t length, index;

    if (expression->error != ERROR_UNKNOWN)
    {
        return create_error(expression->error);
    }

    length = expression->length;
    arguments = malloc(sizeof(handoff_t *) * length);

    for (index = 0; index < length; index++)
    {
        handoff_t *argument;

        argument = apply_expression(expression->arguments[index], objects);

        if (!argument || argument->error != ERROR_UNKNOWN)
        {
            arguments_free(arguments, index - 1);
            return argument;
        }

        arguments[index] = argument;
    }

    result = apply_operator(expression->literal, arguments, length, objects);

    arguments_free(arguments, length);

    return result;
}

static handoff_t *apply_operator(literal_t *literal, handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *operator;

    operator = arguments_get(arguments, length, 0);

    if (!operator)
    {
        if (literal)
        {
            switch (literal->type)
            {
                case TYPE_NULL:
                    return create_null();
                case TYPE_NUMBER:
                    return create_number(((int *) literal->unsafe)[0]);
                case TYPE_STRING:
                    return create_string(literal->unsafe);
                default:
                    break;
            }
        }

        return create_unknown();
    }

    if (operator->type == TYPE_STRING)
    {
        if (strcmp(operator->unsafe, "~") == 0)
        {
            return operator_comment(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "<--") == 0)
        {
            return operator_value(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "-->") == 0)
        {
            return operator_assign(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "+") == 0)
        {
            return operator_add(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "-") == 0)
        {
            return operator_subtract(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "*") == 0)
        {
            return operator_multiply(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "/") == 0)
        {
            return operator_divide(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "&") == 0)
        {
            return operator_and(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "|") == 0)
        {
            return operator_or(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "!") == 0)
        {
            return operator_not(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "?") == 0)
        {
            return operator_conditional(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "<") == 0)
        {
            return operator_less(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, ">") == 0)
        {
            return operator_greater(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "=") == 0)
        {
            return operator_equal(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "#") == 0)
        {
            return operator_number(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "\"") == 0)
        {
            return operator_string(arguments, length, objects);
        }
        else if (strcmp(operator->unsafe, "| |") == 0)
        {
            return operator_length(arguments, length, objects);
        }
    }

    return create_error(ERROR_ARGUMENT);
}

static handoff_t *operator_comment(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *message;

    message = arguments_get(arguments, length, 1);

    if (!message)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (message->type != TYPE_STRING)
    {
        return create_error(ERROR_ARGUMENT);
    }

    return create_unknown();
}

static handoff_t *operator_value(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *identifier;
    object_t *object;

    identifier = arguments_get(arguments, length, 1);

    if (!identifier)
    {
        return create_error(ERROR_ARGUMENT);
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

            return create_handoff(ERROR_UNKNOWN, object->type, unsafe, object->size);
        }
    }

    return create_null();
}

static handoff_t *operator_assign(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *identifier, *handoff;
    object_t *object, *last;

    identifier = arguments_get(arguments, length, 1);
    handoff = arguments_get(arguments, length, 2);

    if (!identifier || !handoff)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (identifier->type != TYPE_STRING)
    {
        return create_error(ERROR_ARGUMENT);
    }

    last = NULL;

    for (object = objects; object != NULL; object = object->next)
    {
        if (object->identifier && strcmp(object->identifier, identifier->unsafe) == 0)
        {
            if (handoff->type != TYPE_UNKNOWN && handoff->type != TYPE_NULL)
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

    if (handoff->type != TYPE_UNKNOWN && handoff->type != TYPE_NULL)
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

static handoff_t *operator_add(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER || right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x + y);
}

static handoff_t *operator_subtract(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER || right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x - y);
}

static handoff_t *operator_multiply(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER || right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x * y);
}

static handoff_t *operator_divide(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER || right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    if (y == 0)
    {
        return create_error(ERROR_ARITHMETIC);
    }

    return create_number(x / y);
}

static handoff_t *operator_and(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER || right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x && y);
}

static handoff_t *operator_or(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER || right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x || y);
}

static handoff_t *operator_not(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left;
    int x;

    left = arguments_get(arguments, length, 1);

    if (!left)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];

    return create_number(!x);
}

static handoff_t *operator_conditional(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *condition, *pass, *fail;
    int x;

    condition = arguments_get(arguments, length, 1);
    pass = arguments_get(arguments, length, 2);
    fail = arguments_get(arguments, length, 3);

    if (!condition || !pass || !fail)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (condition->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) condition->unsafe)[0];

    if (x)
    {
        return create_copy(pass);
    }
    else
    {
        return create_copy(fail);
    }
}

static handoff_t *operator_less(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER || right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x < y);
}

static handoff_t *operator_greater(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;
    int x, y;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_NUMBER || right->type != TYPE_NUMBER)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x > y);
}

static handoff_t *operator_equal(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left, *right;

    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!left || !right)
    {
        return create_error(ERROR_ARGUMENT);
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

    return create_error(ERROR_TYPE);
}

static handoff_t *operator_number(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *handoff;

    handoff = arguments_get(arguments, length, 1);

    if (!handoff)
    {
        return create_error(ERROR_ARGUMENT);
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

    return create_error(ERROR_TYPE);
}

static handoff_t *operator_string(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *handoff;

    handoff = arguments_get(arguments, length, 1);

    if (!handoff)
    {
        return create_error(ERROR_ARGUMENT);
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

        return create_handoff(ERROR_UNKNOWN, TYPE_STRING, unsafe, size);
    }

    return create_error(ERROR_TYPE);
}

static handoff_t *operator_length(handoff_t **arguments, size_t length, object_t *objects)
{
    handoff_t *left;
    int x;

    left = arguments_get(arguments, length, 1);

    if (!left)
    {
        return create_error(ERROR_ARGUMENT);
    }

    if (left->type != TYPE_STRING)
    {
        return create_error(ERROR_ARGUMENT);
    }

    x = strlen(left->unsafe);

    return create_number(x);
}

static handoff_t *arguments_get(handoff_t **arguments, size_t length, size_t index)
{
    if (index >= 0 && index < length)
    {
        return arguments[index];
    }

    return NULL;
}

static void arguments_free(handoff_t **arguments, size_t length)
{
    size_t index;

    for (index = 0; index < length; index++)
    {
        destroy_handoff(arguments[index]);
    }

    free(arguments);
}
