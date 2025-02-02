#pragma once
#include "vector.h"

struct transform
{
    struct vec3 pos;
    struct vec3 scale;
    struct mat4 rot;
};

void transform_init(struct transform *t, struct vec3 pos);
struct vec3 transform_forward(const struct transform *t);
struct vec3 transform_up(const struct transform *t);
struct vec3 transform_right(const struct transform *t);
struct mat4 transform_matrix(const struct transform *t);
void transform_local_rotx(struct transform *t, float delta);
void transform_local_roty(struct transform *t, float delta);
void transform_local_rotz(struct transform *t, float delta);
