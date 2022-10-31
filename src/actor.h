#pragma once
#include <stdlib.h>
#include "transform.h"

enum actor_type
{
    ACTOR_TYPE_PLAYER =         1 << 0,
    ACTOR_TYPE_PROJECTILE =     1 << 1,
    ACTOR_TYPE_ASTEROID =       1 << 2,
    ACTOR_TYPE_ENERGYCELL =     1 << 3,
};

enum
{
    ACTOR_DEAD = 1 << 0,
};

struct actor;
typedef void (*ac_update)(struct actor *ac, float dt);
typedef void (*ac_render)(struct actor *ac);
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
    ac_render render;
    ac_death death;
};

