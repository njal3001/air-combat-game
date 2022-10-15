#pragma once
#include <stdlib.h>

struct hashmap;

struct hashmap *hashmap_new();
void hashmap_put(struct hashmap *map, const char *key, void *value);
void *hashmap_get(struct hashmap *map, const char *key);
size_t hashmap_size(struct hashmap *map);
void hashmap_values(struct hashmap *map, void ** values);
void hashmap_free(struct hashmap *map);
