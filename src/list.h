#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct list_node_t
{
    void *value;
    struct list_node_t *next;
} list_node_t;

typedef struct
{
    void (*destroy)(void *);
    size_t length;
    list_node_t *head;
    list_node_t *tail;
} list_t;

list_t *empty_list(void (*destroy)(void *));
int add_list_item(list_t *list, void *value);
void destroy_list(list_t *list);

#endif
