#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdbool.h>
#include "spatial.h"
#include "transform.h"
#include "camera.h"

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
    struct vec3 norm;
    float uvx;
    float uvy;
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

struct material
{
    struct vec3 ambient;
    struct vec3 diffuse;
    struct vec3 specular;
    float shininess;
    const struct texture *texture;
};

struct mesh
{
    struct vertex *vertices;
    GLushort *indices;
    size_t vertex_count;
    size_t index_count;
    struct material material;
};

struct dir_light
{
    struct vec3 dir;
    struct vec3 ambient;
    struct vec3 diffuse;
    struct vec3 specular;
};

struct point_light
{
    struct vec3 pos;
    struct vec3 ambient;
    struct vec3 diffuse;
    struct vec3 specular;
    float constant;
    float linear;
    float quadratic;
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

bool render_init(GLFWwindow *window);
void render_shutdown();

void set_texture(const struct texture *texture);

void render_scene_begin();
void render_skybox();

void render_mesh(const struct mesh *mesh, const struct transform *transform);
void render_text(const char *str, float x, float y, float size);

struct camera *get_camera();
struct dir_light *get_dir_light();
struct point_light *get_point_light();

struct color color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
struct vec3 color_to_vec3(struct color col);

extern const struct color COLOR_WHITE;
extern const struct color COLOR_BLACK;
extern const struct color COLOR_RED;
extern const struct color COLOR_GREEN;
extern const struct color COLOR_BLUE;
