#ifndef _GAME_TYPES_H_
#define _GAME_TYPES_H_

#include "core/application.h"

/* Represents the basic game state in a game.
 * Called for creation by the application
 */

typedef struct game {
    // application configuration
    application_config config;

    // function pointer to game's initialize function
    b8 (*initialize)(struct game* game_inst);

    // function pointer to game's update function
    b8 (*update)(struct game* game_inst, f32 delta_time);

    // function pointer to game's render function
    b8 (*render)(struct game* game_inst, f32 delta_time);

    //function pointer to handle resizes, if applicable
    void (*on_resize)(struct game* game_inst, u32 width, u32 height);

    // Game specific game state. Created and managed by the game.
    void* state;
} game;

#endif