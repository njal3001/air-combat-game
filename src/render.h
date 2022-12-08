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

#define UI_WIDTH 1920.0f
#define UI_HEIGHT 1080.0f

bool render_init(GLFWwindow *window);
void render_shutdown();

void render_skybox();

void render_mesh_instancing_begin(const struct mesh *mesh);
void render_push_mesh_transform(const struct transform *transform);
void render_mesh_instancing_end();

void render_ui_begin();
void render_ui_end();
void render_push_ui_text(const char *str, struct vec2 pos,
        float size, struct color col);

void render_untextured_begin();
void render_untextured_end();
void render_push_untextured_quad(struct vec3 a, struct vec3 b, struct vec3 c,
        struct vec3 d, struct color col);
void render_push_untextured_volume(struct vec3 p0, struct vec3 p1,
        struct vec3 p2, struct vec3 p3, struct vec3 p4, struct vec3 p5,
        struct vec3 p6, struct vec3 p7, struct color col);
void render_push_untextured_cube(struct vec3 center, struct vec3 size,
        struct color col);
void render_push_untextured_line(struct vec3 a, struct vec3 b, float thickness,
        struct color col);
void render_push_untextured_volume_outline(struct vec3 p0, struct vec3 p1,
        struct vec3 p2, struct vec3 p3, struct vec3 p4, struct vec3 p5,
        struct vec3 p6, struct vec3 p7, float thickness, struct color col);

struct camera *get_camera();
