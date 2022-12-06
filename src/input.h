#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>

#define KEY_DOWN     (1 << 0)
#define KEY_PRESSED  (1 << 1)
#define KEY_RELEASED (1 << 2)

#define KEY_MAX GLFW_KEY_LAST + 1
#define CONTROLLER_MAX GLFW_JOYSTICK_LAST + 1
#define CONTROLLER_AXIS_MAX GLFW_GAMEPAD_AXIS_LAST + 1
#define CONTROLLER_BUTTON_MAX GLFW_GAMEPAD_BUTTON_LAST + 1
#define MOUSE_BUTTON_MAX GLFW_MOUSE_BUTTON_LAST + 1

struct key
{
    uint8_t state;
};

struct controller
{
    struct key buttons[CONTROLLER_BUTTON_MAX];
    float axes[CONTROLLER_AXIS_MAX];
};

struct mouse
{
    struct key buttons[MOUSE_BUTTON_MAX];
    float posx, posy;
    float dx, dy;
};

void input_init(GLFWwindow *window);
void input_update(GLFWwindow *window);

struct key get_key(int key);
bool key_up(int key);
bool key_down(int key);
bool key_pressed(int key);
bool key_released(int key);

bool any_key_pressed();

const struct controller *get_first_controller();
bool any_controller_button_pressed(const struct controller *con);

const struct mouse *get_mouse();
float consume_mouse_scroll();
