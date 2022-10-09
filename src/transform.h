#pragma once
#include "spatial.h"

struct transform
{
    struct vec3 pos;
    struct vec3 size;
    struct vec3 rot;
};

struct transform transform_create(struct vec3 pos);
struct vec3 transform_forward(const struct transform *t);

struct mat4 global_rotation(struct vec3 rot);
