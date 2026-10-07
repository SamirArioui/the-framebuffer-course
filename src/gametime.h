// gametime.h — game time: the wall clock, scaled by one knob.
//
// Lesson 078: the simulation does not advance by the wall clock. It
// advances by game time, and game time is the wall clock's step scaled
// by one number the game sets: 0 is pause, a fraction is hitstop, full
// speed is play. One knob — not a special case threaded through the
// update, and not a second clock.
#ifndef GAMETIME_H
#define GAMETIME_H

namespace engine {

/* Full speed: the scale's top, where a run starts and where play
   lives. */
constexpr double GAMETIME_FULL = 1.0;

/* The game-time scale — the one number the game owns. */
struct GameTime {
    double scale;
};

/* The scale the game sets. It runs from 0 (paused) to GAMETIME_FULL
   (play) and no further: a value outside that range is clamped, so
   "faster than play" and "backwards" are not accidents a caller can
   have. */
void GameTimeSetScale(GameTime &time, double scale);

/* The simulation's step: the wall clock's step, scaled. This is the
   number the update advances the world by, and the only thing the
   scale reaches — the platform clock stays the measurer and the frame
   record keeps measuring wall clock (lesson 079). */
double GameTimeStep(const GameTime &time, double wall_dt);

} /* namespace engine */

#endif
