#include "actor.h"

struct render_spec rspecs[ACTOR_TYPE_END];

void actor_types_init()
{
    rspecs[ACTOR_TYPE_PLAYER].mesh_handle = ASSET_MESH_PLAYER;

    rspecs[ACTOR_TYPE_ORB].mesh_handle = ASSET_MESH_ORB;
}

void actor_init(struct actor *ac, uint16_t id, enum actor_type type,
        uint8_t spawn_tick, struct vec3 pos)
{
    ac->id = id;
    ac->type = type;
    ac->flags = 0;
    ac->spawn_tick = spawn_tick;
    ac->transform = transform_create(pos);
    ac->cbox.bounds = VEC3_ONE;
    ac->cbox.offset = VEC3_ZERO;
    ac->data = NULL;
}

void actor_free(struct actor *ac)
{
    free(ac->data);
}

int actor_type_bit(enum actor_type type)
{
    return 1 << type;
}

struct render_spec actor_type_render_spec(enum actor_type type)
{
    return rspecs[type];
}
