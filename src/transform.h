#pragma once
#include "spatial.h"

struct transform
{
    struct vec3 pos;
    struct vec3 scale;
    struct mat4 rot;
};

struct transform transform_create(struct vec3 pos);
struct vec3 transform_forward(const struct transform *t);
struct vec3 transform_up(const struct transform *t);
void transform_local_rotx(struct transform *t, float delta);
void transform_local_roty(struct transform *t, float delta);
void transform_local_rotz(struct transform *t, float delta);
