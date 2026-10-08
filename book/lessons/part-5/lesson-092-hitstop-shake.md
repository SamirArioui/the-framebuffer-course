# Lesson 092 — hitstop and screenshake

{{#include ../../stability-horizon.md}}

## Prose

Lesson 086 built two hooks and left them waiting: a hitstop that drops
the game-time scale to a fraction and brings it back on its own
wall-time deadline, and a screenshake that moves the camera's additive
offset and rests it at exactly zero. It fired both once from a wall-time
script so their fire-and-rest life would be visible — and said plainly
that the script was a stand-in. This lesson is what the hooks were built
for: **the game's own events fire them**, and one rule organizes
everything that follows.

> **Feedback starts with the event.** A feel effect begins in the frame
> its triggering event happens.

Not on a clock, not a beat later, not "when convenient": the hit and its
weight are one moment, because the player must read cause and effect as
one moment. The rest of this lesson is that rule made concrete — the
events, the weights, and the frame order that keeps the effect inside
the event's frame.

### The events: a hit lands, a death falls

The toolkit answers two events in this lesson, both of them already
happening in the combat's flight (`CombatFly`): **a hit lands** (the
flight's hit branch, where the target's health falls by the shot's
damage) and **a death falls** (the same branch, where a target reaches
zero health). The hit is a short hitstop and a small shake — a quarter
of game time for 0.15 s, a 5 px shake for 0.25 s. The death is the same
two hooks, heavier — a quarter for 0.30 s, a 10 px shake for 0.50 s. A
killing blow answers as both events at once, and the death's weights
win, because firing again is just the toolkit saying *this one is
heavier*.

Here is the rule visible in a run. The real roster, the hero holding
still while wave 1's two bats close in and fire — two bolts land in one
frame, and the toolkit fires beside each hit line:

```
engine: fire: bat -> bolt (damage 1, range 160)
engine: hit: bolt hits hero — damage 1, health 3 -> 2
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: shot bolt retired — hit hero
engine: hit: bolt hits hero — damage 1, health 2 -> 1
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: shot bolt retired — hit hero
frame 102: step 44.293 ms, …
```

Every `fired` line sits between frame 101's account and frame 102's —
the events and their effects are inside **one frame's** account, frame
102's. That is the rule, measurable: no `frame` line separates a hit
from the feedback it caused.

And three seconds later, the third bolt does the killing blow on the
hero — a hit and a death in one frame:

```
engine: hit: bolt hits hero — damage 1, health 1 -> 0
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: shot bolt retired — hit hero
engine: feel: hitstop fired (0.25x, 0.30s)
engine: feel: shake fired (10 px, 0.50s)
engine: hit: bolt hits hero — damage 1, health 0 -> 0
…
engine: state play -> death (the hero's health reached zero)
```

(Honest edge, measured and not hidden: the volley's second bolt lands on
a hero already at zero and answers with feedback again. The hit rule
asks "is it live", the game's actor is never retired — so the corpse
takes one more hit before the state machine reads the defeat condition
on the next frame. One frame of double feedback; the event is still
answered in its own frame.)

The same events fire when the hero is the one doing the killing — a
scratch roster of one fragile, standing kind at the hero's feet, the
blaster's point-blank bolt (the numbers are the scratch's own):

```
engine: fire: hero -> bolt (damage 1, range 160)
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: shot bolt retired — hit bag
engine: bag retired — zero health
engine: feel: hitstop fired (0.25x, 0.30s)
engine: feel: shake fired (10 px, 0.50s)
```

One frame, one killing blow, four firings — the hit's weights, then the
death's over them.

### The frame order the rule demands

Firing in the event's frame is not enough on its own; the frame must
also *show* the effect. So the toolkit's per-frame run (`FeelUpdate`,
lesson 086's decay-and-drive) moved: it used to run before the world
advanced, and now it runs **after the frame's events and before the
frame is drawn** — after the walk that fired it, beside the camera's
follow, before the render. Run it before the walk and the hit's shake
would reach the camera one frame late — the picture would lag the
event, which is exactly what the rule forbids. Run it after the draw
and the effect would miss the event's picture entirely.

One honest limit, visible in the same excerpt: frame 102's `step` is
`44.293 ms` — full speed. The step is the frame's *advance*, already
taken before the walk that finds the hit; a step already spent cannot be
shrunk retroactively. What the event's frame does carry is the fire
itself (above), the shake in its picture (the offset is driven before
the draw), and the hitstop's factor already down — the very next step is
the slowed one.

### The hitstop ends on its own

The hitstop's deadline is wall time — lesson 078's rule: anything that
must *end* while the game is stopped cannot run on game time, and a
hitstop's whole point is to end. The scale drops to a quarter and comes
back without the game intervening, and the frame log measures it:

```
frame 103: step 10.779 ms, …
frame 104: step 11.001 ms, …
engine: feel: hitstop rested — full speed again
frame 105: step 10.808 ms, …
frame 106: step 43.493 ms, …
```

Three frames at `10.8 ms` against the run's paced `43 ms` — the quarter
the hitstop fired at — and then `43.493 ms` again: full speed, no help
from anyone. (Frame 105 is still slow although the rest is printed
before it: its step was computed at its start, and the rest lands in its
account.) Measured against the run's wall clock, the fire at `t=32.265`
and the rest at `t=32.389` are 124 ms apart for the 0.15 s asked — the
deadline answered at the frame's granularity, within one frame, on this
run's ~44 ms paced frames; at 60 fps the same granularity is under 17
ms.

### The shake rests at exactly zero

The shake drives the camera's additive offset — the hook lesson 054
left in the camera — and the rule that matters is where it *ends*:
exactly zero, so the world's view is never left a pixel off. The rest
line says so in every run: `engine: feel: shake rested at 0,0`. And
between the fire and the rest the offset really moves — a scratch probe
printing the offset the draw uses (stated as such: a probe, not the
shipped run) shows it alternating while the shake lasts:

```
engine: probe: camera add 10,0
engine: probe: camera add 10,0
engine: probe: camera add -10,0
engine: probe: camera add -10,0
…
engine: feel: shake rested at 0,0
```

`10` is the death's own magnitude — the heavier shake of the killing
blow, in pixels, on the camera's offset — and the settle is the hook's
own `0,0`, exact.

### What this run verified, and what it did not

- **Feedback starts with the event** — every `fired` line sits inside
  the event's frame's account, with no `frame` line between a hit and
  its feedback; the shake's offset is driven before the frame draws.
- **The hitstop slows the simulation to a fraction and returns to full
  speed on its own wall-time deadline** — `step 10.8 ms` against the
  paced `43 ms` for three frames, then `43.5 ms` again, the rest line
  between them; the deadline answered within one frame of its 0.15 s
  (measured 124 ms of wall time on ~44 ms frames).
- **The shake's offset rests at exactly zero** — `shake rested at 0,0`,
  with the probe showing the offset moving ±10 px while the effect
  lasts.

What this run did **not** verify is whether the weights *feel* right —
a quarter-second stop and a 5 px shake are numbers; whether a hit lands
with the right weight is a judgment this headless run cannot make. That
judgment is the player's, and it is the one thing about game feel that
measuring cannot settle. What the run also cannot show is the shaken
picture itself (there is no screen to look at here) — it verifies the
offset the picture is drawn at, which is the same thing by construction.

## Code step

One change: the toolkit fires from the events. `src/combat.cpp`'s flight
fires `FeelHitstop`/`FeelShake` at the hit and death lines — the event's
own frame, the event's own weights — and `CombatFly` (with `GameWalk`
passing it along) gains `Feedback &feel` to fire through. `src/main.cpp`
loses the wall-time demonstration script and moves the toolkit's run
(`FeelUpdate`) to after the frame's events and before its draw;
`src/feel.cpp`'s hooks report their own firing beside the event's lines,
the demonstration's print dying with the demonstration. The hooks
themselves — the fire-and-rest mechanisms of lesson 086 — are untouched.
Its end state is tagged `lesson-092`.

```diff
diff --git a/src/combat.cpp b/src/combat.cpp
index f894460..a10ffb7 100644
--- a/src/combat.cpp
+++ b/src/combat.cpp
@@ -5,6 +5,9 @@
 // row names; the flight is the projectile's speed over game time; the
 // life is its row's range. Nothing here knows which weapon or which
 // projectile exists — the tables know that.
+//
+// Lesson 092: the hit and the death are the juice toolkit's events, and
+// the feedback hooks fire at the lines where they happen.
 
 #include "combat.h"
 
@@ -113,7 +116,7 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
 }
 
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
-               Entity &shot, double dt)
+               Entity &shot, Feedback &feel, double dt)
 {
     /* The flight, in game time: the shot's speed over dt, sub-stepped
        through the mover so each sub-step is small. What is checked at
@@ -149,6 +152,15 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
             std::printf("engine: hit: %s hits %s — damage %d, health %d -> %d\n",
                         shot.name, target->name, shot.damage, was,
                         target->health);
+
+            /* Lesson 092: feedback starts with the event. The hit is one
+               of the toolkit's triggers, and the toolkit answers here —
+               in the hit's own frame, on this very line of the flight —
+               a short hitstop and a small shake. Not on a clock, not a
+               frame later: the hit and its weight are one moment. */
+            FeelHitstop(feel, 0.25, 0.15);
+            FeelShake(feel, 5.0, 0.25);
+
             std::printf("engine: shot %s retired — hit %s\n", shot.name,
                         target->name);
             EntityRetire(store, shot);
@@ -161,6 +173,14 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                             target->name);
                 EntityRetire(store, *target);
             }
+            if (target->health == 0) {
+                /* Lesson 092: a death is the toolkit's heavier event —
+                   the same two hooks, weighted for it, fired in the same
+                   frame as the hit that made it. A killing blow answers
+                   as both, and the death's weights win. */
+                FeelHitstop(feel, 0.25, 0.30);
+                FeelShake(feel, 10.0, 0.50);
+            }
             return;
         }
 
diff --git a/src/combat.h b/src/combat.h
index 77df2ba..fddb88a 100644
--- a/src/combat.h
+++ b/src/combat.h
@@ -16,6 +16,7 @@
 #define COMBAT_H
 
 #include "entity.h"
+#include "feel.h"
 #include "table.h"
 
 namespace engine {
@@ -56,9 +57,11 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
    health by the shot's damage and retires the shot; a zero-health target
    is retired too — the hero excepted, whose zero health is the game's
    defeat condition (the state machine reads it, the game's actor is not
-   retired out from under the game). */
+   retired out from under the game). Lesson 092: the hit and the death
+   are the toolkit's events — the feedback hooks fire here, in the
+   event's own frame, through `feel`. */
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
-               Entity &shot, double dt);
+               Entity &shot, Feedback &feel, double dt);
 
 /* Lesson 090: the enemy attack, once per frame of game time. An armed
    entity — one whose row names a projectile kind — fires it at the
diff --git a/src/feel.cpp b/src/feel.cpp
index 2651e0c..76eb89a 100644
--- a/src/feel.cpp
+++ b/src/feel.cpp
@@ -1,8 +1,10 @@
 // feel.cpp — the feedback hooks: fire, decay, rest.
 //
 // Lesson 086: each hook fires, runs down its own wall-time, and returns
-// exactly to rest. Nothing here decides *when* to fire — that is the
-// juice toolkit's job (lessons 092-093), reading the game's events.
+// exactly to rest. Lesson 092: the game's own events fire them — a hit
+// lands, a death falls — in the event's own frame, and each hook says
+// when it fires beside the event's own line. The weights are the
+// event's, passed in from where the event happens.
 
 #include "feel.h"
 
@@ -22,12 +24,16 @@ void FeelShake(Feedback &feel, double magnitude, double seconds)
 {
     feel.shake = seconds;
     feel.shake_mag = magnitude;
+    std::printf("engine: feel: shake fired (%.0f px, %.2fs)\n", magnitude,
+                seconds);
 }
 
 void FeelHitstop(Feedback &feel, double fraction, double seconds)
 {
     feel.hitstop = seconds;
     feel.hitstop_k = fraction;
+    std::printf("engine: feel: hitstop fired (%.2fx, %.2fs)\n", fraction,
+                seconds);
 }
 
 double FeelTimeScale(const Feedback &feel)
diff --git a/src/feel.h b/src/feel.h
index d8706e2..5827080 100644
--- a/src/feel.h
+++ b/src/feel.h
@@ -1,10 +1,12 @@
-// feel.h — the feedback hooks the juice toolkit will drive.
+// feel.h — the feedback hooks the juice toolkit drives.
 //
 // Lesson 086: two hooks — a screenshake and a hitstop — each a thing
-// that fires and then rests. These are the hooks the juice toolkit
-// (lessons 092-093) will drive from the game's events: here they are the
-// mechanisms, each with its own fire-and-rest life, and nothing yet says
-// when to fire them. A hook at rest costs nothing and changes nothing.
+// that fires and then rests. Lesson 092: the toolkit fires them from the
+// game's own events — a hit lands, a death falls — in the event's own
+// frame, so the player reads cause and effect as one moment. The hooks
+// are the mechanisms, each with its own fire-and-rest life; the events
+// decide when and how heavily. A hook at rest costs nothing and changes
+// nothing.
 #ifndef FEEL_H
 #define FEEL_H
 
diff --git a/src/game.cpp b/src/game.cpp
index a2f87fe..3da68a4 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -218,7 +218,7 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 }
 
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             const EntityTable &shots, double dt)
+             const EntityTable &shots, Feedback &feel, double dt)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -237,7 +237,11 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
        entity's movement request the way the player's input writes the
        hero's. One branch on the behavior, per-entity work expressed
        once: the boss (lesson 090) is one more value here, not one more
-       shape. */
+       shape.
+
+       Lesson 092: the flight's events — a hit lands, a death falls —
+       fire the feedback hooks in their own frame (the toolkit is handed
+       along through `feel`). */
     int visited = 0;
     for (int i = 0; i < ENTITY_CAP; ++i) {
         if (!store.slots[i].live)
@@ -246,7 +250,7 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
         Entity &e = store.slots[i];
         switch (e.behavior) {
         case BEHAVIOR_FLY:
-            CombatFly(map, store, hero, e, dt);
+            CombatFly(map, store, hero, e, feel, dt);
             continue; /* the flight moves itself, through the mover */
         case BEHAVIOR_CHASE:
             AiChase(e, hero);
diff --git a/src/game.h b/src/game.h
index e171716..30dcc3e 100644
--- a/src/game.h
+++ b/src/game.h
@@ -22,6 +22,7 @@
 
 #include "camera.h"
 #include "entity.h"
+#include "feel.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "gametime.h"
@@ -111,10 +112,12 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
    walls, at its range, at what it hits) instead of the request. Lesson
    089-090: the enemy behaviors and the boss's pattern write the request
    the way the player's input writes the hero's, and an armed entity
-   attacks at its row's rate. The hero is handed along for the combat's
-   rules to know the game's actor by. Returns the visit count. */
+   attacks at its row's rate. Lesson 092: the combat's events (a hit, a
+   death) fire the feedback hooks through `feel`, in their own frame.
+   The hero is handed along for the combat's rules to know the game's
+   actor by. Returns the visit count. */
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             const EntityTable &shots, double dt);
+             const EntityTable &shots, Feedback &feel, double dt);
 
 /* Lesson 091: the waves, once per frame of play. A fresh fight clears
    the last one from the store; a wave spawns its composition from the
diff --git a/src/main.cpp b/src/main.cpp
index c460681..a115e30 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -309,12 +309,12 @@ int Run(void)
     GameInit(game, hero.health);
 
     /* Lesson 086: the feedback hooks — a screenshake and a hitstop, both
-       at rest. The juice toolkit (lessons 092-093) will fire these from
-       the game's events; here a demonstration fires both once so their
-       fire-and-rest life is visible. */
+       at rest. Lesson 092: the juice toolkit fires them from the game's
+       own events now — a hit lands, a death falls — in the event's own
+       frame (the walk's flight, in game.cpp/combat.cpp); the wall-time
+       demonstration that used to fire them here is gone. */
     Feedback feel;
     FeelInit(feel);
-    bool feel_demo = false;
 
     /* Lesson 080: the vertical slice — the game's shape, and nothing
        else. The hero is the row the game asks for by name (it is the
@@ -492,20 +492,6 @@ int Run(void)
            arrows) and left at rest in every other state. */
         GameInput(game, opened.window, hero, wall_dt);
 
-        /* Lesson 086: the feedback hooks run on their own wall-time —
-           each fires, decays, and rests. The demonstration fires both
-           once, in play, so their fire-and-rest life is visible; the
-           juice toolkit (lessons 092-093) will fire them from the game's
-           events instead of this script. */
-        if (!feel_demo && game.state == GAME_PLAY &&
-            platform::Now() - started >= 3.0) {
-            feel_demo = true;
-            FeelShake(feel, 6.0, 0.5);
-            FeelHitstop(feel, 0.25, 0.4);
-            std::printf("engine: feel: shake fired (6 px, 0.5s), hitstop fired (0.25x, 0.4s)\n");
-        }
-        FeelUpdate(feel, wall_dt, game.camera);
-
         /* The game-time scale is the state's (play runs, the rest hold)
            times the hitstop's factor (a fraction during a hitstop, full
            at rest) — one knob, two drivers, multiplied. */
@@ -547,7 +533,7 @@ int Run(void)
             GameWaves(game, store, foes);
 
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, hero, shots, dt);
+        int visited = GameWalk(store, map, hero, shots, feel, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
@@ -618,6 +604,15 @@ int Run(void)
            in game.cpp); the loop keeps no camera of its own. */
         GameFollow(game, hero, map);
 
+        /* Lesson 086: the feedback hooks run on their own wall-time —
+           each fires, decays, and rests. Lesson 092 moved the run to
+           here, after the frame's events and before the frame is drawn:
+           the toolkit settles whatever the walk fired this frame, so a
+           hit's shake is already in the hit's own frame's picture. Each
+           hook returns to rest on its own — the hitstop to full speed,
+           the shake's additive offset to exactly zero. */
+        FeelUpdate(feel, wall_dt, game.camera);
+
         frame.update = platform::Now() - t0;
 
         /* Lesson 060: the audio step — the loop feeds the device the next
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the shake that settles *(extend-the-code)*

The screenshake's offset swings at full magnitude and cuts to zero the
moment its life ends — and the cut is visible: the world snaps back
instead of coming to rest. Make the shake **settle** — its magnitude
decays across the shake's life so the camera eases back to rest, and
the offset still rests at **exactly zero** at its deadline (that rule is
the one that must hold; the shake is still one effect, screenshake, not
a new one). The `Feedback` state may grow what it needs to divide by.
Then show the settle in a run — the run must report the offset while
the shake is firing (the hooks already report their fire and their
rest; the in-between is yours to print) — and quote the ramp down to
the rest.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-092/ex1.md)

### Exercise 2 — the pause that meets the hitstop *(predict-the-output)*

The game-time scale is the state's scale times the hitstop's factor —
one knob, two drivers, multiplied. Before running anything, predict
what happens when the player pauses **while a hitstop is still
running**: what the frame log's `step` column shows before, during, and
after the pause; where the `hitstop rested` line falls relative to the
pause; and whether the game resumes at full speed or with the hitstop
still hanging over it. Write the prediction down. Then run it — pause
right after a hit lands, with a probe beside the run that prints both
factors and the hitstop's remaining wall time every frame of the window
— and compare against your prediction, explaining the product your
probe measured.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-092/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 091 — waves](lesson-091-waves.md) ·
**Next:** [Lesson 093 — particle bursts and easing](lesson-093-bursts-easing.md) ·
**Code tag:** [`lesson-092`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-092)
