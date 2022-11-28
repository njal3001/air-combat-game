#pragma once
#include <GL/glew.h>
#include <stdlib.h>
#include "spatial.h"

struct vert_mesh
{
    struct vec3 pos;
    float uvx;
    float uvy;
};

struct mesh
{
    struct vert_mesh *vertices;
    GLuint *indices;
    size_t vertex_count;
    size_t index_count;
    const struct texture *texture;
};

void mesh_init(struct mesh *mesh, size_t vertex_count, size_t index_count);
void mesh_free(struct mesh *mesh);

struct mesh create_quad_mesh();
struct mesh create_cube_mesh();
