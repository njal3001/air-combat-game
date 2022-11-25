#pragma once
#include <stdlib.h>
#include "transform.h"

enum actor_type
{
    ACTOR_TYPE_PLAYER =         1 << 0,
    ACTOR_TYPE_ASTEROID =       1 << 1,
};

enum
{
    ACTOR_DEAD = 1 << 0,
};

struct actor;
typedef void (*ac_update)(struct actor *ac, float dt);
typedef void (*ac_death)(struct actor *ac);

struct cbox
{
    struct vec3 offset;
    struct vec3 bounds;
};

struct actor
{
    size_t id;
    struct transform transform;
    void *data;
    float hp;
    struct cbox cbox;
    int flags;
    enum actor_type type;
    size_t spawn_tick;
    ac_update update;
    ac_death death;
};

