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
    render_ui_begin();
    render_push_ui_text(GAME_NAME, vec2_create(585.0f, 780.0f),
            2.0f, COLOR_WHITE);
    render_push_ui_text("Press any key to begin", vec2_create(650.0f, 600.0f),
            1.0f, COLOR_WHITE);
    render_ui_end();
}
