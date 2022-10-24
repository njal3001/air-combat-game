#include "game.h"
#include <math.h>
#include <stdio.h>
#include "render.h"
#include "input.h"
#include "assets.h"
#include "world.h"

GLFWwindow *window;
bool camera_free;

double time_prev;
size_t ticks;

bool game_init()
{
    if (!glfwInit())
        return false;

    window = glfwCreateWindow(640, 480, "Air Combat!", NULL, NULL);
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
        printf("Failed to initialize renderer!\n");
        return false;
    }

    input_init(window);

    return true;
}

void game_run()
{
    world_init();

    time_prev = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double time_now = glfwGetTime();
        float dt = time_now - time_prev;
        time_prev = time_now;

        input_update(window);

        // NOTE: Debug only
        if (key_pressed(GLFW_KEY_ESCAPE))
        {
            camera_free = !camera_free;
        }

        if (camera_free)
        {
            camera_free_update(get_camera());
        }
        else
        {
            world_update(dt);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        world_render();

        glfwSwapBuffers(window);
        glfwPollEvents();

        ticks++;
    }
}

void game_shutdown()
{
    assets_free();
    render_shutdown();
    glfwTerminate();
}

GLFWwindow *get_window()
{
    return window;
}

size_t get_ticks()
{
    return ticks;
}
