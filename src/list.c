#include <stdlib.h>
#include "list.h"

static void destroy_list_node(list_node_t *node, void (*destroy)(void *));
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
        destroy_list_node(list->head, list->destroy);
    }

    free(list);
}

static void destroy_list_node(list_node_t *node, void (*destroy)(void *))
{
    if (node->value)
    {
        destroy(node->value);
    }

    if (node->next)
    {
        destroy_list_node(node->next, destroy);
    }

    free(node);
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
