#pragma once
#include "transform.h"
#include "render.h"

struct fighter
{
    struct transform transform;
    float speed;
    float rotation_speed;
    const struct mesh *mesh;
};

void fighter_init(struct fighter *fighter, struct vec3 pos);
void fighter_update(struct fighter *fighter, float dt);
void fighter_render(struct fighter *fighter);
