#include "transform.h"
#include <math.h>
#include <stdio.h>

struct transform transform_create(struct vec3 pos)
{
    return (struct transform)
    {
        .pos = pos,
        .scale = vec3_create(1.0f, 1.0f, 1.0f),
        .rot = VEC3_ZERO,
    };
}

struct vec3 transform_forward(const struct transform *t)
{
    struct vec3 res;
    res.x = -cosf(t->rot.x) * sinf(t->rot.y);
    res.y = sinf(t->rot.x);
    res.z = cosf(t->rot.y) * cosf(t->rot.x);

    return vec3_normalize(res);
}

struct mat4 transform_matrix(struct transform *t)
{
    struct mat4 rot = mat4_mul(mat4_rotz(t->rot.z),
            mat4_mul(mat4_roty(t->rot.y), mat4_rotx(t->rot.x)));
    return mat4_mul(mat4_mul(mat4_translate(t->pos), rot), mat4_scale(t->scale));
}
