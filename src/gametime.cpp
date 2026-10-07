// gametime.cpp — the scale, and the step it makes.
//
// Lesson 078: two functions, one idea. The scale is a value the game
// sets; the step is the wall clock's, multiplied.

#include "gametime.h"

namespace engine {

void GameTimeSetScale(GameTime &time, double scale)
{
    if (scale < 0.0)
        scale = 0.0;
    if (scale > GAMETIME_FULL)
        scale = GAMETIME_FULL;
    time.scale = scale;
}

double GameTimeStep(const GameTime &time, double wall_dt)
{
    return wall_dt * time.scale;
}

} /* namespace engine */
