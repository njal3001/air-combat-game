#pragma once
#include <stdlib.h>
#include <stdint.h>
#include "transform.h"

enum actor_type
{
    ACTOR_TYPE_PLAYER = 1 << 0,
};

enum
{
    ACTOR_DEAD = 1 << 0,
};

struct cbox
{
    struct vec3 offset;
    struct vec3 bounds;
};

struct actor
{
    uint16_t id;
    struct transform transform;
    struct cbox cbox;
    int flags;
    enum actor_type type;
    uint8_t spawn_tick;
    void *data;
};

void actor_init(struct actor *ac, uint16_t id, enum actor_type type,
        uint8_t spawn_tick, struct vec3 pos);

void actor_free(struct actor *ac);
