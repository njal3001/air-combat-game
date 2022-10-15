#pragma once
#include "render.h"

void assets_init();
void assets_free();

const struct texture *get_texture(const char *path);

// NOTE: Only accepts .ply model files and assumes that the file
//       has a comment which specifies the texture
// TODO: Handle materials
const struct mesh *get_mesh(const char *path);
