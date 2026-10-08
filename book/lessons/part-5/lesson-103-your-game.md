# Lesson 103 — now make YOUR game

{{#include ../../stability-horizon.md}}

## Prose

The game is done, and the retrospective looked back honestly and named
the horizon. This is the last lesson of the course, and it is not
about this game anymore. It is about **yours**. The engine and the
toolkit are finished — and a finished engine is only worth anything
once it is turned toward a game nobody has made yet. So this lesson
hands the engine over: **what to keep, what to change, where the seams
are, what to record instead of build, and where to measure.**

### What to keep

The services hold still. `Arena` (the one reservation; no allocation
while the game runs), `EntityTable`/`LoadRunTable` (the tables, loaded
whole, complete or named), `EntityStore`/`CreateEntity`/`EntityRetire`
(the fixed store — refused typed, never stealing), `MoveEntity` (the
mover), `GameTime` (the one scale), the tilemap and its collision
queries, the camera, the framebuffer and the one copy loop, the
mixer's sixteen channels, and `FrameStats`/`PrintFrameBudget` (the
account and its report) — these are the engine Parts 1-4 built, and
Part 5 consumed them without redesigning them for the same reason your
game should: **a service you keep is a service you understand, and a
redesign is only justified by a measured problem.** Keep also the
rules that made them trustworthy: the language law (no allocation
while the game runs, no exceptions, nothing behind the seam's back),
typed failures at every boundary, and `tools/check-boundary.sh`
green before any commit leaves your tree.

### What to change

The game layer's files and everything under `assets/` are yours. Your
kinds are table rows — a new enemy, a new projectile, a new pickup is
a new line in a file that already loads. Your maps, your art, your
sounds are files in the same format the course shipped (and the format
is a seam, below, if your game outgrows it). Your rules live in the
file pairs whose names say what they hold: the state machine and
waves in `game.*`, movement feel in `hero.*`, the weapons and hits in
`combat.*`, the behaviors in `ai.*`, the four effects in `feel.*`, the
readouts in `hud.*`, the trigger-to-sound map in `sound.*`. Rewrite
those files' *insides* freely — that is what they are for. Their
*edges* are the seams.

### Where the seams are

The code step of this lesson is a map of them, left behind in the
header of `src/game.h` so it travels with the code. Four seams:

1. **The platform layer.** `platform.h` is the whole contract; one OS
   answers it in two files. A second OS — the Windows module of the
   epilogue map — implements the same contract in its own files and
   changes no engine file. Never let an OS type cross this line; the
   boundary check is the audit.
2. **The table format.** `table.*`/`load.*` plus the `assets/` files.
   Grow the format only by named columns, additively — every file you
   have ever shipped keeps loading, or the growth is wrong (the
   lesson-087 rule). A new kind is a row; a new fact about kinds may
   be a column.
3. **The game layer's files.** The list above. Everything here is
   yours to change; when one of these files wants something the
   services cannot do, that is a *design* question (change a service
   on purpose, or compose differently), not a permission to reach
   around the seam.
4. **Where to measure.** `frame.*`'s account and the closing report
   are built in: the rows name the phases, the budget line checks 60
   fps frame by frame, the machine line keeps the numbers with their
   name (D12). When you need a function's name instead of a phase's,
   `gprof` (the measure pass, lesson 098); when you need behavior
   rather than cost, the transcript discipline (lesson 097's
   exercise); when your game grows hotspots, the three-pass menu
   straight: **measure, fix what you measured, report what you did
   not.**

### The discipline that outlives the course: record, don't add

Your game will attract ideas — from you, faster than from anyone. The
MVD's rule is the tool for that, and it does not expire: **an idea
outside the frozen checklist is extras — recorded in your ledger,
never added.** Write your game's frozen checklist *before* its first
lesson (the shape of `plan/target-game-mvd.md`, which you have now
read the whole of): the lines that make it done, the toolkit you are
bounding yourself to, the optimization menu you will run. Then every
idea that arrives gets one of two honest answers: it is on the
checklist, or it goes in the ledger. Lesson 102's extras ledger is
this rule's worked example — including the two biggest entries (the
GPU port, the Windows module), which are recorded exactly the way
your ideas should be: named, scoped, and *not built*. Scope is a
promise; the checklist decides when your game is done. "More features"
is a chapter of extras or post-course work — never done.

### What this lesson verified, and what it did not

- **The hand-over is in code, not only in prose**: the code step below
  is the seams map as a header comment on `src/game.h` — a comment,
  no behavior. The build is warning-free at this state and the
  boundary check is clean (a comment naming OSs trips nothing it
  should not).
- **The page renders** (`mdbook build`), and the exercise solutions
  apply cleanly at this lesson's end state — both patches checked with
  `git apply --check` here before publication.

What this lesson did **not** verify is anything about your game — no
run, no measurement, no claim. That is the hand-over's whole shape:
the engine's measured, named, and audited; **your game is the
unmeasured part, and measuring it is the first thing you get to do.**

## Code step

One change, and it is a comment: the hand-over map — the four seams
and the record-don't-add rule — as a header block on `src/game.h`, the
game layer's entry file. No behavior changes; `./build.sh` and
`./tools/check-boundary.sh` run exactly as before. It is small on
purpose: a hand-over is a map, not a mechanism, and the map's value is
that it lives where the next person's eyes already are — at the top of
the file that owns the game. Its end state is tagged `lesson-103`.

```diff
diff --git a/src/game.h b/src/game.h
index b3bcccc..c340fbb 100644
--- a/src/game.h
+++ b/src/game.h
@@ -17,6 +17,32 @@
 // the services (table, store, mover, game-time, camera) never grow game
 // behavior, and the game never grows inside them. The play state's
 // gameplay stands on those services and invents nothing.
+//
+// Lesson 103: the hand-over — the engine and the toolkit are finished,
+// and now they are yours. This header names where the codebase is meant
+// to change and where it is meant to hold still: the four seams.
+//
+//   1. The platform layer — `platform.h` is the seam, and one OS
+//      answers it in two files (the window side, the sound side). A
+//      second OS implements the same contract in its own files, and no
+//      engine file changes when it does (tools/check-boundary.sh audits
+//      that); the Windows module of lesson 102's epilogue map is
+//      exactly this seam, a second time.
+//   2. The table format — `table.*`/`load.*` and the files under
+//      `assets/`. Your game's kinds are rows; grow the format only by
+//      named columns, additively, so every file you ship keeps loading.
+//   3. The game layer's files — game.*, hero.*, combat.*, ai.*, feel.*,
+//      hud.*, sound.*: your game's rules, yours to rewrite. The
+//      services under them hold still until measurement says otherwise
+//      (arena, table, entity, gametime, tilemap, audio, camera,
+//      framebuffer — the engine Parts 1-4 built).
+//   4. Where to measure — `frame.*`'s account and the closing report.
+//      Name the hotspot before you fix it (lesson 098's discipline):
+//      measure, fix what you measured, report what you did not.
+//
+// And one discipline more: an idea outside your game's frozen checklist
+// is extras — recorded in your ledger, never added. Lesson 102 kept
+// that rule for this course; keeping it for your game is now your job.
 #ifndef GAME_H
 #define GAME_H
 
```

## Exercises

The course's last two, and both are the first steps of your game
rather than this one. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — your game's first change *(extend-the-code)*

The hand-over is real when the engine bends to someone else's game.
Make your game's first change, through the seams: at least one **data**
change (a kind of yours as a table row — it may reuse the course's art
and sounds until yours exist) and at least one **rule** change in the
game layer's files (something the old game did that yours does not —
a scoring rule, a behavior weight, a wave shape). Rules you must keep:
no service file changes, the build stays warning-free, the boundary
check stays clean, and every file the course shipped keeps loading.
Then demonstrate your change in a run and show the transcript lines
that prove it — and, beside them, the `git diff --stat` that proves
which files your change was *allowed* to touch.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-103/ex1.md)

### Exercise 2 — your machine's hand-over card *(port-to-your-own-machine)*

Run the engine — with your exercise 1 change in it — on your machine:
a real window, a real sound device if you have one. Produce the
hand-over card: the frame-budget report as your machine prints it,
with your machine's name where ours is (the smallest confirming change
is the name; the patch shows where it lives), what your machine
changed in the numbers compared to our rig's report (name the rows
that moved and why), and your extras ledger's first entry — one idea
your game just had, **recorded, not added**. Then say what the card
asks you to do first. The card is the last artifact of this course and
the first artifact of your game's measurement discipline.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-103/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 102 — the retrospective: our engine against real ones](lesson-102-retrospective.md) ·
**Next:** [the course home](../../index.md) ·
**Code tag:** [`lesson-103`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-103)
