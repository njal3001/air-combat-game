#include "input.h"
#include <assert.h>
#include <math.h>

#define ASSERT_KEY(k) assert((k) > 0 && (k) < KEY_MAX)
#define CONTROLLER_AXIS_DEAD_ZONE 0.050

struct key keys[KEY_MAX];
struct controller controllers[CONTROLLER_MAX];
bool controller_active[CONTROLLER_MAX];

static void update_key(struct key *key, int state)
{
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

void input_init(GLFWwindow *window)
{
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
}

void input_update(GLFWwindow *window)
{
    for (int k = 0; k < KEY_MAX; k++)
    {
        struct key *key = keys + k;
        int state = glfwGetKey(window, k);
        update_key(key, state);
    }

    for (int jid = 0; jid < CONTROLLER_MAX; jid++)
    {
        bool active = glfwJoystickPresent(jid);
        controller_active[jid] = active;

        if (active)
        {
            struct controller *con = controllers + jid;

            GLFWgamepadstate controller_state;
            glfwGetGamepadState(jid, &controller_state);

            for (int a = 0; a < CONTROLLER_AXIS_MAX; a++)
            {
                float axis = controller_state.axes[a];
                if (fabs(axis) <= CONTROLLER_AXIS_DEAD_ZONE)
                {
                    axis = 0.0f;
                }

                con->axes[a] = axis;
            }

            for (int b = 0; b < CONTROLLER_BUTTON_MAX; b++)
            {
                int bstate = controller_state.buttons[b];
                update_key(con->buttons + b, bstate);
            }
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

bool any_key_pressed()
{
    for (int k = 1; k < KEY_MAX; k++)
    {
        if (key_pressed(k))
        {
            return true;
        }
    }

    return false;
}

const struct controller *get_first_controller()
{
    for (size_t i = 0; i < CONTROLLER_MAX; i++)
    {
        if (controller_active[i])
        {
            return controllers + i;
        }
    }

    return NULL;
}

bool any_button_pressed(const struct controller *con)
{
    for (size_t b = 0; b < CONTROLLER_BUTTON_MAX; b++)
    {
        if (con->buttons[b].state & KEY_PRESSED)
        {
            return true;
        }
    }

    return false;
}
