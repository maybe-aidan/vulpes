#include "game.h"

#include <entry.h>

#include <core/vmemory.h>

// Define the function to create the game
b8 create_game(game* out_game){
    out_game->config.start_pos_x = 100;
    out_game->config.start_pos_y = 100;
    out_game->config.start_height = 720;
    out_game->config.start_width = 1280;
    out_game->config.name = "Vulpes Engine Testbed";
    out_game->update = game_update;
    out_game->render = game_render;
    out_game->initialize = game_intitialize;
    out_game->on_resize = game_on_resize;

    // Create the game state
    out_game->state =  vallocate(sizeof(game_state), MEMORY_TAG_GAME);

    return TRUE;
}