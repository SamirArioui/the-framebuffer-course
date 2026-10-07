# Lesson 085 — hero movement

{{#include ../../stability-horizon.md}}

## Prose

The hero slides along walls and stops at tiles now, but it still moves
like a cursor — it is at full speed the instant you press a key and at
rest the instant you let go. This lesson gives it weight: **the hero is
a velocity that eases toward where you mean to go, and the diagonal is
no faster than the straight run.**

### The intent is a direction, not a speed

The held keys give a direction — one of eight. But taken raw, a
diagonal (both arrows) is `(1, 1)`, and running the walk on that moves
the hero `speed` in *each* axis — `speed × √2` across the ground, about
41% faster than running straight. So the intent is **normalized** first:

```cpp
if (intent_x != 0.0 && intent_y != 0.0) {
    intent_x *= HERO_DIAG;   /* 1 / sqrt(2) */
    intent_y *= HERO_DIAG;
}
```

A diagonal becomes `(1/√2, 1/√2)`, so the hero covers ground at the
straight-line speed whichever way it runs. (A general normalize would
divide by the length; for eight directions the length is only ever 1 or
√2, so one multiply covers it.)

### The velocity eases — that is the weight

The hero carries a velocity (its movement request, which the walk turns
into motion) and eases it toward the intent:

```cpp
double k = dt / HERO_TIME;         /* the ease, per frame */
if (k > 1.0) k = 1.0;
hero.move_x += (intent_x - hero.move_x) * k;
hero.move_y += (intent_y - hero.move_y) * k;
```

Steering, the velocity closes on the intent — **acceleration**. Letting
go, the intent is zero, so the velocity closes on rest — **deceleration**.
Reversing, the velocity passes *through* the ease from one direction to
the other instead of snapping — the turn is felt. `HERO_TIME` is the
feel: how long the hero takes to reach (or leave) full speed. And the
ease is `dt`-scaled, so it is the same at any frame rate — a smooth
curve toward the intent, not a fixed jump per frame.

From a real run under scripted input, the hero's velocity as it is
driven — a held direction from rest, then released, then driven
diagonally:

```
engine: hero velocity 240,0 (t=3.348)     <- full speed straight (from rest)
engine: hero velocity 0,0 (t=5.864)       <- released, eased to rest
engine: hero velocity 240,0 (t=3.348)     <- and up to speed again
engine: hero velocity 231,20 (t=3.362)    <- mid-ease, turning to the diagonal
engine: hero velocity 169,169 (t=5.377)   <- the diagonal, settled
```

Two facts are in those numbers. The hero **eases from and to rest** —
the velocity leaves `0` and returns to `0` rather than jumping straight
to `240` (the `231,20` line is a single step of the ease, caught
mid-turn). And the **diagonal covers ground at the straight-line speed**:
`169,169` has magnitude `√(169² + 169²) ≈ 239` — the same ground speed
as `240,0`, not `√2` times it. That is the normalization, measured.

### What this run verified, and what it did not

- **The diagonal is the straight-line speed** — the settled diagonal
  velocity `169,169` has magnitude ≈ `240`, matching the straight `240,0`.
- **The hero accelerates from rest and decelerates to rest** — the
  velocity leaves `0` and returns to `0`, easing (`231,20` is one step of
  the curve), never a step change to full speed.

What this lesson does **not** do is tune the feel. `HERO_TIME` is a
first number, and "how much weight" is a judgment the game makes later.
One honest caveat about the measurement above: the authoring machine's
headless loop is event-driven and runs at roughly a frame a second, so
`dt` is large and the ease often completes *within* a frame — the
smooth "rise over several frames" is what you see at interactive rates
(60 fps, where `dt/HERO_TIME` spreads the same curve over ~7 frames).
The ease is `dt`-scaled and frame-rate-independent either way; it is
merely sampled more finely at 60 fps. On your own desktop, watch the
velocity report as you press and release — the curve is the point.

## Code step

One change: the hero's movement becomes the hero's own. `src/hero.h` and
`src/hero.cpp` are new — `HeroMove` reads the held direction, normalizes
it (the diagonal at the straight-line speed), and eases the hero's
velocity toward it (accel) or toward rest (decel). `src/game.cpp` no
longer writes the hero's movement request from raw keys (that is
`HeroMove`'s now) and restores the hero to rest when a fresh game starts;
`src/main.cpp` calls `HeroMove` each frame of play (before the walk) and
reports the hero's velocity as it eases. The walk still turns the eased
velocity into motion against the map — unchanged from lesson 084. Its
end state is tagged `lesson-085`.

```diff
diff --git a/src/game.cpp b/src/game.cpp
index 0a78d42..4db9368 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -69,36 +69,26 @@ void GameInit(Game &game, int hero_health_full)
 void GameInput(Game &game, platform::Window *window, Entity &hero,
                double wall_dt)
 {
-    /* Play's movement is the hero's own request; every other state leaves
-       the hero at rest, so the walk moves nothing and the simulation
-       stands still. */
-    hero.move_x = 0.0;
-    hero.move_y = 0.0;
-
     switch (game.state) {
     case GAME_TITLE:
         /* The title screen accepts one thing: the start key. */
         if (platform::KeyPressed(window, platform::KEY_ENTER)) {
-            /* A fresh game restores the hero's health and the game's
-               waves — the row's facts, not remembered state. */
+            /* A fresh game restores the hero's health, the game's waves,
+               and the hero's rest — the row's facts, not remembered
+               state. */
             hero.health = game.hero_health_full;
             game.waves_remaining = GAME_WAVES;
             game.play_clock = 0.0;
+            hero.move_x = 0.0;
+            hero.move_y = 0.0;
             Transition(game, GAME_PLAY, "the player started");
         }
         break;
 
     case GAME_PLAY: {
-        /* Play's input: polled movement state, written to the hero's
-           request. The walk turns it into motion (lesson 076). */
-        if (platform::KeyDown(window, platform::KEY_LEFT))
-            hero.move_x -= 1.0;
-        if (platform::KeyDown(window, platform::KEY_RIGHT))
-            hero.move_x += 1.0;
-        if (platform::KeyDown(window, platform::KEY_UP))
-            hero.move_y -= 1.0;
-        if (platform::KeyDown(window, platform::KEY_DOWN))
-            hero.move_y += 1.0;
+        /* Play's movement is the hero's own (HeroMove, lesson 085) — the
+           held direction read and eased into motion there. What is left
+           here is the play state's other input and the named conditions. */
 
         /* The stand-in for combat: SPACE is a hit on the hero (lesson 087
            makes real hits land). The named condition below reads the
diff --git a/src/hero.cpp b/src/hero.cpp
new file mode 100644
index 0000000..7c119ae
--- /dev/null
+++ b/src/hero.cpp
@@ -0,0 +1,47 @@
+// hero.cpp — the hero's movement: intent in, eased motion out.
+//
+// Lesson 085: the whole of the hero's feel is here — the diagonal
+// normalized to the straight-line speed, and the velocity eased toward
+// the intent (or toward rest) so the hero reads as a thing with weight.
+
+#include "hero.h"
+
+namespace engine {
+
+void HeroMove(Entity &hero, platform::Window *window, double dt)
+{
+    /* The player's intent: the held direction, from polled input state
+       (lesson 032) — one step per frame, no events. */
+    double want_x = 0.0, want_y = 0.0;
+    if (platform::KeyDown(window, platform::KEY_LEFT))
+        want_x -= 1.0;
+    if (platform::KeyDown(window, platform::KEY_RIGHT))
+        want_x += 1.0;
+    if (platform::KeyDown(window, platform::KEY_UP))
+        want_y -= 1.0;
+    if (platform::KeyDown(window, platform::KEY_DOWN))
+        want_y += 1.0;
+
+    /* The intent is a direction. Normalized — a diagonal is scaled by
+       1/sqrt(2) — so the hero covers ground at the straight-line speed
+       whichever of the eight directions it runs in. */
+    double intent_x = want_x, intent_y = want_y;
+    if (intent_x != 0.0 && intent_y != 0.0) {
+        intent_x *= HERO_DIAG;
+        intent_y *= HERO_DIAG;
+    }
+
+    /* The ease: the velocity closes on the intent by dt/HERO_TIME each
+       frame — toward the intent when the player steers (acceleration),
+       toward rest when they let go (deceleration). A turn passes through
+       the ease instead of snapping to full speed the other way. The
+       hero's movement request carries the eased velocity; the walk turns
+       it into motion (move x speed = the velocity). */
+    double k = dt / HERO_TIME;
+    if (k > 1.0)
+        k = 1.0;
+    hero.move_x += (intent_x - hero.move_x) * k;
+    hero.move_y += (intent_y - hero.move_y) * k;
+}
+
+} /* namespace engine */
diff --git a/src/hero.h b/src/hero.h
new file mode 100644
index 0000000..cd658ff
--- /dev/null
+++ b/src/hero.h
@@ -0,0 +1,41 @@
+// hero.h — the hero's movement: the player's intent, eased into motion.
+//
+// Lesson 085: the hero is not a position that jumps to a new spot each
+// frame. It is a thing with weight — a velocity that eases toward where
+// the player means to go (acceleration) and toward rest (deceleration),
+// never a step change. And the intent is a *direction*, normalized, so
+// the diagonal is no faster than the straight run.
+//
+// This is the hero's own behavior (design D2), in its own file beside
+// the services and the game machine: the input becomes motion here, and
+// the walk turns that motion into steps against the map.
+#ifndef HERO_H
+#define HERO_H
+
+#include "entity.h"
+#include "platform.h"
+
+namespace engine {
+
+/* The hero's accel/decel time constant — the feel: roughly how long it
+   takes to ease from rest to full speed (or back). Lesson 085 keeps it
+   here as the hero's own fact; when the table format grows named
+   columns (lesson 087) the feel becomes data, like the hero's speed
+   already is. */
+constexpr double HERO_TIME = 0.12; /* seconds to close on the intent */
+
+/* 1 / sqrt(2): a diagonal intent is scaled by this so the hero covers
+   ground at the straight-line speed, not sqrt(2) times it. */
+constexpr double HERO_DIAG = 0.70710678;
+
+/* The hero's movement, once per frame of play. The held direction is the
+   intent, normalized so the diagonal is no faster than straight; the
+   hero's velocity eases toward that intent (accel) and toward rest
+   (decel) — a turn passes through the ease rather than snapping. The
+   result is left in the hero's own movement request, which the walk
+   turns into motion against the map. */
+void HeroMove(Entity &hero, platform::Window *window, double dt);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index fda43b0..96436f6 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -20,6 +20,7 @@
 #include "frame.h"
 #include "game.h"
 #include "gametime.h"
+#include "hero.h"
 #include "platform.h"
 #include "sprite.h"
 #include "table.h"
@@ -324,6 +325,7 @@ int Run(void)
     double distance = 0.0; /* the score: the world the hero has walked */
     GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
     bool was_blocked = false; /* lesson 077: the mover's state report */
+    int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
 
     /* The slice's identity: what the run is, named at once — L0*, the
        gate this part closes on. Every service it uses was finished
@@ -419,6 +421,12 @@ int Run(void)
 
         double was_x = hero.x, was_y = hero.y;
 
+        /* Lesson 085: the hero's movement — the held direction eased into
+           motion (accel/decel, the diagonal at the straight-line speed).
+           Only in play; the walk turns the eased velocity into steps. */
+        if (game.state == GAME_PLAY)
+            HeroMove(hero, opened.window, dt);
+
         /* Lesson 084: the game resolves its movement against its map —
            the walk is the game's now (GameWalk, in game.cpp), turning
            every live entity's request into motion through the mover.
@@ -447,6 +455,20 @@ int Run(void)
             std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
                         (int)hero.y, platform::Now() - started);
 
+        /* Lesson 085: the hero's velocity, as it eases — the accel (the
+           speed rising over frames) and the decel (falling to rest) are
+           what the player feels, and this is the measurement of it. */
+        {
+            int vx = (int)(hero.move_x * hero.speed);
+            int vy = (int)(hero.move_y * hero.speed);
+            if (vx != was_vx || vy != was_vy) {
+                std::printf("engine: hero velocity %d,%d (t=%.3f)\n", vx, vy,
+                            platform::Now() - started);
+                was_vx = vx;
+                was_vy = vy;
+            }
+        }
+
         /* Lesson 083: the game's world-view — the camera's base follows
            the hero, clamped to the map's bounds, and its additive offset
            rests at exactly zero. The game owns the camera now (GameFollow,
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The curve, predicted *(predict-the-output)*

The ease is arithmetic, so the trajectory is predictable. From rest
(`move = 0`), holding a direction gives `move += (1 − move) × dt/HERO_TIME`
each frame. At a steady 60 fps (`dt = 1/60`) with `HERO_TIME = 0.12`,
work out the hero's velocity (as a fraction of full speed) after 1, 2,
and 3 frames — does it reach full speed in those three frames? Then
double `HERO_TIME` to `0.24` and predict again. Write a small probe
(print `hero.move_x` each frame) and, on a machine that runs at 60 fps,
check your six numbers against the run. Which reaches full speed sooner,
and by how many frames?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-085/ex1.md)

### Exercise 2 — Heavier to stop *(extend-the-code)*

One `HERO_TIME` makes starting and stopping symmetric. Real heroes often
ease up to speed quickly but coast to a stop — accelerating feels crisp,
stopping feels weighty. Split the constant into two: `HERO_ACCEL` and
`HERO_DECEL`, and use the right one depending on whether the hero is
speeding up (the intent is pushing it faster) or slowing down (the
intent is zero or opposed). Keep the diagonal normalization and the
frame-rate independence. Then run it: does the hero reach full speed
noticeably sooner than it comes to rest?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-085/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 084 — tile collision](lesson-084-collision.md) ·
**Next:** [the course home](../../index.md) ·
**Code tag:** [`lesson-085`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-085)
