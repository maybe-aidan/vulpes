#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "../defines.h"

// Application configuration
typedef struct application_config {
    // Window starting position x axis, if applicable
    i16 start_pos_x;

    // Window starting position y axis, if applicable
    i16 start_pos_y;

    // Window starting width
    i16 start_width;

    // Window starting height
    i16 start_height;

    // The application name used in windowing
    char* name;
} application_config;

VAPI b8 application_create(application_config* config);

VAPI b8 application_run();

#endif