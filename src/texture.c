#include "texture.h"

static GLenum image_format(const struct image *img)
{
    switch (img->channels)
    {
        case 1:
            return GL_UNSIGNED_BYTE;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
    }

    return 0;
}

void texture_init(struct texture *tex, const struct image *img)
{
    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    GLenum format = image_format(img);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, format,
                    GL_UNSIGNED_BYTE, img->data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    tex->width = img->width;
    tex->height = img->height;
}

void texture_free(struct texture *texture)
{
    glDeleteTextures(1, &texture->id);
}

void cubemap_init(struct cubemap *cmap, const struct image faces[6])
{
    glGenTextures(1, &cmap->id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cmap->id);

    for (size_t i = 0; i < 6; i++)
    {
        const struct image *face = faces + i;
        GLenum format = image_format(face);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
                        face->width, face->height, 0, format,
                        GL_UNSIGNED_BYTE, face->data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void cubemap_free(struct cubemap *cmap)
{
    glDeleteTextures(1, &cmap->id);
}
