// ai.h — the enemy behaviors: chase, keep-distance, flee.
//
// Lesson 089: three small functions, each writing one entity's movement
// request the way the player's input writes the hero's — a direction of
// the eight the movement knows — and every behavior moves through the
// mover (MoveEntity) like every entity does. The walk branches once on
// the entity's behavior, the fact its row carries (design D6):
// per-entity work is expressed once, not per type. The boss (lesson
// 090) composes these same three with a schedule of its own — its own
// pattern, never its own movement machinery.
//
// This is the game layer's AI file pair, beside the services (D2).
#ifndef AI_H
#define AI_H

#include "entity.h"

namespace engine {

/* Lesson 089: the distance the keep-distance behavior keeps, in world
   pixels — the behavior's fact, not any kind's: every kind that keeps,
   keeps this far. */
constexpr double AI_KEEP = 160.0;

/* The band inside which "close enough" holds — a keeper at its
   distance stands instead of twitching across the line. */
constexpr double AI_KEEP_BAND = 8.0;

/* Chase: the request points at the hero, every frame. */
void AiChase(Entity &e, const Entity &hero);

/* Keep-distance: the request points at the hero when it is too far,
   away when it is too close, and is rest at the distance — the ranged
   kind's habit: near enough to shoot, far enough to live. */
void AiKeep(Entity &e, const Entity &hero);

/* Flee: the request points away from the hero, every frame. */
void AiFlee(Entity &e, const Entity &hero);

/* Lesson 090: the boss's pattern — its own schedule, and the one thing
   a row cannot carry. The schedule is per-entity state and timing (the
   entity's `phase` and `phase_t`), and what it schedules is the same
   three behaviors above: the boss has no movement machinery of its
   own. The phases' lengths are the pattern's own facts, here beside
   the keep distance. */
constexpr double BOSS_CHASE_S = 3.0;
constexpr double BOSS_KEEP_S = 2.0;
constexpr double BOSS_FLEE_S = 1.0;

/* One boss's pattern, once per frame of game time: the schedule
   advances, and the phase it lands on writes the request — through
   AiChase, AiKeep, or AiFlee, like every other entity. */
void AiBoss(Entity &e, const Entity &hero, double dt);

} /* namespace engine */

#endif
