#include "player.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "game.h"
#include "render.h"
#include "input.h"
#include "assets.h"
#include "world.h"
#include "audio.h"
#include "calc.h"
#include "timer.h"

#define SPD_NORM 500.0f
#define SPD_MIN 400.0f
#define SPD_MAX 600.0f
#define ACCEL 100.0f
#define ANG_ACCEL 3.0f
#define ANG_SPD_MAX 1.0f
#define ENERGY_MAX 25.0f
#define ENERGY_REFILL 3.0f

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
    data->spd = SPD_NORM;
    data->ang_spdx = 0.0f;
    data->ang_spdy = 0.0f;
    data->energy = ENERGY_MAX;
    data->reload = 0.0f;
    data->mesh = get_mesh("cube.mesh");
    ac->data = data;

    return ac;
}

void player_update(struct actor *ac, float dt)
{
    struct player_data *data = ac->data;
    struct vec3 fwd = transform_forward(&ac->transform);

    int dir = key_down(GLFW_KEY_K) - key_down(GLFW_KEY_J);
    int rdirx = key_down(GLFW_KEY_S) - key_down(GLFW_KEY_W);
    int rdiry = key_down(GLFW_KEY_A) - key_down(GLFW_KEY_D);

    if (rdirx)
    {
        data->ang_spdx = fclamp(-ANG_SPD_MAX,
                data->ang_spdx + ANG_ACCEL * dt * rdirx, ANG_SPD_MAX);
    }
    else
    {
        data->ang_spdx = approach(data->ang_spdx, 0.0f, ANG_ACCEL * dt);
    }
    transform_local_rotx(&ac->transform, data->ang_spdx * dt);

    if (rdiry)
    {
        data->ang_spdy = fclamp(-ANG_SPD_MAX,
                data->ang_spdy + ANG_ACCEL * dt * rdiry, ANG_SPD_MAX);
    }
    else
    {
        data->ang_spdy = approach(data->ang_spdy, 0.0f, ANG_ACCEL * dt);
    }
    transform_local_roty(&ac->transform, data->ang_spdy * dt);

    if (dir)
    {
        data->spd = fclamp(SPD_MIN, data->spd + ACCEL * dt * dir, SPD_MAX);
    }
    else
    {
        data->spd = approach(data->spd, SPD_NORM, ACCEL * dt);
    }

    vec3_add_eq(&ac->transform.pos, vec3_mul(fwd, data->spd * dt));

    data->reload -= dt;
    if (data->reload <= 0.0f && key_down(GLFW_KEY_L))
    {
        struct vec3 pr_pos = vec3_sub(vec3_add(ac->transform.pos, vec3_mul(fwd, 1.2f)),
                vec3_mul(transform_up(&ac->transform), 2.0f));
        struct actor *pr = spawn_projectile(pr_pos, 100.0f + data->spd);
        pr->transform.rot = ac->transform.rot;

        data->reload = 0.25f;
        audio_play("laser7.wav");
    }

    data->energy -= dt;
    if (data->energy <= 0.0f)
    {
        // ac->flags |= ACTOR_DEAD;
    }

    struct camera *cam = get_camera();
    cam->transform.pos = ac->transform.pos;
    cam->transform.rot = ac->transform.rot;
}

void player_death(struct actor *ac)
{
    world_end();
}

void player_energize(struct actor *ac)
{
    struct player_data *data = ac->data;
    data->energy += ENERGY_REFILL;
}

void player_render(struct actor *ac)
{
    struct player_data *data = ac->data;

    set_texture(get_texture("rusted_metal.jpg"));
    push_mesh(data->mesh, &ac->transform);
}

void player_render_debug_panel(struct actor *ac)
{
    struct player_data *data = ac->data;

    struct vec3 pos = ac->transform.pos;
    struct vec3 fwd = transform_forward(&ac->transform);

    static char dpbuf[256];
    snprintf(dpbuf, 256, "FPS: %d\nHP: %f\nEnergy: %f\nPos: (%f, %f, %f)\n"
            "Forward: (%f, %f, %f)\nSpd: %f\nAng Spd: (%f, %f)\n",
            timer_fps(), ac->hp, data->energy, pos.x, pos.y, pos.z, fwd.x, fwd.y, fwd.z,
            data->spd, data->ang_spdx, data->ang_spdy);

    push_text(dpbuf, 50.0f, 200.0f, 0.4f);
}
