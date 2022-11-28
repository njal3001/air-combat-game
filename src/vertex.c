#include "vertex.h"
#include <stdarg.h>

void vert_array_init(struct vert_array *varray, struct vert_array_data *data,
        enum varray_usage usage, size_t attrib_count, ...)
{
    size_t stride = 0;
    va_list attribs;
    va_start(attribs, attrib_count);

    for (size_t i = 0; i < attrib_count; i++)
    {
        struct vert_attrib attr = va_arg(attribs, struct vert_attrib);
        switch (attr.type)
        {
            case VTYPE_FLOAT:
                stride += sizeof(GLfloat);
                break;
            case VTYPE_FLOAT2:
                stride += 2 * sizeof(GLfloat);
                break;
            case VTYPE_FLOAT3:
                stride += 3 * sizeof(GLfloat);
                break;
            case VTYPE_FLOAT4:
                stride += 4 * sizeof(GLfloat);
                break;
            case VTYPE_UBYTE4:
                stride += 4 * sizeof(GLubyte);
                break;
        }
    }

    va_end(attribs);

    glGenVertexArrays(1, &varray->id);
    glBindVertexArray(varray->id);

    glGenBuffers(1, &varray->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, varray->vbo);
    glBufferData(GL_ARRAY_BUFFER, data->vbo_size * stride, data->vbo_data, usage);

    varray->ebo = 0;
    if (data->ebo_size)
    {
        glGenBuffers(1, &varray->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, varray->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->ebo_size * sizeof(GLuint),
                        data->ebo_data, usage);
    }

    va_start(attribs, attrib_count);

    size_t attrib_pointer = 0;
    for (size_t i = 0; i < attrib_count; i++)
    {
        struct vert_attrib attr = va_arg(attribs, struct vert_attrib);

        size_t size;
        GLint count;
        GLenum type;

        switch (attr.type)
        {
            case VTYPE_FLOAT:
                count = 1;
                size = sizeof(GLfloat);
                type = GL_FLOAT;
                break;
            case VTYPE_FLOAT2:
                count = 2;
                size = 2 * sizeof(GLfloat);
                type = GL_FLOAT;
                break;
            case VTYPE_FLOAT3:
                count = 3;
                size = 3 * sizeof(GLfloat);
                type = GL_FLOAT;
                break;
            case VTYPE_FLOAT4:
                count = 4;
                size = 4 * sizeof(GLfloat);
                type = GL_FLOAT;
                break;
            case VTYPE_UBYTE4:
                count = 4;
                size = 4 * sizeof(GLubyte);
                type = GL_UNSIGNED_BYTE;
                break;
        }

        glVertexAttribPointer(i, count, type, attr.normalized, stride,
                (const GLvoid*)attrib_pointer);
        glEnableVertexAttribArray(i);

        attrib_pointer += size;
    }

    va_end(attribs);
}

void vert_array_free(struct vert_array *varray)
{
    glDeleteBuffers(1, &varray->vbo);
    if (varray->ebo)
    {
        glDeleteBuffers(1, &varray->ebo);
    }

    glDeleteVertexArrays(1, &varray->id);
}

void vbo_init(struct vbo *vbo, size_t size, const void *data,
        enum buffer_usage usage)
{
    glGenBuffers(1, &vbo->id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void vbo_bind(struct vbo *vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo->id);
}

void vbo_set_data(struct vbo *vbo, size_t size, const void *data)
{
    vbo_bind(vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

void vbo_free(struct vbo *vbo)
{
    glDeleteBuffers(1, &vbo->id);
}

void ebo_init(struct ebo *ebo, size_t count, const void *data,
        enum ebo_format format, enum buffer_usage usage)
{
    glGenBuffers(1, &ebo->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->id);

    size_t elem_size = format == EBO_FORMAT_U16 ? 16 : 32;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * elem_size, data, usage);
}

void ebo_bind(struct ebo *ebo)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->id);
}

void ebo_set_data(struct ebo *ebo, size_t count, const void *data)
{
    ebo_bind(ebo);
    size_t elem_size = ebo->format == EBO_FORMAT_U16 ? 16 : 32;
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * elem_size, data);
}

void ebo_free(struct ebo *ebo)
{
    glDeleteBuffers(1, &ebo->id);
}

void vao_init(struct vao *vao)
{
    glGenVertexArrays(1, &vao->id);
    vao->attrib_count = 0;
}

void vao_bind(struct vao *vao)
{
    glBindVertexArray(vao->id);
}

void vao_set_ebo(struct vao *vao, struct ebo *ebo)
{
    vao_bind(vao);
    ebo_bind(ebo);
}

void vao_add_vbo(struct vao *vao, struct vbo *vbo, size_t num_attribs, ...)
{
    vao_bind(vao);
    vbo_bind(vbo);

    size_t stride = 0;
    va_list attribs;
    va_start(attribs, num_attribs);

    for (size_t i = 0; i < num_attribs; i++)
    {
        struct vert_attrib attr = va_arg(attribs, struct vert_attrib);
        switch (attr.type)
        {
            case VTYPE_FLOAT:
                stride += sizeof(GLfloat);
                break;
            case VTYPE_FLOAT2:
                stride += 2 * sizeof(GLfloat);
                break;
            case VTYPE_FLOAT3:
                stride += 3 * sizeof(GLfloat);
                break;
            case VTYPE_FLOAT4:
                stride += 4 * sizeof(GLfloat);
                break;
            case VTYPE_FLOAT16:
                stride += 16 * sizeof(GLfloat);
                break;
            case VTYPE_UBYTE4:
                stride += 4 * sizeof(GLubyte);
                break;
        }
    }

    va_end(attribs);

    va_start(attribs, num_attribs);

    size_t attrib_pointer = 0;
    for (size_t i = 0; i < num_attribs; i++)
    {
        struct vert_attrib attr = va_arg(attribs, struct vert_attrib);

        size_t size;
        GLint count;
        GLenum type;
        size_t attrib_delta;

        switch (attr.type)
        {
            case VTYPE_FLOAT:
                count = 1;
                size = sizeof(GLfloat);
                type = GL_FLOAT;
                attrib_delta = 1;
                break;
            case VTYPE_FLOAT2:
                count = 2;
                size = 2 * sizeof(GLfloat);
                type = GL_FLOAT;
                attrib_delta = 1;
                break;
            case VTYPE_FLOAT3:
                count = 3;
                size = 3 * sizeof(GLfloat);
                type = GL_FLOAT;
                attrib_delta = 1;
                break;
            case VTYPE_FLOAT4:
                count = 4;
                size = 4 * sizeof(GLfloat);
                type = GL_FLOAT;
                attrib_delta = 1;
                break;
            case VTYPE_FLOAT16:
                count = 4;
                size = 4 * sizeof(GLfloat);
                type = GL_FLOAT;
                attrib_delta = 4;
                break;
            case VTYPE_UBYTE4:
                count = 4;
                size = 4 * sizeof(GLubyte);
                type = GL_UNSIGNED_BYTE;
                attrib_delta = 1;
                break;
        }

        for (size_t d = 0; d < attrib_delta; d++)
        {
            glVertexAttribPointer(vao->attrib_count, count, type, attr.normalized, stride,
                    (const GLvoid*)attrib_pointer);
            glEnableVertexAttribArray(vao->attrib_count);

            if (attr.divisor)
            {
                glVertexAttribDivisor(vao->attrib_count, attr.divisor);
            }

            attrib_pointer += size;
            vao->attrib_count++;
        }
    }

    va_end(attribs);
}

void vao_free(struct vao *vao)
{
    glDeleteVertexArrays(1, &vao->id);
}
