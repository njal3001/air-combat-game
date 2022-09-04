#pragma once
#include "spatial.h"
#include <stdint.h>
#include <stdbool.h>

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
    // struct color col;
};

bool render_init();
void render_shutdown();

void render_begin();
void render_end();
void render_flush();

void render_tri(struct vec3 a, struct vec3 b, struct vec3 c);
void render_quad(struct vec3 a, struct vec3 b, struct vec3 c, struct vec3 d);
void render_cube(struct vec3 pos, float length);

// void render_cube(struct vec3 pos, float length, struct color col_a,
//         struct color col_b, struct color col_c, struct color col_d);

struct color color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
