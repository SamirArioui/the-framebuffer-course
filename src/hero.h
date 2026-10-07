// hero.h — the hero's movement: the player's intent, eased into motion.
//
// Lesson 085: the hero is not a position that jumps to a new spot each
// frame. It is a thing with weight — a velocity that eases toward where
// the player means to go (acceleration) and toward rest (deceleration),
// never a step change. And the intent is a *direction*, normalized, so
// the diagonal is no faster than the straight run.
//
// This is the hero's own behavior (design D2), in its own file beside
// the services and the game machine: the input becomes motion here, and
// the walk turns that motion into steps against the map.
#ifndef HERO_H
#define HERO_H

#include "entity.h"
#include "platform.h"

namespace engine {

/* The hero's accel/decel time constant — the feel: roughly how long it
   takes to ease from rest to full speed (or back). Lesson 085 keeps it
   here as the hero's own fact; when the table format grows named
   columns (lesson 087) the feel becomes data, like the hero's speed
   already is. */
constexpr double HERO_TIME = 0.12; /* seconds to close on the intent */

/* 1 / sqrt(2): a diagonal intent is scaled by this so the hero covers
   ground at the straight-line speed, not sqrt(2) times it. */
constexpr double HERO_DIAG = 0.70710678;

/* Lesson 086: how long one frame of the walk cycle shows. */
constexpr double ANIM_STEP = 0.12;

/* The hero's movement, once per frame of play. The held direction is the
   intent, normalized so the diagonal is no faster than straight; the
   hero's velocity eases toward that intent (accel) and toward rest
   (decel) — a turn passes through the ease rather than snapping. The
   result is left in the hero's own movement request, which the walk
   turns into motion against the map. */
void HeroMove(Entity &hero, platform::Window *window, double dt);

} /* namespace engine */

#endif
