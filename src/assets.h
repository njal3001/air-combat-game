#pragma once
#include "render.h"
#include "shader.h"

void assets_init();
void assets_free();

const struct texture *get_texture(const char *name);
const struct mesh *get_mesh(const char *name);

const struct mesh *get_quad_mesh();
const struct mesh *get_cube_mesh();

bool load_shader(struct shader *shader, const char *vert_name, const char *frag_name);
bool load_cubemap(struct cubemap *cmap, const char *const *faces);
