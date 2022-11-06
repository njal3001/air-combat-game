#include "render.h"
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "shader.h"
#include "assets.h"
#include "vertex.h"

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#else
#    define APIENTRY
#endif

// TODO: Should have separate maximums for each vao
#define MAX_VERTICES 30000000
#define MAX_INDICES 50000000

#define FOV (M_PI / 2.5f)
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

struct vert_array main_vao;
struct vert_array skybox_vao;
struct vert_array text_vao;
struct vert_array untextured_vao;

struct shader main_shader;
struct shader skybox_shader;
struct shader text_shader;
struct shader untextured_shader;

struct mat4 projection;
struct camera camera;

struct cubemap skybox_map;
struct font font;

struct render_frame current_frame;
const struct mesh *current_mesh;

static void APIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message, const void *user_param);

static void on_window_size_changed(GLFWwindow *window, int width, int height);


static void render_frame_begin(const struct vert_array *vao, const struct texture *texture, const struct shader *shader);
static void render_frame_end();

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

    // Window resize callback
    glfwSetWindowSizeCallback(window, on_window_size_changed);

    // Main vertex array
    struct vert_array_data main_data;
    main_data.vbo_size = MAX_VERTICES;
    main_data.ebo_size = MAX_INDICES;
    main_data.vbo_data = NULL;
    main_data.ebo_data = NULL;

    struct vert_attrib pos_attrib =
    {
        .type = VTYPE_FLOAT3,
        .normalized = false,
    };
    struct vert_attrib uv_attrib =
    {
        .type = VTYPE_FLOAT2,
        .normalized = false,
    };

    vert_array_init(&main_vao, &main_data, VARRAY_DYNAMIC, 2, pos_attrib, uv_attrib);

    // Skybox vertex array
    struct vert_array_data skybox_data;
    skybox_data.vbo_size = 36;
    skybox_data.ebo_size = 0;
    skybox_data.vbo_data = skybox_vertices;
    skybox_data.ebo_data = NULL;

    vert_array_init(&skybox_vao, &skybox_data, VARRAY_STATIC, 1, pos_attrib);

    // Text vertex array
    struct vert_array_data text_data;
    text_data.vbo_size = MAX_VERTICES;
    text_data.ebo_size = MAX_INDICES;
    text_data.vbo_data = NULL;
    text_data.ebo_data = NULL;

    struct vert_attrib text_attrib =
    {
        .type = VTYPE_FLOAT4,
        .normalized = false,
    };

    vert_array_init(&text_vao, &text_data, VARRAY_DYNAMIC, 1, text_attrib);

    // Untextured vertex array
    struct vert_array_data untextured_data;
    untextured_data.vbo_size = MAX_VERTICES;
    untextured_data.ebo_size = MAX_INDICES;
    untextured_data.vbo_data = NULL;
    untextured_data.ebo_data = NULL;

    struct vert_attrib color_attrib =
    {
        .type = VTYPE_UBYTE4,
        .normalized = true,
    };

    vert_array_init(&untextured_vao, &untextured_data, VARRAY_DYNAMIC, 2,
        pos_attrib, color_attrib);

    // Create main shader
    load_shader(&main_shader, "main.vert", "main.frag");
    glUseProgram(main_shader.id);
    shader_set_int(&main_shader, "u_sampler", 0);

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

    // Create untextured shader
    load_shader(&untextured_shader, "untextured.vert", "untextured.frag");
    glUseProgram(untextured_shader.id);

    // Loading assets
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

    projection = mat4_perspective(FOV, ASPECT_RATIO, 0.1f, 10000.0f);
    camera.transform = transform_create(VEC3_ZERO);

    return true;
}

void render_shutdown()
{
    vert_array_free(&main_vao);
    vert_array_free(&skybox_vao);
    vert_array_free(&text_vao);
    vert_array_free(&untextured_vao);
    shader_free(&main_shader);
    shader_free(&skybox_shader);
    shader_free(&text_shader);
    shader_free(&untextured_shader);
    font_free(&font);
}

void render_frame_begin(const struct vert_array *vao, const struct texture *texture, const struct shader *shader)
{
    assert(!current_frame.vbo_count);
    assert(!current_frame.ebo_count);
    assert(!current_frame.vbo_map);
    assert(!current_frame.ebo_map);

    if (texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->id);
    }

    glUseProgram(shader->id);

    glBindVertexArray(vao->id);
    glBindBuffer(GL_ARRAY_BUFFER, vao->vbo);

    current_frame.vbo_map = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (vao->ebo)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->ebo);
        current_frame.ebo_map = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    }
}

void render_frame_end()
{
    assert(current_frame.vbo_map);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    if (current_frame.ebo_map)
    {
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    if (current_frame.vbo_count)
    {
        if (current_frame.ebo_map)
        {
            // glDrawArrays(GL_TRIANGLES, 0, current_frame.vbo_count);
            glDrawElements(GL_TRIANGLES, current_frame.ebo_count, GL_UNSIGNED_INT, (void*)NULL);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, current_frame.vbo_count);
        }
    }

    current_frame.vbo_count = 0;
    current_frame.ebo_count = 0;
    current_frame.ebo_map = NULL;
    current_frame.vbo_map = NULL;
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

void render_skybox()
{
    struct mat4 skybox_view = mat4_remove_translation(camera_view(&camera));

    // Change depth function to draw skybox behind all other objects
    glDepthFunc(GL_LEQUAL);

    glUseProgram(skybox_shader.id);
    shader_set_mat4(&skybox_shader, "u_view", &skybox_view);
    shader_set_mat4(&skybox_shader, "u_projection", &projection);

    glBindVertexArray(skybox_vao.id);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vao.vbo);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_map.id);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
}

void mesh_frame_begin(const struct mesh *mesh)
{
    render_frame_begin(&main_vao, mesh->texture, &main_shader);
    current_mesh = mesh;

    struct mat4 view = camera_view(&camera);
    shader_set_mat4(&main_shader, "u_view", &view);
    shader_set_mat4(&main_shader, "u_projection", &projection);
}

void mesh_frame_end()
{
    render_frame_end();
    current_mesh = NULL;
}

void push_mesh(const struct transform *transform)
{
    assert(current_mesh);
    assert(current_frame.vbo_count + current_mesh->vertex_count <= MAX_VERTICES);
    assert(current_frame.ebo_count + current_mesh->index_count <= MAX_INDICES);

    struct mat4 model = transform_matrix(transform);

    struct vert_textured *vmap = current_frame.vbo_map;
    for (size_t i = 0; i < current_mesh->vertex_count; i++)
    {
        struct vert_textured *vert = current_mesh->vertices + i;
        vmap->pos = mat4_vmul(model, vert->pos);
        vmap->uvx = vert->uvx;
        vmap->uvy = vert->uvy;
        vmap++;
    }

    for (size_t i = 0; i < current_mesh->index_count; i++)
    {
        current_frame.ebo_map[i] = current_frame.vbo_count + current_mesh->indices[i];
    }

    current_frame.vbo_count += current_mesh->vertex_count;
    current_frame.ebo_count += current_mesh->index_count;
    current_frame.vbo_map = vmap;
    current_frame.ebo_map += current_mesh->index_count;
}

void text_frame_begin()
{
    render_frame_begin(&text_vao, &font.bitmap, &text_shader);
}

void text_frame_end()
{
    render_frame_end();
}

void push_text(const char *str, float x, float y, float size)
{
    float curx = x;
    float cury = y;

    struct vert_text *vmap = current_frame.vbo_map;

    uint8_t c;
    while ((c = *str))
    {
        if (c == '\n')
        {
            curx = x;
            // FIXME: Why is the spacing so small?
            // Adding extra spacing as a quick fix
            cury -= (font.lheight + 10.0f) * size;
            str++;
            continue;
        }

        assert(c >= font.start_id && c < font.start_id + font.num_char);
        assert(current_frame.vbo_count + 4 <= MAX_VERTICES);
        assert(current_frame.ebo_count + 6 <= MAX_INDICES);

        struct fchar *fchar = font.chars + c - font.start_id;
        float x0 = curx + fchar->xoff * size;
        float x1 = x0 + fchar->w * size;
        float y0 = cury - (fchar->h + fchar->yoff) * size;
        float y1 = y0 + fchar->h * size;

        float uvx0 = fchar->x / (float)font.bitmap.width;
        float uvx1 = (fchar->x + fchar->w) / (float)font.bitmap.width;
        float uvy0 = (fchar->y + fchar->h) / (float)font.bitmap.height;
        float uvy1 = fchar->y / (float)font.bitmap.height;

        vmap->x = x0;
        vmap->y = y1;
        vmap->uvx = uvx0;
        vmap->uvy = uvy1;
        vmap++;

        vmap->x = x0;
        vmap->y = y0;
        vmap->uvx = uvx0;
        vmap->uvy = uvy0;
        vmap++;

        vmap->x = x1;
        vmap->y = y0;
        vmap->uvx = uvx1;
        vmap->uvy = uvy0;
        vmap++;

        vmap->x = x1;
        vmap->y = y1;
        vmap->uvx = uvx1;
        vmap->uvy = uvy1;
        vmap++;

        current_frame.ebo_map[0] = current_frame.vbo_count;
        current_frame.ebo_map[1] = current_frame.vbo_count + 1;
        current_frame.ebo_map[2] = current_frame.vbo_count + 2;
        current_frame.ebo_map[3] = current_frame.vbo_count;
        current_frame.ebo_map[4] = current_frame.vbo_count + 2;
        current_frame.ebo_map[5] = current_frame.vbo_count + 3;

        current_frame.vbo_count += 4;
        current_frame.ebo_map += 6;
        current_frame.ebo_count += 6;

        curx += fchar->adv * size;
        str++;
    }

    current_frame.vbo_map = vmap;
}

void untextured_frame_begin()
{
    render_frame_begin(&untextured_vao, NULL, &untextured_shader);

    struct mat4 view = camera_view(&camera);
    shader_set_mat4(&untextured_shader, "u_view", &view);
    shader_set_mat4(&untextured_shader, "u_projection", &projection);
}

void untextured_frame_end()
{
    render_frame_end();
}

void push_quad(struct vec3 a, struct vec3 b, struct vec3 c, struct vec3 d, struct color col)
{
    assert(current_frame.vbo_count + 4 <= MAX_VERTICES);
    assert(current_frame.ebo_count + 6 <= MAX_INDICES);

    struct vert_untextured *vmap = current_frame.vbo_map;

    vmap->pos = a;
    vmap->col = col;
    vmap++;

    vmap->pos = b;
    vmap->col = col;
    vmap++;

    vmap->pos = c;
    vmap->col = col;
    vmap++;

    vmap->pos = d;
    vmap->col = col;
    vmap++;

    current_frame.ebo_map[0] = current_frame.vbo_count;
    current_frame.ebo_map[1] = current_frame.vbo_count + 1;
    current_frame.ebo_map[2] = current_frame.vbo_count + 2;
    current_frame.ebo_map[3] = current_frame.vbo_count;
    current_frame.ebo_map[4] = current_frame.vbo_count + 2;
    current_frame.ebo_map[5] = current_frame.vbo_count + 3;

    current_frame.vbo_count += 4;
    current_frame.ebo_map += 6;
    current_frame.ebo_count += 6;
    current_frame.vbo_map = vmap;
}

void push_volume(struct vec3 p0, struct vec3 p1, struct vec3 p2, struct vec3 p3,
        struct vec3 p4, struct vec3 p5, struct vec3 p6, struct vec3 p7, struct color col)
{
    // Near
    push_quad(p0, p1, p2, p3, col);

    // Far
    push_quad(p7, p6, p5, p4, col);

    // Top
    push_quad(p4, p0, p3, p7, col);

    // Bottom
    push_quad(p1, p5, p6, p2, col);

    // Left
    push_quad(p4, p5, p1, p0, col);

    // Right
    push_quad(p3, p2, p6, p7, col);
}

void push_cube(struct vec3 center, struct vec3 size, struct color col)
{
    struct vec3 shalf = vec3_div(size, 2.0f);
    float left = center.x - shalf.x;
    float right = center.x + shalf.x;
    float top = center.y + shalf.y;
    float bot = center.y - shalf.y;
    float near = center.z + shalf.z;
    float far = center.z - shalf.z;

    struct vec3 p0 = vec3_create(left, top, near);
    struct vec3 p1 = vec3_create(left, bot, near);
    struct vec3 p2 = vec3_create(right, bot, near);
    struct vec3 p3 = vec3_create(right, top, near);

    struct vec3 p4 = vec3_create(left, top, far);
    struct vec3 p5 = vec3_create(left, bot, far);
    struct vec3 p6 = vec3_create(right, bot, far);
    struct vec3 p7 = vec3_create(right, top, far);

    push_volume(p0, p1, p2, p3, p4, p5, p6, p7, col);
}

void push_line(struct vec3 a, struct vec3 b, float thickness, struct color col)
{
    float thalf = thickness / 2.0f;

    // FIXME: Determine dx, dy and dz based on line direction
    struct vec3 dx = vec3_mul(VEC3_RIGHT, thalf);
    struct vec3 dy = vec3_mul(VEC3_UP, thalf);
    struct vec3 dz = VEC3_ZERO;

    struct vec3 p0 = vec3_add(vec3_add(vec3_sub(a, dx), dy), dz);
    struct vec3 p1 = vec3_add(vec3_sub(vec3_sub(a, dx), dy), dz);
    struct vec3 p2 = vec3_add(vec3_sub(vec3_add(a, dx), dy), dz);
    struct vec3 p3 = vec3_add(vec3_add(vec3_add(a, dx), dy), dz);

    struct vec3 p4 = vec3_sub(vec3_add(vec3_sub(b, dx), dy), dz);
    struct vec3 p5 = vec3_sub(vec3_sub(vec3_sub(b, dx), dy), dz);
    struct vec3 p6 = vec3_sub(vec3_sub(vec3_add(b, dx), dy), dz);
    struct vec3 p7 = vec3_sub(vec3_add(vec3_add(b, dx), dy), dz);

    push_volume(p0, p1, p2, p3, p4, p5, p6, p7, col);
}

void push_volume_outline(struct vec3 p0, struct vec3 p1, struct vec3 p2, struct vec3 p3,
        struct vec3 p4, struct vec3 p5, struct vec3 p6, struct vec3 p7, float thickness,
        struct color col)
{
    push_line(p0, p1, thickness, col);
    push_line(p1, p2, thickness, col);
    push_line(p2, p3, thickness, col);
    push_line(p3, p0, thickness, col);

    push_line(p4, p5, thickness, col);
    push_line(p5, p6, thickness, col);
    push_line(p6, p7, thickness, col);
    push_line(p7, p4, thickness, col);

    push_line(p0, p4, thickness, col);
    push_line(p3, p7, thickness, col);

    push_line(p1, p5, thickness, col);
    push_line(p2, p6, thickness, col);
}

struct camera *get_camera()
{
    return &camera;
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
