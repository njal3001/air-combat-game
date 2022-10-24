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

#define MAX_TEXT_VERTICES 1024

#define FOV (M_PI / 4.0f)
#define ASPECT_RATIO (1920.0f / 1080.0f)

float skybox_vertices[] =
{
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

const struct texture *current_texture;

GLuint main_vao;
GLuint main_vbo;
GLuint main_ebo;

GLuint skybox_vao;
GLuint skybox_vbo;

GLuint text_vao;
GLuint text_vbo;

struct shader main_shader;
struct shader skybox_shader;
struct cubemap skybox_map;
struct shader text_shader;

struct mat4 projection;
struct mat4 view;
struct camera camera;

struct dir_light dir_light;
struct point_light point_light;

struct font font;

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

    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Skybox vertex array
    glGenVertexArrays(1, &skybox_vao);
    glBindVertexArray(skybox_vao);

    // Skybox vertex buffer
    glGenBuffers(1, &skybox_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);

    // Skybox vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const GLvoid*)NULL);
    glEnableVertexAttribArray(0);

    // Text vertex array
    glGenVertexArrays(1, &text_vao);
    glBindVertexArray(text_vao);

    // Text vertex buffer
    glGenBuffers(1, &text_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_TEXT_VERTICES * 4 * sizeof(GLfloat),
            NULL, GL_DYNAMIC_DRAW);

    // Text vertex attributes
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (const GLvoid*)NULL);
    glEnableVertexAttribArray(0);

    // Main vertex array
    glGenVertexArrays(1, &main_vao);
    glBindVertexArray(main_vao);

    // Main vertex buffer
    glGenBuffers(1, &main_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, main_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(struct vertex), NULL, GL_DYNAMIC_DRAW);

    // Main index buffer
    glGenBuffers(1, &main_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, main_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(GLushort), NULL, GL_DYNAMIC_DRAW);

    // Main vertex attributes
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

    // Create main shader
    load_shader(&main_shader, "main.vert", "main.frag");
    glUseProgram(main_shader.id);
    shader_set_int(&main_shader, "u_sampler", 0);

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

    // Create skybox shader
    load_shader(&skybox_shader, "skybox.vert", "skybox.frag");
    glUseProgram(skybox_shader.id);
    shader_set_int(&skybox_shader, "u_skybox", 0);

    // Create text shader
    load_shader(&text_shader, "text.vert", "text.frag");
    glUseProgram(text_shader.id);
    shader_set_int(&text_shader, "u_bitmap", 0);
    struct mat4 text_proj = mat4_ortho(0.0f, 1920.0f, 0.0f, 1080.0f, 0.0f, 1.0f);
    shader_set_mat4(&text_shader, "u_projection", &text_proj);

    // Window resize callback
    glfwSetWindowSizeCallback(window, on_window_size_changed);

    projection = mat4_perspective(FOV, ASPECT_RATIO, 0.1f, 10000.0f);
    camera.transform = transform_create(VEC3_ZERO);

    const char *skybox_faces[6] =
    {
        "bkg/blue/bkg1_right.png",
        "bkg/blue/bkg1_left.png",
        "bkg/blue/bkg1_top.png",
        "bkg/blue/bkg1_bot.png",
        "bkg/blue/bkg1_front.png",
        "bkg/blue/bkg1_back.png",
    };

    load_cubemap(&skybox_map, skybox_faces);
    load_font(&font, "vcr_osd_mono_regular_48.sfl");

    return true;
}

void render_shutdown()
{
    glDeleteBuffers(1, &main_vbo);
    glDeleteBuffers(1, &main_ebo);
    glDeleteVertexArrays(1, &main_vao);
    shader_free(&main_shader);
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

void render_scene_begin()
{
    view = camera_view(&camera);

    // Dynamic geomtry
    glUseProgram(main_shader.id);
    glBindVertexArray(main_vao);

    // View and projection
    shader_set_mat4(&main_shader, "u_view", &view);
    shader_set_mat4(&main_shader, "u_projection", &projection);
    shader_set_vec3(&main_shader, "u_view_pos", camera.transform.pos);

    // Direction light
    shader_set_vec3(&main_shader, "u_dir_light.dir", dir_light.dir);
    shader_set_vec3(&main_shader, "u_dir_light.ambient", dir_light.ambient);
    shader_set_vec3(&main_shader, "u_dir_light.diffuse", dir_light.diffuse);
    shader_set_vec3(&main_shader, "u_dir_light.specular", dir_light.specular);

    // Point light
    shader_set_vec3(&main_shader, "u_point_light.pos", point_light.pos);
    shader_set_vec3(&main_shader, "u_point_light.ambient", point_light.ambient);
    shader_set_vec3(&main_shader, "u_point_light.diffuse", point_light.diffuse);
    shader_set_vec3(&main_shader, "u_point_light.specular", point_light.specular);
    shader_set_float(&main_shader, "u_point_light.constant", point_light.constant);
    shader_set_float(&main_shader, "u_point_light.linear", point_light.linear);
    shader_set_float(&main_shader, "u_point_light.quadratic", point_light.quadratic);
}

void render_skybox()
{
    struct mat4 skybox_view = mat4_remove_translation(view);

    // Change depth function to draw skybox behind all other objects
    glDepthFunc(GL_LEQUAL);

    glUseProgram(skybox_shader.id);
    shader_set_mat4(&skybox_shader, "u_view", &skybox_view);
    shader_set_mat4(&skybox_shader, "u_projection", &projection);

    glBindVertexArray(skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_map.id);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
}

void render_mesh(const struct mesh *mesh, const struct transform *transform)
{
    glBindVertexArray(main_vao);
    glBindBuffer(GL_ARRAY_BUFFER, main_vbo);
    glUseProgram(main_shader.id);
    if (mesh->material.texture)
    {
        set_texture(mesh->material.texture);
    }

    struct mat4 model_matrix = mat4_mul(mat4_mul(mat4_translate(transform->pos),
                 transform->rot), mat4_scale(transform->scale));
    // NOTE: Need to use scale matrix if non uniform scaling
    struct mat4 normal_matrix = transform->rot;

    // Matrices and view position
    shader_set_mat4(&main_shader, "u_model", &model_matrix);
    shader_set_mat4(&main_shader, "u_normal", &normal_matrix);

    // Material uniforms
    shader_set_vec3(&main_shader, "u_material.ambient", mesh->material.ambient);
    shader_set_vec3(&main_shader, "u_material.diffuse", mesh->material.diffuse);
    shader_set_vec3(&main_shader, "u_material.specular", mesh->material.specular);
    shader_set_float(&main_shader, "u_material.shininess", mesh->material.shininess);


    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->vertex_count * sizeof(struct vertex),
            mesh->vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->index_count * sizeof(GLushort),
            mesh->indices);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_SHORT, 0);
}

void render_text(const char *str, float x, float y)
{
    set_texture(&font.bitmap);
    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glUseProgram(text_shader.id);

    float curx = x;
    float cury = y;

    size_t tri_count = 0;
    float *bmap = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    uint8_t c;
    while ((c = *str))
    {
        if (c == '\n')
        {
            curx = x;
            // FIXME: Why is the spacing so small?
            // Adding extra spacing as a quick fix
            cury -= font.lheight + 10.0f;
            str++;
            continue;
        }

        assert(c >= font.start_id && c < font.start_id + font.num_char);

        struct fchar *fchar = font.chars + c - font.start_id;
        float x0 = curx + fchar->xoff;
        float x1 = x0 + fchar->w;
        float y0 = cury - (fchar->h + fchar->yoff);
        float y1 = y0 + fchar->h;

        float uvx0 = fchar->x / (float)font.bitmap.width;
        float uvx1 = (fchar->x + fchar->w) / (float)font.bitmap.width;
        float uvy0 = (fchar->y + fchar->h) / (float)font.bitmap.height;
        float uvy1 = fchar->y / (float)font.bitmap.height;

        *(bmap++) = x0;
        *(bmap++) = y1;
        *(bmap++) = uvx0;
        *(bmap++) = uvy1;

        *(bmap++) = x0;
        *(bmap++) = y0;
        *(bmap++) = uvx0;
        *(bmap++) = uvy0;

        *(bmap++) = x1;
        *(bmap++) = y0;
        *(bmap++) = uvx1;
        *(bmap++) = uvy0;

        *(bmap++) = x0;
        *(bmap++) = y1;
        *(bmap++) = uvx0;
        *(bmap++) = uvy1;

        *(bmap++) = x1;
        *(bmap++) = y0;
        *(bmap++) = uvx1;
        *(bmap++) = uvy0;

        *(bmap++) = x1;
        *(bmap++) = y1;
        *(bmap++) = uvx1;
        *(bmap++) = uvy1;

        tri_count += 6;
        curx += fchar->adv;
        str++;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDrawArrays(GL_TRIANGLES, 0, tri_count);
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
