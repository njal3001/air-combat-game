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
