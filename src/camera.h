#pragma once
#include "transform.h"

struct camera
{
    struct transform transform;
};

struct mat4 camera_view(const struct camera *camera);

void camera_free_update(struct camera *camera);
