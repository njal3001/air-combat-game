#include "fighter.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "game.h"
#include "render.h"
#include "input.h"
#include "assets.h"

void fighter_init(struct fighter *fighter, struct vec3 pos)
{
    fighter->transform = transform_create(pos);
    fighter->speed = 3.5f;
    fighter->rotation_speed = 0.015f;

    fighter->mesh = get_mesh("cube.mesh");
}

void fighter_update(struct fighter *fighter, float dt)
{
    struct camera *cam = get_camera();
    if (key_down(GLFW_KEY_W))
    {
        transform_local_rotx(&fighter->transform, -fighter->rotation_speed);
    }
    if (key_down(GLFW_KEY_S))
    {
        transform_local_rotx(&fighter->transform, fighter->rotation_speed);
    }
    if (key_down(GLFW_KEY_D))
    {
        transform_local_roty(&fighter->transform, -fighter->rotation_speed);
    }
    if (key_down(GLFW_KEY_A))
    {
        transform_local_roty(&fighter->transform, fighter->rotation_speed);
    }
    if (key_down(GLFW_KEY_Q))
    {
        transform_local_rotz(&fighter->transform, -fighter->rotation_speed);
    }
    if (key_down(GLFW_KEY_E))
    {
        transform_local_rotz(&fighter->transform, fighter->rotation_speed);
    }

    struct vec3 forward = transform_forward(&fighter->transform);
    vec3_add_eq(&fighter->transform.pos, vec3_mul(forward, fighter->speed));

    cam->transform.rot = fighter->transform.rot;
    cam->transform.pos = vec3_add(fighter->transform.pos,
            vec3_mul(transform_up(&cam->transform), 4.0f));

    struct point_light *light = get_point_light();
    light->pos = fighter->transform.pos;
}

void fighter_render(struct fighter *fighter)
{
    render_mesh(fighter->mesh, &fighter->transform);
}
