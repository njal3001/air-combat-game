#include "game.h"
#include <stdio.h>
#include "render.h"
#include "fighter.h"

GLFWwindow *window;

bool game_init()
{
    if (!glfwInit())
        return false;

    window = glfwCreateWindow(640, 480, "WW1 Air Combat!", NULL, NULL);
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

    if (!render_init(window))
    {
        glfwTerminate();
        printf("Failed to initialize renderer!");
        return false;
    }

    return true;
}

void game_run()
{
    struct texture tex = create_texture("../assets/wall.jpg");
    bind_texture(&tex, 0);

    struct fighter fighter;
    fighter_init(&fighter, VEC3_ZERO);

    while (!glfwWindowShouldClose(window))
    {
        fighter_update(&fighter, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_begin();

        render_quad(
                vec3_create(-100.0, -100.0f, -100.0f),
                vec3_create(-100.0, -100.0f, 100.0f),
                vec3_create(100.0f, -100.0f, 100.0f),
                vec3_create(100.0f, -100.0f, -100.0f),
                COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN,
                0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);

        render_quad(
                vec3_create(-100.0, -100.0f, -100.0f),
                vec3_create(-100.0, 100.0f, -100.0f),
                vec3_create(100.0f, 100.0f, -100.0f),
                vec3_create(100.0f, -100.0f, -100.0f),
                COLOR_GREEN, COLOR_GREEN, COLOR_GREEN, COLOR_GREEN,
                0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);

        fighter_render(&fighter);

        render_end();
        render_flush();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    fighter_free(&fighter);
}

void game_shutdown()
{
    render_shutdown();
    glfwTerminate();
}

GLFWwindow *get_window()
{
    return window;
}
