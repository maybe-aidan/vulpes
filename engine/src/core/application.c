#include "application.h"
#include "game_types.h"
#include "logger.h"
#include "../platform/platform.h"
#include "vmemory.h"
#include "event.h"
#include "input.h"
#include "clock.h"

#include "../renderer/renderer_frontend.h"

typedef struct application_state{
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

// Event handlers
b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context);


b8 application_create(game* game_inst){
    if(initialized){
        VERROR("application_create called more than once.");
        return FALSE;
    }

    app_state.game_inst = game_inst;

    // Initialize subsystems.
    initialize_logging();
    input_initialize();

    // TODO: Remove this
    VFATAL("A test message: %f", 3.14f);
    VERROR("A test message: %f", 3.14f);
    VWARN("A test message: %f", 3.14f);
    VINFO("A test message: %f", 3.14f);
    VDEBUG("A test message: %f", 3.14f);
    VTRACE("A test message: %f", 3.14f);

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

    if(!event_initialize()){
        VERROR("Event system failed initialization. Application cannot continue.");
        return FALSE;
    }

    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

    if(!platform_startup(
        &app_state.platform, 
        game_inst->config.name, 
        game_inst->config.start_pos_x, 
        game_inst->config.start_pos_y, 
        game_inst->config.start_width, 
        game_inst->config.start_height)){
        return FALSE;
    }

    // Renderer startup
    if(!renderer_initialize(game_inst->config.name, &app_state.platform)){
        VFATAL("Failed to initialize renderer! Aborting application...");
        return FALSE;
    }

    // Initialize the game
    if(!app_state.game_inst->initialize(app_state.game_inst)){
        VFATAL("Game failed to initialize!");
        return FALSE;
    }

    app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

    initialized = TRUE;

    return TRUE;

}

b8 application_run(){
    clock_start(&app_state.clock);
    clock_update(&app_state.clock);
    app_state.last_time = app_state.clock.elapsed;
    //f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;

    VINFO(get_memory_usage_str());

    while(app_state.is_running){
        if(!platform_pump_messages(&app_state.platform)){
            app_state.is_running = FALSE;
        }

        if(!app_state.is_suspended){
            // Update clock and get delta time.
            clock_update(&app_state.clock);
            f64 current_time = app_state.clock.elapsed;
            f64 delta = current_time - app_state.last_time;
            f64 frame_start_time = platform_get_absolute_time();

            if(!app_state.game_inst->update(app_state.game_inst, (f32)delta)){
                VFATAL("Game update failed, shutting down.");
                app_state.is_running = FALSE;
                break;
            }

            // Call the game's render routine
            if(!app_state.game_inst->render(app_state.game_inst, (f32)delta)){
                VFATAL("Game render failed, shutting down.");
                app_state.is_running = FALSE;
                break;
            }

            // TODO: PLEASE refactor packet creation
            render_packet packet;
            packet.delta_time = delta;
            renderer_draw_frame(&packet);

            // Calculate how long the frame took
            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            //running_time += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if(remaining_seconds > 0){
                u64 remaining_ms = remaining_seconds * 1000;

                // If time left, give it back to the OS
                b8 limit_frames = FALSE;
                if(remaining_ms > 0 && limit_frames){
                    platform_sleep(remaining_ms - 1);
                }

                frame_count++;
            }

            // NOTE: Input update/state copying should always be handled
            // after any input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before
            // this frame ends.
            input_update(delta);

            // Update last time
            app_state.last_time = current_time;
        }
    }
    app_state.is_running = FALSE;

    event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

    event_shutdown();
    input_shutdown();
    renderer_shutdown();
    platform_shutdown(&app_state.platform);

    return TRUE;
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context){
    switch(code){
        case EVENT_CODE_APPLICATION_QUIT: {
            VINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.");
            app_state.is_running = FALSE;
            return TRUE;
        }
    }

    return FALSE;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context) {
    if(code == EVENT_CODE_KEY_PRESSED){
        u16 key_code = context.data.u16[0];
        if(key_code == KEY_ESCAPE){
            // NOTE: Technically firing an event to itself, but there may be other listeners.
            event_context data = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);

            return TRUE;
        }else if(key_code == KEY_A){
            // Example on checking for a key
            VDEBUG("Explicit - A key pressed!");
        }else{
            VDEBUG("'%c' key pressed in window.", key_code);
        }
    }else if(code == EVENT_CODE_KEY_RELEASED){
        u16 key_code = context.data.u16[0];
        if(key_code == KEY_B){
            // Example on checking for a key
            VDEBUG("Explicit - B key released!");
        }else{
            VDEBUG("'%c' key released in window.", key_code);
        }

    }
    return FALSE;
}