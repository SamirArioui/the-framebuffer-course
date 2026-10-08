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
   turns into motion against the map. The ease's time constant is the
   hero's own `accel` — its row's fact since lesson 087 grew the format
   by named columns (the hero's row predates the column and keeps loading
   byte-for-byte, its weight the format's default). */
void HeroMove(Entity &hero, platform::Window *window, double dt);

/* Lesson 087: the hero's weapon, once per frame of play. The number keys
   arm the weapons table's rows — a weapon is a row, and carrying it is
   carrying its values — and the fire key sends a shot along the hero's
   motion (the eight compass points of its velocity) or, at rest, along
   its facing. The shot is an entity like any other; the rate its row
   states is the ceiling on how often the trigger answers. */
void HeroFire(Entity &hero, platform::Window *window,
              const EntityTable &weapons, const EntityTable &shots,
              EntityStore &store, double dt);

} /* namespace engine */

#endif
