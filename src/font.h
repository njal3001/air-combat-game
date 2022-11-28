#pragma once
#include <stdint.h>
#include <stdlib.h>
#include "texture.h"

struct fchar
{
    uint16_t x;
    uint16_t y;
    uint8_t w;
    uint8_t h;
    uint8_t xoff;
    uint8_t yoff;
    uint8_t adv;
};

struct font
{
    size_t start_id;
    size_t num_char;
    uint8_t lheight;
    struct fchar *chars;
    struct texture bitmap;
};

void font_init(struct font *f, size_t num_char, size_t lheight,
        const struct image *img);

void font_set_char(struct font *f, size_t id, struct fchar c);
void font_free(struct font *f);
