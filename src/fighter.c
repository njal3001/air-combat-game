#include "fighter.h"
#include <stdlib.h>
#include <math.h>
#include "game.h"
#include "render.h"

void fighter_init(struct fighter *fighter, struct vec3 pos)
{
    fighter->transform = transform_create(pos);
    fighter->speed = 0.1f;
    fighter->rotation_speed = 0.01f;

    struct vertex *vertices = malloc(4 * sizeof(struct vertex));
    vertices[0].pos = vec3_create(-1.0f, -1.0f, -1.0f);
    vertices[0].col = COLOR_WHITE;
    vertices[0].uvx = 0.0f;
    vertices[0].uvy = 0.0f;

    vertices[1].pos = vec3_create(-1.0f, 1.0f, -1.0f);
    vertices[1].col = COLOR_WHITE;
    vertices[1].uvx = 0.0f;
    vertices[1].uvy = 1.0f;

    vertices[2].pos = vec3_create(1.0f, 1.0f, -1.0f);
    vertices[2].col = COLOR_WHITE;
    vertices[2].uvx = 1.0f;
    vertices[2].uvy = 1.0f;

    vertices[3].pos = vec3_create(1.0f, -1.0f, -1.0f);
    vertices[3].col = COLOR_WHITE;
    vertices[3].uvx = 1.0f;
    vertices[3].uvy = 0.0f;

    GLushort *indices = malloc(6 * sizeof(GLushort));
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 3;
    indices[3] = 1;
    indices[4] = 2;
    indices[5] = 3;

    fighter->mesh.index_count = 6;
    fighter->mesh.vertex_count = 4;
    fighter->mesh.vertices = vertices;
    fighter->mesh.indices = indices;

    fighter->transform.scale.x = 0.5f;
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
        fighter->transform.rot.z -= fighter->rotation_speed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        fighter->transform.rot.z += fighter->rotation_speed;
    }

    struct vec3 forward = transform_forward(&fighter->transform);
    vec3_sub_eq(&fighter->transform.pos, vec3_mul(forward, fighter->speed));

    struct camera *cam = get_camera();
    cam->transform.pos.x = fighter->transform.pos.x;
    cam->transform.pos.y = fighter->transform.pos.y + 20.0f;
    cam->transform.pos.z = fighter->transform.pos.z;

    cam->transform.rot.x = M_PI / 2.0f;
    cam->transform.rot.y = 0.0f;
    cam->transform.rot.z = 0.0f;
}

void fighter_render(struct fighter *fighter)
{
    struct mat4 m = transform_matrix(&fighter->transform);
    render_mpush(&m);
    render_mesh(&fighter->mesh);
    render_mpop();
}

void fighter_free(struct fighter *fighter)
{
    free(fighter->mesh.vertices);
    free(fighter->mesh.indices);
}
