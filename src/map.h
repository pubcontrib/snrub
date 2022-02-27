#ifndef MAP_H
#define MAP_H

#include <stddef.h>
#include "string.h"

typedef struct map_chain_t
{
    string_t *key;
    void *value;
    struct map_chain_t *next;
} map_chain_t;

typedef struct
{
    int (*hash)(string_t *);
    void (*destroy)(void *);
    size_t length;
    size_t capacity;
    map_chain_t **chains;
} map_t;

map_t *empty_map(int (*hash)(string_t *), void (*destroy)(void *), size_t capacity);
int has_map_item(map_t *map, string_t *key);
void *get_map_item(map_t *map, string_t *key);
void set_map_item(map_t *map, string_t *key, void *value);
void remove_map_item(map_t *map, string_t *key);
void destroy_map(map_t *map);

#endif
