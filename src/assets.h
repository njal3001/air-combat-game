#pragma once
#include "render.h"

struct shape
{
    struct transform transform;
    const struct mesh *mesh;
};

void assets_init();
void assets_free();

const struct texture *get_texture(const char *name);

// TODO: Handle materials
const struct mesh *get_mesh(const char *name);

struct shape create_quad();
