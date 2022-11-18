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

#define MAX_CHUNKS 500
#define CHUNK_ASTEROID_COUNT 30
#define CHUNK_DIM 2000.0f

#define CHUNK_SPAN 2

struct projectile_data
{
    float ttl;
    float speed;
};

struct mesh player_mesh;
struct mesh projectile_mesh;

const struct mesh *asteroid_mesh;
const struct mesh *energycell_mesh;

struct asteroid_data
{
    struct vec3 dir;
    size_t size;
    float speed;
};

struct world_chunk
{
    struct ivec3 pos;
    size_t actors[CHUNK_ASTEROID_COUNT];
};

size_t num_chunks;
struct world_chunk chunks[MAX_CHUNKS];

struct actor *player;
struct ivec3 player_chunk_pos;
struct actor actors[MAX_ACTORS];
size_t num_actors;
size_t tick;
bool show_colliders;

size_t num_asteroids;
size_t num_energycells;

static void world_render_type(enum actor_type type);

static void projectile_update(struct actor *ac, float dt);
static void projectile_render(struct actor *ac);

static struct actor *spawn_asteroid(struct vec3 pos, struct vec3 dir, size_t size, float speed);
static void asteroid_update(struct actor *ac, float dt);
static void asteroid_render(struct actor *ac);

static void spawn_energycell(struct vec3 pos);
static void energycell_update(struct actor *ac, float dt);
static void energycell_render(struct actor *ac);

static void generate_chunk(struct ivec3 pos);
static void destroy_chunk(struct ivec3 pos);
static struct ivec3 world_to_chunk_pos(struct vec3 wpos);
static void world_gen_init();
static void world_gen_update();

static bool check_item_out_of_range(const struct actor *ac, float max_dist);


void world_init()
{
    // Spawn tick is 0 for all initial actors
    tick = 0;

    player = spawn_player(VEC3_ZERO);
    world_gen_init();

    projectile_mesh = create_quad_mesh();
    projectile_mesh.texture = get_texture("lasers/11.png");

    player_mesh = create_cube_mesh();
    player_mesh.texture = get_texture("rusted_metal.jpg");

    asteroid_mesh = get_mesh("rock.mesh");
    energycell_mesh = get_mesh("energycell.mesh");
}

void world_update(float dt)
{
    tick = timer_ticks();

    size_t num_ac_found = 0;
    size_t num_ac_target = num_actors;

    world_gen_update();

    size_t i = 0;
    while (num_ac_found != num_ac_target)
    {
        struct actor *ac = actors + i;

        // FIXME: Should not be getting invalid actors with non-zero ids

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

void world_render_type(enum actor_type type)
{
    size_t num_ac_found = 0;
    for (size_t i = 0; i < MAX_ACTORS; i++)
    {
        struct actor *ac = actors + i;
        if (ac->id)
        {
            if (ac->type == type)
            {
                push_mesh_transform(&ac->transform);
            }

            num_ac_found++;
            if (num_ac_found == num_actors)
            {
                break;
            }
        }
    }
}

void world_render()
{
    // TODO: Draw opaque objects first, then
    // draw transparent objects in sorted order

    // render_skybox();

    // TODO: Should not use instancing for small groups of meshes
    if (player)
    {
        mesh_instancing_begin(&player_mesh);
        push_mesh_transform(&player->transform);
        mesh_instancing_end();
    }

    mesh_instancing_begin(asteroid_mesh);
    world_render_type(ACTOR_TYPE_ASTEROID);
    mesh_instancing_end();

    mesh_instancing_begin(energycell_mesh);
    world_render_type(ACTOR_TYPE_ENERGYCELL);
    mesh_instancing_end();

    mesh_instancing_begin(&projectile_mesh);
    world_render_type(ACTOR_TYPE_PROJECTILE);
    mesh_instancing_end();

    if (show_colliders)
    {
        untextured_frame_begin();

        size_t num_ac_found = 0;
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
        player_render_state_info(player);
        text_frame_end();
    }
}

void world_free()
{
    mesh_free(&projectile_mesh);
    mesh_free(&player_mesh);
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

struct actor *spawn_asteroid(struct vec3 pos, struct vec3 dir, size_t size, float speed)
{
    struct actor *ac = new_actor();

    ac->transform = transform_create(pos);
    ac->transform.scale = vec3_mul(vec3_create(5.0f, 2.0f, 2.0f), size);
    ac->transform.rot = forward_to_rotation(dir, VEC3_UP);
    ac->hp = size * 15.0f;
    ac->update = asteroid_update;
    ac->type = ACTOR_TYPE_ASTEROID;
    ac->cbox.offset = VEC3_ZERO;
    ac->cbox.bounds = vec3_create(0.7f, 0.7f, 0.7f);

    struct asteroid_data *data = malloc(sizeof(struct asteroid_data));
    data->dir = dir;
    data->size = size;
    data->speed = speed;
    ac->data = data;

    return ac;
}

void asteroid_update(struct actor *ac, float dt)
{
    struct asteroid_data *data = ac->data;
    float drot = dt * 2.0f / (float)data->size;
    vec3_add_eq(&ac->transform.pos, vec3_mul(data->dir, data->speed * dt));
    transform_local_roty(&ac->transform, drot);
}

void spawn_energycell(struct vec3 pos)
{
    struct actor *ac = new_actor();

    ac->transform = transform_create(pos);
    ac->transform.scale = vec3_create(35.0f, 35.0f, 35.0f);
    ac->hp = 0.0f;
    ac->update = energycell_update;
    ac->type = ACTOR_TYPE_ENERGYCELL;
    ac->cbox.offset = vec3_create(0.0f, 0.0f, 2.5f);
    ac->cbox.bounds = vec3_create(1.0f, 1.0f, 2.4f);
}

void energycell_update(struct actor *ac, float dt)
{
    if (player && check_collide(ac, player))
    {
        player_energize(player);
        ac->flags |= ACTOR_DEAD;
    }
}

void generate_chunk(struct ivec3 pos)
{
    assert(num_chunks < MAX_CHUNKS);
    struct world_chunk *chunk = chunks + num_chunks;
    chunk->pos = pos;

    struct vec3 chunk_center = vec3_create(pos.x * CHUNK_DIM, pos.y * CHUNK_DIM, pos.z * CHUNK_DIM);
    float c2 = CHUNK_DIM / 2.0f;

    for (size_t i = 0; i < CHUNK_ASTEROID_COUNT; i++)
    {
        float dx = frandrange(-c2, c2);
        float dy = frandrange(-c2, c2);
        float dz = frandrange(-c2, c2);

        struct vec3 apos = vec3_add(chunk_center, vec3_create(dx, dy, dz));
        struct vec3 dir = vec3_rand();
        size_t size = randrange(10, 25);
        // float speed = frandrange(5.0f, 100.0f);
        float speed = 0.0f;
        struct actor *a = spawn_asteroid(apos, dir, size, speed);
        chunk->actors[i] = a->id;

        num_asteroids++;
    }

    num_chunks++;
}

void destroy_chunk(struct ivec3 pos)
{
    for (size_t i = 0; i < num_chunks; i++)
    {
        struct world_chunk *chunk = chunks + i;
        if (ivec3_equal(chunk->pos, pos))
        {
            for (size_t ia = 0; ia < CHUNK_ASTEROID_COUNT; ia++)
            {
                size_t id = chunk->actors[ia];
                actors[id - 1].flags |= ACTOR_DEAD;
            }

            if (num_chunks > 1 && i != num_chunks - 1)
            {
                chunks[i] = chunks[num_chunks - 1];
            }

            num_chunks--;
        }
    }
}

struct ivec3 world_to_chunk_pos(struct vec3 wpos)
{
    float c2 = CHUNK_DIM / 2.0f;

    struct ivec3 res;
    res.x = (wpos.x + sign(wpos.x) * c2) / CHUNK_DIM;
    res.y = (wpos.y + sign(wpos.y) * c2) / CHUNK_DIM;
    res.z = (wpos.z + sign(wpos.z) * c2) / CHUNK_DIM;

    return res;
}

void world_gen_init()
{
    player_chunk_pos = world_to_chunk_pos(player->transform.pos);
    for (int z = -CHUNK_SPAN; z <= CHUNK_SPAN; z++)
    {
        for (int y = -CHUNK_SPAN; y <= CHUNK_SPAN; y++)
        {
            for (int x = -CHUNK_SPAN; x <= CHUNK_SPAN; x++)
            {
                if (x || y || z)
                {
                    struct ivec3 delta = ivec3_create(x, y, z);
                    struct ivec3 chunk_pos = ivec3_add(player_chunk_pos, delta);
                    generate_chunk(chunk_pos);
                }
            }
        }
    }
}

void world_gen_update()
{
    if (player)
    {
        struct ivec3 new_pc_pos = world_to_chunk_pos(player->transform.pos);
        int dx = new_pc_pos.x - player_chunk_pos.x;
        int dy = new_pc_pos.y - player_chunk_pos.y;
        int dz = new_pc_pos.z - player_chunk_pos.z;

        if (dx)
        {
            for (int y = -CHUNK_SPAN; y <= CHUNK_SPAN; y++)
            {
                for (int z = -CHUNK_SPAN; z <= CHUNK_SPAN; z++)
                {
                    struct ivec3 delta = ivec3_create(dx * CHUNK_SPAN, y, z);
                    struct ivec3 chunk_pos = ivec3_add(delta, new_pc_pos);
                    generate_chunk(chunk_pos);

                    struct ivec3 ndelta = ivec3_create(-dx * CHUNK_SPAN, y, z);
                    struct ivec3 des_pos = ivec3_add(player_chunk_pos, ndelta);
                    destroy_chunk(des_pos);
                }
            }
        }
        if (dy)
        {
            for (int x = -CHUNK_SPAN; x <= CHUNK_SPAN; x++)
            {
                for (int z = -CHUNK_SPAN; z <= CHUNK_SPAN; z++)
                {
                    struct ivec3 delta = ivec3_create(x, dy * CHUNK_SPAN, z);
                    struct ivec3 chunk_pos = ivec3_add(delta, new_pc_pos);
                    generate_chunk(chunk_pos);

                    struct ivec3 ndelta = ivec3_create(x, -dy * CHUNK_SPAN, z);
                    struct ivec3 des_pos = ivec3_add(player_chunk_pos, ndelta);
                    destroy_chunk(des_pos);
                }
            }
        }
        if (dz)
        {
            for (int x = -CHUNK_SPAN; x <= CHUNK_SPAN; x++)
            {
                for (int y = -CHUNK_SPAN; y <= CHUNK_SPAN; y++)
                {
                    struct ivec3 delta = ivec3_create(x, y, dz * CHUNK_SPAN);
                    struct ivec3 chunk_pos = ivec3_add(delta, new_pc_pos);
                    generate_chunk(chunk_pos);

                    struct ivec3 ndelta = ivec3_create(x, y, -dz * CHUNK_SPAN);
                    struct ivec3 des_pos = ivec3_add(player_chunk_pos, ndelta);
                    destroy_chunk(des_pos);
                }
            }
        }

        player_chunk_pos = new_pc_pos;
    }
}
