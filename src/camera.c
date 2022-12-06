#include "camera.h"

struct mat4 camera_view(const struct camera *camera)
{
    struct vec3 cam_forward = transform_forward(&camera->transform);
    struct vec3 cam_up = transform_up(&camera->transform);
    struct vec3 cam_target = vec3_add(camera->transform.pos, cam_forward);

    return mat4_lookat(camera->transform.pos, cam_target, cam_up);
}

struct mat4 camera_projection(const struct camera *camera)
{
    return mat4_perspective(camera->fov, camera->aspect,
            camera->near, camera->far);
}
