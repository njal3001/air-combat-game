#pragma once
#include <GL/glew.h>
#include <stdbool.h>

struct vert_array
{
    GLuint id;
    GLuint vbo;
    GLuint ebo;
};

struct vert_array_data
{
    size_t vbo_size;
    size_t ebo_size;
    void *vbo_data;
    void *ebo_data;
};

enum varray_usage
{
    VARRAY_STATIC = GL_STATIC_DRAW,
    VARRAY_DYNAMIC = GL_DYNAMIC_DRAW,
};

enum vert_type
{
    VTYPE_FLOAT,
    VTYPE_FLOAT2,
    VTYPE_FLOAT3,
    VTYPE_FLOAT4,
};

struct vert_attrib
{
    enum vert_type type;
    bool normalized;
};

void vert_array_init(struct vert_array *varray, struct vert_array_data *data,
        enum varray_usage usage, size_t attrib_count, ...);

void vert_array_free(struct vert_array *varray);
