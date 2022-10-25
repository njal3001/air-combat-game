#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define GAME_NAME "Asteroids 3D"

bool game_init();
void game_run();
void game_shutdown();

GLFWwindow *get_window();
size_t get_ticks();
