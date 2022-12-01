#include "orb.h"

void spawn_orb(struct world *w, struct vec3 pos)
{
    struct actor *ac = new_actor(w, pos, ACTOR_TYPE_ORB);
    vec3_div_eq(&ac->transform.scale, 2.0f);
}

void orb_update(struct actor *ac, float dt)
{

}
