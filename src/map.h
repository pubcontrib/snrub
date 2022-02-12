#ifndef MAP_H
#define MAP_H

#include "buffer.h"

typedef struct map_chain_t
{
    buffer_t *key;
    void *value;
    struct map_chain_t *next;
} map_chain_t;

typedef struct
{
    int (*hash)(buffer_t *);
    void (*destroy)(void *);
    size_t length;
    size_t capacity;
    map_chain_t **chains;
} map_t;

map_t *empty_map(int (*hash)(buffer_t *), void (*destroy)(void *), size_t capacity);
int has_map_item(map_t *map, buffer_t *key);
void *get_map_item(map_t *map, buffer_t *key);
void set_map_item(map_t *map, buffer_t *key, void *value);
void remove_map_item(map_t *map, buffer_t *key);
void destroy_map(map_t *map);

#endif
