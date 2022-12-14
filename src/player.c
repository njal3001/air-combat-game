#include "player.h"
#include <stdio.h>
#include <math.h>
#include "render.h"
#include "input.h"
#include "calc.h"

#define SPD_BASE           10.0f
#define SCALE               1.0f
#define ACCEL               10.0f
#define ANG_ACCEL           1.5f
#define ANG_DEACCEL         1.0f
#define ANG_SPD_MAX         0.8f
#define ORB_TARGET_START    2
#define ORB_TARGET_MUL      2
#define ORB_SPD_MUL         1.2f
#define LOOK_START          0.4f
#define LOOK_ANG_MAX        0.05f
#define LOOK_ANG_SPD        0.3f

struct player_data
{
    float spd;
    struct vec2 ang_spd;
    struct vec2 look_ang;
    uint32_t orb_level;
    uint32_t orb_amount;
};

static uint32_t calculate_orb_target(struct player_data *data)
{
    return ORB_TARGET_START + data->orb_level * ORB_TARGET_MUL;
}

struct actor *spawn_player(struct world *w, struct vec3 pos)
{
    struct actor *ac = new_actor(w, pos, ACTOR_TYPE_PLAYER);
    ac->transform.scale = vec3_create(SCALE, SCALE, SCALE);

    struct player_data *data = malloc(sizeof(struct player_data));
    data->spd = SPD_BASE;
    data->ang_spd = VEC2_ZERO;
    data->look_ang = VEC2_ZERO;
    data->orb_level = 0;
    data->orb_amount = 0;

    ac->data = data;

    return ac;
}

void player_update(struct actor *ac, float dt)
{
    struct player_data *data = ac->data;

    // Check wall collision
    struct actor *wall_hit =
        first_collide(ac->world, ac, actor_type_bit(ACTOR_TYPE_WALL));
    if (wall_hit)
    {
        actor_kill(ac);
        return;
    }

    struct vec2 rdir;

    const struct controller *cont = get_first_controller();
    if (cont)
    {
        rdir.x = cont->axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        rdir.y = -cont->axes[GLFW_GAMEPAD_AXIS_LEFT_X];
    }
    else
    {
        rdir.x = key_down(GLFW_KEY_S) - key_down(GLFW_KEY_W);
        rdir.y = key_down(GLFW_KEY_A) - key_down(GLFW_KEY_D);
    }

    struct vec2 ang_spd_target = vec2_mul(rdir, ANG_SPD_MAX);
    if (!vec2_eq(ang_spd_target, VEC2_ZERO))
    {
        data->ang_spd = vec2_approach(data->ang_spd, ang_spd_target,
                ANG_ACCEL * dt);
    }
    else
    {
        data->ang_spd = vec2_approach(data->ang_spd, ang_spd_target,
                ANG_DEACCEL * dt);
    }

    transform_local_rotx(&ac->transform, data->ang_spd.x * dt);
    transform_local_roty(&ac->transform, data->ang_spd.y * dt);

    float speed_target = SPD_BASE * powf(ORB_SPD_MUL, data->orb_level);
    data->spd = approach(data->spd, speed_target, ACCEL * dt);

    struct vec3 fwd = transform_forward(&ac->transform);
    vec3_add_eq(&ac->transform.pos, vec3_mul(fwd, data->spd * dt));

    // Rotate the camera towards the direction the player want's to travel
    // Don't change look angle if the direction change is small
    struct vec2 look_amount = vec2_sub(rdir, vec2_mul(rdir, LOOK_START));
    if (vec2_dot(rdir, look_amount) < 0.0f)
    {
        look_amount = VEC2_ZERO;
    }
    vec2_div_eq(&look_amount, 1.0f - LOOK_START);

    struct vec2 look_target = vec2_mul(look_amount, LOOK_ANG_MAX);
    data->look_ang = vec2_approach(data->look_ang, look_target,
            LOOK_ANG_SPD * dt);
}

void player_handle_collide(struct actor *ac, struct actor *hit)
{
    if (hit->type == ACTOR_TYPE_ORB)
    {
        struct player_data *data = ac->data;
        data->orb_amount++;

        uint32_t orb_target = calculate_orb_target(data);
        if (data->orb_amount >= orb_target)
        {
            data->orb_amount = 0;
            data->orb_level++;
        }
    }
}

void player_camera_view(struct actor *ac, struct camera *cam, float dt)
{
    struct player_data *data = ac->data;

    cam->transform.pos = ac->transform.pos;
    cam->transform.rot = ac->transform.rot;

    transform_local_rotx(&cam->transform, data->look_ang.x);
    transform_local_roty(&cam->transform, data->look_ang.y);
}

void player_render_crosshair(struct actor *ac, struct camera *cam)
{
    struct vec3 fwd = transform_forward(&ac->transform);
    struct vec3 wpos = vec3_add(ac->transform.pos, fwd);

    const float foffset = 24.0f;
    const float chsize = 0.5f;

    struct vec2 chpos = world_to_screen_pos(cam, wpos);
    chpos.x = chpos.x * UI_WIDTH - foffset * chsize;
    chpos.y = chpos.y * UI_HEIGHT + foffset * chsize;
    render_push_ui_text("o", chpos, chsize, COLOR_GREEN);

    struct vec2 screen_center;
    screen_center.x = UI_WIDTH / 2.0f - foffset * chsize;
    screen_center.y = UI_HEIGHT / 2.0f + foffset * chsize;
    render_push_ui_text("x", screen_center, chsize, COLOR_GREEN);
}

void player_render_state_info(struct actor *ac)
{
    struct player_data *data = ac->data;

    struct vec3 pos = ac->transform.pos;
    struct vec3 fwd = transform_forward(&ac->transform);
    uint32_t orb_target = calculate_orb_target(data);

    static char pinfo[256];
    snprintf(pinfo, 256, "Pos: (%f, %f, %f)\n"
            "Forward: (%f, %f, %f)\nSpd: %f\nAng Spd: (%f, %f)\nOrb: %d/%d\n",
            pos.x, pos.y, pos.z, fwd.x, fwd.y, fwd.z,
            data->spd, data->ang_spd.x, data->ang_spd.y,
            data->orb_amount, orb_target);

    render_push_ui_text(pinfo, vec2_create(10.0f, 150.0f), 0.4f, COLOR_WHITE);
}
