// feel.h — the feedback hooks the juice toolkit will drive.
//
// Lesson 086: two hooks — a screenshake and a hitstop — each a thing
// that fires and then rests. These are the hooks the juice toolkit
// (lessons 092-093) will drive from the game's events: here they are the
// mechanisms, each with its own fire-and-rest life, and nothing yet says
// when to fire them. A hook at rest costs nothing and changes nothing.
#ifndef FEEL_H
#define FEEL_H

#include "camera.h"
#include "gametime.h"

namespace engine {

/* The feedback state: what is still firing. Every field is at rest at
   zero — a hook that has fired and finished leaves itself exactly here. */
struct Feedback {
    double shake;     /* seconds of screenshake left; 0 = at rest */
    double shake_mag; /* the shake's offset while it lasts, pixels */
    double hitstop;   /* seconds of hitstop left; 0 = full speed */
    double hitstop_k; /* the fraction of game time during the hitstop */
};

void FeelInit(Feedback &feel);

/* Fire a screenshake: the camera's additive offset moves for `seconds`,
   then returns to rest — exactly zero. */
void FeelShake(Feedback &feel, double magnitude, double seconds);

/* Fire a hitstop: the game-time scale drops to `fraction` and returns to
   full speed on its own wall-time deadline (lesson 078's clock: what
   must end while the game is stopped cannot run on game time). */
void FeelHitstop(Feedback &feel, double fraction, double seconds);

/* The game-time factor the hitstop applies right now — a fraction during
   a hitstop, full speed at rest. The state's own scale multiplies this,
   so a pause still freezes and a hitstop only slows play. */
double FeelTimeScale(const Feedback &feel);

/* Advance the feedback once a frame on the wall clock: each hook decays
   toward rest and drives what it owns — the shake, the camera's additive
   offset (resting at exactly zero). */
void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera);

} /* namespace engine */

#endif
