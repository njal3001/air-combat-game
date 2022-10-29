#pragma once
#include "transform.h"
#include "render.h"

struct player_data
{
    float spd;
    float ang_spdx;
    float ang_spdy;
    float energy;
    float reload;
    const struct mesh *mesh;
};

struct actor *spawn_player(struct vec3 pos);
void player_energize(struct actor *ac);
void player_render_debug_panel(struct actor *ac);
