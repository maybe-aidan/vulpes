#ifndef _ENTRY_H_
#define _ENTRY_H_

#include "core/application.h"
#include "core/logger.h"
#include "core/vmemory.h"
#include "game_types.h"

// Externally defined function to create a game
extern b8 create_game(game* out_game);

/* -------------------------------------
 *  Main Entry Point of the Application
 */
int main(void){

    initialize_memory();
    
    // Request a game instance from the application
    game game_inst;
    if(!create_game(&game_inst)){
        VFATAL("Could not create game!");
        return -1;
    }

    // Ensure the fucntion pointer exist
    if(!game_inst.render || !game_inst.initialize || !game_inst.update || !game_inst.on_resize){
        VFATAL("The game's fucntion pointers must be assigned!");
        return -2;
    }

    // Initialization
    if(!application_create(&game_inst)){
        VINFO("Application failed to create!");
        return 1;
    }

    // Begin game loop.
    if(!application_run()){
        VINFO("Application did not shutdown gracefully!");
        return 2;
    }
    
    shutdown_memory();

    return 0;
}

#endif