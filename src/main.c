#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

const char *vert_shader_str =
    "#version 330 core\n"
    "layout (location 0) in vec3 a_pos\n"
    "void main()\n"
    "{\n"
    "   glPosition = vec4(a_pos, 1.0);\n"
    "}";

const char *frag_shader_str =
    "#version 330 core\n"
    "out vec4 o_col"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    "}";

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

    float vertices[] =
    {
        0.0f, 0.5f, 0.0f,
        -0.5f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f
    };

    // Vertex buffer
    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Vertex shader
    GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader_id, 1, &vert_shader_str, NULL);
    glCompileShader(vert_shader_id);

    // Fragment shader
    GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader_id, 1, &frag_shader_str, NULL);
    glCompileShader(frag_shader_id);

    // Shader program
    GLuint shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, vert_shader_id);
    glAttachShader(shader_program_id, frag_shader_id);
    glLinkProgram(shader_program_id);

    glUseProgram(shader_program_id);

    // Delete shaders
    glDeleteShader(vert_shader_id);
    glDeleteShader(frag_shader_id);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)NULL);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
