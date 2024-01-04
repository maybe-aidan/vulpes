#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "../defines.h"

typedef struct clock
{
    f64 start_time;
    f64 elapsed;
} clock;

// Updates the provided clock. Should be called just before checking elapsed time.
// No effect on non-started clocks
void clock_update(clock* clock);

// Starts the provided clock. Resets elapsed time.
void clock_start(clock* clock);

// Stops the provided clock. Dopes not reset elapsed time.
void clock_stop(clock* clock);


#endif