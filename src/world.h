#pragma once
#include "actor.h"

#define MAX_ACTORS 20000

struct world
{
    struct actor *player;
    struct actor actors[MAX_ACTORS];
    uint16_t num_actors;
    uint8_t tick;
    bool show_colliders;
    bool show_hud;
};

void world_init(struct world *w);
void world_free(struct world *w);

void world_start(struct world *w);
void world_update(struct world *w, float dt);
void world_render(struct world *w);

bool world_ended(const struct world *w);

struct actor *new_actor(struct world *w, struct vec3 pos,
        enum actor_type type);
struct actor *get_actor(struct world *w, uint16_t id);

struct actor *first_collide(struct world *w, const struct actor *ac,
        int type_mask);

void toggle_collider_rendering(struct world *w);
void toggle_hud_rendering(struct world *w);
