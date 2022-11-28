#pragma once
#include "actor.h"

void world_init();
void world_start();
void world_update(float dt);
void world_render();
void world_end();
bool world_ended();
void world_free();

void toggle_collider_rendering();

struct actor *new_actor();
void actor_hurt(struct actor *ac, float dmg);

struct actor *first_collide(const struct actor *ac, int type_mask);

struct actor *spawn_projectile(struct vec3 pos, float speed);
