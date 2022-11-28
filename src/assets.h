#pragma once
#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "font.h"

enum asset_type
{
    ASSET_SHADER,
    ASSET_OTHER,
};

void assets_init();
void assets_free();

const char *get_asset_path(enum asset_type type, const char *name);

const struct texture *get_texture(const char *name);
const struct mesh *get_mesh(const char *name);

bool load_shader(struct shader *shader, const char *vert_name,
        const char *frag_name);
bool load_cubemap(struct cubemap *cmap, const char *const *faces);
bool load_font(struct font *font, const char *name);
