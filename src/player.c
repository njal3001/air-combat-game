#include "player.h"
#include <stdio.h>
#include "render.h"
#include "input.h"
#include "calc.h"

#define SPD_START 10.0f
#define SPD_MAX 100.0f
#define ACCEL_START 10.0f
#define ANG_ACCEL 3.0f
#define ANG_SPD_MAX 1.0f

struct player_data
{
    float spd;
    float ang_spdx;
    float ang_spdy;
};

struct actor *spawn_player(struct world *w, struct vec3 pos)
{
    struct actor *ac = new_actor(w, pos, ACTOR_TYPE_PLAYER);
    ac->transform.scale = vec3_create(5.0f, 5.0f, 5.0f);

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

    struct vec3 fwd = transform_forward(&ac->transform);
    float rdirx;
    float rdiry;

    const struct controller *cont = get_first_controller();
    if (cont)
    {
        rdirx = cont->axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        rdiry = -cont->axes[GLFW_GAMEPAD_AXIS_LEFT_X];
    }
    else
    {
        rdirx = key_down(GLFW_KEY_S) - key_down(GLFW_KEY_W);
        rdiry = key_down(GLFW_KEY_A) - key_down(GLFW_KEY_D);
    }

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
    // float accel = ((SPD_MAX - data->spd) / (SPD_MAX - SPD_START)) *
    //     ACCEL_START;
    // data->spd += accel * dt;
    vec3_add_eq(&ac->transform.pos, vec3_mul(fwd, data->spd * dt));
}

void player_render_state_info(struct actor *ac)
{
    struct player_data *data = ac->data;

    struct vec3 pos = ac->transform.pos;
    struct vec3 fwd = transform_forward(&ac->transform);

    static char pinfo[256];
    snprintf(pinfo, 256, "Pos: (%f, %f, %f)\n"
            "Forward: (%f, %f, %f)\nSpd: %f\nAng Spd: (%f, %f)\n",
            pos.x, pos.y, pos.z, fwd.x, fwd.y, fwd.z,
            data->spd, data->ang_spdx, data->ang_spdy);

    push_text(pinfo, 10.0f, 110.0f, 0.4f);
}
