#pragma once
#include "transform.h"
#include "render.h"
#include "world.h"

struct actor *spawn_player(struct world *w, struct vec3 pos);
void player_update(struct actor *ac, float dt);
void player_render_state_info(struct actor *ac);
