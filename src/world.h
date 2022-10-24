#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "transform.h"

enum actor_type
{
    ACTOR_TYPE_PLAYER =     1 << 0,
    ACTOR_TYPE_PROJECTILE = 1 << 1,
    ACTOR_TYPE_ASTEROID =   1 << 2,
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

void world_init();
void world_update(float dt);
void world_render();
void world_end();
void world_free();

struct actor *new_actor();
void actor_hurt(struct actor *ac, float dmg);

struct actor *first_collide(const struct actor *ac, int type_mask);

struct actor *spawn_projectile(struct vec3 pos, float speed);
