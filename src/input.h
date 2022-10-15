#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>

#define KEY_DOWN     (1 << 0)
#define KEY_PRESSED  (1 << 1)
#define KEY_RELEASED (1 << 2)

struct key
{
    uint8_t state;
};

void input_init(GLFWwindow *window);
void input_update(GLFWwindow *window);

struct key get_key(int key);
bool key_up(int key);
bool key_down(int key);
bool key_pressed(int key);
bool key_released(int key);
