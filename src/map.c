#include <stdlib.h>
#include <string.h>
#include "map.h"

static void destroy_chain(map_chain_t *chain, void (*destroy)(void *));
static map_chain_t *create_map_chain(char *key, void *value, map_chain_t *next);
static map_t *create_map(int (*hash)(char *), void (*destroy)(void *), size_t length, size_t capacity, map_chain_t **chains);
static int resize_map(map_t *map);

map_t *empty_map(int (*hash)(char *), void (*destroy)(void *))
{
    map_chain_t **chains;
    size_t capacity;

    capacity = 64;
    chains = calloc(capacity, sizeof(map_t *));

    if (!chains)
    {
        return NULL;
    }

    return create_map(hash, destroy, 0, capacity, chains);
}

void *get_map_item(map_t *map, char *key)
{
    map_chain_t *chain;
    int hash, index;

    hash = abs(map->hash(key));
    index = div(hash, map->capacity).rem;

    for (chain = map->chains[index]; chain != NULL; chain = chain->next)
    {
        if (strcmp(key, chain->key) == 0)
        {
            return chain->value;
        }
    }

    return NULL;
}

int set_map_item(map_t *map, char *key, void *value)
{
    map_chain_t *chain, *last, *created;
    int hash, index;

    hash = abs(map->hash(key));
    index = div(hash, map->capacity).rem;

    for (chain = map->chains[index], last = NULL; chain != NULL; chain = chain->next)
    {
        if (strcmp(key, chain->key) == 0)
        {
            free(key);
            map->destroy(chain->value);
            chain->value = value;
            return 1;
        }

        last = chain;
    }

    created = create_map_chain(key, value, NULL);

    if (!created)
    {
        return 0;
    }

    if (last)
    {
        last->next = created;
    }
    else
    {
        map->chains[index] = created;
    }

    map->length += 1;

    if (map->length == map->capacity)
    {
        return resize_map(map);
    }

    return 1;
}

void remove_map_item(map_t *map, char *key)
{
    map_chain_t *chain, *previous;
    int hash, index;

    hash = abs(map->hash(key));
    index = div(hash, map->capacity).rem;

    for (chain = map->chains[index], previous = NULL; chain != NULL; chain = chain->next)
    {
        if (strcmp(key, chain->key) == 0)
        {
            if (previous)
            {
                previous->next = chain->next;
            }
            else
            {
                map->chains[index] = chain->next;
            }

            chain->next = NULL;
            destroy_chain(chain, map->destroy);
            map->length -= 1;

            return;
        }
    }
}

void destroy_map(map_t *map)
{
    if (map->chains)
    {
        size_t index;

        for (index = 0; index < map->capacity; index++)
        {
            map_chain_t *chain;

            chain = map->chains[index];

            if (chain)
            {
                destroy_chain(chain, map->destroy);
            }
        }

        free(map->chains);
    }

    free(map);
}

static void destroy_chain(map_chain_t *chain, void (*destroy)(void *))
{
    if (chain->key)
    {
        free(chain->key);
    }

    if (chain->value)
    {
        destroy(chain->value);
    }

    if (chain->next)
    {
        destroy_chain(chain->next, destroy);
    }

    free(chain);
}

static map_chain_t *create_map_chain(char *key, void *value, map_chain_t *next)
{
    map_chain_t *chain;

    chain = malloc(sizeof(map_chain_t));

    if (chain)
    {
        chain->key = key;
        chain->value = value;
        chain->next = next;
    }

    return chain;
}

static map_t *create_map(int (*hash)(char *), void (*destroy)(void *), size_t length, size_t capacity, map_chain_t **chains)
{
    map_t *map;

    map = malloc(sizeof(map_t));

    if (map)
    {
        map->hash = hash;
        map->destroy = destroy;
        map->length = length;
        map->capacity = capacity;
        map->chains = chains;
    }

    return map;
}

static int resize_map(map_t *map)
{
    map_chain_t **existing, **chains;
    map_chain_t *chain;
    size_t expand, fill, index;

    existing = map->chains;
    expand = map->capacity * 2;
    fill = map->capacity;
    chains = calloc(expand, sizeof(map_chain_t *));

    if (!chains)
    {
        return 0;
    }

    map->capacity = expand;
    map->length = 0;
    map->chains = chains;

    for (index = 0; index < fill; index++)
    {
        for (chain = existing[index]; chain != NULL; chain = chain->next)
        {
            if (!set_map_item(map, chain->key, chain->value))
            {
                free(map->chains);
                return 0;
            }

            chain->key = NULL;
            chain->value = NULL;
        }
    }

    for (index = 0; index < fill; index++)
    {
        chain = existing[index];

        if (chain)
        {
            destroy_chain(chain, map->destroy);
        }
    }

    free(existing);

    return 1;
}
