#include "render.h"
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "shader.h"
#include "assets.h"

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#else
#    define APIENTRY
#endif

#define MAX_VERTICES 65536
#define MAX_INDICES 131072

#define FOV (M_PI / 4.0f)
#define ASPECT_RATIO (640.0f / 480.0f)

const struct texture *current_texture;

GLuint vao_id;
GLuint vbo_id;
GLuint ebo_id;

struct shader shader;
struct mat4 projection;
struct camera camera;

struct dir_light dir_light;
struct point_light point_light;

static void APIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message, const void *user_param);

static void on_window_size_changed(GLFWwindow *window, int width, int height);

bool render_init(GLFWwindow *window)
{
    // Debug messages
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_message_callback, NULL);

    // Depth testing
    glEnable(GL_DEPTH_TEST);

    // Culling
    glEnable(GL_CULL_FACE);

    // Vertex array
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    // Vertex buffer
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(struct vertex), NULL, GL_DYNAMIC_DRAW);

    // Index buffer
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLushort), NULL, GL_DYNAMIC_DRAW);

    // Vertex attributes
    size_t pos_size = 3 * sizeof(GLfloat);
    size_t norm_size = 3 * sizeof(GLfloat);
    size_t uv_size = 2 * sizeof(GLfloat);
    size_t stride = pos_size + norm_size + uv_size;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)pos_size);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
            (const GLvoid*)(pos_size + norm_size));
    glEnableVertexAttribArray(2);

    // Create shader
    if (!load_shader(&shader, "main.vert", "main.frag"))
    {
        return false;
    }
    glUseProgram(shader.id);

    // Set sampler uniform
    shader_set_int(&shader, "u_sampler", 0);

    // Default light values
    dir_light.dir = vec3_create(-0.2f, -1.0f, -0.2f);
    dir_light.ambient = vec3_create(0.2f, 0.2f, 0.2f);
    dir_light.diffuse = vec3_create(0.5f, 0.5f, 0.5f);
    dir_light.specular = vec3_create(1.0f, 1.0f, 1.0f);

    point_light.pos = VEC3_ZERO;
    point_light.ambient = vec3_create(0.2f, 0.2f, 0.2f);
    point_light.diffuse = vec3_create(0.5f, 0.5f, 0.5f);
    point_light.specular = vec3_create(1.0f, 1.0f, 1.0f);
    point_light.constant = 0.00001f;
    point_light.linear = 0.000009f;
    point_light.quadratic = 0.0000032f;

    // Window resize callback
    glfwSetWindowSizeCallback(window, on_window_size_changed);

    projection = mat4_perspective(FOV, ASPECT_RATIO, 0.1f, 10000.0f);
    camera.transform = transform_create(VEC3_ZERO);

    return true;
}

void render_shutdown()
{
    glDeleteBuffers(1, &vbo_id);
    glDeleteBuffers(1, &ebo_id);
    glDeleteVertexArrays(1, &vao_id);
    shader_free(&shader);
}

void set_texture(const struct texture *texture)
{
    if (current_texture != texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);

        current_texture = texture;
    }
}


void render_mesh(const struct mesh *mesh, const struct transform *transform)
{
    if (mesh->material.texture)
    {
        set_texture(mesh->material.texture);
    }

    struct mat4 model_matrix = mat4_mul(mat4_mul(mat4_translate(transform->pos),
                 transform->rot), mat4_scale(transform->scale));
    // NOTE: Need to use scale matrix if non uniform scaling
    struct mat4 normal_matrix = transform->rot;
    struct mat4 view = camera_view(&camera);

    // Matrices and view position
    shader_set_mat4(&shader, "u_model", &model_matrix);
    shader_set_mat4(&shader, "u_normal", &normal_matrix);
    shader_set_mat4(&shader, "u_view", &view);
    shader_set_mat4(&shader, "u_projection", &projection);
    shader_set_vec3(&shader, "u_view_pos", camera.transform.pos);

    // Material uniforms
    shader_set_vec3(&shader, "u_material.ambient", mesh->material.ambient);
    shader_set_vec3(&shader, "u_material.diffuse", mesh->material.diffuse);
    shader_set_vec3(&shader, "u_material.specular", mesh->material.specular);
    shader_set_float(&shader, "u_material.shininess", mesh->material.shininess);

    // Direction light
    shader_set_vec3(&shader, "u_dir_light.dir", dir_light.dir);
    shader_set_vec3(&shader, "u_dir_light.ambient", dir_light.ambient);
    shader_set_vec3(&shader, "u_dir_light.diffuse", dir_light.diffuse);
    shader_set_vec3(&shader, "u_dir_light.specular", dir_light.specular);

    // Point light
    shader_set_vec3(&shader, "u_point_light.pos", point_light.pos);
    shader_set_vec3(&shader, "u_point_light.ambient", point_light.ambient);
    shader_set_vec3(&shader, "u_point_light.diffuse", point_light.diffuse);
    shader_set_vec3(&shader, "u_point_light.specular", point_light.specular);
    shader_set_float(&shader, "u_point_light.constant", point_light.constant);
    shader_set_float(&shader, "u_point_light.linear", point_light.linear);
    shader_set_float(&shader, "u_point_light.quadratic", point_light.quadratic);

    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->vertex_count * sizeof(struct vertex),
            mesh->vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->index_count * sizeof(GLushort),
            mesh->indices);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_SHORT, 0);
}

struct camera *get_camera()
{
    return &camera;
}

struct dir_light *get_dir_light()
{
    return &dir_light;
}

struct point_light *get_point_light()
{
    return &point_light;
}

struct color color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    struct color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;

    return c;
}

struct vec3 color_to_vec3(struct color col)
{
    return vec3_create(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f);
}

void on_window_size_changed(GLFWwindow *window, int width, int height)
{
    // Preserve aspect ratio
    float vw, vh;
    if (width / ASPECT_RATIO > height)
    {
        vw = height * ASPECT_RATIO;
        vh = height;
    }
    else
    {
        vw = width;
        vh = width / ASPECT_RATIO;
    }

    float vx = (width - vw) / 2.0f;
    float vy = (height - vh) / 2.0f;

    glViewport(vx, vy, vw, vh);
}

void APIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message, const void *user_param)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION &&
        type == GL_DEBUG_TYPE_OTHER)
    {
        return;
    }

    const char *type_name = "";
    const char *severity_name = "";

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            type_name = "ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            type_name = "DEPRECATED BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_MARKER:
            type_name = "MARKER";
            break;
        case GL_DEBUG_TYPE_OTHER:
            type_name = "OTHER";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            type_name = "PEROFRMANCE";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            type_name = "POP GROUP";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            type_name = "PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            type_name = "PUSH GROUP";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            type_name = "UNDEFINED BEHAVIOR";
            break;
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            severity_name = "HIGH";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severity_name = "MEDIUM";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severity_name = "LOW";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severity_name = "NOTIFICATION";
            break;
    }

    if (type == GL_DEBUG_TYPE_ERROR)
    {
        printf("GL (%s:%s) %s\n", type_name, severity_name, message);
    }
    else if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        printf("GL (%s:%s) %s\n", type_name, severity_name, message);
    }
    else
    {
        printf("GL (%s) %s\n", type_name, message);
    }
}

const struct color COLOR_WHITE = { 255, 255, 255, 255 };
const struct color COLOR_BLACK = { 0, 0, 0, 255       };
const struct color COLOR_RED =   { 255, 0, 0, 255     };
const struct color COLOR_GREEN = { 0, 255, 0, 255     };
const struct color COLOR_BLUE =  { 0, 0, 255, 255     };
