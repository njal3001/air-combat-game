#include "mesh.h"

void mesh_init(struct mesh *mesh, size_t vertex_count, size_t index_count)
{
    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;

    mesh->vertices = malloc(vertex_count * sizeof(struct vert_mesh));
    mesh->indices = malloc(index_count * sizeof(GLuint));

    mesh->texture = NULL;
}

void mesh_free(struct mesh *mesh)
{
    free(mesh->indices);
    free(mesh->vertices);
}

struct mesh create_quad_mesh()
{
    struct mesh quad_mesh;
    mesh_init(&quad_mesh, 4, 6);

    quad_mesh.vertices[0].pos = vec3_create(-0.5f, 0.0f, 0.5f);
    quad_mesh.vertices[0].uvx = 0.0f;
    quad_mesh.vertices[0].uvy = 1.0f;

    quad_mesh.vertices[1].pos = vec3_create(-0.5f, 0.0f, -0.5f);
    quad_mesh.vertices[1].uvx = 0.0f;
    quad_mesh.vertices[1].uvy = 0.0f;

    quad_mesh.vertices[2].pos = vec3_create(0.5f, 0.0f, -0.5f);
    quad_mesh.vertices[2].uvx = 1.0f;
    quad_mesh.vertices[2].uvy = 0.0f;

    quad_mesh.vertices[3].pos = vec3_create(0.5f, 0.0f, 0.5f);
    quad_mesh.vertices[3].uvx = 1.0f;
    quad_mesh.vertices[3].uvy = 1.0f;

    quad_mesh.indices[0] = 0;
    quad_mesh.indices[1] = 2;
    quad_mesh.indices[2] = 1;
    quad_mesh.indices[3] = 0;
    quad_mesh.indices[4] = 3;
    quad_mesh.indices[5] = 2;

    return quad_mesh;
}

struct mesh create_cube_mesh()
{
    struct mesh cube_mesh;
    mesh_init(&cube_mesh, 24, 36);

    cube_mesh.vertices[0].pos = vec3_create(1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[0].uvx = 0.875f;
    cube_mesh.vertices[0].uvy = 0.5f;

    cube_mesh.vertices[1].pos = vec3_create(-1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[1].uvx = 0.625f;
    cube_mesh.vertices[1].uvy = 0.75f;

    cube_mesh.vertices[2].pos = vec3_create(-1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[2].uvx = 0.625f;
    cube_mesh.vertices[2].uvy = 0.5f;

    cube_mesh.vertices[3].pos = vec3_create(-1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[3].uvx = 0.625f;
    cube_mesh.vertices[3].uvy = 0.75f;

    cube_mesh.vertices[4].pos = vec3_create(1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[4].uvx = 0.375f;
    cube_mesh.vertices[4].uvy = 1.0f;

    cube_mesh.vertices[5].pos = vec3_create(-1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[5].uvx = 0.375f;
    cube_mesh.vertices[5].uvy = 0.75f;

    cube_mesh.vertices[6].pos = vec3_create(1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[6].uvx = 0.625f;
    cube_mesh.vertices[6].uvy = 0.0f;

    cube_mesh.vertices[7].pos = vec3_create(1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[7].uvx = 0.375f;
    cube_mesh.vertices[7].uvy = 0.25f;

    cube_mesh.vertices[8].pos = vec3_create(1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[8].uvx = 0.375f;
    cube_mesh.vertices[8].uvy = 0.0f;

    cube_mesh.vertices[9].pos = vec3_create(-1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[9].uvx = 0.375f;
    cube_mesh.vertices[9].uvy = 0.5f;

    cube_mesh.vertices[10].pos = vec3_create(1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[10].uvx = 0.125f;
    cube_mesh.vertices[10].uvy = 0.75f;

    cube_mesh.vertices[11].pos = vec3_create(1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[11].uvx = 0.125f;
    cube_mesh.vertices[11].uvy = 0.5f;

    cube_mesh.vertices[12].pos = vec3_create(-1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[12].uvx = 0.625f;
    cube_mesh.vertices[12].uvy = 0.5f;

    cube_mesh.vertices[13].pos = vec3_create(-1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[13].uvx = 0.375f;
    cube_mesh.vertices[13].uvy = 0.75f;

    cube_mesh.vertices[14].pos = vec3_create(-1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[14].uvx = 0.375f;
    cube_mesh.vertices[14].uvy = 0.5f;

    cube_mesh.vertices[15].pos = vec3_create(1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[15].uvx = 0.625f;
    cube_mesh.vertices[15].uvy = 0.25f;

    cube_mesh.vertices[16].pos = vec3_create(-1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[16].uvx = 0.375f;
    cube_mesh.vertices[16].uvy = 0.5f;

    cube_mesh.vertices[17].pos = vec3_create(1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[17].uvx = 0.375f;
    cube_mesh.vertices[17].uvy = 0.25f;

    cube_mesh.vertices[18].pos = vec3_create(1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[18].uvx = 0.875f;
    cube_mesh.vertices[18].uvy = 0.75f;

    cube_mesh.vertices[19].pos = vec3_create(1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[19].uvx = 0.625f;
    cube_mesh.vertices[19].uvy = 1.0f;

    cube_mesh.vertices[20].pos = vec3_create(1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[20].uvx = 0.625f;
    cube_mesh.vertices[20].uvy = 0.25f;

    cube_mesh.vertices[21].pos = vec3_create(-1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[21].uvx = 0.375f;
    cube_mesh.vertices[21].uvy = 0.75f;

    cube_mesh.vertices[22].pos = vec3_create(-1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[22].uvx = 0.625f;
    cube_mesh.vertices[22].uvy = 0.75f;

    cube_mesh.vertices[23].pos = vec3_create(-1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[23].uvx = 0.625f;
    cube_mesh.vertices[23].uvy = 0.5f;

    cube_mesh.indices[0] = 0;
    cube_mesh.indices[1] = 1;
    cube_mesh.indices[2] = 2;
    cube_mesh.indices[3] = 3;
    cube_mesh.indices[4] = 4;
    cube_mesh.indices[5] = 5;
    cube_mesh.indices[6] = 6;
    cube_mesh.indices[7] = 7;
    cube_mesh.indices[8] = 8;
    cube_mesh.indices[9] = 9;
    cube_mesh.indices[10] = 10;
    cube_mesh.indices[11] = 11;
    cube_mesh.indices[12] = 12;
    cube_mesh.indices[13] = 13;
    cube_mesh.indices[14] = 14;
    cube_mesh.indices[15] = 15;
    cube_mesh.indices[16] = 16;
    cube_mesh.indices[17] = 17;
    cube_mesh.indices[18] = 0;
    cube_mesh.indices[19] = 18;
    cube_mesh.indices[20] = 1;
    cube_mesh.indices[21] = 3;
    cube_mesh.indices[22] = 19;
    cube_mesh.indices[23] = 4;
    cube_mesh.indices[24] = 6;
    cube_mesh.indices[25] = 20;
    cube_mesh.indices[26] = 7;
    cube_mesh.indices[27] = 9;
    cube_mesh.indices[28] = 21;
    cube_mesh.indices[29] = 10;
    cube_mesh.indices[30] = 12;
    cube_mesh.indices[31] = 22;
    cube_mesh.indices[32] = 13;
    cube_mesh.indices[33] = 15;
    cube_mesh.indices[34] = 23;
    cube_mesh.indices[35] = 16;

    return cube_mesh;
}
