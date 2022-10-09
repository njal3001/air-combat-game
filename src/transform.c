#include "transform.h"
#include <math.h>
#include <stdio.h>

struct transform transform_create(struct vec3 pos)
{
    return (struct transform)
    {
        .pos = pos,
        .size = vec3_create(1.0f, 1.0f, 1.0f),
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
