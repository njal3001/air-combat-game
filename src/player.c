#include "player.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "game.h"
#include "render.h"
#include "input.h"
#include "assets.h"
#include "world.h"

static void player_update(struct actor *ac, float dt);
static void player_render(struct actor *ac);
static void player_death(struct actor *ac);

struct actor *spawn_player(struct vec3 pos)
{
    struct actor *ac = new_actor();
    ac->transform = transform_create(pos);
    ac->update = player_update;
    ac->render = player_render;
    ac->death = player_death;
    ac->type = ACTOR_TYPE_PLAYER;
    ac->hp = 150.0f;
    ac->cbox.offset = VEC3_ZERO;
    ac->cbox.bounds = VEC3_ONE;

    struct player_data *data = malloc(sizeof(struct player_data));
    data->speed = 16.0f;
    data->rotation_speed = 0.35;
    data->reload = 0.0f;
    data->mesh = get_cube_mesh();
    ac->data = data;

    return ac;
}

void player_update(struct actor *ac, float dt)
{
    struct player_data *data = ac->data;
    float drot = data->rotation_speed * dt;
    float dpos = data->speed * dt;

    if (key_down(GLFW_KEY_W))
    {
        transform_local_rotx(&ac->transform, -drot);
    }
    if (key_down(GLFW_KEY_S))
    {
        transform_local_rotx(&ac->transform, drot);
    }
    if (key_down(GLFW_KEY_D))
    {
        transform_local_roty(&ac->transform, -drot);
    }
    if (key_down(GLFW_KEY_A))
    {
        transform_local_roty(&ac->transform, drot);
    }
    if (key_down(GLFW_KEY_Q))
    {
        transform_local_rotz(&ac->transform, -drot);
    }
    if (key_down(GLFW_KEY_E))
    {
        transform_local_rotz(&ac->transform, drot);
    }

    struct vec3 forward = transform_forward(&ac->transform);

    data->reload -= dt;
    if (data->reload <= 0.0f && key_down(GLFW_KEY_K))
    {
        struct vec3 pr_pos = vec3_add(ac->transform.pos, vec3_mul(forward, 1.2f));
        struct actor *pr = spawn_projectile(pr_pos, 50.0f);
        pr->transform.rot = ac->transform.rot;

        data->reload = 0.25f;
    }

    if (key_down(GLFW_KEY_UP))
    {
        vec3_add_eq(&ac->transform.pos, vec3_mul(forward, dpos));
    }
    else if (key_down(GLFW_KEY_DOWN))
    {
        vec3_sub_eq(&ac->transform.pos, vec3_mul(forward, dpos));
    }

    // vec3_add_eq(&ac->transform.pos, vec3_mul(forward, dpos));

    struct camera *cam = get_camera();
    cam->transform.pos = ac->transform.pos;
    cam->transform.rot = ac->transform.rot;

    // cam->transform.rot = mat4_rotx(M_PI / 2.0f);
    // cam->transform.pos = vec3_add(player->transform.pos, vec3_mul(VEC3_UP, 50.0f));

    // printf("Pos: ");
    // vec3_print(player->transform.pos);
}

void player_death(struct actor *ac)
{
    world_end();
}

void player_render(struct actor *ac)
{
    struct player_data *data = ac->data;

    set_texture(get_texture("rusted_metal.jpg"));
    render_mesh(data->mesh, &ac->transform);
}
