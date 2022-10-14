#include "fighter.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "game.h"
#include "render.h"

void fighter_init(struct fighter *fighter, struct vec3 pos)
{
    fighter->transform = transform_create(pos);
    fighter->speed = 0.5f;
    fighter->rotation_speed = 0.015f;

    fighter->mesh = mesh_create("../assets/WWIairplane.obj");
}

void fighter_update(struct fighter *fighter, float dt)
{
    GLFWwindow *window = get_window();

    struct camera *cam = get_camera();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        transform_local_rotx(&fighter->transform, -fighter->rotation_speed);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        transform_local_rotx(&fighter->transform, fighter->rotation_speed);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        transform_local_roty(&fighter->transform, -fighter->rotation_speed);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        transform_local_roty(&fighter->transform, fighter->rotation_speed);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        transform_local_rotz(&fighter->transform, -fighter->rotation_speed);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        transform_local_rotz(&fighter->transform, fighter->rotation_speed);
    }

    struct vec3 forward = transform_forward(&fighter->transform);
    vec3_add_eq(&fighter->transform.pos, vec3_mul(forward, fighter->speed));

    cam->transform.rot = fighter->transform.rot;
    cam->transform.pos = vec3_add(fighter->transform.pos,
            vec3_mul(transform_up(&cam->transform), 4.0f));
}

void fighter_render(struct fighter *fighter)
{
    render_mpush(transform_matrix(&fighter->transform));
    render_mesh(&fighter->mesh);
    render_mpop();
}

void fighter_free(struct fighter *fighter)
{
    free(fighter->mesh.vertices);
    free(fighter->mesh.indices);
}
