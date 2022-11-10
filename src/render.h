#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdbool.h>
#include "spatial.h"
#include "transform.h"
#include "camera.h"
#include "vertex.h"

struct color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct vert_mesh
{
    struct vec3 pos;
    float uvx;
    float uvy;
};

struct vert_ui
{
    float x, y;
    float uvx, uvy;
};

struct vert_untextured
{
    struct vec3 pos;
    struct color col;
};

struct texture
{
    GLuint id;
    GLsizei width, height;
};

struct cubemap
{
    GLuint id;
};

struct mesh
{
    struct vert_mesh *vertices;
    GLuint *indices;
    size_t vertex_count;
    size_t index_count;
    const struct texture *texture;
};

struct fchar
{
    uint16_t x;
    uint16_t y;
    uint8_t w;
    uint8_t h;
    uint8_t xoff;
    uint8_t yoff;
    uint8_t adv;
};

struct font
{
    size_t start_id;
    size_t num_char;
    uint8_t lheight;
    struct fchar *chars;
    struct texture bitmap;
};

struct render_frame
{
    void *vbo_map;
    GLuint *ebo_map;
    size_t vbo_count;
    size_t ebo_count;
};

bool render_init(GLFWwindow *window);
void render_shutdown();

void set_texture(const struct texture *texture);

void render_skybox();

void mesh_instancing_begin(const struct mesh *mesh);
void push_mesh_transform(const struct transform *transform);
void mesh_instancing_end();

void text_frame_begin();
void text_frame_end();
void push_text(const char *str, float x, float y, float size);

void untextured_frame_begin();
void untextured_frame_end();
void push_quad(struct vec3 a, struct vec3 b, struct vec3 c, struct vec3 d, struct color col);
void push_volume(struct vec3 p0, struct vec3 p1, struct vec3 p2, struct vec3 p3,
        struct vec3 p4, struct vec3 p5, struct vec3 p6, struct vec3 p7, struct color col);
void push_cube(struct vec3 center, struct vec3 size, struct color col);
void push_line(struct vec3 a, struct vec3 b, float thickness, struct color col);
void push_volume_outline(struct vec3 p0, struct vec3 p1, struct vec3 p2, struct vec3 p3,
        struct vec3 p4, struct vec3 p5, struct vec3 p6, struct vec3 p7, float thickness,
        struct color col);

struct camera *get_camera();

struct color color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
struct vec3 color_to_vec3(struct color col);

extern const struct color COLOR_WHITE;
extern const struct color COLOR_BLACK;
extern const struct color COLOR_RED;
extern const struct color COLOR_GREEN;
extern const struct color COLOR_BLUE;
