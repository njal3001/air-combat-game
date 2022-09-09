#pragma once
#include <GL/glew.h>
#include <stdint.h>
#include <stdbool.h>
#include "spatial.h"

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

bool render_init();
void render_shutdown();

struct texture create_texture(const char *img_path);

void render_begin();
void render_end();
void render_flush();

void bind_texture(struct texture *texture, size_t index);

void render_tri(struct vec3 a, struct vec3 b, struct vec3 c,
        struct color col_a, struct color col_b, struct color col_c,
        float uvx_a, float uvy_a, float uvx_b, float uvy_b, float uvx_c, float uvy_c);
void render_quad(struct vec3 a, struct vec3 b, struct vec3 c, struct vec3 d,
        struct color col_a, struct color col_b, struct color col_c, struct color col_d,
        float uvx_a, float uvy_a, float uvx_b, float uvy_b, float uvx_c, float uvy_c,
        float uvx_d, float uvy_d);

void render_model(struct vertex *vertices, size_t vertex_count, GLushort *indices,
        size_t index_count);

struct color color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

extern const struct color COLOR_WHITE;
extern const struct color COLOR_BLACK;
extern const struct color COLOR_RED;
extern const struct color COLOR_GREEN;
extern const struct color COLOR_BLUE;
