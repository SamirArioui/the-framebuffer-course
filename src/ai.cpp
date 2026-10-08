// ai.cpp — the enemy behaviors, one request at a time.
//
// Lesson 089: a behavior is a small function of the entity, the hero,
// and what the entity's row already carried — nothing else. Each writes
// the entity's movement request; the walk turns every request into
// motion through the mover, exactly as it does for the hero. No
// behavior knows about kinds, rows, or the store.

#include "ai.h"

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

} /* namespace engine */
