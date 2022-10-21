#pragma once
#include <GL/glew.h>
#include <stdbool.h>
#include "hashmap.h"
#include "spatial.h"

struct shader
{
    GLuint id;
    struct hashmap *locations;
};

bool shader_init(struct shader *shader, const char *vert_str, const char *frag_str);
void shader_free(struct shader *shader);

void shader_set_float(struct shader *shader, const char *loc, float val);
void shader_set_int(struct shader *shader, const char *loc, int val);
void shader_set_vec3(struct shader *shader, const char *loc, struct vec3 val);
void shader_set_mat4(struct shader *shader, const char *loc, struct mat4 *val);
