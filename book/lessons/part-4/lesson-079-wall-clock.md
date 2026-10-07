# Lesson 079 — measurement is not scaled

{{#include ../../stability-horizon.md}}

## Prose

Lesson 078 gave the game a knob and the update a scaled step. It also
raised a question the lesson deliberately left open: what happens to
everything *around* that step when the knob is at zero. If a paused
game's frame record said `total 0.000 ms`, the frame budget would
report that a paused game costs nothing — which is a lie the machine
can measure against. So this lesson's idea is the discipline that keeps
the record honest: **the scale reaches the simulation's step and nothing
else**. The platform clock stays the measurer, and every phase in the
frame record is a wall-clock duration at any scale.

### The record's one non-duration

`FrameRecord` grows one field, and the field is deliberately not a
phase:

```cpp
    /* ... every field above is wall-clock, at any scale ... */
    double step; /* the game-time step this frame advanced by */
```

`step` is the number the walk multiplied into motion — game seconds,
not machine time. It is in the record because the game's advance is a
fact a reader of the log needs (was this frame paused? how much world
time passed?), and it is *outside* the phases because it answers a
different question. The phases say what the machine did; the step says
what the game did. Adding it to the record is what makes the difference
checkable in one line rather than argued about.

### The paused frame's line

From a real run of this lesson's end state, the first frame the demo's
script paused:

```
frame 50: step 0.000 ms, update 0.020 ms, audio 0.000 ms, render 1.333 ms (sprites 0.006, text 0.007, tilemap 0.922), present 0.295 ms, total 1.649 ms
```

Read it left to right. The **step is 0.000** — the simulation did not
advance by a microsecond; the hero stood still, and the walk multiplied
its request by exactly nothing. And then the phases, which are
unremarkable: `update 0.020 ms`, `render 1.333 ms`, `present 0.295 ms`,
`total 1.649 ms` — ordinary frames, priced the way lesson 036 always
priced them, on the platform clock. The pause cost this machine 1.6
milliseconds of real work: the input was read, eight entities were
walked, the whole scene was drawn and copied to the window. The game
was standing still; the machine was not.

The frames beside it make the rule vivid. Through hitstop the step reads
`3.809 ms` — a quarter of the wall step's `15.2 ms`, exactly lesson
078's product — while the phases read `1.267 ms` of render and `1.647
ms` of total, the same shape as the playing frames around them. At full
speed the step is `15.2 ms`, the wall step whole. Three scales, one
measurement.

### Why the record must not follow the game

The frame record is not a game system; it is an instrument. Its whole
value since lesson 036 is that its numbers are *the machine's*, taken
on the one clock the seam owns (lesson 035: monotonic, fine-grained,
the wall clock cannot move it). Two consumers depend on that and both
would break otherwise:

- **The frame-budget table** (lesson 058) attributes real cost per
  subsystem. A record that scaled would make the cheapest possible game
  — a paused one — look like the fastest one: zero everywhere, and a
  budget table that says the engine costs nothing when the game stops.
  The truth is the opposite and more useful: *pausing the simulation is
  free for the game and not for the machine*, and the difference is the
  cost of presentation — the thing a pause screen is made of.
- **The attribution lesson 081 adds** (the update's entity work) is a
  share of real time. If the update's durations scaled with game time,
  a hitstop would make entity work look cheap exactly when there is
  most of it on screen.

And the seam's contract is the third consumer, the one that never
signed up for any of this: `platform::Now()` is the measurer. If the
scale reached the clock, every schedule built on it — the audio step's
buffer horizon, the run's own timestamps — would inherit game time
without asking for it. The engine's answer is the shape lesson 078
introduced and this one keeps: the clock measures, the step scales, and
the two never meet.

### What this run verified, and what it did not

From a real run of this lesson's end state on this machine, driven by
scripted input while the demo's script turned the knob at three, five,
and seven seconds:

- **The frame record's phases are wall-clock at scale 0.** The paused
  frames above: `step 0.000 ms` with `update 0.020`, `render 1.333`,
  `present 0.295`, `total 1.649` — every phase a real duration, and the
  frame log's own arithmetic still closes (`update + audio + render +
  present ≈ total`).
- **The step is the game's, at every scale.** Hitstop's frames read
  `step 3.809 ms` against wall steps of `15.2 ms` — the quarter,
  reconciled — and play's read the wall step whole.
- **The scale does not reach the seam.** The run's timestamps (`t=…` on
  the hero's and camera's reports) are wall clock through the pause
  window — the demo's script turned the knob at three, five, and seven
  *wall* seconds — and the audio step's schedule was untouched.

What this lesson does **not** do is make the budget table show the
step: the table is a table of durations and `step` is not one. Lesson
081 grows the table with the update's entity attribution — a measured
duration, like every row before it.

## Code step

One change for this lesson, from argument to evidence: `src/frame.h`
grows `FrameRecord.step` — the game-time advance, documented as the
record's one non-duration — and `src/main.cpp` records it where the
update computes it and prints it first in the frame log, the game's
advance beside the machine's durations. The scale's service, the walk,
the mover, the store, the sound, and the budget table are untouched. Its
end state is tagged `lesson-079`.

```diff
diff --git a/src/frame.h b/src/frame.h
index f4b45e3..45d4d88 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -27,6 +27,13 @@ struct FrameRecord {
     double sprites; /* sprite draws through the blit */
     double text;    /* lesson 051: text drawing — glyphs through the blit */
     double tilemap; /* lesson 053: the map's walk — tiles through the blit */
+
+    /* Lesson 079: the game-time step this frame advanced the simulation
+       by — not a duration. Every field above is wall-clock, at any
+       scale: the measurement is the machine's, not the game's. This one
+       is where game time is visible, so a paused frame reads `step
+       0.000 ms` beside wall-clock phases that took what they took. */
+    double step;
 };
 
 /* The running account: every frame measured so far. */
diff --git a/src/main.cpp b/src/main.cpp
index ee2f1b4..1904f32 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -413,8 +413,11 @@ int Run(void)
 
         /* Lesson 078: the update advances by game time — the wall
            clock's step, scaled. Everything the simulation does with dt
-           is scaled; nothing else is. */
+           is scaled; nothing else is. Lesson 079: the step is recorded
+           beside the phases — the one field in the record that is game
+           time, and the rest are wall clock at any scale. */
         double dt = GameTimeStep(game_time, wall_dt);
+        frame.step = dt;
 
         /* Lesson 076: the hero's intent — polled input state, read once
            per frame and written to the hero's own movement request. The
@@ -672,9 +675,12 @@ int Run(void)
 
         /* The frame log: one line per record — the format grows its named
            fields, one per subsystem, as the parts name them. The audio
-           phase (lesson 060) joins in the record's own order. */
-        std::printf("frame %ld: update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
-                    frame.number, frame.update * 1e3, frame.audio * 1e3,
+           phase (lesson 060) joins in the record's own order, and
+           lesson 079's step leads it: the game's advance beside the
+           machine's durations. */
+        std::printf("frame %ld: step %.3f ms, update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
+                    frame.number, frame.step * 1e3, frame.update * 1e3,
+                    frame.audio * 1e3,
                     frame.render * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
                     frame.tilemap * 1e3, frame.present * 1e3,
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The paused frame, predicted *(predict-the-output)*

Before running anything, write down the frame log's line for a frame
that runs while the scale is at 0 — every field, as many of the real
numbers as you can predict and the *shape* of the rest — and then write
down what the frame-budget table's rows would look like after a run that
ends during pause, compared with the same run at full speed. Which
numbers move between the two runs and which do not? Run both (ending one
run mid-pause is a matter of closing the window in the right second) and
reconcile — and say in one sentence what the paused run's table proves
about the difference between the game and the machine.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-079/ex1.md)

### Exercise 2 — What a paused game costs *(measure-the-performance)*

"Pause is free" is a claim, and this lesson says it is false. Measure
it: account each scale window's frames and their wall-clock cost and
report the average frame at play, at hitstop, and at pause — on your
machine, with your numbers. Where does the paused frame's time go (the
record's phases will tell you), and what would it cost a game to *also*
stop the presentation during pause? Then answer the design question the
numbers raise: if a pause screen wants to be cheaper, what should it
change — the scale, the draw, or the machine's schedule?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-079/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 078 — the game-time scale](lesson-078-game-time.md) ·
**Next:** [Lesson 080 — the vertical slice](lesson-080-slice.md) ·
**Code tag:** [`lesson-079`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-079)
