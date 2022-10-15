#include "hashmap.h"
#include <assert.h>
#include <string.h>

#define NUM_BUCKETS_START 16
#define BUCKET_SIZE 8

struct hashelem
{
    char *key;
    void *value;
};

struct hashbucket
{
    struct hashelem elements[BUCKET_SIZE];
    size_t count;
};

struct hashmap
{
    size_t size;
    size_t bucket_count;
    struct hashbucket *buckets;
};

// djb2 hash algorithm
static size_t str_hash(const char *str)
{
    size_t hash = 5381;
    int c;

    while ((c = *(str++)))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

struct hashmap *hashmap_new()
{
    struct hashmap *m = malloc(sizeof(struct hashmap));
    m->size = 0;
    m->bucket_count = NUM_BUCKETS_START;

    m->buckets = calloc(NUM_BUCKETS_START, sizeof(struct hashbucket));

    return m;
}

void hashmap_put(struct hashmap *map, const char *key, void *value)
{
    size_t hash = str_hash(key) % map->bucket_count;
    struct hashbucket *bucket = map->buckets + hash;

    // TODO: Do rehash instead
    assert(bucket->count < BUCKET_SIZE);

    for (size_t i = 0; i < bucket->count; i++)
    {
        struct hashelem *e = bucket->elements + i;
        if (strcmp(e->key, key) == 0)
        {
            e->value = value;
            return;
        }
    }

    struct hashelem *elem_new = bucket->elements + bucket->count;
    elem_new->key = strdup(key);
    elem_new->value = value;

    bucket->count++;
    map->size++;
}

void *hashmap_get(struct hashmap *map, const char *key)
{
    size_t hash = str_hash(key) & map->bucket_count;
    struct hashbucket *bucket = map->buckets + hash;

    for (size_t i = 0; i < bucket->count; i++)
    {
        struct hashelem *e = bucket->elements + i;
        if (strcmp(e->key, key) == 0)
        {
            return e->value;
        }
    }

    return NULL;
}

size_t hashmap_size(struct hashmap *map)
{
    return map->size;
}

void hashmap_values(struct hashmap *map, void **values)
{
    size_t i = 0;
    for (size_t b = 0; b < map->bucket_count; b++)
    {
        struct hashbucket *bucket = map->buckets + b;
        for (size_t e = 0; e < bucket->count; e++)
        {
            values[i] = bucket->elements->value;
            i++;
        }
    }
}

void hashmap_free(struct hashmap *map)
{
    for (size_t b = 0; b < map->bucket_count; b++)
    {
        struct hashbucket *bucket = map->buckets + b;
        for (size_t e = 0; e < bucket->count; e++)
        {
            struct hashelem *elem = bucket->elements + e;
            free(elem->key);
        }

        free(bucket->elements);
    }

    free(map->buckets);
    free(map);
}
