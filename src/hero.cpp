// hero.cpp — the hero's movement: intent in, eased motion out.
//
// Lesson 085: the whole of the hero's feel is here — the diagonal
// normalized to the straight-line speed, and the velocity eased toward
// the intent (or toward rest) so the hero reads as a thing with weight.

#include "hero.h"

#include "combat.h"

namespace engine {

void HeroMove(Entity &hero, platform::Window *window, double dt)
{
    /* The player's intent: the held direction, from polled input state
       (lesson 032) — one step per frame, no events. */
    double want_x = 0.0, want_y = 0.0;
    if (platform::KeyDown(window, platform::KEY_LEFT))
        want_x -= 1.0;
    if (platform::KeyDown(window, platform::KEY_RIGHT))
        want_x += 1.0;
    if (platform::KeyDown(window, platform::KEY_UP))
        want_y -= 1.0;
    if (platform::KeyDown(window, platform::KEY_DOWN))
        want_y += 1.0;

    /* The intent is a direction. Normalized — a diagonal is scaled by
       1/sqrt(2) — so the hero covers ground at the straight-line speed
       whichever of the eight directions it runs in. */
    double intent_x = want_x, intent_y = want_y;
    if (intent_x != 0.0 && intent_y != 0.0) {
        intent_x *= HERO_DIAG;
        intent_y *= HERO_DIAG;
    }

    /* The ease: the velocity closes on the intent by dt/accel each frame
       — toward the intent when the player steers (acceleration), toward
       rest when they let go (deceleration). A turn passes through the
       ease instead of snapping to full speed the other way. The feel is
       the hero's own `accel` — its row's fact, milliseconds (lesson
       087); an accel of 0 is instant weightless motion. The hero's
       movement request carries the eased velocity; the walk turns it
       into motion (move x speed = the velocity). */
    double k = hero.accel > 0 ? dt / (hero.accel / 1000.0) : 1.0;
    if (k > 1.0)
        k = 1.0;
    hero.move_x += (intent_x - hero.move_x) * k;
    hero.move_y += (intent_y - hero.move_y) * k;

    /* Lesson 086: the walk cycle — the frame advances while the hero
       steps, one frame per ANIM_STEP, and holds at frame 0 at rest. The
       sheet's frame count is its width over one frame's width. */
    if (want_x != 0.0 || want_y != 0.0) {
        hero.frame_t += dt;
        if (hero.frame_t >= ANIM_STEP) {
            hero.frame_t -= ANIM_STEP;
            int count = hero.sprite->width / ANIM_FRAME_W;
            if (count < 1)
                count = 1;
            hero.frame = (hero.frame + 1) % count;
        }
    } else {
        hero.frame = 0;
        hero.frame_t = 0.0;
    }
}

void HeroFire(Entity &hero, platform::Window *window,
              const EntityTable &weapons, const EntityTable &shots,
              EntityStore &store, double dt, Sound &sound)
{
    /* Lesson 087: the number keys arm the weapons table's rows. A weapon
       is a row — arming carries its values — so the weapons grow as
       rows: one row more is one key more, and no weapon code. */
    if (platform::KeyPressed(window, platform::KEY_1) && weapons.count > 0)
        CombatArm(hero, weapons.rows[0]);
    if (platform::KeyPressed(window, platform::KEY_2) && weapons.count > 1)
        CombatArm(hero, weapons.rows[1]);

    /* The rate is the row's, in rounds per minute: the trigger answers
       again only when the cooldown it earns has run out. At most one
       shot per frame — a long frame is caught up by the next shot, never
       by a burst of them. */
    if (hero.cooldown > 0.0) {
        hero.cooldown -= dt;
        return;
    }
    if (!platform::KeyDown(window, platform::KEY_SPACE))
        return;
    if (hero.rate <= 0 || !hero.fires[0])
        return; /* unarmed, or a row that never fires */

    /* The aim: the compass point of the hero's motion — the eight
       directions, a diagonal at 1/sqrt(2) — or its facing at rest. */
    double dir_x = 0.0, dir_y = 0.0;
    CombatAim(hero.move_x, hero.move_y, dir_x, dir_y);
    if (dir_x == 0.0 && dir_y == 0.0) {
        if (hero.facing == 0)
            dir_x = 1.0;
        else if (hero.facing == 1)
            dir_y = 1.0;
        else if (hero.facing == 2)
            dir_x = -1.0;
        else
            dir_y = -1.0;
    }
    if (CombatFire(store, shots, hero, dir_x, dir_y, sound))
        hero.cooldown = 60.0 / (double)hero.rate;
}

} /* namespace engine */
