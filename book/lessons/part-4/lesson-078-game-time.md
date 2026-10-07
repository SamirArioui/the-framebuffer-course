# Lesson 078 — the game-time scale

{{#include ../../stability-horizon.md}}

## Prose

Every step this game has taken since lesson 035 has been multiplied by
one thing: the wall clock. `dt` is what the platform clock says passed,
and the simulation advances by exactly that. It is honest and it is
also inflexible — because the two effects every game of this kind wants
are both *the same request*: "the world should slow down". A pause
screen is the world at zero speed. Hitstop — Part 5's juice, that
half-second of near-stillness when a hit lands — is the world at a
fraction. And right now there is nothing to set. So this lesson's idea
is the hook Part 5 will drive: **the game-time scale** — one number the
game sets, from stopped to full speed, and the simulation's step is the
wall-clock step scaled.

### One number, three settings

```cpp
struct GameTime {
    double scale;
};

void GameTimeSetScale(GameTime &time, double scale);
double GameTimeStep(const GameTime &time, double wall_dt);
```

The knob is one `double`. `GameTimeSetScale` is how the game turns it —
and it runs from `0.0` to `GAMETIME_FULL` (1.0) and no further: a value
outside that range is clamped, so "faster than play" and "backwards
time" are not accidents a caller can have. The three settings the game
makes are the same call:

| Setting | Scale | The step |
| ------- | ----- | -------- |
| pause | 0 | none — the simulation stands still |
| hitstop | a fraction (the demo uses 0.25) | the wall step, halved and halved again |
| play | `GAMETIME_FULL` | the wall step, unchanged |

The scale is *a value the game owns*. Nothing in the engine sets it;
the demo's script does, exactly as Part 5's pause screen (L1) and
hitstop (L11) will. And the value in force for a frame is the one the
frame's step reflects — set the scale mid-frame and that frame's step
carries it, because the step is computed once, where the update begins.

### The step is the only thing it reaches

The update's first arithmetic is now two lines:

```cpp
double wall_dt = now - last;
double dt = GameTimeStep(game_time, wall_dt);
```

`dt` — the number the walk multiplies the hero's speed by — is game
time. At scale 0 it is 0 and the hero does not move; at 0.25 it is a
quarter of the wall's step and the hero crawls; at full speed it is the
wall's step and nothing about today's game changes. The scale reaches
that number and **nothing else**. The platform clock stays the measurer
lesson 035 taught it to be; the frame record keeps measuring the wall
clock's durations; the audio step still feeds the device at the buffer
horizon (sound does not slow down when the game pauses — a paused game
keeps its music). That last half is lesson 079's whole subject; this
lesson's claim is only that the step is where the scale lands.

Why a scale and not three mechanisms: a pause that is "the update does
not run" would leave the game unable to read its own input (how do you
*unpause*?), and a hitstop that is "a second, slower clock" would be two
clocks to reason about in a loop that lesson 035 argued should have
exactly one. One knob, turned to different positions, is pause and
hitstop and play.

### What this run verified, and what it did not

From a real run of this lesson's end state on this machine, driven by
scripted input while the demo's script turned the knob at three, five,
and seven seconds:

- **The step scales at 0, at a fraction, and at full speed.** The run
  names each transition with the step that comes out:
  `engine: game-time: scale 0.25 (hitstop) — step 3.807 ms of a 15.230
  ms wall step` — 15.230 × 0.25 = 3.807, to the digit;
  `engine: game-time: scale 0.00 (pause) — step 0.000 ms of a 1018.469
  ms wall step` — a wall step of a full second (the loop slept between
  input news) and a game step of nothing; `engine: game-time: scale
  1.00 (play) — step 15.122 ms of a 15.122 ms wall step` — the wall
  step, whole.
- **The world obeys it.** The hero's position reports show the scale at
  work: at play, one step of 3-4 pixels per pressed frame (`hero at
  705,232` … `709,232`); through hitstop, one *pixel* a frame (`hero at
  728,232`, `729`, `730`, `731` … — the same presses, a quarter of the
  motion); through pause, no movement at all between t 5.0 and t 7.0,
  and the run kept drawing and reporting the whole time.

What this lesson does **not** verify is the frame record — the claim
that measurement is not scaled. That is lesson 079, and it is the kind
of claim worth its own page: the difference between "the game is
paused" and "the machine is idle" is exactly what the frame record must
keep showing.

## Code step

One change for this lesson, from wall clock to game time:
`src/gametime.h` / `src/gametime.cpp` grow `GameTime`, the scale's
clamp, and `GameTimeStep` — the wall clock's step, scaled. `src/main.cpp`
grows the run around it: the knob in the run's bookkeeping (at play), the
demo's script turning it through hitstop, pause, and play at three, five,
and seven seconds, and the update's step computed through `GameTimeStep`
where `dt` used to be the raw clock difference. The walk, the mover, the
store, the sound, and the frame record are untouched — which is exactly
the point. Its end state is tagged `lesson-078`.

```diff
diff --git a/src/gametime.cpp b/src/gametime.cpp
new file mode 100644
index 0000000..75bff29
--- /dev/null
+++ b/src/gametime.cpp
@@ -0,0 +1,24 @@
+// gametime.cpp — the scale, and the step it makes.
+//
+// Lesson 078: two functions, one idea. The scale is a value the game
+// sets; the step is the wall clock's, multiplied.
+
+#include "gametime.h"
+
+namespace engine {
+
+void GameTimeSetScale(GameTime &time, double scale)
+{
+    if (scale < 0.0)
+        scale = 0.0;
+    if (scale > GAMETIME_FULL)
+        scale = GAMETIME_FULL;
+    time.scale = scale;
+}
+
+double GameTimeStep(const GameTime &time, double wall_dt)
+{
+    return wall_dt * time.scale;
+}
+
+} /* namespace engine */
diff --git a/src/gametime.h b/src/gametime.h
new file mode 100644
index 0000000..2948440
--- /dev/null
+++ b/src/gametime.h
@@ -0,0 +1,36 @@
+// gametime.h — game time: the wall clock, scaled by one knob.
+//
+// Lesson 078: the simulation does not advance by the wall clock. It
+// advances by game time, and game time is the wall clock's step scaled
+// by one number the game sets: 0 is pause, a fraction is hitstop, full
+// speed is play. One knob — not a special case threaded through the
+// update, and not a second clock.
+#ifndef GAMETIME_H
+#define GAMETIME_H
+
+namespace engine {
+
+/* Full speed: the scale's top, where a run starts and where play
+   lives. */
+constexpr double GAMETIME_FULL = 1.0;
+
+/* The game-time scale — the one number the game owns. */
+struct GameTime {
+    double scale;
+};
+
+/* The scale the game sets. It runs from 0 (paused) to GAMETIME_FULL
+   (play) and no further: a value outside that range is clamped, so
+   "faster than play" and "backwards" are not accidents a caller can
+   have. */
+void GameTimeSetScale(GameTime &time, double scale);
+
+/* The simulation's step: the wall clock's step, scaled. This is the
+   number the update advances the world by, and the only thing the
+   scale reaches — the platform clock stays the measurer and the frame
+   record keeps measuring wall clock (lesson 079). */
+double GameTimeStep(const GameTime &time, double wall_dt);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index dbc83e7..ee2f1b4 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -19,6 +19,7 @@
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
+#include "gametime.h"
 #include "platform.h"
 #include "sprite.h"
 #include "table.h"
@@ -305,6 +306,8 @@ int Run(void)
     double last = started;
     double distance = 0.0; /* the score: the world the hero has walked */
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
+    GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale, at play */
+    int scale_phase = 0;  /* lesson 078: the demo's script, by wall seconds */
     bool was_blocked = false; /* lesson 077: the mover's state report */
 
     /* The demo's identity: what the run is, named at once — the hero,
@@ -378,9 +381,41 @@ int Run(void)
 
         /* Update: a frame reads state — it never handles events. */
         double now = platform::Now();
-        double dt = now - last;
+        double wall_dt = now - last;
         last = now;
 
+        /* Lesson 078: the game-time scale — the one knob the game sets.
+           The demo's script is the game here: play, then hitstop (a
+           fraction of full speed), then pause (0), then play again —
+           the same three settings Part 5's juice toolkit and pause
+           screen will make. Each transition names the step that comes
+           out: the wall clock's step, scaled. */
+        double running = now - started;
+        if (scale_phase == 0 && running >= 3.0) {
+            GameTimeSetScale(game_time, 0.25);
+            scale_phase = 1;
+            std::printf("engine: game-time: scale %.2f (hitstop) — step %.3f ms of a %.3f ms wall step\n",
+                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
+                        wall_dt * 1e3);
+        } else if (scale_phase == 1 && running >= 5.0) {
+            GameTimeSetScale(game_time, 0.0);
+            scale_phase = 2;
+            std::printf("engine: game-time: scale %.2f (pause) — step %.3f ms of a %.3f ms wall step\n",
+                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
+                        wall_dt * 1e3);
+        } else if (scale_phase == 2 && running >= 7.0) {
+            GameTimeSetScale(game_time, GAMETIME_FULL);
+            scale_phase = 3;
+            std::printf("engine: game-time: scale %.2f (play) — step %.3f ms of a %.3f ms wall step\n",
+                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
+                        wall_dt * 1e3);
+        }
+
+        /* Lesson 078: the update advances by game time — the wall
+           clock's step, scaled. Everything the simulation does with dt
+           is scaled; nothing else is. */
+        double dt = GameTimeStep(game_time, wall_dt);
+
         /* Lesson 076: the hero's intent — polled input state, read once
            per frame and written to the hero's own movement request. The
            walk turns every entity's request into motion; the game never
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Hitstop that ends *(extend-the-code)*

A hitstop nobody ends is a pause. Give the game a hitstop it can *fire*:
one call that sets the scale to a fraction and remembers to put it back
at full speed after a duration — and then answer the question that call
forces: **a duration of what?** If the countdown runs in game time, a
hitstop of 0.2 s at scale 0.25 lasts 0.8 s of real time; if it runs in
wall time, it lasts 0.2 s and the game's own clock is not what ended it.
Pick one, wire it to the demo (SPACE is free in this lesson's run if you
give the shake a rest), and verify with a run that the scale comes back
on its own. Which did you pick, and what does each choice feel like?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-078/ex1.md)

### Exercise 2 — One knob, not three *(explain-in-prose)*

Pause and hitstop are the same call at different positions, and that is
a design claim. Defend it in your own words, with the code in front of
you: what does the update still do at scale 0 (name the things that run
and the one that does not), and why does a pause screen *need* those
things to keep running? What would break if the scale reached the
platform clock instead of the step (walk through the frame record and
the audio step in your answer)? And what is the case *against* this
design — when would a game want pause to be a different mechanism
entirely?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-078/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 077 — the mover on an entity](lesson-077-mover.md) ·
**Next:** [Lesson 079 — measurement is not scaled](lesson-079-wall-clock.md) ·
**Code tag:** [`lesson-078`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-078)
