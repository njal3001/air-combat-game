#include "timer.h"
#include <GLFW/glfw3.h>

double time_prev;
double time_now;

uint32_t ticks;

uint32_t fps_ticks;
uint32_t fps_ticks_next;
float sec_acummulator;
float elapsed;

void timer_init()
{
    time_now = glfwGetTime();
}

void timer_preupdate()
{
    time_prev = time_now;
    time_now = glfwGetTime();

    elapsed += timer_delta();
}

void timer_postupdate()
{
    ticks++;
    fps_ticks_next++;
    sec_acummulator += timer_delta();
    if (sec_acummulator >= 1.0f)
    {
        fps_ticks = fps_ticks_next;
        fps_ticks_next = 0;
        sec_acummulator = 0.0f;
    }
}

float timer_delta()
{
    return time_now - time_prev;
}

uint32_t timer_ticks()
{
    return ticks;
}

float timer_elapsed()
{
    return elapsed;
}

uint32_t timer_fps()
{
    return fps_ticks;
}
