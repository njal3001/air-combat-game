#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdbool.h>
#include "spatial.h"
#include "transform.h"

struct color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct vertex
{
    struct vec3 pos;
    struct color col;
    float uvx;
    float uvy;
};

struct texture
{
    GLuint id;
    GLsizei width, height;
};

struct mesh
{
    struct vertex *vertices;
    GLushort *indices;
    size_t vertex_count;
    size_t index_count;
};

struct camera
{
    struct transform transform;
};

bool render_init(GLFWwindow *window);
void render_shutdown();

struct texture texture_create(const char *img_path);
void texture_free(struct texture *texture);
struct mesh mesh_create(const char *obj_path);
void mesh_free(struct mesh *mesh);

void render_begin();
void render_end();
void render_flush();

void render_mpush(struct mat4 m);
void render_mpop();

void bind_texture(const struct texture *texture, size_t index);

void render_tri(struct vec3 a, struct vec3 b, struct vec3 c,
        struct color col_a, struct color col_b, struct color col_c,
        float uvx_a, float uvy_a, float uvx_b, float uvy_b, float uvx_c, float uvy_c);
void render_quad(struct vec3 a, struct vec3 b, struct vec3 c, struct vec3 d,
        struct color col_a, struct color col_b, struct color col_c, struct color col_d,
        float uvx_a, float uvy_a, float uvx_b, float uvy_b, float uvx_c, float uvy_c,
        float uvx_d, float uvy_d);

void render_mesh(const struct mesh *mesh);


struct camera *get_camera();

struct color color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

extern const struct color COLOR_WHITE;
extern const struct color COLOR_BLACK;
extern const struct color COLOR_RED;
extern const struct color COLOR_GREEN;
extern const struct color COLOR_BLUE;
