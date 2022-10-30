#include "menu.h"
#include "render.h"
#include "input.h"
#include "game.h"

enum menu_event menu_update()
{
    if (key_pressed(GLFW_KEY_ENTER))
    {
        return MENU_EVENT_PLAY;
    }

    return MENU_EVENT_NONE;
}

void menu_render()
{
    render_skybox();

    text_frame_begin();
    push_text(GAME_NAME, 585.0f, 780.0f, 2.0f);
    push_text("Press enter to begin", 650.0f, 600.0f, 1.0f);
    text_frame_end();
}
