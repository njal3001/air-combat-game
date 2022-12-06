#include "menu.h"
#include "render.h"
#include "input.h"
#include "game.h"

enum menu_event menu_update()
{
    const struct controller *cont = get_first_controller();
    if (any_key_pressed() || (cont && any_controller_button_pressed(cont)))
    {
        return MENU_EVENT_PLAY;
    }

    return MENU_EVENT_NONE;
}

void menu_render()
{
    ui_begin();
    push_text(GAME_NAME, 585.0f, 780.0f, 2.0f);
    push_text("Press any key to begin", 650.0f, 600.0f, 1.0f);
    ui_end();
}
