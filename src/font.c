#include "font.h"

void font_init(struct font *f, size_t num_char, size_t lheight,
        const struct image *img)
{
    f->num_char = num_char;
    f->lheight = lheight;
    texture_init(&f->bitmap, img);
    f->chars = malloc(sizeof(struct fchar) * num_char);
    f->start_id = 0;
}

void font_set_char(struct font *f, size_t id, struct fchar c)
{
    if (!f->start_id)
    {
        f->start_id = id;
    }

    f->chars[id - f->start_id] = c;
}

void font_free(struct font *font)
{
    free(font->chars);
    texture_free(&font->bitmap);
}
