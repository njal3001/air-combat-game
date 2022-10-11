#include "fighter.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "game.h"
#include "render.h"

void fighter_init(struct fighter *fighter, struct vec3 pos)
{
    fighter->transform = transform_create(pos);
    fighter->speed = 0.1f;
    fighter->rotation_speed = 0.01f;

    fighter->mesh = mesh_create("../assets/WWIairplane.obj");
}

void fighter_update(struct fighter *fighter, float dt)
{
    GLFWwindow *window = get_window();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        fighter->transform.rot.x += fighter->rotation_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        fighter->transform.rot.x -= fighter->rotation_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        fighter->transform.rot.y += fighter->rotation_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        fighter->transform.rot.y -= fighter->rotation_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        // TODO: Should it not be add?
        fighter->transform.rot.z -= fighter->rotation_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        fighter->transform.rot.z += fighter->rotation_speed;
    }

    struct vec3 forward = transform_forward(&fighter->transform);
    vec3_add_eq(&fighter->transform.pos, vec3_mul(forward, fighter->speed));

    struct camera *cam = get_camera();
    cam->transform.pos.x = fighter->transform.pos.x;
    cam->transform.pos.y = fighter->transform.pos.y + 20.0f;
    cam->transform.pos.z = fighter->transform.pos.z;

    cam->transform.rot.x = M_PI / 2.0f;
    cam->transform.rot.y = 0.0f;
    cam->transform.rot.z = 0.0f;

    vec3_print(fighter->transform.pos);
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
