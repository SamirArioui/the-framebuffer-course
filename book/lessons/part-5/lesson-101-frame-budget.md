# Lesson 101 — pass 3: the frame-budget report

{{#include ../../stability-horizon.md}}

## Prose

The three-pass menu is spent: the measure pass named two hotspots with
numbers from real frames, the two fix passes took them down 43% and
47% with the deep dives' levers, and nothing else was touched. This is
pass 3 and the course's finale: **the final frame-budget report** —
per-frame time attributed to each major subsystem, measured from real
frames of the finished game, produced from `frame-accounting`'s own
account (design D11: measured, never modeled). It is one page of
numbers. It is also the honest answer to the MVD's hardest line:
*60 fps on modest hardware*.

### The report

The finished game, played hard — ten legs of walking, firing, dying,
restarting: **2,583 frames**, 2,447 of them play, 136 of them screens,
on this machine. The account's table, whole and unedited:

```
engine: frame budget — 2583 frames, avg 1.283 ms, worst 3.124 ms (frame 1101)
engine:   subsystem   avg ms    share
engine:   update       0.013       1%
engine:     entities   0.005       0%
engine:   audio        0.033       3%
engine:   render       0.799      62%
engine:     clear      0.244      19%
engine:     sprites    0.006       0%
engine:     text       0.011       1%
engine:     tilemap    0.538      42%
engine:   present      0.438      34%
engine:   total        1.283     100%
engine:   by state    2447 play frames at 1.313 ms, 136 screen frames at 0.749 ms
engine:   budget      60 fps is 16.667 ms a frame — 0 of 2583 frames over it, worst 3.124 ms (19% of it)
engine:   machine     WSL2, Xvfb :99, no sound hardware (the course's authoring machine)
```

Read it in the order it grew. The **attribution** is the account's
original contract (the spec's "at least the world update, the render,
and the presentation"): every major subsystem's average over the
frames that actually ran, the named sub-phases inside their phases,
and every number a measured sum — `clear`, `sprites`, `text`,
`tilemap` add up to `render` with no unnamed remainder, because
lesson 098 named the last of them. The **by-state** line is the
lesson of the mix made permanent: a play frame costs `1.313 ms`, a
screen frame `0.749 ms`, and an average over both is neither. The
**budget** line checks the 60 fps claim frame by frame — not "the
average is small" but *this many frames crossed the line*: zero. And
the **machine** line is design D12's rule — a performance claim
carries its machine — printed by the report itself, so the numbers
cannot travel without their name.

### Where the play frame goes

One play frame of the finished game, `1.313 ms` on average, and the
two passes' fingerprints are the two rows that fell:

```
                          lesson-098   lesson-099   lesson-100   the finale
  tilemap (map's draw)      0.981        0.559        0.539        0.538
  clear (the frame's)       0.456        0.449        0.239        0.244
  present (the seam's)      0.444        0.434        0.428        0.438
  audio + update            0.045        0.046        0.046        0.046
  sprites + text            0.018        0.016        0.016        0.017
  total                     1.944        1.505        1.269        1.313
```

(the finale column is its own run — runs wobble a few percent, the
noise floor lesson 097 measured; the rows' *shapes* are the claim.)
The two named rows fell `1.437 → 0.782 ms` together — a third of the
frame — and what remains is the picture the menu always promised:
`present`, the seam's wait, is now the frame's largest single row. It
was measured, it was named, and it was not the menu's to fix.

### The 60 fps line, checked as far as this machine honestly measures

Design D12 says this claim cannot be proven on the authoring machine,
and says what to do instead: check it as far as honest measurement
goes, and say the rest out loud. So, in order:

**What this machine measured.** 2,583 real frames of the finished
game: **0 over the 16.667 ms budget**; the worst frame of the run
`3.124 ms` — 19% of a budget frame. Even the worst frame ever
recorded across these lessons (`4.387 ms`, lesson 098's run) is 26%
of the budget — and that at `-O0`, the slowest build the course
ships. The frame's cost is not near the line.

**What this machine is.** WSL2 under Windows, an Xvfb display, no
sound hardware, the audio mixed in silence — and, the shape fact that
governs everything: this loop is **event-driven**, waking on X news or
the audio feed's deadline. With no audio device to pace it, the runs
in this course were paced by window-move jiggles at ~25 fps. This
machine therefore **cannot demonstrate 60 fps** — it never ran at 60
— and no number above pretends otherwise. What it demonstrates is the
frame's *cost*, which is what 60 fps is made of.

**What is left to check, and where.** "On modest hardware" is a claim
about *your* machine, not this one — so the checklist's perf line
routes where design D12 says it routes: to you. Exercise 1 sends the
report to your hardware with the one instrument the average lacks —
the frame times' percentiles — so the 60 fps line is answered by *the
frames that miss*, on a machine that actually renders at its own rate.
When that is done, the line is checked all the way. Until then it is
checked exactly this far, and this sentence is the rest, said out
loud.

### The ledger: what the passes spent, and what they did not

The frozen menu spent its two fixes and stopped. Everything else the
measure pass named stays named, measured, and untouched — the future
work, on the record (D11):

- **the presentation's copy** — `present 0.438 ms` of wall per frame,
  `0.004 ms` of CPU: the seam's wait on the X server. A
  double-buffered or MIT-SHM present is a *platform-layer* change —
  the seam's second OS is where it would live.
- **the tiles' pixel format** — the map's copy moves 3 bytes in and 4
  out; pixels living in the framebuffer's order at load would let the
  compiler widen it (lesson 099's census: it declines today). A
  layout change that breaks the one-copy-loop rule — measurement must
  ask for it.
- **the audio mix at full load** — `audio 0.033 ms` with the music and
  a scatter of effects; all sixteen channels firing is unmeasured.
- **the update at full store** — `entities 0.005 ms` at a handful of
  live entities; sixty-four sparks is unmeasured.
- **the reports' own printing** — the probes and the HUD's report
  print inside `update` and `text`; the frame log's line prints
  outside every measured phase. A quieter run measures cheaper phases.

### What this run verified, and what it did not

- **The report attributes per-frame time to each major subsystem from
  real frames of the finished game** — 2,583 frames of the instrumented
  game played through its whole scenario, every row a measured sum,
  the by-state split reading the run's own records.
- **The 60 fps line is checked frame by frame** — `0 of 2583 frames`
  over the budget — **on a named machine**, and the claim's unprovable
  half ("modest hardware") is named as unproven here and routed to
  the port exercise, exactly as D12 prescribes.

What this run did **not** verify is the checklist's perf line on real
hardware — and the definition of done says what that makes this: the
finished game is done as far as this machine honestly measures, the
rest is said out loud, and the last word of the three-pass menu is not
a number but a discipline: measure, fix what you measured, report what
you did not.

## Code step

One change: the report's close. `src/frame.h/.cpp`'s account grows the
finale's own rows — the by-state split (the record's `step` says which
frames played), the budget's frame-by-frame count against
`FRAME_BUDGET_MS` (the 60 fps frame), and the machine's name printed
with the numbers (D12); `src/main.cpp` names `RUN_MACHINE` — this
machine, in one string the report cannot lose. The table's rows are
untouched: the attribution was already measured; this is where it
becomes the report. Its end state is tagged `lesson-101`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 349af4f..383b424 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -21,13 +21,23 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.tilemap_sum += frame.tilemap;
     stats.clear_sum += frame.clear;
     stats.entities_sum += frame.entities;
+    /* Lesson 101: the by-state split, and the budget line's count. */
+    if (frame.step > 0.0) {
+        stats.play_frames += 1;
+        stats.play_sum += frame.total;
+    } else {
+        stats.screen_frames += 1;
+        stats.screen_sum += frame.total;
+    }
+    if (frame.total * 1e3 > FRAME_BUDGET_MS)
+        stats.over_budget += 1;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
     }
 }
 
-void PrintFrameBudget(const FrameStats &stats)
+void PrintFrameBudget(const FrameStats &stats, const char *machine)
 {
     if (!stats.frames)
         return;
@@ -79,6 +89,22 @@ void PrintFrameBudget(const FrameStats &stats)
     std::printf("engine:   present     %6.3f      %2.0f%%\n", present,
                 100.0 * present / (avg * 1e3));
     std::printf("engine:   total       %6.3f     100%%\n", avg * 1e3);
+
+    /* Lesson 101: the final report's close — the frames split by what
+       they were doing, the 60 fps line checked frame by frame, and the
+       machine these numbers belong to (D12). */
+    double play = stats.play_frames ? stats.play_sum / (double)stats.play_frames
+                                    : 0.0;
+    double screen =
+        stats.screen_frames ? stats.screen_sum / (double)stats.screen_frames
+                            : 0.0;
+    std::printf("engine:   by state    %ld play frames at %.3f ms, %ld screen frames at %.3f ms\n",
+                stats.play_frames, play * 1e3, stats.screen_frames,
+                screen * 1e3);
+    std::printf("engine:   budget      60 fps is %.3f ms a frame — %ld of %ld frames over it, worst %.3f ms (%.0f%% of it)\n",
+                FRAME_BUDGET_MS, stats.over_budget, stats.frames,
+                stats.worst * 1e3, 100.0 * stats.worst * 1e3 / FRAME_BUDGET_MS);
+    std::printf("engine:   machine     %s\n", machine ? machine : "(unnamed)");
 }
 
 } /* namespace engine */
diff --git a/src/frame.h b/src/frame.h
index 1234c17..7fb1cd6 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -59,15 +59,33 @@ struct FrameStats {
     double entities_sum; /* lesson 081: the update's entity work, summed */
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
+
+    /* Lesson 101: the final report's own account — the frames split by
+       what they were doing (the record's `step` says it: a frame that
+       advanced game time was playing, one at zero was showing a
+       screen), and the count that checks the 60 fps line frame by
+       frame. */
+    long play_frames;
+    double play_sum;
+    long screen_frames;
+    double screen_sum;
+    long over_budget; /* frames that spent more than the 60 fps budget */
 };
 
+/* Lesson 101: the 60 fps frame — the budget the finished game is
+   measured against (the MVD's perf line). One sixtieth of a second. */
+constexpr double FRAME_BUDGET_MS = 1000.0 / 60.0;
+
 void AccountFrame(FrameStats &stats, const FrameRecord &frame);
 
 /* Lesson 058: the frame-budget table — the account, attributed per
    subsystem, as the report Part 5's finale grows. Every number in it is
    a measured sum from the frames that actually ran; the shares are of
-   the average frame. */
-void PrintFrameBudget(const FrameStats &stats);
+   the average frame. Lesson 101: it is the final frame-budget report
+   now — the attribution, the by-state split, the 60 fps budget line,
+   and the machine the numbers came from (D12: a performance claim
+   carries its machine). */
+void PrintFrameBudget(const FrameStats &stats, const char *machine);
 
 } /* namespace engine */
 
diff --git a/src/main.cpp b/src/main.cpp
index 112049a..5ef73e3 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -29,6 +29,16 @@
 
 namespace engine {
 
+/* Lesson 101: the machine these measurements belong to (D12 — a
+   performance claim carries its machine). The report prints it with
+   its numbers; a run on different hardware names different hardware.
+   What this name means for the numbers: a paced headless run (the
+   loop is event-driven; the pacing is window-move jiggles at ~25 fps),
+   a `-O0` build, the audio mixed in silence (no sound device), and the
+   display's copy through the X server. */
+constexpr const char *RUN_MACHINE =
+    "WSL2, Xvfb :99, no sound hardware (the course's authoring machine)";
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -321,7 +331,7 @@ int Run(void)
        (the frame account's own table) and the arena's. */
     ReportEnd(world.sound, feed, world.store, hero, walk_visits,
               frame_number);
-    PrintFrameBudget(stats);
+    PrintFrameBudget(stats, RUN_MACHINE);
     std::printf("engine: arena: %zu of %zu bytes used\n", world.arena.used,
                 world.arena.memory.size);
 
```

## Exercises

The finale's two challenges — the check this machine cannot make, and
the one number the report still hides. Each ends with its solution — a
diff against this lesson's end state plus a walkthrough — after the
prompt.

### Exercise 1 — the 60 fps line on your machine *(port-to-your-own-machine)*

This machine cannot demonstrate 60 fps; yours can. Run the finished
game on your hardware — a window on your desktop, a sound device if
you have one, played hard — and answer the MVD's perf line with
measured frames. The average is not the instrument for this: extend
the account with the frame times' **percentiles** (the median, the
95th, the 99th — a fixed histogram in the account's own memory, no
allocation), and report your machine's report: the table, the
percentiles, the budget line, and your machine's name where ours is.
Then say the sentence the checklist needs — *my machine holds 60 fps*
or not — with the frames that miss it counted.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-101/ex1.md)

### Exercise 2 — the worst frame, predicted *(predict-the-output)*

The report names the worst frame's number and its cost (`worst 3.124
ms (frame 1101)`) but not its shape. Before you run anything, predict
it: which subsystem eats the worst frame of a played run, and roughly
what share — the map's draw, the clear, the seam's wait, a wave's
spawn in the update? Then make the report attribute the worst frame
(it already remembers which one it was — teach the account to remember
*what* it was) and run the same ten-leg scenario: was your prediction
right, in the report's own numbers? And answer the counterfactual in
your words: if the worst frame is the seam's, what does that say about
who owns the fix?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-101/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 100 — pass 2b: fix the clear](lesson-100-clear.md) ·
**Next:** [Lesson 102 — the retrospective: our engine against real ones](lesson-102-retrospective.md) ·
**Code tag:** [`lesson-101`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-101)
