#pragma once
#include <stdlib.h>
#include <stdint.h>
#include "transform.h"
#include "asset.h"

enum actor_type
{
    ACTOR_TYPE_PLAYER,
    ACTOR_TYPE_ORB,
    ACTOR_TYPE_END,
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
    struct world *world;
    struct transform transform;
    struct cbox cbox;
    int flags;
    enum actor_type type;
    uint8_t spawn_tick;
    void *data;
};

struct render_spec
{
    enum asset_mesh mesh_handle;
};

void actor_types_init();

void actor_init(struct actor *ac, struct world *world, uint16_t id, enum actor_type type,
        uint8_t spawn_tick, struct vec3 pos);
void actor_free(struct actor *ac);

void actor_kill(struct actor *ac);

int actor_type_bit(enum actor_type type);
struct render_spec actor_type_render_spec(enum actor_type type);
