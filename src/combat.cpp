// combat.cpp — the combat's mechanics: arm, fire, fly, hit, retire.
//
// Lesson 087: every mechanic here is the data's. The damage a hit does is
// the row's damage the shooter carries; the projectile is the kind its
// row names; the flight is the projectile's speed over game time; the
// life is its row's range. Nothing here knows which weapon or which
// projectile exists — the tables know that.
//
// Lesson 092: the hit and the death are the juice toolkit's events, and
// the feedback hooks fire at the lines where they happen.
//
// Lesson 093: the same events burst particles — cosmetic entities of
// the kind the game names, bounded by the store's policy.

#include "combat.h"

#include <cstdio>

namespace engine {
namespace {

/* The two boxes of a shot and its target: what each draws is what each
   is hit as — one ANIM_FRAME_W-wide frame of its art (lesson 086). */
bool Overlaps(const Entity &a, const Entity &b)
{
    return a.x < b.x + ANIM_FRAME_W && b.x < a.x + ANIM_FRAME_W &&
           a.y < b.y + b.sprite->height && b.y < a.y + a.sprite->height;
}

/* Two kinds' names, the same or not. */
bool SameName(const char *a, const char *b)
{
    int i = 0;
    while (a[i] && a[i] == b[i])
        ++i;
    return a[i] == b[i];
}

} /* namespace */

void CombatAim(double vx, double vy, double &dir_x, double &dir_y)
{
    /* The aim is the compass point of the motion — one of the eight
       directions the movement already knows, a diagonal at 1/sqrt(2) —
       never a direction of some other length. At rest this is (0, 0). */
    dir_x = 0.0;
    dir_y = 0.0;
    if (vx > 0.0)
        dir_x = 1.0;
    else if (vx < 0.0)
        dir_x = -1.0;
    if (vy > 0.0)
        dir_y = 1.0;
    else if (vy < 0.0)
        dir_y = -1.0;
    if (dir_x != 0.0 && dir_y != 0.0) {
        dir_x *= AIM_DIAG;
        dir_y *= AIM_DIAG;
    }
}

void CombatArm(Entity &shooter, const EntityDef &weapon)
{
    /* A weapon is a row; carrying it means carrying its values. The
       shooter is not tied to the row afterwards — it carries the facts
       and may be armed with another row's. */
    shooter.damage = weapon.damage;
    shooter.rate = weapon.rate;
    for (int i = 0; i < TABLE_NAME_MAX; ++i)
        shooter.fires[i] = weapon.fires[i];
    shooter.cooldown = 0.0;
    std::printf("engine: arm: %s arms %s (damage %d, rate %d, fires %s)\n",
                shooter.name, weapon.name, weapon.damage, weapon.rate,
                weapon.fires[0] ? weapon.fires : "none");
}

bool CombatFire(EntityStore &store, const EntityTable &shots,
                Entity &shooter, double dir_x, double dir_y)
{
    if (!shooter.fires[0])
        return false; /* an unarmed shooter fires nothing */

    /* The projectile kind is the row's fact, looked up where the kinds
       live. A kind the table does not hold is a typed failure — never a
       shot with assumed attributes. */
    DefResult kind = TableFind(shots, shooter.fires);
    if (kind.error != DEF_OK) {
        std::printf("engine: fire refused — %s is no projectile kind\n",
                    shooter.fires);
        return false;
    }

    /* The projectile is an entity: created from its definition, through
       the store, in a slot like every entity. */
    EntityResult made = EntityCreate(store, *kind.def);
    if (made.error != ENTITY_OK) {
        std::printf("engine: fire refused — the store is full\n");
        return false;
    }

    Entity &shot = *made.entity;
    shot.x = shooter.x; /* a shot leaves its shooter's box */
    shot.y = shooter.y;
    shot.owner = &shooter; /* a shot never hits its owner */
    shot.damage = shooter.damage; /* the row's damage, carried into the hit */
    shot.move_x = dir_x;
    shot.move_y = dir_y;
    if (dir_x > 0.0)
        shot.facing = 0;
    else if (dir_y > 0.0)
        shot.facing = 1;
    else if (dir_x < 0.0)
        shot.facing = 2;
    else
        shot.facing = 3;
    std::printf("engine: fire: %s -> %s (damage %d, range %d)\n", shooter.name,
                shot.name, shot.damage, shot.range);
    return true;
}

void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
               Entity &shot, Feedback &feel, const EntityDef &spark,
               double dt)
{
    /* The flight, in game time: the shot's speed over dt, sub-stepped
       through the mover so each sub-step is small. What is checked at
       every position — including the one it is fired at, so a shot fired
       point-blank lands — is what a projectile must notice: the entity
       it hit, a wall (the step refused), and its range running out. */
    double left = (double)shot.speed * dt;
    for (;;) {
        /* The first actor the shot overlaps, other than itself, its
           owner, and its owner's kind. Slots are checked in order; the
           first hit is the hit. A shot flies through other shots — and
           through its own kind: a wave's copies do not shoot each
           other down. A shot's targets are the other kinds. */
        Entity *target = 0;
        for (int i = 0; i < ENTITY_CAP && !target; ++i) {
            Entity &e = store.slots[i];
            if (!e.live || &e == &shot || &e == shot.owner)
                continue;
            if (e.behavior == BEHAVIOR_FLY)
                continue;
            if (e.behavior == BEHAVIOR_SETTLE)
                continue; /* a spark is cosmetic — a shot flies through
                            it, the way it flies through other shots */
            if (shot.owner && SameName(e.name, shot.owner->name))
                continue;
            if (Overlaps(shot, e))
                target = &e;
        }
        if (target) {
            /* The hit: the target's health falls by the row's damage,
               and the shot is spent on it. */
            int was = target->health;
            target->health -= shot.damage;
            if (target->health < 0)
                target->health = 0;
            std::printf("engine: hit: %s hits %s — damage %d, health %d -> %d\n",
                        shot.name, target->name, shot.damage, was,
                        target->health);

            /* Lesson 092: feedback starts with the event. The hit is one
               of the toolkit's triggers, and the toolkit answers here —
               in the hit's own frame, on this very line of the flight —
               a short hitstop and a small shake. Not on a clock, not a
               frame later: the hit and its weight are one moment. */
            FeelHitstop(feel, 0.25, 0.15);
            FeelShake(feel, 5.0, 0.25);

            /* Lesson 093: and the impact scatters sparks — a burst of
               particles from the same event's own frame. */
            FeelBurst(store, spark, shot.x, shot.y, 4);

            std::printf("engine: shot %s retired — hit %s\n", shot.name,
                        target->name);
            EntityRetire(store, shot);
            if (target->health == 0 && target != &hero) {
                /* A zero-health entity is retired — the hero excepted:
                   its zero health is the game's defeat condition, which
                   the state machine reads; the game's actor is not
                   retired out from under the game. */
                std::printf("engine: %s retired — zero health\n",
                            target->name);
                EntityRetire(store, *target);
            }
            if (target->health == 0) {
                /* Lesson 092: a death is the toolkit's heavier event —
                   the same two hooks, weighted for it, fired in the same
                   frame as the hit that made it. A killing blow answers
                   as both, and the death's weights win. */
                FeelHitstop(feel, 0.25, 0.30);
                FeelShake(feel, 10.0, 0.50);

                /* Lesson 093: and the death bursts harder — the thing
                   that fell scatters its sparks from its own centre. */
                FeelBurst(store, spark,
                          target->x + ANIM_FRAME_W / 2.0,
                          target->y + target->sprite->height / 2.0, 8);
            }
            return;
        }

        if (left <= 0.0)
            return; /* this frame's flight is spent */
        if (shot.traveled >= shot.range) {
            /* The range is the shot's life: a projectile that has flown
               its row's range retires in the air — after its last look
               at what it might have hit. */
            std::printf("engine: shot %s retired — range\n", shot.name);
            EntityRetire(store, shot);
            return;
        }

        double step = left < COMBAT_STEP ? left : COMBAT_STEP;
        double was_x = shot.x, was_y = shot.y;
        MoveEntity(map, shot, shot.move_x * step, shot.move_y * step);
        bool moved_x = shot.move_x == 0.0 || shot.x != was_x;
        bool moved_y = shot.move_y == 0.0 || shot.y != was_y;
        if (!moved_x || !moved_y) {
            /* The mover refused the step where the shot meant to go: a
               projectile that meets a wall retires at it. */
            std::printf("engine: shot %s retired — wall\n", shot.name);
            EntityRetire(store, shot);
            return;
        }
        shot.traveled += step;
        left -= step;
    }
}

void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
                  const Entity &hero, double dt)
{
    /* An unarmed entity never fires — and the hero is never its own
       attacker: its trigger is the player's (HeroFire). */
    if (!e.fires[0] || e.rate <= 0 || &e == &hero)
        return;
    if (e.cooldown > 0.0) {
        e.cooldown -= dt;
        return;
    }

    /* The shot's reach is its kind's row: an attacker threatens only
       as far as its shot flies, and stays quiet beyond it. */
    DefResult kind = TableFind(shots, e.fires);
    if (kind.error != DEF_OK)
        return;
    double dx = hero.x - e.x, dy = hero.y - e.y;
    double reach = (double)kind.def->range;
    if (dx * dx + dy * dy > reach * reach)
        return;

    /* The attack aims the way everything else does: the compass point
       at the hero. */
    double dir_x = 0.0, dir_y = 0.0;
    CombatAim(dx, dy, dir_x, dir_y);
    if (dir_x == 0.0 && dir_y == 0.0)
        return;
    if (CombatFire(store, shots, e, dir_x, dir_y))
        e.cooldown = 60.0 / (double)e.rate;
}

} /* namespace engine */
