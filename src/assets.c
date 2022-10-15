#include "assets.h"
#include "platform.h"
#include "hashmap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

#define MAX_ASSET_PATH 512

struct hashmap *textures;
struct hashmap *meshes;

size_t asset_base_length;
char asset_path[MAX_ASSET_PATH];

static void texture_free(struct texture *texture);
static void mesh_free(struct mesh *mesh);

static void load_asset_path(const char *name)
{
    // Clear previous asset name
    memset(asset_path + asset_base_length, 0, MAX_ASSET_PATH - asset_base_length);

    // Append name to base asset path
    strncat(asset_path, name, MAX_ASSET_PATH - asset_base_length);
}

void assets_init()
{
    get_exec_path(asset_path, MAX_ASSET_PATH);
    get_dir_path(asset_path, 1);
    strcat(asset_path, "assets/");

    asset_base_length = strlen(asset_path);

    textures = hashmap_new();
    meshes = hashmap_new();
}

void assets_free()
{
    size_t num_textures = hashmap_size(textures);
    struct texture *t = malloc(num_textures * sizeof(void*));
    for (size_t i = 0; i < num_textures; i++)
    {
        texture_free(t + i);
    }

    free(t);

    size_t num_meshes = hashmap_size(meshes);
    struct texture *m = malloc(num_meshes * sizeof(void*));
    for (size_t i = 0; i < num_meshes; i++)
    {
        texture_free(m + i);
    }

    free(m);

    hashmap_free(meshes);
    hashmap_free(textures);
}

const struct texture *get_texture(const char *name)
{
    struct texture *cached_tex = hashmap_get(textures, name);
    if (cached_tex)
    {
        return cached_tex;
    }

    load_asset_path(name);

    int width, height, nchannels;
    unsigned char *data = stbi_load(asset_path, &width, &height, &nchannels, 0);

    if (!data)
    {
        return NULL;
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);

    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    struct texture *tex = malloc(sizeof(struct texture));
    tex->id = tex_id;
    tex->width = width;
    tex->height = height;

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

    load_asset_path(name);

    FILE *f = fopen(asset_path, "r");
    if (!f)
    {
        return NULL;
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

    struct mesh *mesh = malloc(sizeof(struct mesh));
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
                    mesh->vertex_count = strtol(scount, NULL, 10);
                }
                else if (strcmp(word, "face") == 0)
                {
                    mesh->index_count = strtol(scount, NULL, 10) * 3;
                }
            }
            else if (strcmp(word, "comment") == 0)
            {
                word = strtok(NULL, " ");
                if (strcmp(word, "texture") == 0)
                {
                    word = strtok(NULL, " ");
                    mesh->texture = get_texture(word);

                    assert(mesh->texture);
                }
            }
            else if (strcmp(word, "end_header") == 0)
            {
                read_state = read_vertices;
                mesh->vertices = malloc(mesh->vertex_count * sizeof(struct vertex));
                mesh->indices = malloc(mesh->index_count * sizeof(GLushort));
            }
        }
        else if (read_state == read_vertices)
        {
            struct vertex *v = mesh->vertices + vertices_read;

            char *word = strtok(line, " ");
            v->pos.x = strtof(word, NULL);
            word = strtok(NULL, " ");
            v->pos.y = strtof(word, NULL);
            word = strtok(NULL, " ");
            v->pos.z = strtof(word, NULL);

            word = strtok(NULL, " ");
            v->norm.x = strtof(word, NULL);
            word = strtok(NULL, " ");
            v->norm.y = strtof(word, NULL);
            word = strtok(NULL, " ");
            v->norm.z = strtof(word, NULL);

            word = strtok(NULL, " ");
            v->uvx = strtof(word, NULL);
            word = strtok(NULL, " ");
            v->uvy = strtof(word, NULL);

            v->col = COLOR_WHITE;

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

    hashmap_put(meshes, name, mesh);
    return mesh;
}

void texture_free(struct texture *texture)
{
    glDeleteTextures(1, &texture->id);
}

void mesh_free(struct mesh *mesh)
{
    free(mesh->indices);
    free(mesh->vertices);
}
