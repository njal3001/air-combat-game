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

struct vec2 world_to_screen_pos(const struct camera *cam, struct vec3 wpos)
{
    struct mat4 view = camera_view(cam);
    struct mat4 proj = camera_projection(cam);

    struct vec4 wpos4 = vec4_create(wpos.x, wpos.y, wpos.z, 1.0f);
    struct vec4 cpos = mat4_v4mul(mat4_mul(proj, view), wpos4);
    struct vec4 cpos_norm = vec4_div(cpos, cpos.w);

    // Screen coordinates are from 0 to 1
    struct vec2 spos = vec2_create(cpos_norm.x * 0.5f + 0.5f,
            cpos_norm.y * 0.5f + 0.5f);

    return spos;
}
