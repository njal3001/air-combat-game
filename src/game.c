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
#include "log.h"
#include "calc.h"

enum gstate
{
    GSTATE_MENU,
    GSTATE_PLAY,
};

GLFWwindow *window;

bool camera_free_mode;
float camera_free_mode_spd;
float camera_free_mode_mspd;
float camera_free_mode_rspd;

enum gstate state = GSTATE_MENU;

static void camera_free_mode_update(float dt)
{
    if (key_pressed(GLFW_KEY_W))
    {
        camera_free_mode_spd *= 2.0f;
        camera_free_mode_mspd *= 2.0f;
        camera_free_mode_rspd *= 1.25f;
    }
    if (key_pressed(GLFW_KEY_S))
    {
        camera_free_mode_spd /= 2.0f;
        camera_free_mode_mspd /= 2.0f;
        camera_free_mode_rspd /= 1.25f;
    }

    struct camera *cam = get_camera();
    const struct mouse *mouse = get_mouse();

    if (mouse->buttons[GLFW_MOUSE_BUTTON_MIDDLE].state & KEY_DOWN)
    {
        struct vec3 right = transform_right(&cam->transform);
        struct vec3 up = transform_up(&cam->transform);

        float dispx = mouse->dx * camera_free_mode_mspd;
        float dispy = mouse->dy * camera_free_mode_mspd;

        vec3_add_eq(&cam->transform.pos, vec3_mul(right, dispx));
        vec3_add_eq(&cam->transform.pos, vec3_mul(up, dispy));
    }

    if (mouse->buttons[GLFW_MOUSE_BUTTON_RIGHT].state & KEY_DOWN)
    {
        float rx = -mouse->dy * camera_free_mode_rspd;
        float ry = mouse->dx * camera_free_mode_rspd;

        transform_local_rotx(&cam->transform, rx);
        transform_local_roty(&cam->transform, ry);
    }

    struct vec3 forward = transform_forward(&cam->transform);
    float famount = consume_mouse_scroll() * camera_free_mode_spd;
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

    // NOTE: Need to initialize assets before renderer
    // because of shader loading
    assets_init();
    if (!render_init(window))
    {
        assets_free();
        glfwTerminate();
        log_err("Failed to initialize renderer");
        return false;
    }

    if (!audio_init())
    {
        assets_free();
        glfwTerminate();
        log_err("Failed to initialize audio");
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
                    world_begin(&world);
                }

                menu_render();
                break;
            }
            case GSTATE_PLAY:
            {
                // NOTE: Debug only
                if (key_pressed(GLFW_KEY_ESCAPE))
                {
                    camera_free_mode_spd = 20.0f;
                    camera_free_mode_mspd = 0.1f;
                    camera_free_mode_rspd = 0.005f;

                    camera_free_mode = !camera_free_mode;
                    toggle_hud_rendering(&world);
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
                struct vec3 cpos = get_camera()->transform.pos;
                snprintf(dinfo, 256,
                        "Frame time: %.2fms\nFPS: %d\n"
                        "Camera pos: (%.2f, %.2f, %.2f)",
                        dt * 100.0f, timer_fps(),
                        cpos.x, cpos.y, cpos.z);

                render_ui_begin();
                render_push_ui_text(dinfo, vec2_create(1300.0f, 1060.0f),
                        0.4f, COLOR_WHITE);

                render_ui_end();

                if (world_should_end(&world))
                {
                    world_end(&world);
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
