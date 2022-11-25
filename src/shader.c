#include "shader.h"
#include "render.h"
#include <stdio.h>

static GLuint shader_get_location(struct shader *shader, const char *name);

bool shader_init(struct shader *shader, const char *vert_str, const char *frag_str)
{
    // Vertex shader
    GLuint vert_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_id, 1, &vert_str, NULL);
    glCompileShader(vert_id);

    // Check for vertex shader errors
    GLint success;
    char shader_error_msg[512];
    glGetShaderiv(vert_id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vert_id, 512, NULL, shader_error_msg);
        printf("Error! Vertex shader compilation failed!\n%s", shader_error_msg);
        return false;
    }

    // Fragment shader
    GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_id, 1, &frag_str, NULL);
    glCompileShader(frag_id);

    glGetShaderiv(frag_id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(frag_id, 512, NULL, shader_error_msg);
        printf("Error! Fragment shader compilation failed!\n%s", shader_error_msg);
        glDeleteShader(vert_id);
        return false;
    }

    // Shader program
    shader->id = glCreateProgram();
    glAttachShader(shader->id, vert_id);
    glAttachShader(shader->id, frag_id);
    glLinkProgram(shader->id);

    // Delete shaders
    glDeleteShader(vert_id);
    glDeleteShader(frag_id);

    glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader->id, 512, NULL, shader_error_msg);
        printf("Error! Shader linking failed!\n%s", shader_error_msg);
        return false;
    }

    shader->locations = hashmap_new();

    return true;
}


void shader_free(struct shader *shader)
{
    size_t n_locations = hashmap_size(shader->locations);
    if (n_locations)
    {
        GLuint **locations = malloc(n_locations * sizeof(void*));
        hashmap_values(shader->locations, (void**)locations);
        for (size_t i = 0; i < n_locations; i++)
        {
            free(locations[i]);
        }

        free(locations);
    }

    hashmap_free(shader->locations);
    glDeleteProgram(shader->id);
}

void shader_set_float(struct shader *shader, const char *loc, float val)
{
    glUniform1f(shader_get_location(shader, loc), val);
}

void shader_set_int(struct shader *shader, const char *loc, int val)
{
    glUniform1i(shader_get_location(shader, loc), val);
}

void shader_set_vec3(struct shader *shader, const char *loc, struct vec3 val)
{
    glUniform3f(shader_get_location(shader, loc), val.x, val.y, val.z);
}

void shader_set_mat4(struct shader *shader, const char *loc, struct mat4 *val)
{
    glUniformMatrix4fv(shader_get_location(shader, loc), 1,
            GL_FALSE, &val->m11);
}

void shader_set_color(struct shader *shader, const char *loc, struct color col)
{
    float r = col.r / 255.0f;
    float g = col.g / 255.0f;
    float b = col.b / 255.0f;
    float a = col.a / 255.0f;

    glUniform4f(shader_get_location(shader, loc), r, g, b, a);
}

GLuint shader_get_location(struct shader *shader, const char *name)
{
    GLuint* cached_loc = hashmap_get(shader->locations, name);
    if (cached_loc)
    {
        return *cached_loc;
    }

    GLuint *loc = malloc(sizeof(GLuint));
    *loc = glGetUniformLocation(shader->id, name);

    hashmap_put(shader->locations, name, loc);
    return *loc;
}
