# Lesson 058 — the frame-budget table

{{#include ../../stability-horizon.md}}

## Prose

The instrumentation of lesson 036 was planted for exactly this: the
**frame-budget table** — the account attributed per subsystem, printed
as the report Part 5's finale grows. This is O1's payoff and the part's
last lesson: not new capability, but *accountability* — the frame's cost
named, row by row, from measured sums of real frames. Every number in
the table was paid for by a frame that actually ran.

### The table

The demo runs, and at the end the account prints as a budget:

```
engine: frame budget — 27 frames, avg 2.570 ms, worst 4.162 ms (frame 17)
engine:   subsystem   avg ms    share
engine:   update       0.023       1%
engine:   render       1.917      75%
engine:     sprites    0.001       0%
engine:     text       0.008       0%
engine:     tilemap    1.202      47%
engine:   present      0.630      25%
engine:   total        2.570     100%
```

The header is the spec's floor, on purpose: **how many frames** were
measured, **what they cost on average**, and **the worst one by number**
— the three facts any budget discussion opens with. Every row below it
is a measured sum divided by the frame count:

- **update, render, present** — the three phases lesson 036 named,
  exactly as its record measures them. Nothing about the phase
  definitions changed to make the table; the table is what the record
  was for.
- **the indented rows** — the render phase's named subsystems (lessons
  046, 051, 053): what the sprite draws, the text, and the tilemap walk
  each cost, *inside* render. They explain the row above them; they do
  not replace it.
- **shares** — each row as a percentage of the average frame.

Every number is checkable against the run's own log — and the
authoring check for this lesson did exactly that: averaging the `frame
N:` lines over the session reproduces the table row for row (update
0.023, render 1.917, sprites 0.001, text 0.008, tilemap 1.202, present
0.630, total 2.570). A budget whose numbers disagree with its own
ledger is decoration; this one is the ledger.

### What the table says

Three readings, and all of them were visible in earlier lessons — the
table's job is to make them impossible to forget:

1. **The tilemap walk owns the render.** 1.2 ms of the 1.9 ms render is
   1,536 blits redrawing the world every frame (lesson 053 measured the
   walk's cost at three placements; this is the on-screen one). If a
   scene ever needs to be faster, this is the row that pays.
2. **The presentation is the machine's row.** 25% here is Xvfb's copy;
   on a desktop it moves with the driver and the compositor (Part 1's
   lesson 036 found the same shape). It is the row least under the
   engine's control and most under the platform's.
3. **Drawing one sprite is free; drawing the world is not.** sprites and
   text together are 0.009 ms — below the noise floor. The renderer's
   per-object cost is not the problem at this scale; the *number of
   objects* (the whole map, every frame) is.

And the frame's arithmetic closes: `0.023 + 1.917 + 0.630 = 2.570` — the
three phases account for the total (the tiny residue is the measurement
itself: the clock reads around the phases). The subsystem rows do not
yet close against render — the difference is the clear and the phase
bookkeeping, and exercise 1 puts a number on it.

### The format Part 5 grows

This table is explicitly a *first draft*. What the finale's frame-budget
report adds, and what this format already has room for:

- **more rows as subsystems exist** — sound, entities, the game's own
  update work (Parts 3-5 add their named phases exactly like text and
  tilemap did);
- **before/after columns** — the three-pass optimization menu (measure,
  fix top-2, report) reports each pass against this same table;
- **the machine** — the report names its hardware and build flags, the
  rule every measurement in this course has carried since Part 0.

What the format will *not* change: rows are measured sums, shares are of
the average frame, the header carries count/average/worst, and the named
phases live inside their phase rather than replacing it. Those are the
rules that make the numbers mean what they say.

### O1, delivered

The MVD's instrumentation obligation is complete and demonstrable: a
record per frame, a log line per record, an account that summarizes the
run, and now the budget attributed per subsystem — all of it on the
platform clock behind the seam, all of it checkable against the log.
Part 5's profiler lesson starts from this table and cannot start
without it.

## Code step

One change for this lesson: `frame.h` / `frame.cpp` grow
`PrintFrameBudget` — the account printed as the table (the header's
count/average/worst and the rows' measured sums and shares) — and
`main.cpp`'s account block becomes the call. The demo loop, the world,
and the drawing path are untouched. Its end state is tagged
`lesson-058`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 22cb968..3f9f2dd 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -4,6 +4,8 @@
 
 #include "frame.h"
 
+#include <cstdio>
+
 namespace engine {
 
 void AccountFrame(FrameStats &stats, const FrameRecord &frame)
@@ -22,4 +24,42 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     }
 }
 
+void PrintFrameBudget(const FrameStats &stats)
+{
+    if (!stats.frames)
+        return;
+    double n = (double)stats.frames;
+    double avg = stats.total_sum / n;
+
+    /* The account the spec requires: how many frames, what they cost on
+       average, and the worst one by number. */
+    std::printf("engine: frame budget — %ld frames, avg %.3f ms, worst %.3f ms (frame %ld)\n",
+                stats.frames, avg * 1e3, stats.worst * 1e3,
+                stats.worst_number);
+
+    /* The attribution: every row a measured sum, every share of the
+       average frame. The named phases live inside render — they say
+       where it went, they do not replace it. */
+    double update = stats.update_sum / n * 1e3;
+    double render = stats.render_sum / n * 1e3;
+    double sprites = stats.sprites_sum / n * 1e3;
+    double text = stats.text_sum / n * 1e3;
+    double tilemap = stats.tilemap_sum / n * 1e3;
+    double present = stats.present_sum / n * 1e3;
+    std::printf("engine:   subsystem   avg ms    share\n");
+    std::printf("engine:   update      %6.3f      %2.0f%%\n", update,
+                100.0 * update / (avg * 1e3));
+    std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
+                100.0 * render / (avg * 1e3));
+    std::printf("engine:     sprites   %6.3f      %2.0f%%\n", sprites,
+                100.0 * sprites / (avg * 1e3));
+    std::printf("engine:     text      %6.3f      %2.0f%%\n", text,
+                100.0 * text / (avg * 1e3));
+    std::printf("engine:     tilemap   %6.3f      %2.0f%%\n", tilemap,
+                100.0 * tilemap / (avg * 1e3));
+    std::printf("engine:   present     %6.3f      %2.0f%%\n", present,
+                100.0 * present / (avg * 1e3));
+    std::printf("engine:   total       %6.3f     100%%\n", avg * 1e3);
+}
+
 } /* namespace engine */
diff --git a/src/frame.h b/src/frame.h
index 8f42a7d..8a998b4 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -43,6 +43,12 @@ struct FrameStats {
 
 void AccountFrame(FrameStats &stats, const FrameRecord &frame);
 
+/* Lesson 058: the frame-budget table — the account, attributed per
+   subsystem, as the report Part 5's finale grows. Every number in it is
+   a measured sum from the frames that actually ran; the shares are of
+   the average frame. */
+void PrintFrameBudget(const FrameStats &stats);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index bc94247..95f09af 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -275,19 +275,10 @@ int Run(void)
                     frame.total * 1e3);
     }
 
-    /* The account: what the frames actually cost, subsystem by subsystem —
-       the frame-budget table's first data (lesson 058 prints the table). */
-    if (stats.frames) {
-        double n = (double)stats.frames;
-        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, tilemap %.3f, present %.3f)\n",
-                    stats.frames, stats.total_sum / n * 1e3,
-                    stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
-                    stats.sprites_sum / n * 1e3, stats.text_sum / n * 1e3,
-                    stats.tilemap_sum / n * 1e3, stats.present_sum / n * 1e3);
-        std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
-                    stats.worst * 1e3, stats.worst_number,
-                    100.0 * stats.present_sum / stats.total_sum);
-    }
+    /* The account as the frame-budget table (lesson 058): the frame
+       count, the average, the worst frame — and the render attributed to
+       its subsystems, the report Part 5's finale grows. */
+    PrintFrameBudget(stats);
     std::printf("engine: arena: %zu of %zu bytes used\n", arena.used,
                 arena.memory.size);
 
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The row that isn't there *(extend-the-code)*

The subsystem rows do not add up to the render row — and the difference
has a name and a size. Add the missing row (`render` minus the named
subsystems) to the table, run the demo, and reconcile the arithmetic.
Then answer the question the row asks: the difference is mostly the
frame's `ClearBuffer` — why is the single biggest per-frame pixel cost
not a named subsystem, and should it be?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-058/ex1.md)

### Exercise 2 — The worst frame's column *(predict-the-output)*

The header names the worst frame; the table does not say what it spent
its time on. Before you run anything, find the worst frame in the log's
`frame N:` lines and predict which subsystem owns it — is the worst
frame a *rendering* event or something else? Then keep the worst frame's
record and print its attribution beside the table; reconcile, and
explain why the worst frame's shape can differ from the average's.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-058/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 057 — the closing demo](lesson-057-demo.md) ·
**Next:** — ·
**Code tag:** [`lesson-058`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-058)
