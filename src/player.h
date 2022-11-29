#pragma once
#include "transform.h"
#include "render.h"

struct player_data
{
    float spd;
    float ang_spdx;
    float ang_spdy;
};

struct actor *spawn_player(struct vec3 pos);
void player_render_state_info(struct actor *ac);
float player_speed(struct actor *ac);
