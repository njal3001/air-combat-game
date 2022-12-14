#include "asset.h"
#include <stdbool.h>
#include <stdio.h>
#include "string.h"
#include "platform.h"
#include "hashmap.h"
#include "log.h"
#include "mesh.h"
#include "font.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#define MAX_ASSET_PATH 512

char asset_path[MAX_ASSET_PATH];
size_t root_length;

struct texture textures[ASSET_TEXTURE_END];
struct mesh meshes[ASSET_MESH_END];
struct shader shaders[ASSET_SHADER_END];
struct font fonts[ASSET_FONT_END];
char *audio_paths[ASSET_AUDIO_END];

static void load_asset_path(enum asset_type type, const char *name)
{
    // Clear previous asset name
    memset(asset_path + root_length, 0, MAX_ASSET_PATH - root_length);

    // Select directory
    switch (type)
    {
        case ASSET_TYPE_SHADER:
            strcat(asset_path, "src/shaders/");
            break;
        default:
            strcat(asset_path, "assets/");
            break;
    }

    // Append name
    strncat(asset_path, name, MAX_ASSET_PATH - root_length);
}

static bool load_texture(enum asset_texture handle, const char *name)
{
    struct texture *tex = textures + handle;

    load_asset_path(ASSET_TYPE_TEXTURE, name);

    struct image img;
    img.data = stbi_load(asset_path, &img.width, &img.height,
            &img.channels, 0);
    if (!img.data)
    {
        log_err("Could not load texture %s", name);
        return false;
    }

    texture_init(tex, &img);

    stbi_image_free(img.data);
    return true;
}

static bool load_mesh(enum asset_mesh handle, const char *name)
{
    struct mesh *mesh = meshes + handle;

    load_asset_path(ASSET_TYPE_MESH, name);
    FILE *f = fopen(asset_path, "r");
    if (!f)
    {
        log_warn("Could not load mesh %s", name);
        return false;
    }

    enum read_state
    {
        read_header,
        read_vertices,
        read_faces,
    };
    enum read_state read_state = read_header;
    size_t vertices_read = 0;
    size_t indices_read = 0;

    size_t vertex_count;
    size_t index_count;

    char *line = NULL;
    size_t blength = 0;
    int read;

    while ((read = getline(&line, &blength, f)) != -1)
    {
        if (!read)
        {
            continue;
        }

        // Remove newline
        line[strcspn(line, "\n")] = '\0';

        if (read_state == read_header)
        {
            char *word = strtok(line, " ");
            if (strcmp(word, "element") == 0)
            {
                word = strtok(NULL, " ");
                char *scount = strtok(NULL, " ");

                if (strcmp(word, "vertex") == 0)
                {
                    vertex_count = strtol(scount, NULL, 10);
                }
                else if (strcmp(word, "face") == 0)
                {
                    index_count = strtol(scount, NULL, 10) * 3;
                }
            }
            else if (strcmp(word, "end_header") == 0)
            {
                read_state = read_vertices;
                mesh_init(mesh, vertex_count, index_count);
            }
        }
        else if (read_state == read_vertices)
        {
            struct vert_mesh *v = mesh->vertices + vertices_read;

            char *word = strtok(line, " ");
            v->pos.x = strtof(word, NULL);
            word = strtok(NULL, " ");
            v->pos.y = strtof(word, NULL);
            word = strtok(NULL, " ");
            v->pos.z = strtof(word, NULL);

            // Skip normals
            strtok(NULL, " ");
            strtok(NULL, " ");
            strtok(NULL, " ");

            word = strtok(NULL, " ");
            v->uvx = strtof(word, NULL);
            word = strtok(NULL, " ");
            v->uvy = strtof(word, NULL);

            vertices_read++;
            if (vertices_read == mesh->vertex_count)
            {
                read_state = read_faces;
            }
        }
        else if (read_state == read_faces)
        {
            strtok(line, " ");

            char *word = strtok(NULL, " ");
            mesh->indices[indices_read++] = strtol(word, NULL, 10);
            word = strtok(NULL, " ");
            mesh->indices[indices_read++] = strtol(word, NULL, 10);
            word = strtok(NULL, " ");
            mesh->indices[indices_read++] = strtol(word, NULL, 10);
        }
    }

    free(line);
    fclose(f);

    return true;
}

static bool load_shader(enum asset_shader handle, const char *vert_name,
        const char *frag_name)
{
    struct shader *shader = shaders + handle;

    load_asset_path(ASSET_TYPE_SHADER, vert_name);
    char *vert_str = read_file(asset_path);
    if (!vert_str)
    {
        log_err("Could not read vertex shader file: %s", vert_name);
        return false;
    }

    load_asset_path(ASSET_TYPE_SHADER, frag_name);
    char *frag_str = read_file(asset_path);
    if (!frag_str)
    {
        log_err("Could not read fragment shader file: %s", frag_name);
        free(vert_str);
        return false;
    }

    bool success = shader_init(shader, vert_str, frag_str);
    if (!success)
    {
        log_err("Failed to load shader (%s, %s)", vert_name, frag_name);
    }

    free(vert_str);
    free(frag_str);

    return success;
}

static bool load_font(enum asset_font handle, const char *name)
{
    struct font *font = fonts + handle;

    load_asset_path(ASSET_TYPE_FONT, name);
    FILE *f = fopen(asset_path, "r");
    if (!f)
    {
        log_err("Failed to load font %s. Could not open file %s", name,
                asset_path);
        return false;
    }

    size_t num_char;
    uint8_t lheight;
    struct image img;

    char *word;
    char *line = NULL;
    size_t blength = 0;
    int read;

    // Skip font name
    getline(&line, &blength, f);

    // Font size and line height
    getline(&line, &blength, f);
    strtok(line, " ");
    word = strtok(line, " ");
    lheight = strtol(word, NULL, 10);

    // Bitmap
    getline(&line, &blength, f);
    line[strcspn(line, "\n")] = '\0';
    load_asset_path(ASSET_TYPE_TEXTURE, line);

    img.data = stbi_load(asset_path, &img.width, &img.height,
            &img.channels, 0);
    if (!img.data)
    {
        log_warn("Failed to load font %s. Could not load bitmap %s", name,
                asset_path);
        free(line);
        return false;
    }

    // Number of characters
    getline(&line, &blength, f);
    num_char = strtol(line, NULL, 10);

    font_init(font, num_char, lheight, &img);
    stbi_image_free(img.data);

    // Character information
    while (getline(&line, &blength, f) != -1)
    {
        word = strtok(line, " ");
        size_t id = strtol(word, NULL, 10);
        if (!id)
        {
            break;
        }

        struct fchar c;
        c.x = strtol(strtok(NULL, " "), NULL, 10);
        c.y = strtol(strtok(NULL, " "), NULL, 10);
        c.w = strtol(strtok(NULL, " "), NULL, 10);
        c.h = strtol(strtok(NULL, " "), NULL, 10);
        c.xoff = strtol(strtok(NULL, " "), NULL, 10);
        c.yoff = strtol(strtok(NULL, " "), NULL, 10);
        c.adv = strtol(strtok(NULL, " "), NULL, 10);

        font_set_char(font, id, c);
    }

    free(line);

    return true;
}

static void load_audio_path(enum asset_audio handle, const char *name)
{
    load_asset_path(ASSET_TYPE_AUDIO, name);
    size_t n = strlen(asset_path) + 1;

    audio_paths[handle] = malloc(sizeof(char) * n);
    memcpy(audio_paths[handle], asset_path, n);
}

void assets_init()
{
    // Find root path
    get_exec_path(asset_path, MAX_ASSET_PATH);
    get_dir_path(asset_path, 1);
    root_length = strlen(asset_path);

    // Textures
    load_texture(ASSET_TEXTURE_METAL, "rusted_metal.jpg");

    // Meshes
    meshes[ASSET_MESH_PLAYER] = create_cube_mesh();
    meshes[ASSET_MESH_PLAYER].texture = get_texture(ASSET_TEXTURE_METAL);

    meshes[ASSET_MESH_ORB] = create_cube_mesh();
    meshes[ASSET_MESH_ORB].texture = get_texture(ASSET_TEXTURE_METAL);

    meshes[ASSET_MESH_WALL] = create_cube_mesh();
    meshes[ASSET_MESH_WALL].texture = get_texture(ASSET_TEXTURE_METAL);

    // Shaders
    load_shader(ASSET_SHADER_MESH, "mesh_instance.vert",
            "mesh_instance.frag");
    load_shader(ASSET_SHADER_UI, "ui.vert", "ui.frag");
    load_shader(ASSET_SHADER_UNTEXTURED, "untextured.vert",
            "untextured.frag");

    // Fonts
    load_font(ASSET_FONT_VCR, "vcr_osd_mono_regular_48.sfl");

    // Audio
    load_audio_path(ASSET_AUDIO_SONG, "outthere.wav");
}

void assets_free()
{
    for (int i = 0; i < ASSET_TEXTURE_END; i++)
    {
        texture_free(textures + i);
    }
    for (int i = 0; i < ASSET_MESH_END; i++)
    {
        mesh_free(meshes + i);
    }
    for (int i = 0; i < ASSET_SHADER_END; i++)
    {
        shader_free(shaders + i);
    }
    for (int i = 0; i < ASSET_FONT_END; i++)
    {
        font_free(fonts + i);
    }
    for (int i = 0; i < ASSET_AUDIO_END; i++)
    {
        free(audio_paths[i]);
    }
}

struct texture *get_texture(enum asset_texture handle)
{
    return textures + handle;
}

struct mesh *get_mesh(enum asset_mesh handle)
{
    return meshes + handle;
}

struct shader *get_shader(enum asset_shader handle)
{
    return shaders + handle;
}

struct font *get_font(enum asset_font handle)
{
    return fonts + handle;
}

const char *get_audio_path(enum asset_audio handle)
{
    return audio_paths[handle];
}
