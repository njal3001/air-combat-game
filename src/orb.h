#pragma once
#include "actor.h"
#include "world.h"

void spawn_orb(struct world *w, struct vec3 pos);
void orb_update(struct actor *ac, float dt);
void orb_handle_collide(struct actor *ac, struct actor *hit);
