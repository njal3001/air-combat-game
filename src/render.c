#include "render.h"
#include <assert.h>
#include <math.h>
#include "shader.h"
#include "assets.h"
#include "vertex.h"
#include "texture.h"
#include "log.h"

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#else
#    define APIENTRY
#endif

#define NEAR 0.1f
#define FAR 10000.0f
#define FOV (M_PI / 4.0f)
#define ASPECT_RATIO (1920.0f / 1080.0f)

#define MAX_MESH_VERTICES 10000
#define MAX_MESH_INDICES 15000
#define MAX_MESH_INSTANCES 30000

#define MAX_UI_VERTICES 1000
#define MAX_UI_INDICES 2000

#define MAX_UNTEXTURED_VERTICES 30000000
#define MAX_UNTEXTURED_INDICES 50000000

#define SKYBOX_FOG_AMOUNT 0.8f
#define FOG_START 3500.0f
#define FOG_END 5000.0f

struct vert_ui
{
    float x, y;
    float uvx, uvy;
};

struct vert_untextured
{
    struct vec3 pos;
    struct color col;
};

const float skybox_vertices[] =
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

struct mat4 projection;
struct camera camera;

struct vao mesh_vao;
struct ebo mesh_ebo;
struct vbo mesh_vbo;
struct vbo mesh_model_vbo;
struct shader mesh_instancing_shader;
const struct mesh *instance_mesh;
struct mat4 instancing_models[MAX_MESH_INSTANCES];
size_t instance_count;

struct vao skybox_vao;
struct vbo skybox_vbo;
struct shader skybox_shader;
struct cubemap skybox_map;

struct vao ui_vao;
struct vbo ui_vbo;
struct ebo ui_ebo;
struct shader ui_shader;
struct font font;
struct vert_ui ui_vertices[MAX_UI_VERTICES];
GLuint ui_indices[MAX_UI_INDICES];
size_t ui_vert_count;
size_t ui_index_count;

struct vao untextured_vao;
struct vbo untextured_vbo;
struct ebo untextured_ebo;
struct shader untextured_shader;
struct vert_untextured untextured_vertices[MAX_UNTEXTURED_VERTICES];
GLuint untextured_indices[MAX_UNTEXTURED_INDICES];
size_t untextured_vert_count;
size_t untextured_index_count;

struct color fog_color;

static void APIENTRY gl_message_callback(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message,
                                  const void *user_param);

static void on_window_size_changed(GLFWwindow *window, int width, int height);

bool render_init(GLFWwindow *window)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

    projection = mat4_perspective(FOV, ASPECT_RATIO, NEAR, FAR);
    camera.transform = transform_create(VEC3_ZERO);
    fog_color = color_create(0, 0, 0, 255);

    struct vert_attrib pos_attrib =
    {
        .type = VTYPE_FLOAT3,
        .normalized = false,
        .divisor = 0,
    };
    struct vert_attrib uv_attrib =
    {
        .type = VTYPE_FLOAT2,
        .normalized = false,
        .divisor = 0,
    };
    struct vert_attrib model_attrib =
    {
        .type = VTYPE_FLOAT16,
        .normalized = false,
        .divisor = 1,
    };
    struct vert_attrib color_attrib =
    {
        .type = VTYPE_UBYTE4,
        .normalized = true,
        .divisor = 0,
    };
    struct vert_attrib ui_attrib =
    {
        .type = VTYPE_FLOAT4,
        .normalized = false,
        .divisor = 0,
    };

    // Mesh rendering setup
    vao_init(&mesh_vao);
    vao_bind(&mesh_vao);

    ebo_init(&mesh_ebo, MAX_MESH_INDICES, NULL, BUFFER_DYNAMIC);
    vbo_init(&mesh_vbo, MAX_MESH_VERTICES * sizeof(struct vert_mesh),
            NULL, BUFFER_DYNAMIC);
    vbo_init(&mesh_model_vbo, MAX_MESH_INSTANCES * sizeof(struct mat4),
            NULL, BUFFER_DYNAMIC);

    vao_set_ebo(&mesh_vao, &mesh_ebo);
    vao_add_vbo(&mesh_vao, &mesh_vbo, 2, pos_attrib, uv_attrib);
    vao_add_vbo(&mesh_vao, &mesh_model_vbo, 1, model_attrib);

    instance_mesh = NULL;
    instance_count = 0;

    load_shader(&mesh_instancing_shader, "mesh_instance.vert",
            "mesh_instance.frag");
    glUseProgram(mesh_instancing_shader.id);
    shader_set_int(&mesh_instancing_shader, "u_sampler", 0);
    shader_set_color(&mesh_instancing_shader, "u_fog_color", fog_color);
    shader_set_float(&mesh_instancing_shader, "u_fog_start", FOG_START);
    shader_set_float(&mesh_instancing_shader, "u_fog_end", FOG_END);

    // Skybox rendering setup
    vao_init(&skybox_vao);
    vao_bind(&skybox_vao);
    vbo_init(&skybox_vbo, sizeof(skybox_vertices), skybox_vertices,
            BUFFER_STATIC);
    vao_add_vbo(&skybox_vao, &skybox_vbo, 1, pos_attrib);

    load_shader(&skybox_shader, "skybox.vert", "skybox.frag");
    glUseProgram(skybox_shader.id);
    shader_set_int(&skybox_shader, "u_skybox", 0);
    shader_set_color(&skybox_shader, "u_fog_color", fog_color);
    shader_set_float(&skybox_shader, "u_fog_amount", SKYBOX_FOG_AMOUNT);

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

    // UI rendering setup
    vao_init(&ui_vao);
    vao_bind(&ui_vao);
    vbo_init(&ui_vbo, MAX_UI_VERTICES * sizeof(struct vert_ui), NULL,
            BUFFER_DYNAMIC);
    ebo_init(&ui_ebo, MAX_UI_INDICES, NULL, BUFFER_DYNAMIC);

    vao_set_ebo(&ui_vao, &ui_ebo);
    vao_add_vbo(&ui_vao, &ui_vbo, 1, ui_attrib);

    load_shader(&ui_shader, "text.vert", "text.frag");
    glUseProgram(ui_shader.id);
    shader_set_int(&ui_shader, "u_bitmap", 0);
    struct mat4 ui_proj = mat4_ortho(0.0f, 1920.0f, 0.0f, 1080.0f, 0.0f, 1.0f);
    shader_set_mat4(&ui_shader, "u_projection", &ui_proj);

    load_font(&font, "vcr_osd_mono_regular_48.sfl");

    // Untextured rendering setup
    vao_init(&untextured_vao);
    vao_bind(&untextured_vao);
    vbo_init(&untextured_vbo, MAX_UNTEXTURED_VERTICES, NULL, BUFFER_DYNAMIC);
    ebo_init(&untextured_ebo, MAX_UNTEXTURED_INDICES, NULL, BUFFER_DYNAMIC);

    vao_set_ebo(&untextured_vao, &untextured_ebo);
    vao_add_vbo(&untextured_vao, &untextured_vbo, 2,
            pos_attrib, color_attrib);

    load_shader(&untextured_shader, "untextured.vert", "untextured.frag");

    return true;
}

void render_shutdown()
{
    ebo_free(&mesh_ebo);
    vbo_free(&mesh_vbo);
    vbo_free(&mesh_model_vbo);
    vao_free(&mesh_vao);
    shader_free(&mesh_instancing_shader);

    ebo_free(&ui_ebo);
    vbo_free(&ui_vbo);
    vao_free(&ui_vao);
    shader_free(&ui_shader);

    ebo_free(&untextured_ebo);
    vbo_free(&untextured_vbo);
    vao_free(&untextured_vao);
    shader_free(&untextured_shader);

    vbo_free(&skybox_vbo);
    vao_free(&skybox_vao);
    shader_free(&skybox_shader);

    cubemap_free(&skybox_map);
    font_free(&font);
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
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_map.id);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
}

void mesh_instancing_begin(const struct mesh *mesh)
{
    assert(!instance_count);
    assert(!instance_mesh);
    assert(mesh->texture);

    instance_mesh = mesh;

    vao_bind(&mesh_vao);

    ebo_set_data(&mesh_ebo, mesh->index_count, mesh->indices);
    vbo_set_data(&mesh_vbo, mesh->vertex_count * sizeof(struct vert_mesh),
            mesh->vertices);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh->texture->id);

    glUseProgram(mesh_instancing_shader.id);
    struct mat4 view = camera_view(&camera);
    shader_set_mat4(&mesh_instancing_shader, "u_view", &view);
    shader_set_mat4(&mesh_instancing_shader, "u_projection", &projection);
    shader_set_vec3(&mesh_instancing_shader, "u_view_pos", camera.transform.pos);
}

void push_mesh_transform(const struct transform *transform)
{
    assert(instance_mesh);
    assert(instance_count < MAX_MESH_INSTANCES);

    instancing_models[instance_count] = transform_matrix(transform);
    instance_count++;
}

void mesh_instancing_end()
{
    assert(instance_mesh);

    if (instance_count)
    {
        vbo_set_data(&mesh_model_vbo, instance_count * sizeof(struct mat4),
                instancing_models);
        glDrawElementsInstanced(GL_TRIANGLES, instance_mesh->index_count,
                GL_UNSIGNED_INT, 0, instance_count);
    }

    instance_count = 0;
    instance_mesh = NULL;
}

void ui_begin()
{
    vao_bind(&ui_vao);
    glUseProgram(ui_shader.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font.bitmap.id);
}

void ui_end()
{
    if (ui_vert_count && ui_index_count)
    {
        vbo_set_data(&ui_vbo, ui_vert_count * sizeof(struct vert_ui),
                ui_vertices);
        ebo_set_data(&ui_ebo, ui_index_count, ui_indices);
        glDrawElements(GL_TRIANGLES, ui_index_count,
                GL_UNSIGNED_INT, (void*)NULL);
    }

    ui_vert_count = 0;
    ui_index_count = 0;
}

void push_text(const char *str, float x, float y, float size)
{
    float curx = x;
    float cury = y;

    struct vert_ui *vert = ui_vertices + ui_vert_count;

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
        assert(ui_vert_count + 4 <= MAX_UI_VERTICES);
        assert(ui_index_count + 6 <= MAX_UI_INDICES);

        struct fchar *fchar = font.chars + c - font.start_id;
        float x0 = curx + fchar->xoff * size;
        float x1 = x0 + fchar->w * size;
        float y0 = cury - (fchar->h + fchar->yoff) * size;
        float y1 = y0 + fchar->h * size;

        float uvx0 = fchar->x / (float)font.bitmap.width;
        float uvx1 = (fchar->x + fchar->w) / (float)font.bitmap.width;
        float uvy0 = (fchar->y + fchar->h) / (float)font.bitmap.height;
        float uvy1 = fchar->y / (float)font.bitmap.height;

        vert->x = x0;
        vert->y = y1;
        vert->uvx = uvx0;
        vert->uvy = uvy1;
        vert++;

        vert->x = x0;
        vert->y = y0;
        vert->uvx = uvx0;
        vert->uvy = uvy0;
        vert++;

        vert->x = x1;
        vert->y = y0;
        vert->uvx = uvx1;
        vert->uvy = uvy0;
        vert++;

        vert->x = x1;
        vert->y = y1;
        vert->uvx = uvx1;
        vert->uvy = uvy1;
        vert++;

        GLuint *index = ui_indices + ui_index_count;
        index[0] = ui_vert_count;
        index[1] = ui_vert_count + 1;
        index[2] = ui_vert_count + 2;
        index[3] = ui_vert_count;
        index[4] = ui_vert_count + 2;
        index[5] = ui_vert_count + 3;

        ui_vert_count += 4;
        ui_index_count += 6;

        curx += fchar->adv * size;
        str++;
    }
}

void untextured_frame_begin()
{
    vao_bind(&untextured_vao);
    glUseProgram(untextured_shader.id);
    struct mat4 view = camera_view(&camera);
    shader_set_mat4(&untextured_shader, "u_view", &view);
    shader_set_mat4(&untextured_shader, "u_projection", &projection);
}

void untextured_frame_end()
{
    if (untextured_vert_count && untextured_index_count)
    {
        vbo_set_data(&untextured_vbo,
                untextured_vert_count * sizeof(struct vert_untextured),
                untextured_vertices);

        ebo_set_data(&untextured_ebo, untextured_index_count,
                untextured_indices);

        glDrawElements(GL_TRIANGLES, untextured_index_count,
                GL_UNSIGNED_INT, (void*)NULL);
    }

    untextured_vert_count = 0;
    untextured_index_count = 0;
}

void push_quad(struct vec3 a, struct vec3 b, struct vec3 c,
        struct vec3 d, struct color col)
{
    assert(untextured_vert_count + 4 <= MAX_UNTEXTURED_VERTICES);
    assert(untextured_index_count + 6 <= MAX_UNTEXTURED_INDICES);

    struct vert_untextured *vert = untextured_vertices + untextured_vert_count;

    vert->pos = a;
    vert->col = col;
    vert++;

    vert->pos = b;
    vert->col = col;
    vert++;

    vert->pos = c;
    vert->col = col;
    vert++;

    vert->pos = d;
    vert->col = col;
    vert++;

    GLuint *index = untextured_indices + untextured_index_count;

    index[0] = untextured_vert_count;
    index[1] = untextured_vert_count + 1;
    index[2] = untextured_vert_count + 2;
    index[3] = untextured_vert_count;
    index[4] = untextured_vert_count + 2;
    index[5] = untextured_vert_count + 3;

    untextured_vert_count += 4;
    untextured_index_count += 6;
}

void push_volume(struct vec3 p0, struct vec3 p1, struct vec3 p2, struct vec3 p3,
        struct vec3 p4, struct vec3 p5, struct vec3 p6, struct vec3 p7,
        struct color col)
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

void push_volume_outline(struct vec3 p0, struct vec3 p1, struct vec3 p2,
        struct vec3 p3, struct vec3 p4, struct vec3 p5, struct vec3 p6,
        struct vec3 p7, float thickness, struct color col)
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
        log_info("GL (%s:%s) %s", type_name, severity_name, message);
    }
    else if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        log_info("GL (%s:%s) %s\n", type_name, severity_name, message);
    }
    else
    {
        log_info("GL (%s) %s", type_name, message);
    }
}
