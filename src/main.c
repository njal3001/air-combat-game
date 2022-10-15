#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "game.h"


int main(int argc, char **argv)
{
    if (!game_init())
    {
        return EXIT_FAILURE;
    }

    game_run();
    game_shutdown();

    return EXIT_SUCCESS;
}
