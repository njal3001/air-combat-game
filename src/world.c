#include "world.h"
#include <assert.h>
#include <stdio.h>
#include "player.h"
#include "assets.h"
#include "collide.h"
#include "game.h"

#define MAX_ACTORS 1024

struct projectile_data
{
    float ttl;
    float speed;
};

struct asteroid_data
{
    struct vec3 dir;
    size_t size;
};

struct actor *player;
struct actor actors[MAX_ACTORS];
size_t num_actors;
bool world_ended;

static void projectile_update(struct actor *ac, float dt);
static void projectile_render(struct actor *ac);

static void spawn_asteroid(struct vec3 pos, struct vec3 dir, size_t size);
static void asteroid_update(struct actor *ac, float dt);
static void asteroid_render(struct actor *ac);
static void asteroid_death(struct actor *ac);

void world_init()
{
    player = spawn_player(VEC3_ZERO);

    const int range = 50;
    for (size_t i = 0; i < 10; i++)
    {
        float x = (rand() % (2 * range)) - range;
        float y = (rand() % (2 * range)) - range;
        float z = 3 * range;

        float dx = (2.0f * rand() / (float)RAND_MAX) - 1.0f;
        float dy = (2.0f * rand() / (float)RAND_MAX) - 1.0f;
        float dz = (2.0f * rand() / (float)RAND_MAX) - 1.0f;

        spawn_asteroid(vec3_create(x, y, z), vec3_normalize(vec3_create(dx, dy, dz)), 4);
    }
}

void world_update(float dt)
{
    if (world_ended)
    {
        return;
    }

    size_t tick = get_ticks();
    size_t num_ac_found = 0;
    size_t num_ac_target = num_actors;
    size_t i = 0;

    while (num_ac_found != num_ac_target)
    {
        struct actor *ac = actors + i;
        if (ac->id)
        {
            if (ac->flags & ACTOR_DEAD)
            {
                if (ac->death)
                {
                    ac->death(ac);
                }
                free(ac->data);
                ac->id = 0;
                num_actors--;
            }
            else if (ac->spawn_tick != tick)
            {
                ac->update(ac, dt);
            }

            num_ac_found++;
        }

        i++;
    }
}

void world_render()
{
    render_begin();
    set_texture(get_texture("wall.jpg"));

    size_t num_ac_found = 0;
    for (size_t i = 0; i < MAX_ACTORS; i++)
    {
        struct actor *pr = actors + i;
        if (pr->id)
        {
            pr->render(pr);
            num_ac_found++;

            if (num_ac_found == num_actors)
            {
                break;
            }
        }
    }

    render_end();
}

void world_end()
{
    printf("GAME OVER!\n");
    world_ended = true;
}

struct actor *new_actor()
{
    assert(num_actors < MAX_ACTORS);
    size_t i = num_actors;
    while (1)
    {
        struct actor *ac = actors + i;
        if (!ac->id)
        {
            num_actors++;

            ac->id = i + 1;
            ac->flags = 0;
            ac->spawn_tick = get_ticks();
            ac->death = NULL;
            return ac;
        }

        i = (i + 1) % MAX_ACTORS;
    }
}

struct actor *first_collide(const struct actor *ac, enum actor_type type)
{
    size_t num_ac_found = 0;
    for (size_t i = 0; i < MAX_ACTORS; i++)
    {
        struct actor *other = actors + i;
        if (other->id)
        {
            if (other->id != ac->id && other->type == type)
            {
                if (check_collide(ac, other))
                {
                    return other;
                }
            }

            num_ac_found++;
            if (num_ac_found == num_actors)
            {
                break;
            }
        }
    }

    return NULL;
}

void actor_hurt(struct actor *ac, float dmg)
{
    ac->hp -= dmg;
    if (ac->hp <= 0.0f)
    {
        ac->flags |= ACTOR_DEAD;
    }
}

struct actor *spawn_projectile(struct vec3 pos, float speed)
{
    struct actor *pr = new_actor();
    struct projectile_data *data = malloc(sizeof(struct projectile_data));

    pr->transform = transform_create(pos);
    pr->transform.scale = vec3_create(0.05f, 0.05f, 0.05f);
    pr->update = projectile_update;
    pr->render = projectile_render;
    pr->type = ACTOR_TYPE_PROJECTILE;
    pr->cbox.offset = VEC3_ZERO;
    pr->cbox.bounds = VEC3_ONE;

    data->speed = speed;
    data->ttl = 5.0f;
    pr->data = data;

    return pr;
}

void projectile_update(struct actor *pr, float dt)
{
    struct projectile_data *data = pr->data;
    struct vec3 forward = transform_forward(&pr->transform);
    vec3_add_eq(&pr->transform.pos, vec3_mul(forward, data->speed * dt));

    struct actor *hit = first_collide(pr, ACTOR_TYPE_ASTEROID);
    if (hit)
    {
        actor_hurt(hit, 12.5f);
        pr->flags |= ACTOR_DEAD;
    }
    else
    {
        data->ttl -= dt;
        if (data->ttl <= 0)
        {
            pr->flags |= ACTOR_DEAD;
        }
    }
}

void projectile_render(struct actor *en)
{
    const struct mesh *m = get_cube_mesh();
    render_mesh(m, &en->transform);
}

void spawn_asteroid(struct vec3 pos, struct vec3 dir, size_t size)
{
    struct actor *ac = new_actor();

    ac->transform = transform_create(pos);
    ac->transform.scale = vec3_mul(vec3_create(5.0f, 2.0f, 2.0f), size);
    ac->transform.rot = forward_to_rotation(dir, VEC3_UP);
    ac->hp = size * 15.0f;
    ac->update = asteroid_update;
    ac->render = asteroid_render;
    ac->death = asteroid_death;
    ac->type = ACTOR_TYPE_ASTEROID;
    ac->cbox.offset = VEC3_ZERO;
    ac->cbox.bounds = VEC3_ONE;

    struct asteroid_data *data = malloc(sizeof(struct asteroid_data));
    data->dir = dir;
    data->size = size;
    ac->data = data;
}

void asteroid_update(struct actor *ac, float dt)
{
    if (check_collide(ac, player))
    {
        actor_hurt(player, 100.0f);
        ac->flags |= ACTOR_DEAD;
    }
    else
    {
        struct asteroid_data *data = ac->data;
        float drot = dt * 2.0f * 1.0f / (float)data->size;
        vec3_add_eq(&ac->transform.pos, vec3_mul(data->dir, 10.0f * dt));
        transform_local_roty(&ac->transform, drot);
    }
}

void asteroid_death(struct actor *ac)
{
    struct asteroid_data *data = ac->data;
    if (data->size > 1)
    {
        struct vec3 new_dir = vec3_cross(data->dir, VEC3_UP);
        spawn_asteroid(ac->transform.pos, new_dir, data->size - 1);
        spawn_asteroid(ac->transform.pos, vec3_neg(new_dir), data->size - 1);
    }
}

void asteroid_render(struct actor *ac)
{
    const struct mesh *m = get_mesh("rock.mesh");
    render_mesh(m, &ac->transform);
}
