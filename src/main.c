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

    struct mat4 model = mat4_identity();
    struct mat4 view;
    struct mat4 projection = mat4_perspective(M_PI / 4.0f, 640.0f / 480.0f, 0.1f, 100.0f);

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

    struct mesh mesh =
    {
        .vertices = vertices,
        .indices = indices,
        .vertex_count = 4,
        .index_count = 6,
    };

    struct vec3 cam_pos = vec3_create(0.0f, 0.0f, 3.0f);
    struct vec3 cam_front;
    struct vec3 cam_right;
    struct vec3 cam_up;
    float cam_pitch = 0.0f;
    float cam_yaw = 0.0f;
    float cam_speed = 0.05f;
    float cam_rot_speed = 0.01f;

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            vec3_add_eq(&cam_pos, vec3_mul(cam_front, cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            vec3_sub_eq(&cam_pos, vec3_mul(cam_front, cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            vec3_add_eq(&cam_pos, vec3_mul(
                        vec3_normalize(vec3_cross(cam_front, cam_up)), cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            vec3_sub_eq(&cam_pos, vec3_mul(
                        vec3_normalize(vec3_cross(cam_front, cam_up)), cam_speed));
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cam_pitch += cam_rot_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cam_pitch -= cam_rot_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cam_yaw += cam_rot_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cam_yaw -= cam_rot_speed;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: This seems to not work correctly...
        cam_front.x = cosf(cam_yaw) * cosf(cam_pitch);
        cam_front.y = sinf(cam_pitch);
        cam_front.z = sinf(cam_yaw) * cosf(cam_pitch);
        cam_front = vec3_normalize(cam_front);
        cam_right = vec3_normalize(vec3_cross(cam_front, VEC3_UP));
        cam_up = vec3_normalize(vec3_cross(cam_right, cam_front));

        printf("%s\n", "Pos:");
        vec3_print(cam_pos);
        printf("%s\n", "Front:");
        vec3_print(cam_front);

        view = mat4_lookat(cam_pos, vec3_add(cam_pos, cam_front), cam_up);

        render_begin();

        render_quad(
                vec3_create(-1.0, -1.0, 0.0f),
                vec3_create(-1.0, -0.25f, 0.0f),
                vec3_create(-0.25f, -0.25f, 0.0f),
                vec3_create(-0.25f, -1.0f, 0.0f),
                COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE,
                0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);

        render_mesh(&mesh);

        render_end();
        render_flush(&model, &view, &projection);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    render_shutdown();

    glfwTerminate();
    return EXIT_SUCCESS;
}
