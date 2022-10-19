#include "game.h"
#include <math.h>
#include <stdio.h>
#include "render.h"
#include "fighter.h"
#include "input.h"
#include "assets.h"

const char *app_name;
GLFWwindow *window;
bool camera_free;

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

    if (!render_init(window))
    {
        glfwTerminate();
        printf("Failed to initialize renderer!");
        return false;
    }

    input_init(window);
    assets_init();

    return true;
}

void game_run()
{
    const struct texture *tex = get_texture("wall.jpg");

    struct fighter fighter;
    fighter_init(&fighter, VEC3_ZERO);

    const float cube_scale = 3000.0f;

    struct shape cube_bot = create_quad();
    cube_bot.transform.scale = vec3_mul(VEC3_ONE, cube_scale);
    cube_bot.transform.pos.y = -cube_scale / 2.0f;

    struct shape cube_top = create_quad();
    cube_top.transform.scale = vec3_mul(VEC3_ONE, cube_scale);
    cube_top.transform.pos.y = cube_scale / 2.0f;
    cube_top.transform.rot = mat4_rotz(M_PI);

    struct shape cube_left = create_quad();
    cube_left.transform.scale = vec3_mul(VEC3_ONE, cube_scale);
    cube_left.transform.pos.x = -cube_scale / 2.0f;
    cube_left.transform.rot = mat4_rotz(M_PI / 2.0f);

    struct shape cube_right = create_quad();
    cube_right.transform.scale = vec3_mul(VEC3_ONE, cube_scale);
    cube_right.transform.pos.x = cube_scale / 2.0f;
    cube_right.transform.rot = mat4_rotz(-M_PI / 2.0f);

    struct shape cube_near = create_quad();
    cube_near.transform.scale = vec3_mul(VEC3_ONE, cube_scale);
    cube_near.transform.pos.z = cube_scale / 2.0f;
    cube_near.transform.rot = mat4_rotx(M_PI / 2.0f);

    struct shape cube_far = create_quad();
    cube_far.transform.scale = vec3_mul(VEC3_ONE, cube_scale);
    cube_far.transform.pos.z = -cube_scale / 2.0f;
    cube_far.transform.rot = mat4_rotx(-M_PI / 2.0f);

    while (!glfwWindowShouldClose(window))
    {
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
            fighter_update(&fighter, 1.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_begin();

        set_texture(tex);

        render_mesh(cube_bot.mesh, &cube_bot.transform);
        render_mesh(cube_top.mesh, &cube_top.transform);
        render_mesh(cube_left.mesh, &cube_left.transform);
        render_mesh(cube_right.mesh, &cube_right.transform);
        render_mesh(cube_near.mesh, &cube_near.transform);
        render_mesh(cube_far.mesh, &cube_far.transform);

        fighter_render(&fighter);

        render_end();
        render_flush();

        glfwSwapBuffers(window);
        glfwPollEvents();
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
