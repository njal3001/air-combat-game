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

bool render_init(GLFWwindow *window);
void render_shutdown();

void set_texture(const struct texture *texture);

void render_mesh(const struct mesh *mesh, const struct transform *transform);

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
