#pragma once
#include "transform.h"
#include "world.h"
#include "camera.h"

struct actor *spawn_player(struct world *w, struct vec3 pos);
void player_update(struct actor *ac, float dt);
void player_camera_view(struct actor *ac, struct camera *cam, float dt);
void player_render_crosshair(struct actor *ac, struct camera *cam);
void player_render_state_info(struct actor *ac);
