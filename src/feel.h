// feel.h — the juice toolkit the game's events drive.
//
// Lesson 086: two hooks — a screenshake and a hitstop — each a thing
// that fires and then rests. Lesson 092: the toolkit fires them from the
// game's own events — a hit lands, a death falls — in the event's own
// frame, so the player reads cause and effect as one moment. The hooks
// are the mechanisms, each with its own fire-and-rest life; the events
// decide when and how heavily. A hook at rest costs nothing and changes
// nothing.
//
// Lesson 093: the toolkit's other two effects — particle bursts (the
// store's cosmetic work) and easing (values that arrive at their
// targets). Four effects, and no fifth: hitstop, screenshake, particle
// bursts, easing.
#ifndef FEEL_H
#define FEEL_H

#include "camera.h"
#include "entity.h"
#include "gametime.h"

namespace engine {

/* Lesson 093: the store's cosmetic share — the slots the toolkit's
   particles may hold. A burst takes these slots and no others: the rest
   of the store is kept for the game's own spawns, so a flood of sparks
   drops cosmetic work and never a gameplay one. A dropped particle is
   invisible; a dropped enemy is a bug the player experiences (lesson
   074's contrast). */
constexpr int FEEL_COSMETIC_SLOTS = ENTITY_CAP / 2;

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

/* Lesson 093: easing — a small set of shapes for values that animate
   from where they are to where they belong. Each takes t in [0, 1] and
   answers the fraction travelled: exactly 0 at 0, exactly 1 at 1. The
   ends are clamped on purpose — an eased value arrives at its target,
   it does not approach it forever. */
double EaseInQuad(double t);     /* slow out, arriving with weight */
double EaseOutQuad(double t);    /* fast out, settling into place */
double EaseInOutQuad(double t);  /* both */

/* Lesson 093: a particle burst — the toolkit's cosmetic spawn, fired in
   the triggering event's own frame like every feel effect. Up to
   `count` particles of `kind` take the store's cosmetic slots and ease
   out from (x, y) along the burst's lanes; whatever finds no slot is
   dropped and counted — cosmetic work may be dropped, gameplay work may
   not. Answers how many were made. */
int FeelBurst(EntityStore &store, const EntityDef &kind, double x, double y,
              int count);

/* One particle's settle, once a frame of game time: its eased travel
   out from its burst point — following the ease's curve and arriving
   exactly at its row's range — and its retirement there, when its life
   ends. */
void FeelParticle(EntityStore &store, Entity &e, double dt);

} /* namespace engine */

#endif
