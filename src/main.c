#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include "spatial.h"
#include "render.h"
#include <stdio.h>
#include <math.h>

int main()
{
    GLFWwindow *window;
    if (!glfwInit())
        return EXIT_FAILURE;

    window = glfwCreateWindow(640, 480, "WW1 Air Combat!", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    if (!render_init(window))
    {
        glfwTerminate();
        printf("Failed to initialize renderer!");
        return EXIT_FAILURE;
    }

    struct texture tex = create_texture("../assets/wall.jpg");

    bind_texture(&tex, 0);

    struct vertex vertices[] =
    {
        { vec3_create(-0.5f, -0.5f, -1.0f), COLOR_WHITE, 0.0f, 0.0f},
        { vec3_create(-0.5f, 0.5f, -1.0f), COLOR_WHITE, 0.0f, 1.0f},
        { vec3_create(0.5f, 0.5f, -1.0f), COLOR_WHITE, 1.0f, 1.0f},
        { vec3_create(0.5f, -0.5f, -1.0f), COLOR_WHITE, 1.0f, 0.0f},
    };

    GLushort indices[] =
    {
        0, 1, 3, 1, 2, 3,
    };

    struct mesh mesh =
    {
        .vertices = vertices,
        .indices = indices,
        .vertex_count = 4,
        .index_count = 6,
    };

    struct camera *cam = get_camera();

    float speed = 0.1f;
    float rot_speed = 0.01f;

    while (!glfwWindowShouldClose(window))
    {
        struct vec3 forward = transform_forward(&cam->transform);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            vec3_add_eq(&cam->transform.pos, vec3_mul(transform_forward(&cam->transform), speed));
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            vec3_sub_eq(&cam->transform.pos, vec3_mul(transform_forward(&cam->transform), speed));
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cam->transform.rot.x += rot_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cam->transform.rot.x -= rot_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cam->transform.rot.y += rot_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cam->transform.rot.y -= rot_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            cam->transform.rot.z -= rot_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            cam->transform.rot.z += rot_speed;
        }


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_begin();

        render_quad(
                vec3_create(-1.0, -1.0, -1.0f),
                vec3_create(-1.0, -0.25f, -1.0f),
                vec3_create(-0.25f, -0.25f, -1.0f),
                vec3_create(-0.25f, -1.0f, -1.0f),
                COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
                0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);

        render_mesh(&mesh);

        render_end();
        render_flush();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    render_shutdown();

    glfwTerminate();
    return EXIT_SUCCESS;
}
