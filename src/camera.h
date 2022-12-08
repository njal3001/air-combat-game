#pragma once
#include "transform.h"

struct camera
{
    struct transform transform;
    float fov;
    float aspect;
    float near;
    float far;
};

struct mat4 camera_view(const struct camera *camera);
struct mat4 camera_projection(const struct camera *camera);

struct vec2 world_to_screen_pos(const struct camera *cam, struct vec3 wpos);
