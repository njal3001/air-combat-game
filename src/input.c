#include "input.h"
#include <stdio.h>
#include <assert.h>

#define ASSERT_KEY(k) assert((k) > 0 && (k) <= GLFW_KEY_LAST)

struct key keys[GLFW_KEY_LAST + 1];

static void key_callback(GLFWwindow *window, int k, int scancode, int action, int mods)
{
    if (k < 0) return;

    struct key *key = keys + k;

    switch (action)
    {
        case GLFW_PRESS:
        {

        }
    }
}

void input_init(GLFWwindow *window)
{
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
}

void input_update(GLFWwindow *window)
{
    for (int k = 0; k <= GLFW_KEY_LAST; k++)
    {
        struct key *key = keys + k;
        int state = glfwGetKey(window, k);

        key->state &= ~(KEY_PRESSED | KEY_RELEASED);

        if (state == GLFW_PRESS)
        {
            if (!(key->state & KEY_DOWN))
            {
                key->state |= KEY_PRESSED;
            }

            key->state |= KEY_DOWN;
        }
        else if (state == GLFW_RELEASE)
        {
            if ((key->state & KEY_DOWN))
            {
                key->state |= KEY_RELEASED;
            }

            key->state &= ~KEY_DOWN;
        }
    }
}

struct key get_key(int key)
{
    ASSERT_KEY(key);
    return keys[key];
}

bool key_up(int key)
{
    ASSERT_KEY(key);
    return keys[key].state ^ KEY_DOWN;
}

bool key_down(int key)
{
    ASSERT_KEY(key);
    return keys[key].state & KEY_DOWN;
}

bool key_pressed(int key)
{
    ASSERT_KEY(key);
    return keys[key].state & KEY_PRESSED;
}

bool key_released(int key)
{
    ASSERT_KEY(key);
    return keys[key].state & KEY_RELEASED;
}
