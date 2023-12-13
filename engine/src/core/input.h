#ifndef _INPUT_H_
#define _INPUT_H_

#include "../defines.h"

typedef enum buttons{
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_MIDDLE,
    BUTTON_MAX_BUTTONS
} buttons;

#define DEFINE_KEY(name, code) KEY_##name = code

typedef enum keys{
    // TODO: Get all keycodes
    // EX:
    // DEFINE_KEY(EXAMPLE, 0xFF),

} keys;


#endif