#pragma once
#include <GL/glew.h>
#include <stdbool.h>

enum vert_type
{
    VTYPE_FLOAT,
    VTYPE_FLOAT2,
    VTYPE_FLOAT3,
    VTYPE_FLOAT4,
    VTYPE_FLOAT16,
    VTYPE_UBYTE4,
};

enum buffer_usage
{
    BUFFER_STATIC = GL_STATIC_DRAW,
    BUFFER_DYNAMIC = GL_DYNAMIC_DRAW,
};

struct vert_attrib
{
    enum vert_type type;
    bool normalized;
    uint32_t divisor;
};

struct vbo
{
    GLuint id;
};

struct ebo
{
    GLuint id;
};

struct vao
{
    GLuint id;
    size_t attrib_count;
};

void vbo_init(struct vbo *vbo, size_t size, const void *data,
        enum buffer_usage usage);
void vbo_bind(struct vbo *vbo);
void vbo_set_data(struct vbo *vbo, size_t size, const void *data);
void vbo_free(struct vbo *vbo);

void ebo_init(struct ebo *ebo, size_t count, const void *data,
        enum buffer_usage usage);
void ebo_bind(struct ebo *ebo);
void ebo_set_data(struct ebo *ebo, size_t count, const void *data);
void ebo_free(struct ebo *ebo);

void vao_init(struct vao *vao);
void vao_bind(struct vao *vao);
void vao_set_ebo(struct vao *vao, struct ebo *ebo);
void vao_add_vbo(struct vao *vao, struct vbo *vbo, size_t num_attribs, ...);
void vao_free(struct vao *vao);
