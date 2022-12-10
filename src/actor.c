#include "actor.h"

struct render_spec rspecs[ACTOR_TYPE_END];

void actor_types_init()
{
    rspecs[ACTOR_TYPE_PLAYER].mesh_handle = ASSET_MESH_PLAYER;
    rspecs[ACTOR_TYPE_ORB].mesh_handle = ASSET_MESH_ORB;
}

void cbox_init(struct cbox *c)
{
    c->bounds = VEC3_ONE;
    c->offset = VEC3_ZERO;
}

void actor_init(struct actor *ac, struct world *world, uint16_t id,
        enum actor_type type, uint8_t spawn_tick, struct vec3 pos)
{
    ac->id = id;
    ac->world = world;
    ac->type = type;
    ac->flags = 0;
    ac->spawn_tick = spawn_tick;
    transform_init(&ac->transform, pos);
    cbox_init(&ac->cbox);
    ac->data = NULL;
}

void actor_free(struct actor *ac)
{
    free(ac->data);
}

void actor_kill(struct actor *ac)
{
    ac->flags |= ACTOR_DEAD;
}

int actor_type_bit(enum actor_type type)
{
    return 1 << type;
}

struct render_spec actor_type_render_spec(enum actor_type type)
{
    return rspecs[type];
}
