// combat.h — the combat: weapons armed, shots fired, shots in flight.
//
// Lesson 087: weapons are rows, projectiles are entities (design D5). A
// weapon is a table row that names the projectile definition it fires and
// carries its rate and damage; arming a shooter carries those values onto
// it. Firing creates a projectile entity from that definition — through
// the store, like every entity — and the projectile moves through the
// mover in game time. It retires at a wall, at its range's end, and at
// the entity it hit; a hit reduces the target's health by the row's
// damage and retires the projectile. A zero-health entity is retired —
// the hero excepted, whose zero health is the game's defeat condition.
//
// This is the game layer's combat file pair, beside the services (D2):
// the store, the mover, and the table stay exactly what they are.
#ifndef COMBAT_H
#define COMBAT_H

#include "entity.h"
#include "feel.h"
#include "table.h"

namespace engine {

/* Lesson 087: the flight's sub-step, in world pixels. A projectile moves
   through the mover a few pixels at a time, checking what it hit along
   the way — a fast shot cannot pass what it hits, or cross a thin wall
   inside one long frame. */
constexpr double COMBAT_STEP = 4.0;

/* 1 / sqrt(2): a diagonal aim is scaled by this, exactly as the hero's
   diagonal intent is (lesson 085), so a shot covers ground at its speed
   whichever of the eight directions it flies. */
constexpr double AIM_DIAG = 0.70710678;

/* The eight compass points: the aim of the motion (vx, vy), normalized.
   At rest the result is (0, 0) and the caller picks its own fallback —
   the hero fires along its facing. */
void CombatAim(double vx, double vy, double &dir_x, double &dir_y);

/* Arm a shooter from a weapon row: the row's damage, rate, and the
   projectile kind it fires become the shooter's own values — the weapon
   is a row, and carrying a weapon is carrying its row's facts. */
void CombatArm(Entity &shooter, const EntityDef &weapon);

/* Fire: a projectile entity created from the definition the shooter's
   `fires` names, sent along (dir_x, dir_y) — a direction of unit length.
   Answers false — the report names why — when the shooter names no
   projectile kind, when the table holds no such definition, or when the
   store has no slot: never a stolen entity, never a shot with assumed
   attributes. */
bool CombatFire(EntityStore &store, const EntityTable &shots,
                Entity &shooter, double dir_x, double dir_y);

/* One projectile's flight, once a frame of game time: sub-stepped
   through the mover, retiring at a wall (the step refused), at its
   range's end, and at the entity it hit. A hit reduces the target's
   health by the shot's damage and retires the shot; a zero-health target
   is retired too — the hero excepted, whose zero health is the game's
   defeat condition (the state machine reads it, the game's actor is not
   retired out from under the game). Lesson 092: the hit and the death
   are the toolkit's events — the feedback hooks fire here, in the
   event's own frame, through `feel`. Lesson 093: they burst particles
   of `spark` too — the cosmetic kind the game names, spawned in the
   same frame from the same event. */
void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
               Entity &shot, Feedback &feel, const EntityDef &spark,
               double dt);

/* Lesson 090: the enemy attack, once per frame of game time. An armed
   entity — one whose row names a projectile kind — fires it at the
   hero at its row's rate, while the hero is within the shot's reach.
   The hero itself is never its own attacker: its trigger is the
   player's. */
void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
                  const Entity &hero, double dt);

} /* namespace engine */

#endif
