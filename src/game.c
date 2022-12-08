#include "game.h"
#include <math.h>
#include <stdio.h>
#include "render.h"
#include "input.h"
#include "asset.h"
#include "world.h"
#include "menu.h"
#include "audio.h"
#include "timer.h"

enum gstate
{
    GSTATE_MENU,
    GSTATE_PLAY,
};

GLFWwindow *window;

bool camera_free_mode;

enum gstate state = GSTATE_MENU;

static void camera_free_mode_update(float dt)
{
    struct camera *cam = get_camera();
    const struct mouse *mouse = get_mouse();

    if (mouse->buttons[GLFW_MOUSE_BUTTON_MIDDLE].state & KEY_DOWN)
    {
        struct vec3 right = transform_right(&cam->transform);
        struct vec3 up = transform_up(&cam->transform);

        float dispx = mouse->dx * dt;
        float dispy = mouse->dy * dt;

        vec3_add_eq(&cam->transform.pos, vec3_mul(right, dispx));
        vec3_add_eq(&cam->transform.pos, vec3_mul(up, dispy));
    }

    if (mouse->buttons[GLFW_MOUSE_BUTTON_RIGHT].state & KEY_DOWN)
    {
        const float rspeed = 0.25f;
        float rx = -mouse->dy * rspeed * dt;
        float ry = mouse->dx * rspeed * dt;

        transform_local_rotx(&cam->transform, rx);
        transform_local_roty(&cam->transform, ry);
    }

    struct vec3 forward = transform_forward(&cam->transform);
    float famount = consume_mouse_scroll() * 100.0f * dt;
    vec3_add_eq(&cam->transform.pos, vec3_mul(forward, famount));
}

bool game_init()
{
    if (!glfwInit())
        return false;

    window = glfwCreateWindow(640, 480, GAME_NAME, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        glfwTerminate();
        return false;
    }

    // NOTE: Need to initialize assets before renderer because of shader loading
    assets_init();
    if (!render_init(window))
    {
        assets_free();
        glfwTerminate();
        printf("Failed to initialize renderer!");
        return false;
    }

    if (!audio_init())
    {
        assets_free();
        glfwTerminate();
        printf("Failed to initialize audio\n");
        return false;
    }

    input_init(window);

    actor_types_init();

    return true;
}

void game_run()
{
    audio_mute();

    struct world world;

    world_init(&world);
    audio_play(ASSET_AUDIO_SONG);

    timer_init();

    while (!glfwWindowShouldClose(window))
    {
        timer_preupdate();
        input_update(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float dt = timer_delta();

        switch (state)
        {
            case GSTATE_MENU:
            {
                enum menu_event mevent = menu_update();
                if (mevent == MENU_EVENT_PLAY)
                {
                    state = GSTATE_PLAY;
                    world_start(&world);
                }

                menu_render();
                break;
            }
            case GSTATE_PLAY:
            {
                // NOTE: Debug only
                if (key_pressed(GLFW_KEY_ESCAPE))
                {
                    camera_free_mode = !camera_free_mode;
                }
                else if (key_pressed(GLFW_KEY_F10))
                {
                    toggle_collider_rendering(&world);
                }

                if (camera_free_mode)
                {
                    camera_free_mode_update(dt);
                }
                else
                {
                    world_update(&world, dt);
                }

                world_render(&world);

                static char dinfo[256];
                snprintf(dinfo, 256, "Frame time: %fms\nFPS: %d\n",
                        dt * 100.0f, timer_fps());

                render_ui_begin();
                render_push_ui_text(dinfo, vec2_create(1550.0f, 1060.0f), 0.4f);
                render_ui_end();

                if (world_ended(&world))
                {
                    state = GSTATE_MENU;
                }

                break;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        timer_postupdate();
    }

    world_free(&world);
}

void game_shutdown()
{
    assets_free();
    render_shutdown();
    audio_shutdown();
    glfwTerminate();
}

GLFWwindow *get_window()
{
    return window;
}
