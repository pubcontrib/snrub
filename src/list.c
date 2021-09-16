#include <stdlib.h>
#include "list.h"
#include "common.h"

static list_t *create_list(void (*destroy)(void *), size_t length, list_node_t *head, list_node_t *tail);
static list_node_t *create_list_node(void *value, list_node_t *next);

list_t *empty_list(void (*destroy)(void *))
{
    return create_list(destroy, 0, NULL, NULL);
}

void add_list_item(list_t *list, void *value)
{
    list_node_t *node;

    node = create_list_node(value, NULL);

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

    list = allocate(sizeof(list_t));
    list->destroy = destroy;
    list->length = length;
    list->head = head;
    list->tail = tail;

    return list;
}

static list_node_t *create_list_node(void *value, list_node_t *next)
{
    list_node_t *node;

    node = allocate(sizeof(list_node_t));
    node->value = value;
    node->next = next;

    return node;
}
