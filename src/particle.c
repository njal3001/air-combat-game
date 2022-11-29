#include "particle.h"
#include <assert.h>
#include <stdio.h>
#include "calc.h"
#include "render.h"

#define MAX_SPEED_LINES 3000

struct particle speed_lines[MAX_SPEED_LINES];
size_t speed_line_count;

static void create_speed_line(size_t index, float t)
{
    struct particle *p = speed_lines + index;

    p->dir = vec3_normalize(vec3_create(frandrange(-0.2f, 0.2f),
            frandrange(-0.2f, 0.2f), -1.0f));
    p->t = t;
}

void speed_lines_update_and_render(size_t count,
        float ttl, float speed, float length, float off, struct vec3 offset,
        struct mat4 rot, float dt)
{
    assert(count <= MAX_SPEED_LINES);

    while (speed_line_count < count)
    {
        create_speed_line(speed_line_count, frand() * ttl);
        speed_line_count++;
    }
    speed_line_count = count;

    untextured_frame_begin();
    for (size_t i = 0; i < speed_line_count; i++)
    {
        struct particle *p = speed_lines + i;
        p->t += dt;
        if (p->t >= ttl)
        {
            create_speed_line(i, 0.0f);
        }
        else
        {
            struct vec3 dir = mat4_vmul(rot, p->dir);

            struct vec3 start_pos = p->dir;
            start_pos.z = 0.0f;
            start_pos = mat4_vmul(rot, start_pos);
            start_pos = vec3_mul(vec3_normalize(start_pos), off);

            struct vec3 pos = vec3_add(start_pos, vec3_mul(dir,
                        p->t * speed));
            struct vec3 lstart = vec3_add(pos, offset);
            struct vec3 lend = vec3_add(lstart,
                    vec3_mul(dir, length));
            push_line(lstart, lend, 0.01f, COLOR_WHITE);
        }
    }

    untextured_frame_end();
}
