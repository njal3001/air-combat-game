#include "world.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "player.h"
#include "assets.h"
#include "collide.h"
#include "game.h"
#include "audio.h"
#include "timer.h"
#include "calc.h"

#define MAX_ACTORS 20000

#define ITEM_MAX_DIST 2500.0f
#define ITEM_SPAWN_MIN_DIST 1000.0f
#define ITEM_SPAWN_MAX_DIST 2000.0f

#define ASTEROID_TARGET_COUNT 3500
#define ENERGYCELL_TARGET_COUNT 100

struct projectile_data
{
    float ttl;
    float speed;
};

struct mesh projectile_mesh;

struct asteroid_data
{
    struct vec3 dir;
    size_t size;
    float no_collide;
};

struct actor *player;
struct actor actors[MAX_ACTORS];
size_t num_actors;
size_t tick;
bool show_colliders;

size_t num_asteroids;
size_t num_energycells;

static void world_populate();

static void projectile_update(struct actor *ac, float dt);
static void projectile_render(struct actor *ac);

static void spawn_asteroid(struct vec3 pos, struct vec3 dir, size_t size);
static void asteroid_update(struct actor *ac, float dt);
static void asteroid_render(struct actor *ac);

static void spawn_energycell(struct vec3 pos);
static void energycell_update(struct actor *ac, float dt);
static void energycell_render(struct actor *ac);

static bool check_player_out_of_range(const struct actor *ac, float max_dist);

void world_init()
{
    // Spawn tick is 0 for all initial actors
    tick = 0;

    player = spawn_player(VEC3_ZERO);

    const struct texture *projectile_texture = get_texture("lasers/11.png");
    projectile_mesh = create_quad_mesh();
    projectile_mesh.material.ambient = vec3_create(1.0f, 1.0f, 1.0f);
    projectile_mesh.material.diffuse = VEC3_ZERO;
    projectile_mesh.material.specular = VEC3_ZERO;
    projectile_mesh.material.shininess = 1.0f;
    projectile_mesh.material.texture = projectile_texture;

    world_populate();
}

void world_populate()
{
    struct vec3 ppos = player->transform.pos;

    while (num_asteroids < ASTEROID_TARGET_COUNT)
    {
        struct vec3 pos = vec3_add(ppos, vec3_randrange(ITEM_SPAWN_MIN_DIST, ITEM_SPAWN_MAX_DIST));
        struct vec3 dir = vec3_rand();
        size_t size = randrange(3, 5);
        spawn_asteroid(pos, dir, size);

        num_asteroids++;
    }

    while (num_energycells < ENERGYCELL_TARGET_COUNT)
    {
        struct vec3 pos = vec3_add(ppos, vec3_randrange(ITEM_SPAWN_MIN_DIST, ITEM_SPAWN_MAX_DIST));
        spawn_energycell(pos);

        num_energycells++;
    }
}

void world_update(float dt)
{
    tick = timer_ticks();

    size_t num_ac_found = 0;
    size_t num_ac_target = num_actors;

    if (player)
    {
        world_populate();
    }

    size_t i = 0;
    while (num_ac_found != num_ac_target)
    {
        struct actor *ac = actors + i;

        // Only update actor if it was not spawned this tick
        if (ac->id && (!ac->spawn_tick || ac->spawn_tick != tick))
        {
            if (ac->flags & ACTOR_DEAD)
            {
                // Remove dead actor
                if (ac->death)
                {
                    ac->death(ac);
                }

                if (ac->type == ACTOR_TYPE_ASTEROID)
                {
                    num_asteroids--;
                }
                else if (ac->type == ACTOR_TYPE_ENERGYCELL)
                {
                    num_energycells--;
                }

                free(ac->data);
                ac->id = 0;
                num_actors--;
            }
            else
            {
                ac->spawn_tick = 0;
                ac->update(ac, dt);
            }

            num_ac_found++;
        }

        i++;
    }
}

void world_render()
{
    // TODO: Draw opaque objects first, then
    // draw transparent objects in sorted order

    render_skybox();
    mesh_frame_begin();

    size_t num_ac_found = 0;
    for (size_t i = 0; i < MAX_ACTORS; i++)
    {
        struct actor *ac = actors + i;
        if (ac->id)
        {
            ac->render(ac);

            num_ac_found++;
            if (num_ac_found == num_actors)
            {
                break;
            }
        }
    }

    mesh_frame_end();

    if (show_colliders)
    {
        untextured_frame_begin();

        num_ac_found = 0;
        for (size_t i = 0; i < MAX_ACTORS; i++)
        {
            struct actor *ac = actors + i;
            if (ac->id)
            {
                if (ac->type != ACTOR_TYPE_PLAYER)
                {
                    render_collider_outline(ac);
                }

                num_ac_found++;
                if (num_ac_found == num_actors)
                {
                    break;
                }
            }
        }

        untextured_frame_end();
    }

    if (player)
    {
        text_frame_begin();
        player_render_debug_panel(player);
        text_frame_end();
    }
}

void world_free()
{
    mesh_free(&projectile_mesh);
}

void world_end()
{
    player = NULL;
    printf("GAME OVER!\n");
}

void toggle_collider_rendering()
{
   show_colliders = !show_colliders;
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
            ac->spawn_tick = tick;
            ac->death = NULL;
            ac->data = NULL;
            return ac;
        }

        i = (i + 1) % MAX_ACTORS;
    }
}

struct actor *first_collide(const struct actor *ac, int type_mask)
{
    size_t num_ac_found = 0;
    for (size_t i = 0; i < MAX_ACTORS; i++)
    {
        struct actor *other = actors + i;
        if (other->id)
        {
            if (other->id != ac->id && other->type & type_mask)
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
    pr->transform.scale = vec3_create(2.0f, 2.0f, 2.0f);
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
        audio_play("bomb.wav");
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

void projectile_render(struct actor *ac)
{
    // FIXME: Hack to get correct orientation, should change texture coordinates
    // or image orientation
    transform_local_roty(&ac->transform, -M_PI / 2.0f);
    push_mesh(&projectile_mesh, &ac->transform);
    transform_local_roty(&ac->transform, M_PI / 2.0f);
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
    ac->type = ACTOR_TYPE_ASTEROID;
    ac->cbox.offset = VEC3_ZERO;
    ac->cbox.bounds = vec3_create(0.7f, 0.7f, 0.7f);

    struct asteroid_data *data = malloc(sizeof(struct asteroid_data));
    data->dir = dir;
    data->size = size;
    ac->data = data;
}

void asteroid_update(struct actor *ac, float dt)
{
    if (check_player_out_of_range(ac, ITEM_MAX_DIST))
    {
        ac->flags |= ACTOR_DEAD;
    }
    else
    {
        struct asteroid_data *data = ac->data;
        struct actor *hit = first_collide(ac, ACTOR_TYPE_PLAYER);
        if (hit)
        {
            actor_hurt(hit, 100.0f);
            ac->flags |= ACTOR_DEAD;
        }
        else
        {
            float drot = dt * 2.0f * 1.0f / (float)data->size;
            vec3_add_eq(&ac->transform.pos, vec3_mul(data->dir, 10.0f * dt));
            transform_local_roty(&ac->transform, drot);
        }
    }
}

void asteroid_render(struct actor *ac)
{
    const struct mesh *m = get_mesh("rock.mesh");
    push_mesh(m, &ac->transform);
}

void spawn_energycell(struct vec3 pos)
{
    struct actor *ac = new_actor();

    ac->transform = transform_create(pos);
    ac->transform.scale = vec3_create(5.0f, 5.0f, 5.0f);
    ac->hp = 0.0f;
    ac->update = energycell_update;
    ac->render = energycell_render;
    ac->type = ACTOR_TYPE_ENERGYCELL;
    ac->cbox.offset = vec3_create(0.0f, 0.0f, 2.5f);
    ac->cbox.bounds = vec3_create(1.0f, 1.0f, 2.4f);
}

void energycell_update(struct actor *ac, float dt)
{
    if (check_player_out_of_range(ac, ITEM_MAX_DIST))
    {
        ac->flags |= ACTOR_DEAD;
    }
    else
    {
        struct actor *hit = first_collide(ac, ACTOR_TYPE_PLAYER);
        if (hit)
        {
            player_energize(hit);
            ac->flags |= ACTOR_DEAD;
        }
    }
}

void energycell_render(struct actor *ac)
{
    const struct mesh *m = get_mesh("energycell.mesh");
    push_mesh(m, &ac->transform);
}

bool check_player_out_of_range(const struct actor *ac, float max_dist)
{
    if (player)
    {
        struct vec3 pdiff = vec3_sub(player->transform.pos, ac->transform.pos);
        float dist2 = vec3_length2(pdiff);
        if (dist2 >= max_dist * max_dist)
        {
            return true;
        }
    }

    return false;
}
