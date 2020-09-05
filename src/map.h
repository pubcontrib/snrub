#ifndef MAP_H
#define MAP_H

typedef struct map_chain_t
{
    char *key;
    void *value;
    struct map_chain_t *next;
} map_chain_t;

typedef struct
{
    int (*hash)(char *);
    void (*destroy)(void *);
    size_t length;
    size_t capacity;
    map_chain_t **chains;
} map_t;

map_t *empty_map(int (*hash)(char *), void (*destroy)(void *));
void *get_map_item(map_t *map, char *key);
int set_map_item(map_t *map, char *key, void *value);
void remove_map_item(map_t *map, char *key);
void destroy_map(map_t *map);

#endif
