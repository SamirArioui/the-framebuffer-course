# Lesson 036 — frame time as measured data

{{#include ../../stability-horizon.md}}

## Prose

The engine now owns its pixels, its input, and its time. Today it starts
keeping **records**: what each frame cost, measured — not estimated — on
the platform clock. This is the seed of two things the course will grow:
Part 2's frame-time instrumentation and Part 5's frame-budget report both
read the record this lesson defines. And the first number it reports is one
worth being honest about early: **the presentation copy is real work, and
it happens every frame**.

### The record

```c++
struct FrameRecord {
    long number;
    double update;  /* reading state, moving the world */
    double render;  /* drawing the scene into the framebuffer */
    double present; /* the copy to the window, sync included */
    double total;   /* the whole frame step */
};
```

One record per frame, four stopwatches. The measurement discipline is the
same one lesson 020 used on `snek`'s loop: read the clock before a phase,
read it after, subtract. `Present` is measured *with* its `XSync` — which
is what makes the number honest: lesson 031's contract says the pixels are
on screen when `Present` returns, so the measurement ends when the work
ends. An async present could only ever be timed as "time to hand the work
to someone else".

The record's fields are the frame's shape as data. `frame.h` carries the
format — one line of the log is one record:

```
frame 30: update 0.000 ms, render 0.470 ms, present 0.518 ms, total 0.988 ms
```

Part 2 grows the record with its own phases (it will want to know what the
*renderer* cost, separately from the platform copy); Part 5 aggregates
records into a budget report. Neither will have to change what a record
is — only what the engine puts in it.

### What the frames actually cost

A moving session — the marker driven across the screen for a couple of
seconds — measured 101 frames:

```
frame 1:  update 0.000 ms, render 0.809 ms, present 0.664 ms, total 1.474 ms
frame 2:  update 0.000 ms, render 0.446 ms, present 1.395 ms, total 1.842 ms
frame 30: update 0.000 ms, render 0.470 ms, present 0.518 ms, total 0.988 ms
frame 60: update 0.000 ms, render 0.435 ms, present 0.359 ms, total 0.794 ms
...
engine: 101 frames — avg 1.015 ms (update 0.000, render 0.462, present 0.552)
engine: worst frame 1.842 ms (frame 2); present is 54% of the frame
```

Read the numbers as what they are — measurements on *this* machine, under
Xvfb, with one square on screen — and three things are still worth saying:

- **The present is 54% of the frame.** The copy to the window is the
  single biggest thing the engine does per frame, and it is not even
  engine code — it is the price of pixels leaving our process. Exercise 1
  splits it into the copy and the wait.
- **The engine's own render is not free.** `ClearBuffer` over 307,200
  pixels plus the marker costs about as much as the copy. "We own our
  pixels" means we also own their cost — Part 2 will make that cost
  interesting.
- **The worst frame is early.** Frame 1 is slow (first touches of a
  1.2 MB buffer are not free — lesson 039 will explain why), and the worst
  frame here is 1.8 ms against an average of 1.0 ms. A frame-budget report
  exists for exactly this tail, not for the average.

And the honest caveat, part of the measurement: Xvfb is a virtual display —
no compositor, no other clients, no real screen. Desktop numbers move. The
*shape* — the present being a real share of every frame — does not.

### Why measure now

Because everything that follows will want these numbers and cannot
retrofit them. Part 2's renderer needs to know what it costs *before* the
optimization passes in Part 5 argue about it. The frame-budget report at
the end of the course is only credible if the measurement started here and
never stopped. (This is the "profiler precedes optimization" rule of the
toolchain curriculum, applied to our own engine first.)

## Code step

One change for this lesson: `frame.h` and `frame.cpp` grow the per-frame
record and its running account, `main.cpp` measures every phase of every
frame and logs one line per record, and the run ends with the account —
including the presentation copy's share of the frame. Its end state is
tagged `lesson-036`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
new file mode 100644
index 0000000..d5e2920
--- /dev/null
+++ b/src/frame.cpp
@@ -0,0 +1,22 @@
+// frame.cpp — the frame account: sums, worst case, nothing else.
+//
+// Lesson 036: measured data is just data — this file does arithmetic on it.
+
+#include "frame.h"
+
+namespace engine {
+
+void AccountFrame(FrameStats &stats, const FrameRecord &frame)
+{
+    stats.frames += 1;
+    stats.update_sum += frame.update;
+    stats.render_sum += frame.render;
+    stats.present_sum += frame.present;
+    stats.total_sum += frame.total;
+    if (frame.total > stats.worst) {
+        stats.worst = frame.total;
+        stats.worst_number = frame.number;
+    }
+}
+
+} /* namespace engine */
diff --git a/src/frame.h b/src/frame.h
new file mode 100644
index 0000000..3ca1cdb
--- /dev/null
+++ b/src/frame.h
@@ -0,0 +1,37 @@
+// frame.h — the per-frame record: what a frame cost, measured on the
+// platform clock.
+//
+// Lesson 036: frame time as measured data. The engine does not guess what
+// its frames cost — it measures each phase and keeps the numbers. Part 2
+// grows this record with its own phases; Part 5's frame-budget report
+// reads it. The format of one line of the log is the format of one record.
+#ifndef FRAME_H
+#define FRAME_H
+
+namespace engine {
+
+/* Seconds, each field: how long one frame's phase took. */
+struct FrameRecord {
+    long number;   /* the frame's count since the run started */
+    double update; /* reading state, moving the world */
+    double render; /* drawing the scene into the framebuffer */
+    double present;/* the copy to the window, sync included */
+    double total;  /* the whole frame step */
+};
+
+/* The running account: every frame measured so far. */
+struct FrameStats {
+    long frames;
+    double update_sum;
+    double render_sum;
+    double present_sum;
+    double total_sum;
+    double worst;      /* the longest frame so far */
+    long worst_number; /* and which one it was */
+};
+
+void AccountFrame(FrameStats &stats, const FrameRecord &frame);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index c66a75f..942d825 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -8,6 +8,7 @@
 #include <cstdio>
 
 #include "framebuffer.h"
+#include "frame.h"
 #include "platform.h"
 
 namespace engine {
@@ -76,13 +77,20 @@ int Run(void)
 
     /* The frame step: read news, update from polled state, draw, present.
        This is the shape every later part fills in — Part 2 draws into it,
-       Part 5 measures it. */
+       Part 5 measures it. Every phase is now measured: the frame record is
+       data, not guesswork. */
     int exit_code = 0;
+    long frame_number = 0;
+    FrameStats stats = {};
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
             break;
 
+        FrameRecord frame;
+        frame.number = ++frame_number;
+        double t0 = platform::Now();
+
         /* Update: a frame reads state — it never handles events. The step
            is speed × elapsed: the marker moves 240 pixels per second no
            matter how often frames happen. */
@@ -110,6 +118,9 @@ int Run(void)
         if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
             marker_y = FRAME_HEIGHT - MARKER_SIZE;
 
+        frame.update = platform::Now() - t0;
+        double t1 = platform::Now();
+
         if ((int)marker_x != old_x || (int)marker_y != old_y)
             std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
                         (int)marker_y, platform::Now() - started);
@@ -118,6 +129,9 @@ int Run(void)
         ClearBuffer(*fb, 32, 32, 64);
         DrawMarker(*fb, (int)marker_x, (int)marker_y);
 
+        frame.render = platform::Now() - t1;
+        double t2 = platform::Now();
+
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
             /* A present can fail because the window died mid-copy — that
@@ -129,6 +143,29 @@ int Run(void)
             exit_code = 1;
             break;
         }
+
+        frame.present = platform::Now() - t2;
+        frame.total = platform::Now() - t0;
+        AccountFrame(stats, frame);
+
+        /* The frame log: one line per record. This is the format Part 2
+           grows and Part 5's frame-budget report reads. */
+        std::printf("frame %ld: update %.3f ms, render %.3f ms, present %.3f ms, total %.3f ms\n",
+                    frame.number, frame.update * 1e3, frame.render * 1e3,
+                    frame.present * 1e3, frame.total * 1e3);
+    }
+
+    /* The account: what the frames actually cost, including the honest
+       price of the presentation copy. */
+    if (stats.frames) {
+        double n = (double)stats.frames;
+        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f, present %.3f)\n",
+                    stats.frames, stats.total_sum / n * 1e3,
+                    stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
+                    stats.present_sum / n * 1e3);
+        std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
+                    stats.worst * 1e3, stats.worst_number,
+                    100.0 * stats.present_sum / stats.total_sum);
     }
 
     if (platform::CloseRequested(opened.window))
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The copy, isolated *(measure-the-performance)*

`Present`'s measured time glues two different things together: `XPutImage`
— pushing the pixels to the server — and `XSync` — waiting until the
server is done. Instrument the two halves separately (two stopwatches
inside the implementation, one line per present) and find out which half is
the copy and which half is the round trip. Then answer the question the
design keeps open: which half would shared memory (MIT-SHM) remove, and
what would be left?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-036/ex1.md)

### Exercise 2 — The frame budget *(extend-the-code)*

A measurement becomes a decision when it has a line to be on the wrong side
of. Give the account a **frame budget** — the 60 fps line is 1/60 = 16.7 ms
— and have the run report how many frames fit inside it:
`within the 16.7 ms budget: N/M frames`. Where should the budget live: in
the platform layer or in the engine's record code? Answer before you place
it, and let the answer decide the file.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-036/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 035 — the platform clock](lesson-035-clock.md) ·
**Next:** [Lesson 037 — whole-file reads](lesson-037-file-read.md) ·
**Code tag:** [`lesson-036`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-036)
