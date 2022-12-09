#include "orb.h"
#include "collide.h"
#include "player.h"

#define DEACCEL             1.0f
#define SPD_MAX             2.5f
#define ATTRACT_RANGE       7.5f
#define ATTRACT_ACCEL_MAX   10.0f

struct orb_data
{
    struct vec3 vel;
};

void spawn_orb(struct world *w, struct vec3 pos)
{
    struct actor *ac = new_actor(w, pos, ACTOR_TYPE_ORB);
    vec3_div_eq(&ac->transform.scale, 4.0f);

    struct orb_data *data = malloc(sizeof(struct orb_data));
    data->vel = VEC3_ZERO;

    ac->data = data;
}

void orb_update(struct actor *ac, float dt)
{
    struct orb_data *data = ac->data;

    struct vec3 target_vel = VEC3_ZERO;
    float accel = DEACCEL;

    struct actor *player = ac->world->player;
    if (player)
    {
        struct vec3 ppos = player->transform.pos;
        struct vec3 diff = vec3_sub(ppos, ac->transform.pos);

        float len = vec3_length(diff);
        if (len < ATTRACT_RANGE)
        {
            struct vec3 dir = vec3_div(diff, len);
            target_vel = vec3_mul(dir, SPD_MAX);
            accel = ((ATTRACT_RANGE - len) / ATTRACT_RANGE) * ATTRACT_ACCEL_MAX;

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
