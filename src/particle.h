#pragma once
#include <stdlib.h>
#include "spatial.h"
#include "color.h"

struct particle
{
    struct vec3 dir;
    float t;
};

void speed_lines_update_and_render(size_t count, float ttl,
        float speed, float length, float off, struct vec3 offset,
        struct mat4 rot, float dt);
