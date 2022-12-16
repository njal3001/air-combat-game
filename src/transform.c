#include "transform.h"
#include <math.h>
#include <stdio.h>

void transform_init(struct transform *t, struct vec3 pos)
{
    t->pos = pos;
    t->scale = vec3_create(1.0f, 1.0f, 1.0f);
    t->rot = mat4_identity();
}

struct vec3 transform_forward(const struct transform *t)
{
    return mat4_v3mul(t->rot, VEC3_FORWARD);
}

struct vec3 transform_up(const struct transform *t)
{
    return mat4_v3mul(t->rot, VEC3_UP);
}

struct vec3 transform_right(const struct transform *t)
{
    return mat4_v3mul(t->rot, VEC3_RIGHT);
}

struct mat4 transform_matrix(const struct transform *t)
{
    return mat4_mul(mat4_mul(mat4_translate(t->pos),
        t->rot), mat4_scale(t->scale));
}

void transform_local_rotx(struct transform *t, float delta)
{
    t->rot = mat4_mul(t->rot, mat4_rotx(delta));
}

void transform_local_roty(struct transform *t, float delta)
{
    t->rot = mat4_mul(t->rot, mat4_roty(delta));
}

void transform_local_rotz(struct transform *t, float delta)
{
    t->rot = mat4_mul(t->rot, mat4_rotz(delta));
}
