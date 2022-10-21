#include "assets.h"
#include <stdbool.h>
#include "platform.h"
#include "hashmap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

#define MAX_ASSET_PATH 512

enum asset_type
{
    ASSET_SHADER,
    ASSET_OTHER,
};

struct hashmap *textures;
struct hashmap *meshes;

char asset_path[MAX_ASSET_PATH];
size_t root_length;

struct mesh quad_mesh;

static bool read_polygon(const char *name, struct mesh *mesh);
static void texture_free(struct texture *texture);
static void mesh_init(struct mesh *mesh, size_t vertex_count, size_t index_count);
static void mesh_free(struct mesh *mesh);

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

    mesh_init(&quad_mesh, 4, 6);

    quad_mesh.vertices[0].pos = vec3_create(-0.5f, 0.0f, 0.5f);
    quad_mesh.vertices[0].norm = VEC3_UP;
    quad_mesh.vertices[0].uvx = 0.0f;
    quad_mesh.vertices[0].uvy = 1.0f;

    quad_mesh.vertices[1].pos = vec3_create(-0.5f, 0.0f, -0.5f);
    quad_mesh.vertices[1].norm = VEC3_UP;
    quad_mesh.vertices[1].uvx = 0.0f;
    quad_mesh.vertices[1].uvy = 0.0f;

    quad_mesh.vertices[2].pos = vec3_create(0.5f, 0.0f, -0.5f);
    quad_mesh.vertices[2].norm = VEC3_UP;
    quad_mesh.vertices[2].uvx = 1.0f;
    quad_mesh.vertices[2].uvy = 0.0f;

    quad_mesh.vertices[3].pos = vec3_create(0.5f, 0.0f, 0.5f);
    quad_mesh.vertices[3].norm = VEC3_UP;
    quad_mesh.vertices[3].uvx = 1.0f;
    quad_mesh.vertices[3].uvy = 1.0f;

    quad_mesh.indices[0] = 0;
    quad_mesh.indices[1] = 2;
    quad_mesh.indices[2] = 1;
    quad_mesh.indices[3] = 0;
    quad_mesh.indices[4] = 3;
    quad_mesh.indices[5] = 2;

    quad_mesh.material.ambient = vec3_create(0.8f, 0.8f, 0.8f);
    quad_mesh.material.diffuse = vec3_create(0.8f, 0.8f, 0.8f);
    quad_mesh.material.specular = VEC3_ZERO;
    quad_mesh.material.shininess = 1.0f;
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

    mesh_free(&quad_mesh);
}

const struct texture *get_texture(const char *name)
{
    struct texture *cached_tex = hashmap_get(textures, name);
    if (cached_tex)
    {
        return cached_tex;
    }

    load_asset_path(ASSET_OTHER, name);

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

    load_asset_path(ASSET_OTHER, name);

    FILE *f_mesh = fopen(asset_path, "r");
    assert(f_mesh);

    struct mesh *mesh = malloc(sizeof(struct mesh));

    char *line = NULL;
    size_t blength = 0;

    getline(&line, &blength, f_mesh);
    line[strcspn(line, "\n")] = '\0';
    bool has_polygon = read_polygon(line, mesh);
    assert(has_polygon);

    getline(&line, &blength, f_mesh);
    line[strcspn(line, "\n")] = '\0';
    mesh->material.texture = get_texture(line);
    assert(mesh->material.texture);

    // FIXME: Read values from file?
    mesh->material.ambient = vec3_create(0.8f, 0.8f, 0.8f);
    mesh->material.diffuse = vec3_create(0.8f, 0.8f, 0.8f);
    mesh->material.specular = VEC3_ZERO;
    mesh->material.shininess = 1.0f;

    free(line);
    fclose(f_mesh);

    hashmap_put(meshes, name, mesh);
    return mesh;
}

bool read_polygon(const char *name, struct mesh *mesh)
{
    load_asset_path(ASSET_OTHER, name);
    FILE *f = fopen(asset_path, "r");
    if (!f)
    {
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

struct shape create_quad()
{
    return (struct shape)
    {
        .transform = transform_create(VEC3_ZERO),
        .mesh = &quad_mesh,
    };
}

bool load_shader(struct shader *shader, const char *vert_name, const char *frag_name)
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

void mesh_init(struct mesh *mesh, size_t vertex_count, size_t index_count)
{
    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;

    mesh->vertices = malloc(vertex_count * sizeof(struct vertex));
    mesh->indices = malloc(index_count * sizeof(GLushort));

    mesh->material.texture = NULL;
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
