#pragma once
#include <GL/glew.h>

struct image
{
    unsigned char *data;
    int width, height;
    int channels;
};

struct texture
{
    GLuint id;
    int width, height;
};

struct cubemap
{
    GLuint id;
};

void texture_init(struct texture *texture, const struct image *img);
void texture_free(struct texture *texture);

void cubemap_init(struct cubemap *cubemap, const struct image img[6]);
void cubemap_free(struct cubemap *cmap);
