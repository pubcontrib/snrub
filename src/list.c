#include <stdlib.h>
#include "list.h"

static list_t *create_list(void (*destroy)(void *), size_t length, list_node_t *head, list_node_t *tail);
static list_node_t *create_list_node(void *value, list_node_t *next);

list_t *empty_list(void (*destroy)(void *))
{
    return create_list(destroy, 0, NULL, NULL);
}

int add_list_item(list_t *list, void *value)
{
    list_node_t *node;

    node = create_list_node(value, NULL);

    if (!node)
    {
        return 0;
    }

    list->length += 1;

    if (list->length == 1)
    {
        list->head = node;
        list->tail = node;
    }
    else
    {
        list->tail->next = node;
        list->tail = node;
    }

    return 1;
}

void destroy_list(list_t *list)
{
    if (list->head)
    {
        list_node_t *node, *next;

        for (node = list->head; node != NULL; node = next)
        {
            next = node->next;

            if (node->value)
            {
                list->destroy(node->value);
            }

            free(node);
        }
    }

    free(list);
}

static list_t *create_list(void (*destroy)(void *), size_t length, list_node_t *head, list_node_t *tail)
{
    list_t *list;

    list = malloc(sizeof(list_t));

    if (list)
    {
        list->destroy = destroy;
        list->length = length;
        list->head = head;
        list->tail = tail;
    }

    return list;
}

static list_node_t *create_list_node(void *value, list_node_t *next)
{
    list_node_t *node;

    node = malloc(sizeof(list_node_t));

    if (node)
    {
        node->value = value;
        node->next = next;
    }

    return node;
}
