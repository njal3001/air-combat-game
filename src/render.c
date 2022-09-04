#include "render.h"
#include <stdbool.h>
#include <GL/glew.h>
#include <stdio.h>

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#else
#    define APIENTRY
#endif

#define MAX_VERTICES 1024
#define MAX_INDICES 2048

const char *vert_shader_str =
    "#version 330 core\n"
    "layout (location = 0) in vec3 a_pos;\n"
    "layout (location = 1) in vec4 a_col;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec4 v_col;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(a_pos, 1.0) * model * view * projection;\n"
    "   v_col = a_col;\n"
    "}";

const char *frag_shader_str =
    "#version 330 core\n"
    "out vec4 o_col;"
    "in vec4 v_col;\n"
    "void main()\n"
    "{\n"
    "   o_col = v_col;\n"
    "}";

size_t vertex_count;
size_t index_count;
struct vertex *vertex_map;
GLushort *index_map;

GLuint vao_id;
GLuint vbo_id;
GLuint ebo_id;
GLuint shader_id;

GLint model_location;
GLint view_location;
GLint projection_location;

static void APIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message, const void *user_param);

static void make_vertex(float x, float y, float z, struct color c);

bool render_init()
{
    // Debug messages
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_message_callback, NULL);

    // Depth testing
    glEnable(GL_DEPTH_TEST);

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
    glUseProgram(shader_id);

    // Store shader uniform locations
    model_location = glGetUniformLocation(shader_id, "model");
    view_location = glGetUniformLocation(shader_id, "view");
    projection_location = glGetUniformLocation(shader_id, "projection");


    // Delete shaders
    glDeleteShader(vert_shader_id);
    glDeleteShader(frag_shader_id);

    // Vertex attributes
    size_t pos_size = 3 * sizeof(GLfloat);
    size_t color_size = 4 * sizeof(GLubyte);
    size_t stride = pos_size + color_size;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const GLvoid*)pos_size);
    glEnableVertexAttribArray(1);

    return true;
}

void render_shutdown()
{
    glDeleteBuffers(1, &vbo_id);
    glDeleteBuffers(1, &ebo_id);
    glDeleteVertexArrays(1, &vao_id);
}

void render_begin()
{
    vertex_map = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    index_map = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void render_end()
{
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    vertex_map = NULL;
    index_map = NULL;
}

void render_flush(struct mat4 *model, struct mat4 *view, struct mat4 *projection)
{
    // Upload uniforms
    glUniformMatrix4fv(model_location, 1, GL_FALSE, &model->m11);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, &view->m11);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, &projection->m11);

    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_SHORT, 0);

    vertex_count = 0;
    index_count = 0;
}

void render_tri(struct vec3 a, struct vec3 b, struct vec3 c,
        struct color col_a, struct color col_b, struct color col_c)
{
    make_vertex(a.x, a.y, a.z, col_a);
    make_vertex(b.x, b.y, b.z, col_b);
    make_vertex(c.x, c.y, c.z, col_c);

    *index_map = vertex_count;
    index_map++;
    *index_map = vertex_count + 1;
    index_map++;
    *index_map = vertex_count + 2;
    index_map++;

    vertex_count += 3;
    index_count += 3;
}

void render_quad(struct vec3 a, struct vec3 b, struct vec3 c, struct vec3 d,
        struct color col_a, struct color col_b, struct color col_c, struct color col_d)
{
    make_vertex(a.x, a.y, a.z, col_a);
    make_vertex(b.x, b.y, b.z, col_b);
    make_vertex(c.x, c.y, c.z, col_c);
    make_vertex(d.x, d.y, d.z, col_d);

    *index_map = vertex_count;
    index_map++;
    *index_map = vertex_count + 1;
    index_map++;
    *index_map = vertex_count + 3;
    index_map++;
    *index_map = vertex_count + 1;
    index_map++;
    *index_map = vertex_count + 2;
    index_map++;
    *index_map = vertex_count + 3;
    index_map++;

    vertex_count += 4;
    index_count += 6;
}

void render_cube(struct vec3 pos, float length, struct color top_col,
        struct color bot_col, struct color left_col, struct color right_col,
        struct color near_col, struct color far_col)
{
    // TODO: Some overlapping vertices could be removed

    float l2 = length / 2.0f;
    // Top
    render_quad(
            vec3_create(pos.x - l2, pos.y + l2, pos.z - l2),
            vec3_create(pos.x - l2, pos.y + l2, pos.z + l2),
            vec3_create(pos.x + l2, pos.y + l2, pos.z + l2),
            vec3_create(pos.x + l2, pos.y + l2, pos.z - l2),
            top_col, top_col, top_col, top_col);
    // Bottom
    render_quad(
            vec3_create(pos.x - l2, pos.y - l2, pos.z - l2),
            vec3_create(pos.x - l2, pos.y - l2, pos.z + l2),
            vec3_create(pos.x + l2, pos.y - l2, pos.z + l2),
            vec3_create(pos.x + l2, pos.y - l2, pos.z - l2),
            bot_col, bot_col, bot_col, bot_col);
    // Left
    render_quad(
            vec3_create(pos.x - l2, pos.y - l2, pos.z + l2),
            vec3_create(pos.x - l2, pos.y + l2, pos.z + l2),
            vec3_create(pos.x - l2, pos.y + l2, pos.z - l2),
            vec3_create(pos.x - l2, pos.y - l2, pos.z - l2),
            left_col, left_col, left_col, left_col);
    // Right
    render_quad(
            vec3_create(pos.x + l2, pos.y - l2, pos.z + l2),
            vec3_create(pos.x + l2, pos.y + l2, pos.z + l2),
            vec3_create(pos.x + l2, pos.y + l2, pos.z - l2),
            vec3_create(pos.x + l2, pos.y - l2, pos.z - l2),
            right_col, right_col, right_col, right_col);
    // Near
    render_quad(
            vec3_create(pos.x - l2, pos.y - l2, pos.z - l2),
            vec3_create(pos.x - l2, pos.y + l2, pos.z - l2),
            vec3_create(pos.x + l2, pos.y + l2, pos.z - l2),
            vec3_create(pos.x + l2, pos.y - l2, pos.z - l2),
            near_col, near_col, near_col, near_col);
    // Far
    render_quad(
            vec3_create(pos.x - l2, pos.y - l2, pos.z + l2),
            vec3_create(pos.x - l2, pos.y + l2, pos.z + l2),
            vec3_create(pos.x + l2, pos.y + l2, pos.z + l2),
            vec3_create(pos.x + l2, pos.y - l2, pos.z + l2),
            far_col, far_col, far_col, far_col);
}

void make_vertex(float x, float y, float z, struct color c)
{
    vertex_map->pos.x = x;
    vertex_map->pos.y = y;
    vertex_map->pos.z = z;
    vertex_map->col = c;

    vertex_map++;
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
