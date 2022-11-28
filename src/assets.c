#include "assets.h"
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

struct hashmap *textures;
struct hashmap *meshes;

char asset_path[MAX_ASSET_PATH];
size_t root_length;

static bool read_polygon(const char *name, struct mesh *mesh);

static void load_asset_path(enum asset_type type, const char *name)
{
    // Clear previous asset name
    memset(asset_path + root_length, 0, MAX_ASSET_PATH - root_length);

    // Select directory
    switch (type)
    {
        case ASSET_SHADER:
            strcat(asset_path, "src/shaders/");
            break;
        case ASSET_OTHER:
            strcat(asset_path, "assets/");
            break;
    }

    // Append name
    strncat(asset_path, name, MAX_ASSET_PATH - root_length);
}

void assets_init()
{
    // Find root path
    get_exec_path(asset_path, MAX_ASSET_PATH);
    get_dir_path(asset_path, 1);
    root_length = strlen(asset_path);

    textures = hashmap_new();
    meshes = hashmap_new();
}

void assets_free()
{
    size_t num_textures = hashmap_size(textures);
    if (num_textures)
    {
        struct texture **t = malloc(num_textures * sizeof(void*));
        hashmap_values(textures, (void**)t);
        for (size_t i = 0; i < num_textures; i++)
        {
            texture_free(t[i]);
            free(t[i]);
        }

        free(t);
    }

    size_t num_meshes = hashmap_size(meshes);
    if (num_meshes)
    {
        struct mesh **m = malloc(num_meshes * sizeof(void*));
        hashmap_values(meshes, (void**)m);
        for (size_t i = 0; i < num_meshes; i++)
        {
            mesh_free(m[i]);
            free(m[i]);
        }

        free(m);
    }

    hashmap_free(meshes);
    hashmap_free(textures);
}

const char *get_asset_path(enum asset_type type, const char *name)
{
    load_asset_path(type, name);
    return asset_path;
}

const struct texture *get_texture(const char *name)
{
    struct texture *cached_tex = hashmap_get(textures, name);
    if (cached_tex)
    {
        return cached_tex;
    }

    load_asset_path(ASSET_OTHER, name);

    struct image img;
    img.data = stbi_load(asset_path, &img.width, &img.height, &img.channels, 0);

    if (!img.data)
    {
        return NULL;
    }

    struct texture *tex = malloc(sizeof(struct texture));
    texture_init(tex, &img);

    stbi_image_free(img.data);

    hashmap_put(textures, name, tex);
    return tex;
}

const struct mesh *get_mesh(const char *name)
{
    struct mesh *cached_mesh = hashmap_get(meshes, name);
    if (cached_mesh)
    {
        return cached_mesh;
    }

    log_info("Loading mesh: %s", name);

    load_asset_path(ASSET_OTHER, name);

    FILE *f_mesh = fopen(asset_path, "r");
    if (!f_mesh)
    {
        log_warn("Could not open file %s", asset_path);
        return NULL;
    }

    struct mesh *mesh = malloc(sizeof(struct mesh));
    mesh->indices = NULL;
    mesh->vertices = NULL;
    mesh->texture = NULL;

    char *line = NULL;
    size_t blength = 0;
    int read;
    char *word;

    while ((read = getline(&line, &blength, f_mesh)) != -1)
    {
        if (!read)
        {
            continue;
        }

        line[strcspn(line, "\n")] = '\0';
        word = strtok(line, " ");

        if (strcmp(word, "model") == 0)
        {
            word = strtok(NULL, " ");
            bool model_success = read_polygon(word, mesh);
            if (!model_success)
            {
                log_warn("Could not load model");
                free(line);
                fclose(f_mesh);
                mesh_free(mesh);
                return NULL;
            }
        }
        else if (strcmp(word, "texture") == 0)
        {
            word = strtok(NULL, " ");
            mesh->texture = get_texture(word);
            if (!mesh->texture)
            {
                log_warn("Could not load texture");
                free(line);
                fclose(f_mesh);
                mesh_free(mesh);
                return NULL;
            }
        }
    }

    free(line);
    fclose(f_mesh);

    hashmap_put(meshes, name, mesh);
    return mesh;
}

bool read_polygon(const char *name, struct mesh *mesh)
{
    log_info("Loading polygon: %s", name);

    load_asset_path(ASSET_OTHER, name);
    FILE *f = fopen(asset_path, "r");
    if (!f)
    {
        log_warn("Could not open file %s", asset_path);
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


bool load_shader(struct shader *shader, const char *vert_name,
        const char *frag_name)
{
    load_asset_path(ASSET_SHADER, vert_name);
    char *vert_str = read_file(asset_path);
    if (!vert_str)
    {
        return false;
    }

    load_asset_path(ASSET_SHADER, frag_name);
    char *frag_str = read_file(asset_path);
    if (!frag_str)
    {
        free(vert_str);
        return false;
    }

    bool success = shader_init(shader, vert_str, frag_str);

    free(vert_str);
    free(frag_str);

    return success;
}

bool load_cubemap(struct cubemap *cmap, const char *const *face_names)
{
    struct image faces[6];

    for (size_t i = 0; i < 6; i++)
    {
        load_asset_path(ASSET_OTHER, face_names[i]);
        struct image *face = faces + i;
        face->data = stbi_load(asset_path, &face->width, &face->height,
                &face->channels, 0);
        if (!face->data)
        {
            log_warn("Could not load cubemap face %s", face_names[i]);
            for (size_t j = 0; j < i; j++)
            {
                stbi_image_free(faces[j].data);
            }

            return false;
        }
    }

    cubemap_init(cmap, faces);

    for (size_t i = 0; i < 6; i++)
    {
        stbi_image_free(faces[i].data);
    }

    return true;
}

bool load_font(struct font *font, const char *name)
{
    log_info("Loading font: %s", name);

    load_asset_path(ASSET_OTHER, name);
    FILE *f = fopen(asset_path, "r");
    if (!f)
    {
        log_warn("Could not open file %s", asset_path);
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
    load_asset_path(ASSET_OTHER, line);

    img.data = stbi_load(asset_path, &img.width, &img.height,
            &img.channels, 0);
    if (!img.data)
    {
        log_warn("Could not open file %s", asset_path);
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

