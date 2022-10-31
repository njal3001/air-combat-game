#include "game.h"
#include <math.h>
#include <stdio.h>
#include "render.h"
#include "input.h"
#include "assets.h"
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
bool camera_free;

enum gstate state = GSTATE_MENU;

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

    return true;
}

void game_run()
{
    audio_mute();

    world_init();
    audio_play("outthere.wav");

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
                }

                menu_render();
                break;
            }
            case GSTATE_PLAY:
            {
                // NOTE: Debug only
                if (key_pressed(GLFW_KEY_ESCAPE))
                {
                    camera_free = !camera_free;
                }
                else if (key_pressed(GLFW_KEY_F10))
                {
                    toggle_collider_rendering();
                }

                if (camera_free)
                {
                    camera_free_update(get_camera());
                }
                else
                {
                    world_update(dt);
                }

                world_render();
                break;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        timer_postupdate();
    }

    world_free();
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
