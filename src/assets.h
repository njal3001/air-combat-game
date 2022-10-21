#pragma once
#include "render.h"
#include "shader.h"

struct shape
{
    struct transform transform;
    const struct mesh *mesh;
};

void assets_init();
void assets_free();

const struct texture *get_texture(const char *name);
const struct mesh *get_mesh(const char *name);

struct shape create_quad();

bool load_shader(struct shader *shader, const char *vert_name, const char *frag_name);
