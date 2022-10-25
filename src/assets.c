#include "assets.h"
#include <stdbool.h>
#include "platform.h"
#include "hashmap.h"
#include "log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#define MAX_ASSET_PATH 512

struct hashmap *textures;
struct hashmap *meshes;

char asset_path[MAX_ASSET_PATH];
size_t root_length;

static bool read_polygon(const char *name, struct mesh *mesh);
static void texture_free(struct texture *texture);
static void mesh_init(struct mesh *mesh, size_t vertex_count, size_t index_count);

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

    int width, height, nchannels;
    unsigned char *data = stbi_load(asset_path, &width, &height, &nchannels, 0);

    if (!data)
    {
        return NULL;
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    GLenum format = nchannels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);

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
    mesh->material.texture = NULL;

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
            mesh->material.texture = get_texture(word);
            if (!mesh->material.texture)
            {
                log_warn("Could not load texture");
                free(line);
                fclose(f_mesh);
                mesh_free(mesh);
                return NULL;
            }
        }
        else if (strcmp(word, "shininess") == 0)
        {
            float val = strtof(strtok(NULL, " "), NULL);
            mesh->material.shininess = val;
        }
        else
        {
            char *sx, *sy, *sz;
            float x, y, z;

            sx = strtok(NULL, " ");
            sy = strtok(NULL, " ");
            sz = strtok(NULL, " ");

            x = strtof(sx, NULL);
            y = strtof(sy, NULL);
            z = strtof(sz, NULL);

            if (strcmp(word, "ambient") == 0)
            {
                mesh->material.ambient = vec3_create(x, y, z);
            }
            else if (strcmp(word, "diffuse") == 0)
            {
                mesh->material.diffuse = vec3_create(x, y, z);
            }
            else if (strcmp(word, "specular") == 0)
            {
                mesh->material.specular = vec3_create(x, y, z);
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

struct mesh create_quad_mesh()
{
    struct mesh quad_mesh;
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

    return quad_mesh;
}

struct mesh create_cube_mesh()
{
    struct mesh cube_mesh;
    mesh_init(&cube_mesh, 24, 36);

    cube_mesh.vertices[0].pos = vec3_create(1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[0].norm = vec3_create(-0.0f, 0.0f, 1.0f);
    cube_mesh.vertices[0].uvx = 0.875f;
    cube_mesh.vertices[0].uvy = 0.5f;

    cube_mesh.vertices[1].pos = vec3_create(-1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[1].norm = vec3_create(-0.0f, 0.0f, 1.0f);
    cube_mesh.vertices[1].uvx = 0.625f;
    cube_mesh.vertices[1].uvy = 0.75f;

    cube_mesh.vertices[2].pos = vec3_create(-1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[2].norm = vec3_create(-0.0f, 0.0f, 1.0f);
    cube_mesh.vertices[2].uvx = 0.625f;
    cube_mesh.vertices[2].uvy = 0.5f;

    cube_mesh.vertices[3].pos = vec3_create(-1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[3].norm = vec3_create(0.0f, 1.0f, -0.0f);
    cube_mesh.vertices[3].uvx = 0.625f;
    cube_mesh.vertices[3].uvy = 0.75f;

    cube_mesh.vertices[4].pos = vec3_create(1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[4].norm = vec3_create(0.0f, 1.0f, -0.0f);
    cube_mesh.vertices[4].uvx = 0.375f;
    cube_mesh.vertices[4].uvy = 1.0f;

    cube_mesh.vertices[5].pos = vec3_create(-1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[5].norm = vec3_create(0.0f, 1.0f, -0.0f);
    cube_mesh.vertices[5].uvx = 0.375f;
    cube_mesh.vertices[5].uvy = 0.75f;

    cube_mesh.vertices[6].pos = vec3_create(1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[6].norm = vec3_create(1.0f, 0.0f, -0.0f);
    cube_mesh.vertices[6].uvx = 0.625f;
    cube_mesh.vertices[6].uvy = 0.0f;

    cube_mesh.vertices[7].pos = vec3_create(1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[7].norm = vec3_create(1.0f, 0.0f, -0.0f);
    cube_mesh.vertices[7].uvx = 0.375f;
    cube_mesh.vertices[7].uvy = 0.25f;

    cube_mesh.vertices[8].pos = vec3_create(1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[8].norm = vec3_create(1.0f, 0.0f, -0.0f);
    cube_mesh.vertices[8].uvx = 0.375f;
    cube_mesh.vertices[8].uvy = 0.0f;

    cube_mesh.vertices[9].pos = vec3_create(-1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[9].norm = vec3_create(-0.0f, 0.0f, -1.0f);
    cube_mesh.vertices[9].uvx = 0.375f;
    cube_mesh.vertices[9].uvy = 0.5f;

    cube_mesh.vertices[10].pos = vec3_create(1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[10].norm = vec3_create(-0.0f, 0.0f, -1.0f);
    cube_mesh.vertices[10].uvx = 0.125f;
    cube_mesh.vertices[10].uvy = 0.75f;

    cube_mesh.vertices[11].pos = vec3_create(1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[11].norm = vec3_create(-0.0f, 0.0f, -1.0f);
    cube_mesh.vertices[11].uvx = 0.125f;
    cube_mesh.vertices[11].uvy = 0.5f;

    cube_mesh.vertices[12].pos = vec3_create(-1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[12].norm = vec3_create(-1.0f, 0.0f, 0.0f);
    cube_mesh.vertices[12].uvx = 0.625f;
    cube_mesh.vertices[12].uvy = 0.5f;

    cube_mesh.vertices[13].pos = vec3_create(-1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[13].norm = vec3_create(-1.0f, 0.0f, 0.0f);
    cube_mesh.vertices[13].uvx = 0.375f;
    cube_mesh.vertices[13].uvy = 0.75f;

    cube_mesh.vertices[14].pos = vec3_create(-1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[14].norm = vec3_create(-1.0f, 0.0f, 0.0f);
    cube_mesh.vertices[14].uvx = 0.375f;
    cube_mesh.vertices[14].uvy = 0.5f;

    cube_mesh.vertices[15].pos = vec3_create(1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[15].norm = vec3_create(0.0f, -1.0f, 0.0f);
    cube_mesh.vertices[15].uvx = 0.625f;
    cube_mesh.vertices[15].uvy = 0.25f;

    cube_mesh.vertices[16].pos = vec3_create(-1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[16].norm = vec3_create(0.0f, -1.0f, 0.0f);
    cube_mesh.vertices[16].uvx = 0.375f;
    cube_mesh.vertices[16].uvy = 0.5f;

    cube_mesh.vertices[17].pos = vec3_create(1.0f, -1.0f, -1.0f);
    cube_mesh.vertices[17].norm = vec3_create(0.0f, -1.0f, 0.0f);
    cube_mesh.vertices[17].uvx = 0.375f;
    cube_mesh.vertices[17].uvy = 0.25f;

    cube_mesh.vertices[18].pos = vec3_create(1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[18].norm = vec3_create(-0.0f, 0.0f, 1.0f);
    cube_mesh.vertices[18].uvx = 0.875f;
    cube_mesh.vertices[18].uvy = 0.75f;

    cube_mesh.vertices[19].pos = vec3_create(1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[19].norm = vec3_create(0.0f, 1.0f, -0.0f);
    cube_mesh.vertices[19].uvx = 0.625f;
    cube_mesh.vertices[19].uvy = 1.0f;

    cube_mesh.vertices[20].pos = vec3_create(1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[20].norm = vec3_create(1.0f, 0.0f, 0.0f);
    cube_mesh.vertices[20].uvx = 0.625f;
    cube_mesh.vertices[20].uvy = 0.25f;

    cube_mesh.vertices[21].pos = vec3_create(-1.0f, 1.0f, -1.0f);
    cube_mesh.vertices[21].norm = vec3_create(-0.0f, -0.0f, -1.0f);
    cube_mesh.vertices[21].uvx = 0.375f;
    cube_mesh.vertices[21].uvy = 0.75f;

    cube_mesh.vertices[22].pos = vec3_create(-1.0f, 1.0f, 1.0f);
    cube_mesh.vertices[22].norm = vec3_create(-1.0f, 0.0f, 0.0f);
    cube_mesh.vertices[22].uvx = 0.625f;
    cube_mesh.vertices[22].uvy = 0.75f;

    cube_mesh.vertices[23].pos = vec3_create(-1.0f, -1.0f, 1.0f);
    cube_mesh.vertices[23].norm = vec3_create(0.0f, -1.0f, 0.0f);
    cube_mesh.vertices[23].uvx = 0.625f;
    cube_mesh.vertices[23].uvy = 0.5f;

    cube_mesh.indices[0] = 0;
    cube_mesh.indices[1] = 1;
    cube_mesh.indices[2] = 2;
    cube_mesh.indices[3] = 3;
    cube_mesh.indices[4] = 4;
    cube_mesh.indices[5] = 5;
    cube_mesh.indices[6] = 6;
    cube_mesh.indices[7] = 7;
    cube_mesh.indices[8] = 8;
    cube_mesh.indices[9] = 9;
    cube_mesh.indices[10] = 10;
    cube_mesh.indices[11] = 11;
    cube_mesh.indices[12] = 12;
    cube_mesh.indices[13] = 13;
    cube_mesh.indices[14] = 14;
    cube_mesh.indices[15] = 15;
    cube_mesh.indices[16] = 16;
    cube_mesh.indices[17] = 17;
    cube_mesh.indices[18] = 0;
    cube_mesh.indices[19] = 18;
    cube_mesh.indices[20] = 1;
    cube_mesh.indices[21] = 3;
    cube_mesh.indices[22] = 19;
    cube_mesh.indices[23] = 4;
    cube_mesh.indices[24] = 6;
    cube_mesh.indices[25] = 20;
    cube_mesh.indices[26] = 7;
    cube_mesh.indices[27] = 9;
    cube_mesh.indices[28] = 21;
    cube_mesh.indices[29] = 10;
    cube_mesh.indices[30] = 12;
    cube_mesh.indices[31] = 22;
    cube_mesh.indices[32] = 13;
    cube_mesh.indices[33] = 15;
    cube_mesh.indices[34] = 23;
    cube_mesh.indices[35] = 16;

    return cube_mesh;
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

bool load_cubemap(struct cubemap *cmap, const char *const *faces)
{
    glGenTextures(1, &cmap->id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cmap->id);

    int width, height, nchannels;
    for (size_t i = 0; i < 6; i++)
    {
        load_asset_path(ASSET_OTHER, faces[i]);
        unsigned char *data = stbi_load(asset_path, &width, &height, &nchannels, 0);
        if (!data)
        {
            return false;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

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
    font->lheight = strtol(word, NULL, 10);

    // Bitmap
    int width, height, nchannels;
    getline(&line, &blength, f);
    line[strcspn(line, "\n")] = '\0';
    load_asset_path(ASSET_OTHER, line);

    unsigned char *data = stbi_load(asset_path, &width, &height, &nchannels, 0);
    if (!data)
    {
        log_warn("Could not open file %s", asset_path);
        free(line);
        return false;
    }

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    font->bitmap.id = tex_id;
    font->bitmap.width = width;
    font->bitmap.height = height;

    // Number of characters
    getline(&line, &blength, f);
    font->num_char = strtol(line, NULL, 10);
    font->chars = malloc(sizeof(struct fchar) * font->num_char);

    // Character information
    bool first_char = true;
    while (getline(&line, &blength, f) != -1)
    {
        word = strtok(line, " ");
        size_t id = strtol(word, NULL, 10);
        if (!id)
        {
            break;
        }

        if (first_char)
        {
            font->start_id = id;
            first_char = false;
        }

        struct fchar *c = font->chars + id - font->start_id;
        c->x = strtol(strtok(NULL, " "), NULL, 10);
        c->y = strtol(strtok(NULL, " "), NULL, 10);
        c->w = strtol(strtok(NULL, " "), NULL, 10);
        c->h = strtol(strtok(NULL, " "), NULL, 10);
        c->xoff = strtol(strtok(NULL, " "), NULL, 10);
        c->yoff = strtol(strtok(NULL, " "), NULL, 10);
        c->adv = strtol(strtok(NULL, " "), NULL, 10);
    }

    free(line);

    return true;
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

void font_free(struct font *font)
{
    free(font->chars);
    texture_free(&font->bitmap);
}
