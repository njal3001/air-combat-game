#pragma once
#include "transform.h"
#include "render.h"

struct player_data
{
    float speed;
    float rotation_speed;
    float reload;
    const struct mesh *mesh;
};

struct actor *spawn_player(struct vec3 pos);
