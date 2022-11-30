#include "world.h"
#include <assert.h>
#include <string.h>
#include "player.h"
#include "collide.h"
#include "calc.h"

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

void world_init(struct world *w)
{
    w->show_colliders = false;
}

void world_start(struct world *w)
{
    memset(w->actors, 0, MAX_ACTORS * sizeof(struct actor));
    w->num_actors = 0;

    // Spawn tick is 0 for all initial actors
    w->tick = 0;

    w->player = spawn_player(w, VEC3_ZERO);
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
            }
        }
    }

    if (w->player)
    {
        // Camera is always following the player
        struct camera *cam = get_camera();
        cam->transform.pos = w->player->transform.pos;
        cam->transform.rot = w->player->transform.rot;

        if (w->player->flags & ACTOR_DEAD)
        {
            w->player = NULL;
        }
    }
}

void world_render(struct world *w)
{
    if (w->show_colliders)
    {
        untextured_frame_begin();

        struct actor_iter iter;
        actor_iter_init(&iter, w, false);

        struct actor *ac;
        while ((ac = actor_iter_next(&iter)))
        {
            render_collider_outline(ac);
        }

        untextured_frame_end();
    }

    if (w->player)
    {
        ui_begin();
        player_render_state_info(w->player);
        ui_end();
    }
}

void world_free(struct world *w)
{
}

bool world_ended(const struct world *w)
{
    return !w->player;
}

struct actor *new_actor(struct world *w, struct vec3 pos,
        enum actor_type type)
{
    assert(w->num_actors < MAX_ACTORS);

    uint16_t new_id = find_free_actor(w);
    struct actor *new_ac = w->actors + new_id - 1;

    actor_init(new_ac, new_id, type, w->tick, pos);
    w->num_actors++;

    return new_ac;
}

struct actor *first_collide(struct world *w, const struct actor *ac,
        int type_mask)
{
    struct actor_iter iter;
    actor_iter_init(&iter, w, true);

    struct actor *other;
    while ((other = actor_iter_next(&iter)))
    {
        if (other->id != ac->id && other->type & type_mask)
        {
            if (check_collide(ac, other))
            {
                return other;
            }
        }
    }

    return NULL;
}

void toggle_collider_rendering(struct world *w)
{
   w->show_colliders = !w->show_colliders;
}
