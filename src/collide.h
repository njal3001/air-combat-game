#pragma once
#include "world.h"

bool check_collide(const struct actor *a, const struct actor *b);
void render_collider_outline(const struct actor *ac);
