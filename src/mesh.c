#include "mesh.h"
#include <math.h>

static void add_vertex(struct mesh *mesh, size_t index,
        struct vec3 pos, float uvx, float uvy)
{
    mesh->vertices[index].pos = pos;
    mesh->vertices[index].uvx = uvx;
    mesh->vertices[index].uvy = uvy;
}

static void add_triangle(struct mesh *mesh, size_t tri_index,
        GLuint v0, GLuint v1, GLuint v2)
{
    size_t index = tri_index * 3;

    mesh->indices[index] = v0;
    mesh->indices[index + 1] = v1;
    mesh->indices[index + 2] = v2;
}

void mesh_init(struct mesh *mesh, size_t vertex_count, size_t tri_count)
{
    mesh->vertex_count = vertex_count;
    mesh->index_count = 3 * tri_count;

    mesh->vertices = malloc(vertex_count * sizeof(struct vert_mesh));
    mesh->indices = malloc(mesh->index_count * sizeof(GLuint));

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
    mesh_init(&quad_mesh, 4, 2);

    add_vertex(&quad_mesh, 0, vec3_create(-0.5f, 0.0f, 0.5f), 0.0f, 1.0f);
    add_vertex(&quad_mesh, 1, vec3_create(-0.5f, 0.0f, -0.5f), 0.0f, 0.0f);
    add_vertex(&quad_mesh, 2, vec3_create(0.5f, 0.0f, -0.5f), 1.0f, 0.0f);
    add_vertex(&quad_mesh, 3, vec3_create(0.5f, 0.0f, 0.5f), 1.0f, 1.0f);

    add_triangle(&quad_mesh, 0, 0, 2, 1);
    add_triangle(&quad_mesh, 1, 0, 3, 2);

    return quad_mesh;
}

struct mesh create_cube_mesh()
{
    struct mesh cube_mesh;
    mesh_init(&cube_mesh, 24, 12);

    struct vec3 p0 = vec3_create(-1.0f, -1.0f, -1.0f);
    struct vec3 p1 = vec3_create(1.0f, -1.0f, -1.0f);
    struct vec3 p2 = vec3_create(-1.0f, 1.0f, -1.0f);
    struct vec3 p3 = vec3_create(1.0f, 1.0f, -1.0f);
    struct vec3 p4 = vec3_create(-1.0f, -1.0f, 1.0f);
    struct vec3 p5 = vec3_create(1.0f, -1.0f, 1.0f);
    struct vec3 p6 = vec3_create(-1.0f, 1.0f, 1.0f);
    struct vec3 p7 = vec3_create(1.0f, 1.0f, 1.0f);

    add_vertex(&cube_mesh, 0, p5, 0.875f, 0.5f);
    add_vertex(&cube_mesh, 1, p6, 0.625f, 0.75f);
    add_vertex(&cube_mesh, 2, p4, 0.625f, 0.5f);
    add_vertex(&cube_mesh, 3, p6, 0.625f, 0.75f);
    add_vertex(&cube_mesh, 4, p3, 0.375f, 1.0f);
    add_vertex(&cube_mesh, 5, p2, 0.375f, 0.75f);
    add_vertex(&cube_mesh, 6, p7, 0.625f, 0.0f);
    add_vertex(&cube_mesh, 7, p1, 0.375f, 0.25f);
    add_vertex(&cube_mesh, 8, p3, 0.375f, 0.0f);
    add_vertex(&cube_mesh, 9, p0, 0.375f, 0.5f);
    add_vertex(&cube_mesh, 10, p3, 0.125f, 0.75f);
    add_vertex(&cube_mesh, 11, p1, 0.125f, 0.5f);
    add_vertex(&cube_mesh, 12, p4, 0.625f, 0.5f);
    add_vertex(&cube_mesh, 13, p2, 0.375f, 0.75f);
    add_vertex(&cube_mesh, 14, p0, 0.375f, 0.5f);
    add_vertex(&cube_mesh, 15, p5, 0.625f, 0.25f);
    add_vertex(&cube_mesh, 16, p0, 0.375f, 0.5f);
    add_vertex(&cube_mesh, 17, p1, 0.375f, 0.25f);
    add_vertex(&cube_mesh, 18, p7, 0.875f, 0.75f);
    add_vertex(&cube_mesh, 19, p7, 0.625f, 1.0f);
    add_vertex(&cube_mesh, 20, p5, 0.625f, 0.25f);
    add_vertex(&cube_mesh, 21, p2, 0.375f, 0.75f);
    add_vertex(&cube_mesh, 22, p6, 0.625f, 0.75f);
    add_vertex(&cube_mesh, 23, p4, 0.625f, 0.5f);

    add_triangle(&cube_mesh, 0, 0, 1, 2);
    add_triangle(&cube_mesh, 1, 3, 4, 5);
    add_triangle(&cube_mesh, 2, 6, 7, 8);
    add_triangle(&cube_mesh, 3, 9, 10, 11);
    add_triangle(&cube_mesh, 4, 12, 13, 14);
    add_triangle(&cube_mesh, 5, 15, 16, 17);
    add_triangle(&cube_mesh, 6, 0, 18, 1);
    add_triangle(&cube_mesh, 7, 3, 19, 4);
    add_triangle(&cube_mesh, 8, 6, 20, 7);
    add_triangle(&cube_mesh, 9, 9, 21, 10);
    add_triangle(&cube_mesh, 10, 12, 22, 13);
    add_triangle(&cube_mesh, 11, 15, 23, 16);

    return cube_mesh;
}

struct mesh create_icosahedron_mesh()
{
    struct mesh mesh;
    mesh_init(&mesh, 12, 20);

    float phi = (1.0f + sqrtf(5.0f)) * 0.5f;
    float a = 1.0f;
    float b = 1.0f / phi;

    struct vec3 p0 = vec3_create(0.0f, b, -a);
    struct vec3 p1 = vec3_create(b, a, 0.0f);
    struct vec3 p2 = vec3_create(-b, a, 0.0f);
    struct vec3 p3 = vec3_create(0.0f, b, a);
    struct vec3 p4 = vec3_create(0.0f, -b, a);
    struct vec3 p5 = vec3_create(-a, 0.0f, b);
    struct vec3 p6 = vec3_create(-a, 0.0f, b);
    struct vec3 p7 = vec3_create(a, 0.0f, -b);
    struct vec3 p8 = vec3_create(a, 0.0f, b);
    struct vec3 p9 = vec3_create(-a, 0.0f, b);
    struct vec3 p10 = vec3_create(b, -a, 0.0f);
    struct vec3 p11 = vec3_create(-b, -a, 0.0f);

    add_vertex(&mesh, 0, p0, 0.0f, 0.0f);
    add_vertex(&mesh, 1, p1, 0.0f, 0.0f);
    add_vertex(&mesh, 2, p2, 0.0f, 0.0f);
    add_vertex(&mesh, 3, p3, 0.0f, 0.0f);
    add_vertex(&mesh, 4, p4, 0.0f, 0.0f);
    add_vertex(&mesh, 5, p5, 0.0f, 0.0f);
    add_vertex(&mesh, 6, p6, 0.0f, 0.0f);
    add_vertex(&mesh, 7, p7, 0.0f, 0.0f);
    add_vertex(&mesh, 8, p8, 0.0f, 0.0f);
    add_vertex(&mesh, 9, p9, 0.0f, 0.0f);
    add_vertex(&mesh, 10, p10, 0.0f, 0.0f);
    add_vertex(&mesh, 11, p11, 0.0f, 0.0f);

    add_triangle(&mesh, 0, 2, 1, 0);
    add_triangle(&mesh, 1, 1, 2, 3);
    add_triangle(&mesh, 2, 5, 4, 3);
    add_triangle(&mesh, 3, 4, 8, 3);
    add_triangle(&mesh, 4, 7, 6, 0);
    add_triangle(&mesh, 5, 6, 9, 0);
    add_triangle(&mesh, 6, 11, 10, 4);
    add_triangle(&mesh, 7, 10, 11, 6);
    add_triangle(&mesh, 8, 9, 5, 2);
    add_triangle(&mesh, 9, 5, 9, 11);
    add_triangle(&mesh, 10, 8, 7, 1);
    add_triangle(&mesh, 11, 7, 8, 10);
    add_triangle(&mesh, 12, 2, 5, 3);
    add_triangle(&mesh, 13, 8, 1, 3);
    add_triangle(&mesh, 14, 9, 2, 0);
    add_triangle(&mesh, 15, 1, 7, 0);
    add_triangle(&mesh, 16, 11, 9, 6);
    add_triangle(&mesh, 17, 7, 10, 6);
    add_triangle(&mesh, 18, 5, 11, 4);
    add_triangle(&mesh, 19, 10, 8, 4);

    return mesh;
}
