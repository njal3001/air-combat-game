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

    render_init();

    struct mat4 model = mat4_identity();
    struct mat4 view;
    struct mat4 projection = mat4_perspective(M_PI / 4.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    struct vec3 cam_pos = vec3_create(0.0f, 0.0f, 1.0f);
    float cam_rot_y = 0.0f;

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

        render_cube(vec3_create(0.0f, 0.0f, 0.0f), 0.5f,
                color_create(255, 0, 0, 255),
                color_create(0, 255, 0, 255),
                color_create(0, 0, 255, 255),
                color_create(255, 255, 0, 255),
                color_create(255, 0, 255, 255),
                color_create(0, 255, 255, 255));

        render_end();
        render_flush(&model, &view, &projection);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    render_shutdown();

    glfwTerminate();
    return EXIT_SUCCESS;
}
