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

    if (!render_init())
    {
        glfwTerminate();
        printf("Failed to initialize renderer!");
        return EXIT_FAILURE;
    }

    struct texture tex = create_texture("../assets/wall.jpg");
    if (tex.id)
    {
        printf("Texture (id: %d, w: %d, h: %d)\n", tex.id, tex.width, tex.height);
    }
    else
    {
        printf("Could not load texture\n");
    }

    struct mat4 model = mat4_identity();
    struct mat4 view;
    struct mat4 projection = mat4_perspective(M_PI / 4.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    struct vec3 cam_pos = vec3_create(0.0f, 0.0f, 1.0f);
    float cam_rot_y = 0.0f;

    bind_texture(&tex, 0);

    struct vertex vertices[] =
    {
        { vec3_create(-0.5f, -0.5f, 0.0f), COLOR_WHITE, 0.0f, 0.0f},
        { vec3_create(-0.5f, 0.5f, 0.0f), COLOR_WHITE, 0.0f, 1.0f},
        { vec3_create(0.5f, 0.5f, 0.0f), COLOR_WHITE, 1.0f, 1.0f},
        { vec3_create(0.5f, -0.5f, 0.0f), COLOR_WHITE, 1.0f, 0.0f},
    };

    GLushort indices[] =
    {
        0, 1, 3, 1, 2, 3,
    };

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            cam_pos.x += 0.01f;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            cam_pos.x -= 0.01f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            cam_pos.y += 0.01f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            cam_pos.y -= 0.01f;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            cam_pos.z += 0.01f;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            cam_pos.z -= 0.01f;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam_rot_y += 0.01f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam_rot_y -= 0.01f;

         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = mat4_mul(mat4_translate(vec3_neg(cam_pos)), mat4_roty(cam_rot_y));

        render_begin();

        render_quad(
                vec3_create(-1.0, -1.0, 0.0f),
                vec3_create(-1.0, -0.25f, 0.0f),
                vec3_create(-0.25f, -0.25f, 0.0f),
                vec3_create(-0.25f, -1.0f, 0.0f),
                COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
                0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);

        render_model(vertices, 4, indices, 6);

        render_end();
        render_flush(&model, &view, &projection);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    render_shutdown();

    glfwTerminate();
    return EXIT_SUCCESS;
}
