#include <stdlib.h>
#include "game.h"

int main()
{
    if (!game_init())
    {
        return EXIT_FAILURE;
    }

    game_run();
    game_shutdown();

    return EXIT_SUCCESS;
}
