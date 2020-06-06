#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "parse.h"
#include "common.h"

static execute_store_t *create_store(execute_object_t *objects, execute_error_t error);
static execute_object_t *create_object(execute_type_t type, void *unsafe, size_t size, char *key, execute_object_t *next);
static execute_passback_t *create_passback(execute_type_t type, void *unsafe, size_t size, execute_error_t error);
static execute_passback_t *create_error(execute_error_t error);
static execute_passback_t *create_unknown();
static execute_passback_t *create_null();
static execute_passback_t *create_number(int number);
static execute_passback_t *create_string(char *string);
static execute_passback_t *create_copy(execute_passback_t *this);
static execute_passback_t *apply_expression(parse_expression_t *expression, execute_store_t *store);
static execute_passback_t *apply_operator(parse_value_t *value, execute_passback_t **arguments, size_t length, execute_store_t *store);
static execute_passback_t *operator_comment(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_value(execute_passback_t *left, execute_passback_t *right, execute_store_t *store);
static execute_passback_t *operator_assign(execute_passback_t *left, execute_passback_t *right, execute_store_t *store);
static execute_passback_t *operator_add(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_subtract(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_multiply(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_divide(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_and(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_or(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_not(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_conditional(execute_passback_t *condition, execute_passback_t *pass, execute_passback_t *fail);
static execute_passback_t *operator_less(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_greater(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_equal(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_number(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *operator_string(execute_passback_t *left, execute_passback_t *right);
static execute_passback_t *arguments_get(execute_passback_t **arguments, size_t length, size_t index);
static void arguments_free(execute_passback_t **arguments, size_t length);

execute_passback_t *execute_do_document(parse_link_t *head)
{
    parse_link_t *current;
    execute_store_t *store;
    execute_passback_t *last;

    last = NULL;

    store = create_store(NULL, EXECUTE_ERROR_UNKNOWN);

    if (!store)
    {
        parse_destroy_link(head);
        return NULL;
    }

    for (current = head; current != NULL; current = current->next)
    {
        execute_passback_t *passback;

        passback = apply_expression(current->expression, store);

        if (!passback)
        {
            if (last)
            {
                execute_destroy_passback(last);
                last = NULL;
            }

            break;
        }

        if (last)
        {
            if (passback->type == EXECUTE_TYPE_UNKNOWN)
            {
                execute_destroy_passback(passback);
            }
            else
            {
                execute_destroy_passback(last);
                last = passback;
            }
        }
        else
        {
            last = passback;
        }

        if (last->error != EXECUTE_ERROR_UNKNOWN)
        {
            break;
        }
    }

    execute_destroy_store(store);

    return last;
}

void execute_destroy_store(execute_store_t *store)
{
    if (store->objects)
    {
        execute_destroy_object(store->objects);
    }

    free(store);
}

void execute_destroy_object(execute_object_t *object)
{
    if (object->next)
    {
        execute_destroy_object(object->next);
    }

    if (object->unsafe)
    {
        free(object->unsafe);
    }

    if (object->key)
    {
        free(object->key);
    }

    free(object);
}

void execute_destroy_passback(execute_passback_t *passback)
{
    if (passback->unsafe)
    {
        free(passback->unsafe);
    }

    free(passback);
}

static execute_store_t *create_store(execute_object_t *objects, execute_error_t error)
{
    execute_store_t *store;

    store = malloc(sizeof(execute_store_t));

    if (store)
    {
        store->objects = objects;
        store->error = error;
    }

    return store;
}

static execute_object_t *create_object(execute_type_t type, void *unsafe, size_t size, char *key, execute_object_t *next)
{
    execute_object_t *object;

    object = malloc(sizeof(execute_object_t));

    if (object)
    {
        object->type = type;
        object->unsafe = unsafe;
        object->size = size;
        object->key = key;
        object->next = next;
    }

    return object;
}

static execute_passback_t *create_passback(execute_type_t type, void *unsafe, size_t size, execute_error_t error)
{
    execute_passback_t *passback;

    passback = malloc(sizeof(execute_passback_t));

    if (passback)
    {
        passback->type = type;
        passback->unsafe = unsafe;
        passback->size = size;
        passback->error = error;
    }

    return passback;
}

static execute_passback_t *create_error(execute_error_t error)
{
    return create_passback(EXECUTE_TYPE_NULL, NULL, 0, error);
}

static execute_passback_t *create_unknown()
{
    return create_passback(EXECUTE_TYPE_UNKNOWN, NULL, 0, EXECUTE_ERROR_UNKNOWN);
}

static execute_passback_t *create_null()
{
    return create_passback(EXECUTE_TYPE_NULL, NULL, 0, EXECUTE_ERROR_UNKNOWN);
}

static execute_passback_t *create_number(int number)
{
    int *unsafe;
    size_t size;

    unsafe = integer_to_array(number);

    if (!unsafe)
    {
        return NULL;
    }

    size = sizeof(int);

    return create_passback(EXECUTE_TYPE_NUMBER, unsafe, size, EXECUTE_ERROR_UNKNOWN);
}

static execute_passback_t *create_string(char *string)
{
    char *unsafe;
    size_t size;

    unsafe = copy_string(string);

    if (!unsafe)
    {
        return NULL;
    }

    size = sizeof(char) * (strlen(unsafe) + 1);

    return create_passback(EXECUTE_TYPE_STRING, unsafe, size, EXECUTE_ERROR_UNKNOWN);
}

static execute_passback_t *create_copy(execute_passback_t *this)
{
    switch (this->type)
    {
        case EXECUTE_TYPE_NULL:
            return create_null();
        case EXECUTE_TYPE_NUMBER:
            return create_number(((int *) this->unsafe)[0]);
        case EXECUTE_TYPE_STRING:
            return create_string(this->unsafe);
        default:
            return create_error(EXECUTE_ERROR_TYPE);
    }
}

static execute_passback_t *apply_expression(parse_expression_t *expression, execute_store_t *store)
{
    execute_passback_t **arguments;
    execute_passback_t *result;
    size_t length, index;

    if (expression->error != PARSE_ERROR_UNKNOWN)
    {
        switch (expression->error)
        {
            case PARSE_ERROR_SYNTAX:
                return create_error(EXECUTE_ERROR_SYNTAX);
            case PARSE_ERROR_DEPTH:
                return create_error(EXECUTE_ERROR_DEPTH);
            case PARSE_ERROR_TYPE:
                return create_error(EXECUTE_ERROR_TYPE);
            case PARSE_ERROR_ARGUMENT:
                return create_error(EXECUTE_ERROR_ARGUMENT);
            default:
                break;
        }
    }

    length = expression->length;
    arguments = malloc(sizeof(execute_passback_t *) * length);

    for (index = 0; index < length; index++)
    {
        execute_passback_t *argument;

        argument = apply_expression(expression->arguments[index], store);

        if (!argument || argument->error != EXECUTE_ERROR_UNKNOWN)
        {
            arguments_free(arguments, index - 1);
            return argument;
        }

        arguments[index] = argument;
    }

    result = apply_operator(expression->value, arguments, length, store);

    arguments_free(arguments, length);

    return result;
}

static execute_passback_t *apply_operator(parse_value_t *value, execute_passback_t **arguments, size_t length, execute_store_t *store)
{
    execute_passback_t *operator, *left, *right;

    operator = arguments_get(arguments, length, 0);
    left = arguments_get(arguments, length, 1);
    right = arguments_get(arguments, length, 2);

    if (!operator)
    {
        if (value)
        {
            switch (value->type)
            {
                case PARSE_TYPE_NULL:
                    return create_null();
                case PARSE_TYPE_NUMBER:
                    return create_number(((int *) value->unsafe)[0]);
                case PARSE_TYPE_STRING:
                    return create_string(value->unsafe);
                default:
                    break;
            }
        }

        return create_unknown();
    }

    if (operator->type == EXECUTE_TYPE_STRING)
    {
        if (strcmp(operator->unsafe, "~") == 0)
        {
            return operator_comment(left, right);
        }
        else if (strcmp(operator->unsafe, "<--") == 0)
        {
            return operator_value(left, right, store);
        }
        else if (strcmp(operator->unsafe, "-->") == 0)
        {
            return operator_assign(left, right, store);
        }
        else if (strcmp(operator->unsafe, "+") == 0)
        {
            return operator_add(left, right);
        }
        else if (strcmp(operator->unsafe, "-") == 0)
        {
            return operator_subtract(left, right);
        }
        else if (strcmp(operator->unsafe, "*") == 0)
        {
            return operator_multiply(left, right);
        }
        else if (strcmp(operator->unsafe, "/") == 0)
        {
            return operator_divide(left, right);
        }
        else if (strcmp(operator->unsafe, "&") == 0)
        {
            return operator_and(left, right);
        }
        else if (strcmp(operator->unsafe, "|") == 0)
        {
            return operator_or(left, right);
        }
        else if (strcmp(operator->unsafe, "!") == 0)
        {
            return operator_not(left, right);
        }
        else if (strcmp(operator->unsafe, "?") == 0)
        {
            execute_passback_t *conditional, *pass, *fail;

            conditional = arguments_get(arguments, length, 1);
            pass = arguments_get(arguments, length, 2);
            fail = arguments_get(arguments, length, 3);

            return operator_conditional(conditional, pass, fail);
        }
        else if (strcmp(operator->unsafe, "<") == 0)
        {
            return operator_less(left, right);
        }
        else if (strcmp(operator->unsafe, ">") == 0)
        {
            return operator_greater(left, right);
        }
        else if (strcmp(operator->unsafe, "=") == 0)
        {
            return operator_equal(left, right);
        }
        else if (strcmp(operator->unsafe, "#") == 0)
        {
            return operator_number(left, right);
        }
        else if (strcmp(operator->unsafe, "\"") == 0)
        {
            return operator_string(left, right);
        }
    }

    return create_error(EXECUTE_ERROR_ARGUMENT);
}

static execute_passback_t *operator_comment(execute_passback_t *left, execute_passback_t *right)
{
    if (!left)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_STRING)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    return create_unknown();
}

static execute_passback_t *operator_value(execute_passback_t *left, execute_passback_t *right, execute_store_t *store)
{
    if (!left)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_STRING)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (store->objects)
    {
        execute_object_t *current;

        for (current = store->objects; current != NULL; current = current->next)
        {
            if (strcmp(current->key, left->unsafe) == 0)
            {
                if (current->unsafe)
                {
                    void *unsafe;
                    size_t size;

                    size = current->size;
                    unsafe = copy_memory(current->unsafe, size);

                    if (!unsafe)
                    {
                        return NULL;
                    }

                    return create_passback(current->type, unsafe, size, EXECUTE_ERROR_UNKNOWN);
                }
            }
        }
    }

    return create_null();
}

static execute_passback_t *operator_assign(execute_passback_t *left, execute_passback_t *right, execute_store_t *store)
{
    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_STRING)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (store->objects)
    {
        execute_object_t *current, *last;

        last = NULL;

        for (current = store->objects; current != NULL; current = current->next)
        {
            if (strcmp(current->key, left->unsafe) == 0)
            {
                if (right->type != EXECUTE_TYPE_UNKNOWN && right->type != EXECUTE_TYPE_NULL)
                {
                    if (current->unsafe)
                    {
                        free(current->unsafe);
                    }

                    current->type = right->type;
                    current->unsafe = right->unsafe;
                    current->size = right->size;

                    right->unsafe = NULL;
                }
                else
                {
                    if (last)
                    {
                        last->next = current->next;
                    }
                    else
                    {
                        store->objects = current->next;
                    }

                    current->next = NULL;
                    execute_destroy_object(current);
                }

                return create_null();
            }

            last = current;
        }

        if (right->type != EXECUTE_TYPE_UNKNOWN && right->type != EXECUTE_TYPE_NULL)
        {
            last->next = create_object(right->type, right->unsafe, right->size, left->unsafe, NULL);

            if (!last->next)
            {
                return NULL;
            }

            left->unsafe = NULL;
            right->unsafe = NULL;
        }
    }
    else
    {
        if (right->type != EXECUTE_TYPE_UNKNOWN && right->type != EXECUTE_TYPE_NULL)
        {
            store->objects = create_object(right->type, right->unsafe, right->size, left->unsafe, NULL);

            if (!store->objects)
            {
                return NULL;
            }

            left->unsafe = NULL;
            right->unsafe = NULL;
        }
    }

    return create_null();
}

static execute_passback_t *operator_add(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x + y);
}

static execute_passback_t *operator_subtract(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x - y);
}

static execute_passback_t *operator_multiply(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x * y);
}

static execute_passback_t *operator_divide(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    if (y == 0)
    {
        return create_error(EXECUTE_ERROR_ARITHMETIC);
    }

    return create_number(x / y);
}

static execute_passback_t *operator_and(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x && y);
}

static execute_passback_t *operator_or(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x || y);
}

static execute_passback_t *operator_not(execute_passback_t *left, execute_passback_t *right)
{
    int x;

    if (!left)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];

    return create_number(!x);
}

static execute_passback_t *operator_conditional(execute_passback_t *condition, execute_passback_t *pass, execute_passback_t *fail)
{
    int x;

    if (!condition || !pass || !fail)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (condition->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
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

static execute_passback_t *operator_less(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x < y);
}

static execute_passback_t *operator_greater(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x > y);
}

static execute_passback_t *operator_equal(execute_passback_t *left, execute_passback_t *right)
{
    int x, y;

    if (!left || !right)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type != EXECUTE_TYPE_NUMBER || right->type != EXECUTE_TYPE_NUMBER)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    x = ((int *) left->unsafe)[0];
    y = ((int *) right->unsafe)[0];

    return create_number(x == y);
}

static execute_passback_t *operator_number(execute_passback_t *left, execute_passback_t *right)
{
    if (!left)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type == EXECUTE_TYPE_NULL)
    {
        return create_null();
    }

    if (left->type == EXECUTE_TYPE_NUMBER)
    {
        return create_number(((int *) left->unsafe)[0]);
    }

    if (left->type == EXECUTE_TYPE_STRING)
    {
        if (is_integer(left->unsafe))
        {
            return create_number(atoi(left->unsafe));
        }
        else
        {
            return create_error(EXECUTE_ERROR_TYPE);
        }
    }

    return create_error(EXECUTE_ERROR_TYPE);
}

static execute_passback_t *operator_string(execute_passback_t *left, execute_passback_t *right)
{
    if (!left)
    {
        return create_error(EXECUTE_ERROR_ARGUMENT);
    }

    if (left->type == EXECUTE_TYPE_NULL)
    {
        return create_null();
    }

    if (left->type == EXECUTE_TYPE_STRING)
    {
        return create_string(left->unsafe);
    }

    if (left->type == EXECUTE_TYPE_NUMBER)
    {
        void *unsafe;
        size_t size;

        unsafe = integer_to_string(((int *) left->unsafe)[0]);

        if (!unsafe)
        {
            return NULL;
        }

        size = sizeof(char) * (strlen(unsafe) + 1);

        return create_passback(EXECUTE_TYPE_STRING, unsafe, size, EXECUTE_ERROR_UNKNOWN);
    }

    return create_error(EXECUTE_ERROR_TYPE);
}

static execute_passback_t *arguments_get(execute_passback_t **arguments, size_t length, size_t index)
{
    if (index >= 0 && index < length)
    {
        return arguments[index];
    }

    return NULL;
}

static void arguments_free(execute_passback_t **arguments, size_t length)
{
    size_t index;

    for (index = 0; index < length; index++)
    {
        execute_destroy_passback(arguments[index]);
    }

    free(arguments);
}
