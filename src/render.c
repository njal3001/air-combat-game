#include "render.h"
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <math.h>

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

const char *vert_shader_str =
    "#version 330 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "layout (location = 1) in vec3 a_norm;\n"
    "layout (location = 2) in vec2 a_uv;\n"
    "uniform mat4 u_model;\n"
    "uniform mat4 u_normal;\n"
    "uniform mat4 u_view;\n"
    "uniform mat4 u_projection;\n"
    "out vec3 v_pos;\n"
    "out vec3 v_norm;\n"
    "out vec2 v_uv;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(a_pos, 1.0) * u_model * u_view * u_projection;\n"
    "   v_pos = vec3(vec4(a_pos, 1.0) * u_model);\n"
    "   v_norm = vec3(vec4(a_norm, 1.0) * u_normal);\n"
    "   v_uv = a_uv;\n"
    "}";

const char *frag_shader_str =
    "#version 330 core\n"
    "struct material\n"
    "{\n"
    "   vec3 ambient;\n"
    "   vec3 diffuse;\n"
    "   vec3 specular;\n"
    "   float shininess;\n"
    "};\n"
    "struct light\n"
    "{\n"
    "   vec3 pos;\n"
    "   vec4 col;\n"
    "};\n"
    "out vec4 o_col;\n"
    "in vec3 v_pos;\n"
    "in vec3 v_norm;\n"
    "in vec2 v_uv;\n"
    "uniform material u_material;\n"
    "uniform sampler2D u_sampler;\n"
    "uniform vec3 u_view_pos;\n"
    "uniform light u_light;\n"
    "void main()\n"
    "{\n"
    "   vec3 light_dir = normalize(u_light.pos - v_pos);\n"
    "   vec3 view_dir = normalize(u_view_pos - v_pos);\n"
    "   vec3 reflect_dir = reflect(-light_dir, v_norm);\n"
    "   vec3 diffuse = max(dot(v_norm, light_dir), 0.0) * u_material.diffuse;\n"
    "   vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess) * u_material.specular;\n"
    "   o_col = vec4((u_material.ambient + diffuse + specular), 1.0) * u_light.col * texture(u_sampler, v_uv);\n"
    "}";

const struct texture *current_texture;

GLuint vao_id;
GLuint vbo_id;
GLuint ebo_id;
GLuint shader_id;

GLint u_model_location;
GLint u_normal_location;
GLint u_view_location;
GLint u_projection_location;
GLint u_view_pos_location;
GLint u_light_pos_location;
GLint u_light_color_location;
GLint u_sampler_location;
GLint u_ambient_location;
GLint u_diffuse_location;
GLint u_specular_location;
GLint u_shininess_location;

struct mat4 projection;

struct camera camera;

struct vec3 light_pos;
struct color light_color;

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

    // Vertex shader
    GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader_id, 1, &vert_shader_str, NULL);
    glCompileShader(vert_shader_id);

    // Check for vertex shader errors
    GLint success;
    char shader_error_msg[512];
    glGetShaderiv(vert_shader_id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vert_shader_id, 512, NULL, shader_error_msg);
        printf("Error! Vertex shader compilation failed!\n%s", shader_error_msg);
        return false;
    }

    // Fragment shader
    GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader_id, 1, &frag_shader_str, NULL);
    glCompileShader(frag_shader_id);

    glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(frag_shader_id, 512, NULL, shader_error_msg);
        printf("Error! Fragment shader compilation failed!\n%s", shader_error_msg);
        glDeleteShader(vert_shader_id);
        return false;
    }

    // Shader program
    shader_id = glCreateProgram();
    glAttachShader(shader_id, vert_shader_id);
    glAttachShader(shader_id, frag_shader_id);
    glLinkProgram(shader_id);

    // Delete shaders
    glDeleteShader(vert_shader_id);
    glDeleteShader(frag_shader_id);

    glGetProgramiv(shader_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_id, 512, NULL, shader_error_msg);
        printf("Error! Shader linking failed!\n%s", shader_error_msg);
        return false;
    }

    glUseProgram(shader_id);

    // Store shader uniform locations
    u_model_location = glGetUniformLocation(shader_id, "u_model");
    u_normal_location = glGetUniformLocation(shader_id, "u_normal");
    u_view_location = glGetUniformLocation(shader_id, "u_view");
    u_projection_location = glGetUniformLocation(shader_id, "u_projection");
    u_view_pos_location = glGetUniformLocation(shader_id, "u_view_pos");
    u_light_pos_location = glGetUniformLocation(shader_id, "u_light.pos");
    u_light_color_location = glGetUniformLocation(shader_id, "u_light.col");
    u_sampler_location = glGetUniformLocation(shader_id, "u_sampler");
    u_ambient_location = glGetUniformLocation(shader_id, "u_material.ambient");
    u_diffuse_location = glGetUniformLocation(shader_id, "u_material.diffuse");
    u_specular_location = glGetUniformLocation(shader_id, "u_material.specular");
    u_shininess_location = glGetUniformLocation(shader_id, "u_material.shininess");

    // Set sampler uniforms
    glUniform1i(u_sampler_location, 0);

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

    // Window resize callback
    glfwSetWindowSizeCallback(window, on_window_size_changed);

    // Initialize matrices
    projection = mat4_perspective(FOV, ASPECT_RATIO, 0.1f, 10000.0f);

    camera.transform = transform_create(vec3_create(0.0f, 0.0f, -30.0f));

    light_color = COLOR_WHITE;

    return true;
}

void render_shutdown()
{
    glDeleteBuffers(1, &vbo_id);
    glDeleteBuffers(1, &ebo_id);
    glDeleteVertexArrays(1, &vao_id);
    glDeleteProgram(shader_id);
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

    // Upload uniforms

    glUniformMatrix4fv(u_model_location, 1, GL_FALSE, &model_matrix.m11);
    glUniformMatrix4fv(u_normal_location, 1, GL_FALSE, &normal_matrix.m11);
    glUniformMatrix4fv(u_view_location, 1, GL_FALSE, &view.m11);
    glUniformMatrix4fv(u_projection_location, 1, GL_FALSE, &projection.m11);

    glUniform3f(u_view_pos_location, camera.transform.pos.x, camera.transform.pos.y,
            camera.transform.pos.z);
    glUniform3f(u_light_pos_location, light_pos.x, light_pos.y, light_pos.z);

    struct vec3 col = color_to_vec3(light_color);
    glUniform4f(u_light_color_location, col.x, col.y, col.z, 1.0f);

    glUniform3f(u_ambient_location, mesh->material.ambient.x, mesh->material.ambient.y, mesh->material.ambient.z);
    glUniform3f(u_diffuse_location, mesh->material.diffuse.x, mesh->material.diffuse.y, mesh->material.diffuse.z);
    glUniform3f(u_specular_location, mesh->material.specular.x, mesh->material.specular.y, mesh->material.specular.z);
    glUniform1f(u_shininess_location, mesh->material.shininess);

    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->vertex_count * sizeof(struct vertex), mesh->vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->index_count * sizeof(GLushort), mesh->indices);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_SHORT, 0);
}

struct camera *get_camera()
{
    return &camera;
}

void set_light_pos(struct vec3 pos)
{
    light_pos = pos;
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
