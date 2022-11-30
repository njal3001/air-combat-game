
#include "actor.h"

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
