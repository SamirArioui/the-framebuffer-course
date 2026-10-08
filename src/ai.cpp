// ai.cpp — the enemy behaviors, one request at a time.
//
// Lesson 089: a behavior is a small function of the entity, the hero,
// and what the entity's row already carried — nothing else. Each writes
// the entity's movement request; the walk turns every request into
// motion through the mover, exactly as it does for the hero. No
// behavior knows about kinds, rows, or the store.

#include "ai.h"

#include <cstdio>

#include "combat.h" /* CombatAim — the same eight compass points */

namespace engine {

void AiChase(Entity &e, const Entity &hero)
{
    /* The request is the compass point at the hero — the same eight
       directions the player's arrows write for the hero, so the chaser
       covers ground at its own speed whichever way it runs. */
    CombatAim(hero.x - e.x, hero.y - e.y, e.move_x, e.move_y);
}

void AiKeep(Entity &e, const Entity &hero)
{
    /* The distance is squared throughout — no square root runs at run
       time. Beyond the band the keeper closes; inside it, it backs
       away; within it, it stands at its distance. */
    double dx = hero.x - e.x, dy = hero.y - e.y;
    double d2 = dx * dx + dy * dy;
    double near = AI_KEEP - AI_KEEP_BAND;
    double far = AI_KEEP + AI_KEEP_BAND;
    if (d2 > far * far)
        AiChase(e, hero);
    else if (d2 < near * near)
        AiFlee(e, hero);
    else {
        e.move_x = 0.0;
        e.move_y = 0.0;
    }
}

void AiFlee(Entity &e, const Entity &hero)
{
    /* Away — the compass point of the reverse delta. */
    CombatAim(e.x - hero.x, e.y - hero.y, e.move_x, e.move_y);
}

void AiBoss(Entity &e, const Entity &hero, double dt)
{
    /* The schedule: one timer per entity (its `phase_t`) counting how
       long the current phase has run, in game time — so a frozen world
       freezes the pattern too — and the phase index cycling when a
       phase's length is met. This is the whole of the boss's own
       machinery: state and timing. */
    e.phase_t += dt;
    double length = e.phase == 0 ? BOSS_CHASE_S
                                 : e.phase == 1 ? BOSS_KEEP_S : BOSS_FLEE_S;
    if (e.phase_t >= length) {
        e.phase_t = 0.0;
        e.phase = (e.phase + 1) % 3;
        length = e.phase == 0 ? BOSS_CHASE_S
                              : e.phase == 1 ? BOSS_KEEP_S : BOSS_FLEE_S;
        std::printf("engine: boss: %s's pattern -> %s (%.0f s)\n", e.name,
                    e.phase == 0 ? "chase" : e.phase == 1 ? "keep" : "flee",
                    length);
    }

    /* And what the schedule schedules: the same three behaviors every
       other entity uses. The boss composes them; it does not own any
       movement of its own. */
    if (e.phase == 0)
        AiChase(e, hero);
    else if (e.phase == 1)
        AiKeep(e, hero);
    else
        AiFlee(e, hero);
}

} /* namespace engine */
