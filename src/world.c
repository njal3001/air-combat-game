#include "world.h"
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "asset.h"
#include "render.h"
#include "player.h"
#include "orb.h"
#include "collide.h"
#include "calc.h"

#define WORLD_BOUNDS    100.0f
#define ORB_COUNT       5000
#define ORB_MIN_DIST    20.0f
#define ORB_PADDING     10.0f

struct actor_iter
{
    struct world *world;
    size_t next;
    size_t found;
    size_t target;
    bool ignore_spawn;
};

static void actor_iter_init(struct actor_iter *iter, struct world *world,
        bool ignore_spawn)
{
    iter->world = world;
    iter->next = 0;
    iter->found = 0;
    iter->target = world->num_actors;
    iter->ignore_spawn = ignore_spawn;
}

static struct actor *actor_iter_next(struct actor_iter *iter)
{
    if (iter->found == iter->target)
    {
        return NULL;
    }

    struct actor *ac;
    do
    {
        ac = iter->world->actors + iter->next;
        iter->next++;
    } while (!ac->id ||
            (iter->ignore_spawn && ac->spawn_tick == iter->world->tick));

    iter->found++;
    return ac;
}

static uint16_t find_free_actor(struct world *w)
{
    uint16_t i = w->num_actors;
    do
    {
        if (!w->actors[i].id)
        {
            return i + 1;
        }

        i = (i + 1) % MAX_ACTORS;
    } while (i != w->num_actors);

    return 0;
}

static void add_wall(struct world *w, struct mat4 rot)
{
    const float wall_width = 1.0f;
    struct vec3 scale = vec3_create(WORLD_BOUNDS, WORLD_BOUNDS, wall_width);

    struct actor *wall = new_actor(w, VEC3_ZERO, ACTOR_TYPE_WALL);
    wall->collide_mask =
        actor_type_bit(ACTOR_TYPE_PLAYER) | actor_type_bit(ACTOR_TYPE_ORB);
    wall->transform.rot = rot;
    wall->transform.scale = scale;

    struct vec3 fwd = transform_forward(&wall->transform);
    vec3_sub_eq(&wall->transform.pos, vec3_mul(fwd, WORLD_BOUNDS));
}

static void add_walls(struct world *w)
{
    add_wall(w, mat4_identity());
    add_wall(w, mat4_roty(M_PI));
    add_wall(w, mat4_rotx(M_PI / 2.0f));
    add_wall(w, mat4_rotx(-M_PI / 2.0f));
    add_wall(w, mat4_roty(M_PI / 2.0f));
    add_wall(w, mat4_roty(-M_PI / 2.0f));
}

static void all_collide(struct world *w, struct actor *ac)
{
    struct actor_iter iter;
    actor_iter_init(&iter, w, true);

    struct actor *other;
    while ((other = actor_iter_next(&iter)))
    {
        if (other->id != ac->id &&
                actor_type_bit(other->type) & ac->collide_mask)
        {
            if (check_collide(ac, other))
            {
                if (ac->on_collide)
                {
                    ac->on_collide(ac, other);
                }
                if (other->on_collide)
                {
                    other->on_collide(other, ac);
                }
            }
        }
    }
}

void world_init(struct world *w)
{
    w->show_colliders = false;
    w->show_hud = true;
    w->actors = calloc(MAX_ACTORS, sizeof(struct actor));

    // Spawn tick is 0 for all initial actors
    w->tick = 0;

    w->num_actors = 0;
    w->player = NULL;
}

void world_begin(struct world *w)
{
    add_walls(w);

    w->player = spawn_player(w, VEC3_ZERO);

    for (size_t i = 0; i < ORB_COUNT; i++)
    {
        struct vec3 pos = vec3_randrange(ORB_MIN_DIST,
                WORLD_BOUNDS - ORB_PADDING);
        spawn_orb(w, pos);
    }
}

void world_end(struct world *w)
{
    struct actor_iter iter;
    actor_iter_init(&iter, w, false);

    struct actor *ac;
    while ((ac = actor_iter_next(&iter)))
    {
        actor_free(ac);
    }

    memset(w->actors, 0, sizeof(struct actor) * MAX_ACTORS);
    w->num_actors = 0;
    w->tick = 0;
    w->player = NULL;
}

void world_update(struct world *w, float dt)
{
    // Make sure that tick is not 0
    w->tick = min(1, w->tick + 1);

    struct actor_iter iter;
    actor_iter_init(&iter, w, true);

    struct actor *ac;
    while((ac = actor_iter_next(&iter)))
    {
        if (ac->flags & ACTOR_DEAD)
        {
            actor_free(ac);
            ac->id = 0;
            w->num_actors--;
        }
        else
        {
            // Ensure that the actor won't be skipped
            ac->spawn_tick = 0;

            switch (ac->type)
            {
                case ACTOR_TYPE_PLAYER:
                    player_update(ac, dt);
                    break;
                case ACTOR_TYPE_ORB:
                    orb_update(ac, dt);
                    break;
                default:
                    break;
            }

            if (ac->collide_mask)
            {
                all_collide(w, ac);
            }
        }
    }

    if (w->player)
    {
        struct camera *cam = get_camera();
        player_camera_view(w->player, cam, dt);

        if (w->player->flags & ACTOR_DEAD)
        {
            w->player = NULL;
        }
    }
}

void world_render(struct world *w)
{
    for (enum actor_type type = 0; type < ACTOR_TYPE_END; type++)
    {
        struct render_spec rspec = actor_type_render_spec(type);
        render_mesh_instancing_begin(get_mesh(rspec.mesh_handle));

        struct actor_iter iter;
        actor_iter_init(&iter, w, false);

        struct actor *ac;
        while ((ac = actor_iter_next(&iter)))
        {
            if (ac->type == type)
            {
                render_push_mesh_transform(&ac->transform);
            }
        }

        render_mesh_instancing_end();
    }

    if (w->show_colliders)
    {
        render_untextured_begin();

        struct actor_iter iter;
        actor_iter_init(&iter, w, false);

        struct actor *ac;
        while ((ac = actor_iter_next(&iter)))
        {
            struct vec3 scale = ac->transform.scale;
            struct vec3 bounds = ac->cbox.bounds;
            float cmax = fmax(fmax(scale.x * bounds.x, scale.y * bounds.y),
                    scale.z * bounds.z);
            render_collider_outline(ac, cmax * 0.1f, COLOR_RED);
        }

        render_untextured_end();
    }

    if (w->player && w->show_hud)
    {
        struct camera *cam = get_camera();

        render_ui_begin();
        player_render_hud(w->player, cam);
        player_render_state_info(w->player);
        render_ui_end();
    }
}

void world_free(struct world *w)
{
    world_end(w);
    free(w->actors);
}

bool world_should_end(const struct world *w)
{
    return !w->player;
}

struct actor *new_actor(struct world *w, struct vec3 pos,
        enum actor_type type)
{
    assert(w->num_actors < MAX_ACTORS);

    uint16_t new_id = find_free_actor(w);
    struct actor *new_ac = get_actor(w, new_id);

    actor_init(new_ac, w, new_id, type, w->tick, pos);
    w->num_actors++;

    return new_ac;
}

struct actor *get_actor(struct world *w, uint16_t id)
{
    if (!id)
    {
        return NULL;
    }

    return w->actors + id - 1;
}

void toggle_collider_rendering(struct world *w)
{
   w->show_colliders = !w->show_colliders;
}

void toggle_hud_rendering(struct world *w)
{
    w->show_hud = !w->show_hud;
}
