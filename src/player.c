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

#define SPD_START 1000.0f
#define SPD_MAX 8000.0f
#define ACCEL_START 100.0f
#define ANG_ACCEL 3.0f
#define ANG_SPD_MAX 1.0f

static void player_update(struct actor *ac, float dt);
static void player_death(struct actor *ac);

struct actor *spawn_player(struct vec3 pos)
{
    struct actor *ac = new_actor();
    ac->transform = transform_create(pos);
    ac->transform.scale = vec3_create(2.0f, 2.0f, 2.0f);
    ac->update = player_update;
    ac->death = player_death;
    ac->type = ACTOR_TYPE_PLAYER;
    ac->hp = 50.0f;
    ac->cbox.offset = VEC3_ZERO;
    ac->cbox.bounds = VEC3_ONE;

    struct player_data *data = malloc(sizeof(struct player_data));
    data->spd = SPD_START;
    data->ang_spdx = 0.0f;
    data->ang_spdy = 0.0f;
    ac->data = data;

    return ac;
}

void player_update(struct actor *ac, float dt)
{
    struct player_data *data = ac->data;

    struct actor *hit = first_collide(ac, ACTOR_TYPE_ASTEROID);
    if (hit)
    {
        hit->flags |= ACTOR_DEAD;
        actor_hurt(ac, 10.0f);
    }

    struct vec3 fwd = transform_forward(&ac->transform);

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

    // Acceleration decreases as speed increases
    float accel = ((SPD_MAX - data->spd) / (SPD_MAX - SPD_START)) * ACCEL_START;
    data->spd += accel * dt;
    vec3_add_eq(&ac->transform.pos, vec3_mul(fwd, data->spd * dt));

    struct camera *cam = get_camera();
    cam->transform.pos = ac->transform.pos;
    cam->transform.rot = ac->transform.rot;
}

void player_death(struct actor *ac)
{
    world_end();
}

void player_render_state_info(struct actor *ac)
{
    struct player_data *data = ac->data;

    struct vec3 pos = ac->transform.pos;
    struct vec3 fwd = transform_forward(&ac->transform);

    static char pinfo[256];
    snprintf(pinfo, 256, "HP: %f\nPos: (%f, %f, %f)\n"
            "Forward: (%f, %f, %f)\nSpd: %f\nAng Spd: (%f, %f)\n",
            ac->hp, pos.x, pos.y, pos.z, fwd.x, fwd.y, fwd.z,
            data->spd, data->ang_spdx, data->ang_spdy);

    push_text(pinfo, 15.0f, 160.0f, 0.4f);
}
