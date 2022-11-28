#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdbool.h>
#include "color.h"
#include "mesh.h"
#include "spatial.h"
#include "transform.h"
#include "camera.h"

bool render_init(GLFWwindow *window);
void render_shutdown();

void render_skybox();

void mesh_instancing_begin(const struct mesh *mesh);
void push_mesh_transform(const struct transform *transform);
void mesh_instancing_end();

void ui_begin();
void ui_end();
void push_text(const char *str, float x, float y, float size);

void untextured_frame_begin();
void untextured_frame_end();
void push_quad(struct vec3 a, struct vec3 b, struct vec3 c,
        struct vec3 d, struct color col);
void push_volume(struct vec3 p0, struct vec3 p1, struct vec3 p2, struct vec3 p3,
        struct vec3 p4, struct vec3 p5, struct vec3 p6, struct vec3 p7,
        struct color col);
void push_cube(struct vec3 center, struct vec3 size, struct color col);
void push_line(struct vec3 a, struct vec3 b, float thickness, struct color col);
void push_volume_outline(struct vec3 p0, struct vec3 p1, struct vec3 p2,
        struct vec3 p3, struct vec3 p4, struct vec3 p5, struct vec3 p6,
        struct vec3 p7, float thickness, struct color col);

struct camera *get_camera();
