#include "camera.h"
#include <GLFW/glfw3.h>
#include "input.h"

struct mat4 camera_view(const struct camera *camera)
{
    struct vec3 cam_forward = transform_forward(&camera->transform);
    struct vec3 cam_up = transform_up(&camera->transform);
    struct vec3 cam_target = vec3_add(camera->transform.pos, cam_forward);

    return mat4_lookat(camera->transform.pos, cam_target, cam_up);
}

void camera_free_update(struct camera *camera)
{
    const float rotation_speed = 0.015f;
    const float speed = 5.0f;

    if (key_down(GLFW_KEY_W))
    {
        transform_local_rotx(&camera->transform, -rotation_speed);
    }
    if (key_down(GLFW_KEY_S))
    {
        transform_local_rotx(&camera->transform, rotation_speed);
    }
    if (key_down(GLFW_KEY_D))
    {
        transform_local_roty(&camera->transform, -rotation_speed);
    }
    if (key_down(GLFW_KEY_A))
    {
        transform_local_roty(&camera->transform, rotation_speed);
    }
    if (key_down(GLFW_KEY_Q))
    {
        transform_local_rotz(&camera->transform, -rotation_speed);
    }
    if (key_down(GLFW_KEY_E))
    {
        transform_local_rotz(&camera->transform, rotation_speed);
    }

    struct vec3 forward = transform_forward(&camera->transform);
    if (key_down(GLFW_KEY_UP))
    {
        vec3_add_eq(&camera->transform.pos, vec3_mul(forward, speed));
    }
    if (key_down(GLFW_KEY_DOWN))
    {
        vec3_sub_eq(&camera->transform.pos, vec3_mul(forward, speed));
    }
}
