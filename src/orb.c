#include "orb.h"
#include <math.h>
#include "collide.h"
#include "player.h"

#define SPD_NORM            2.0f
#define ACCEL_NORM          1.0f
#define ATTRACT_SPD_MAX     3.5f
#define ATTRACT_RANGE       3.5f
#define ATTRACT_ACCEL_MAX   10.0f
#define WIGGLE_AMPLITUDE    0.035f
#define WIGGLE_PERIOD       2.0f

struct orb_data
{
    struct vec3 vel;
    struct vec3 dir;
};

void spawn_orb(struct world *w, struct vec3 pos)
{
    struct actor *ac = new_actor(w, pos, ACTOR_TYPE_ORB);
    vec3_div_eq(&ac->transform.scale, 4.0f);

    struct orb_data *data = malloc(sizeof(struct orb_data));
    data->vel = VEC3_ZERO;
    data->dir = vec3_rand();

    ac->data = data;
}

void orb_update(struct actor *ac, float dt)
{
    struct orb_data *data = ac->data;

    struct vec3 target_vel = vec3_mul(data->dir, SPD_NORM);
    float accel = ACCEL_NORM;

    struct actor *player = ac->world->player;
    if (player)
    {
        struct vec3 ppos = player->transform.pos;
        struct vec3 diff = vec3_sub(ppos, ac->transform.pos);

        float len = vec3_length(diff);
        if (len < ATTRACT_RANGE)
        {
            struct vec3 dir = vec3_div(diff, len);
            target_vel = vec3_mul(dir, ATTRACT_SPD_MAX);
            accel =
                ((ATTRACT_RANGE - len) / ATTRACT_RANGE) * ATTRACT_ACCEL_MAX;

            if (check_collide(player, ac))
            {
                player_handle_collide(player, ac);
                actor_kill(ac);
            }
        }
    }

    data->vel = vec3_approach(data->vel, target_vel, accel * dt);
    vec3_add_eq(&ac->transform.pos, vec3_mul(data->vel, dt));
}

void orb_handle_collide(struct actor *ac, struct actor *hit)
{
    struct orb_data *data = ac->data;
    if (ac->type == ACTOR_TYPE_WALL)
    {
        struct vec3 norm = transform_forward(&hit->transform);
        if (vec3_dot(norm, data->dir) <= 0.0f)
        {
            data->dir = vec3_reflect(data->dir, norm);
        }
    }
}
