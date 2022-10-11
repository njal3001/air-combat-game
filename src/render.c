#include "render.h"
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

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
    "layout (location = 1) in vec4 a_col;\n"
    "layout (location = 2) in vec2 a_uv;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "out vec4 v_col;\n"
    "out vec2 v_uv;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(a_pos, 1.0) * model * view * projection;\n"
    "   v_col = a_col;\n"
    "   v_uv = a_uv;\n"
    "}";

const char *frag_shader_str =
    "#version 330 core\n"
    "out vec4 o_col;"
    "in vec4 v_col;\n"
    "in vec2 v_uv;\n"
    "uniform sampler2D sampler;\n"
    "void main()\n"
    "{\n"
    "   o_col = texture(sampler, v_uv) * v_col;\n"
    "}";

size_t vertex_count;
size_t index_count;
struct vertex *vertex_map;
GLushort *index_map;

GLuint vao_id;
GLuint vbo_id;
GLuint ebo_id;
GLuint shader_id;

GLint u_model_location;
GLint u_view_location;
GLint u_projection_location;
GLint u_sampler_location;

struct mat4 model;
struct mat4 projection;

struct camera camera;

#define MAX_MSTACK 32
struct mat4 mstack[MAX_MSTACK];
size_t mstack_size;
struct mat4 mstack_top;

static void APIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message, const void *user_param);

static void push_vertex(struct vec3 pos, struct color c, float uvx, float uvy);

static void on_window_size_changed(GLFWwindow *window, int width, int height);

bool render_init(GLFWwindow *window)
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
    u_model_location = glGetUniformLocation(shader_id, "model");
    u_view_location = glGetUniformLocation(shader_id, "view");
    u_projection_location = glGetUniformLocation(shader_id, "projection");
    u_sampler_location = glGetUniformLocation(shader_id, "sampler");

    // Set sampler uniforms
    glUniform1i(u_sampler_location, 0);

    // Vertex attributes
    size_t pos_size = 3 * sizeof(GLfloat);
    size_t color_size = 4 * sizeof(GLubyte);
    size_t uv_size = 2 * sizeof(GLfloat);
    size_t stride = pos_size + color_size + uv_size;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (const GLvoid*)pos_size);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)(pos_size + color_size));
    glEnableVertexAttribArray(2);

    // Window resize callback
    glfwSetWindowSizeCallback(window, on_window_size_changed);

    // Initialize matrices
    model = mat4_identity();
    projection = mat4_perspective(FOV, ASPECT_RATIO, 0.1f, 1000.0f);

    camera.transform = transform_create(vec3_create(0.0f, 0.0f, 10.0f));
    camera.transform.rot.y = M_PI;

    mstack_top = mat4_identity();

    return true;
}

void render_shutdown()
{
    glDeleteBuffers(1, &vbo_id);
    glDeleteBuffers(1, &ebo_id);
    glDeleteVertexArrays(1, &vao_id);
}

struct texture texture_create(const char *img_path)
{
    int width, height, nchannels;
    unsigned char *data = stbi_load(img_path, &width, &height, &nchannels, 0);

    if (!data)
    {
        return (struct texture)
        {
            .id = 0,
            .width = 0,
            .height = 0,
        };
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);

    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return (struct texture)
    {
        .id = tex_id,
        .width = width,
        .height = height,
    };
}

void texture_free(struct texture *texture)
{
    glDeleteTextures(1, &texture->id);
}

struct mesh mesh_create(const char *obj_path)
{
    FILE *f = fopen(obj_path, "r");
    if (!f)
    {
        printf("Could not read file: %s (%s)\n", obj_path, strerror(errno));
        return (struct mesh)
        {
            .vertices = NULL,
            .indices = NULL,
            .vertex_count = 0,
            .index_count = 0,
        };
    }

    struct mesh mesh;
    mesh.vertex_count = 0;
    mesh.index_count = 0;

    // Determine number of vertices and indicies
    char *line = NULL;
    size_t blength = 0;
    int read;
    while ((read = getline(&line, &blength, f)) != -1)
    {
        if (!read || line[0] == '#')
        {
            continue;
        }

        if (line[0] == 'v' && line[1] == ' ')
        {
            mesh.vertex_count++;
        }
        else if (line[0] == 'f')
        {
            mesh.index_count += 3;
        }
    }

    // Prepare to read file again
    rewind(f);

    mesh.vertices = malloc(mesh.vertex_count * sizeof(struct vertex));
    mesh.indices = malloc(mesh.index_count * sizeof(GLushort));

    // Add data
    size_t current_vertex = 0;
    size_t current_index = 0;
    while ((read = getline(&line, &blength, f)) != -1)
    {
        if (!read || line[0] == '#')
        {
            continue;
        }

        if (line[0] == 'v' && line[1] == ' ')
        {
            char *xs = strtok(line + 2, " ");
            char *ys = strtok(NULL, " ");
            char *zs = strtok(NULL, " ");

            struct vertex *v = mesh.vertices + current_vertex;
            v->pos.x = strtof(xs, NULL);
            v->pos.y = strtof(ys, NULL);
            v->pos.z = strtof(zs, NULL);
            v->col = COLOR_WHITE;

            // TODO: Read uvs
            v->uvx = 0.0f;
            v->uvy = 0.0f;

            current_vertex++;
        }
        else if (line[0] == 'f')
        {
            char  *s = strtok(line + 2, " ");
            while (s && *s != '\n')
            {
                char *end = s;
                while (*end)
                {
                    if (*end == '/')
                    {
                        *end = '\0';
                        break;
                    }

                    end++;
                }

                mesh.indices[current_index++] = strtol(s, NULL, 10) - 1;

                s = strtok(NULL, " ");
            }
        }
    }

    free(line);

    return mesh;
}

void mesh_free(struct mesh *mesh)
{
    free(mesh->indices);
    free(mesh->vertices);
}

void bind_texture(const struct texture *texture, size_t index)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture->id);
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

void render_flush()
{
    // Calculate view matrix from camera transform
    struct mat4 view_rot = mat4_mul(mat4_rotz(camera.transform.rot.z),
            mat4_mul(mat4_roty(camera.transform.rot.y - M_PI), mat4_rotx(-camera.transform.rot.x)));
    struct mat4 view = mat4_mul(view_rot, mat4_translate(vec3_neg(camera.transform.pos)));

    // struct mat4 view = mat4_identity();
    // Upload uniforms
    glUniformMatrix4fv(u_model_location, 1, GL_FALSE, &model.m11);
    glUniformMatrix4fv(u_view_location, 1, GL_FALSE, &view.m11);
    glUniformMatrix4fv(u_projection_location, 1, GL_FALSE, &projection.m11);

    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_SHORT, 0);

    vertex_count = 0;
    index_count = 0;
}


void render_mpush(struct mat4 m)
{
    assert(mstack_size < MAX_MSTACK);

    memcpy(mstack + mstack_size, &mstack_top, sizeof(struct mat4));
    mstack_size++;

    mstack_top = mat4_mul(mstack_top, m);
}

void render_mpop()
{
    assert(mstack_size > 0);

    mstack_size--;
    mstack_top = mstack[mstack_size];
}

void render_tri(struct vec3 a, struct vec3 b, struct vec3 c,
        struct color col_a, struct color col_b, struct color col_c,
        float uvx_a, float uvy_a, float uvx_b, float uvy_b, float uvx_c, float uvy_c)
{
    push_vertex(a, col_a, uvx_a, uvy_a);
    push_vertex(b, col_b, uvx_b, uvy_b);
    push_vertex(c, col_c, uvx_c, uvy_c);

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
        struct color col_a, struct color col_b, struct color col_c, struct color col_d,
        float uvx_a, float uvy_a, float uvx_b, float uvy_b, float uvx_c, float uvy_c,
        float uvx_d, float uvy_d)
{
    push_vertex(a, col_a, uvx_a, uvy_a);
    push_vertex(b, col_b, uvx_b, uvy_b);
    push_vertex(c, col_c, uvx_c, uvy_c);
    push_vertex(d, col_d, uvx_d, uvy_d);

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

void render_mesh(const struct mesh *mesh)
{
    for (size_t i = 0; i < mesh->vertex_count; i++)
    {
        struct vertex *v = mesh->vertices + i;
        push_vertex(v->pos, v->col, v->uvx, v->uvy);
    }

    for (size_t i = 0; i < mesh->index_count; i++)
    {
        *index_map = vertex_count + mesh->indices[i];
        index_map++;
    }

    vertex_count += mesh->vertex_count;
    index_count += mesh->index_count;
}

struct camera *get_camera()
{
    return &camera;
}

void push_vertex(struct vec3 pos, struct color c, float uvx, float uvy)
{
    vertex_map->pos = mat4_vmul(mstack_top, pos);
    vertex_map->col = c;
    vertex_map->uvx = uvx;
    vertex_map->uvy = uvy;

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
