#pragma once

enum menu_event
{
    MENU_EVENT_NONE,
    MENU_EVENT_PLAY,
};

enum menu_event menu_update();
void menu_render();
