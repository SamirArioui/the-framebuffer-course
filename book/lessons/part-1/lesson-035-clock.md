# Lesson 035 — the platform clock

{{#include ../../stability-horizon.md}}

## Prose

Lesson 034 ended with an apology in the code: the marker's *speed* still
belonged to the keyboard, because frames happened when news happened. Today
the engine gets its own clock — the same monotonic clock lesson 020 taught
you to read inside `snek`, now behind the platform seam — and time becomes
something the engine measures instead of something that happens to it. From
here on, movement is **pixels per second**.

### The clock's contract

One function on the seam:

```c++
double Now(void);
```

Seconds since an arbitrary starting point — and the contract is the part
that matters:

- **Monotonic.** The readings never go backwards. Not "usually don't":
  never, and no adjustment by anyone can make them.
- **Wall-clock immune.** Setting the system clock forward, backward, or
  sideways changes nothing here. The wall clock answers "what time is it?";
  this one answers "how long has it been?" — and only the second question
  has an answer a frame can subtract.
- **Fine enough for a frame.** The resolution is far below a millisecond —
  the clock can measure the things lesson 036 will put in the frame record.

Behind the seam the implementation is the call lesson 020 already taught:
`clock_gettime(CLOCK_MONOTONIC, ...)`, POSIX's clock interface. The
`CLOCK_REALTIME` next to it is the wall clock, and it is *not* what the seam
exposes — exercise 2 puts the two side by side and explains why.

### The contract, checked

The engine checks the clock before anything depends on it — a hundred
thousand samples in a row, watching for a reading smaller than the one
before and for the finest gap between two readings:

```
$ DISPLAY=:99 ./build/game
engine: clock never backwards over 100000 samples, finest step 20 ns
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
...
```

Never backwards over a hundred thousand samples, and the finest step
between two of them is twenty nanoseconds — four orders of magnitude below
the sub-millisecond the contract asks for. (The absolute value of `Now()`
means nothing — about 37,725 seconds on the machine this was authored on,
roughly its uptime. Arbitrary starting point, remember.)

### The step is speed × elapsed

The update shrinks to arithmetic anyone can check:

```c++
double dt = now - last;
if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
    marker_x += MARKER_SPEED * dt;
```

`MARKER_SPEED` is 240 — *pixels per second* — and `dt` turns it into this
frame's step. If frames come fast, the steps are small; if a frame is slow,
its step is proportionally larger; the speed is the same either way. The
keyboard no longer decides anything but direction.

The proof is a comparison: run the marker for one second with the frames
coming fast, then again with the frames coming three times slower. Same
duration, same key, same speed — the marker must travel the same distance.

```
frames at 33 ms:  engine: marker at 548,228 (t=1.002)
frames at 100 ms: engine: marker at 548,228 (t=1.002)
```

Identical displacement at identical time across a 3× difference in frame
rate. That is what "the engine owns its speed" means, measured.

### What the clock is for around the seam

This clock is not a one-lesson prop. Part 2's frame timing instruments on
it, Part 5's frame-budget report reads it, and lesson 036 starts the
per-frame record both will grow from — the frame's own duration as data the
engine keeps. The frame cap `snek` needed (lesson 020's `SleepSec`) is
still ahead of us in some form: with `dt` in hand, pacing becomes a
question of *policy* (how fast should frames happen?) instead of a
prerequisite for correctness.

## Code step

One change for this lesson: the seam grows `Now` — the monotonic platform
clock — the engine checks its contract at startup and moves the marker by
`speed × dt` instead of a fixed step, and the report carries the time each
movement happened. (A stray unused include left over from lesson 031's
debugging is tidied away in the same file.) Its end state is tagged
`lesson-035`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 47d7360..c66a75f 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -12,11 +12,10 @@
 
 namespace engine {
 
-/* The marker: one square the arrow keys move. Its position is whole
-   pixels and its speed is pixels-per-frame — lesson 035's clock turns
-   that into pixels-per-second. */
+/* The marker: one square the arrow keys move. Its speed is the engine's —
+   pixels per second — and the clock's dt turns it into a per-frame step. */
 constexpr int MARKER_SIZE = 24;
-constexpr int MARKER_STEP = 8;
+constexpr double MARKER_SPEED = 240.0; /* pixels per second */
 
 static void DrawMarker(Framebuffer &fb, int x, int y)
 {
@@ -47,13 +46,33 @@ int Run(void)
         return 1;
     }
 
+    /* The clock's contract, checked before anything depends on it: the
+       readings never go backwards, and the finest step between two of them
+       is far below a frame. */
+    double prev = platform::Now();
+    double finest = 1e9;
+    int backwards = 0;
+    for (int i = 0; i < 100000; ++i) {
+        double t = platform::Now();
+        if (t < prev)
+            ++backwards;
+        else if (t > prev && t - prev < finest)
+            finest = t - prev;
+        prev = t;
+    }
+    std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
+                backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);
+
     /* The scene: a marker the arrow keys move. The report below is its
-       position — the interactive frame makes itself observable. */
+       position and the time it moved — the interactive frame makes itself
+       observable. */
     Framebuffer *fb = GetFramebuffer();
-    int marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2;
-    int marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2;
+    double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
+    double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
+    double started = platform::Now();
+    double last = started;
     std::printf("engine: arrow keys move the marker; close the window to stop\n");
-    std::printf("engine: marker at %d,%d\n", marker_x, marker_y);
+    std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
 
     /* The frame step: read news, update from polled state, draw, present.
        This is the shape every later part fills in — Part 2 draws into it,
@@ -64,16 +83,22 @@ int Run(void)
         if (platform::CloseRequested(opened.window))
             break;
 
-        /* Update: a frame reads state — it never handles events. */
-        int old_x = marker_x, old_y = marker_y;
+        /* Update: a frame reads state — it never handles events. The step
+           is speed × elapsed: the marker moves 240 pixels per second no
+           matter how often frames happen. */
+        double now = platform::Now();
+        double dt = now - last;
+        last = now;
+
+        int old_x = (int)marker_x, old_y = (int)marker_y;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            marker_x -= MARKER_STEP;
+            marker_x -= MARKER_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            marker_x += MARKER_STEP;
+            marker_x += MARKER_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_UP))
-            marker_y -= MARKER_STEP;
+            marker_y -= MARKER_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            marker_y += MARKER_STEP;
+            marker_y += MARKER_SPEED * dt;
 
         /* The marker stays on screen — lesson 015's fold at frame scale. */
         if (marker_x < 0)
@@ -85,12 +110,13 @@ int Run(void)
         if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
             marker_y = FRAME_HEIGHT - MARKER_SIZE;
 
-        if (marker_x != old_x || marker_y != old_y)
-            std::printf("engine: marker at %d,%d\n", marker_x, marker_y);
+        if ((int)marker_x != old_x || (int)marker_y != old_y)
+            std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
+                        (int)marker_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then marker. */
         ClearBuffer(*fb, 32, 32, 64);
-        DrawMarker(*fb, marker_x, marker_y);
+        DrawMarker(*fb, (int)marker_x, (int)marker_y);
 
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
diff --git a/src/platform.h b/src/platform.h
index 643bdef..cea5fc6 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -56,6 +56,12 @@ bool KeyPressed(Window *window, Key key);
    for them. */
 bool HasFocus(const Window *window);
 
+/* The platform clock: seconds since an arbitrary starting point. It is
+   monotonic — it never goes backwards, and the wall clock cannot move it —
+   and its resolution is fine enough to measure one frame. This is the
+   clock Part 2's frame timing and Part 5's frame-budget report stand on. */
+double Now(void);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 4e23966..d35cf08 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -7,6 +7,11 @@
 //
 // Lesson 028: the event pump. The OS's news arrives here as X events and is
 // folded into state the engine polls — the engine never reads an event.
+//
+// Lesson 035: the platform clock. POSIX, not ISO C — clock_gettime is the
+// OS's clock interface (the one lesson 020 taught inside snek, now behind
+// the seam), so the feature-test macro goes before the includes.
+#define _POSIX_C_SOURCE 200809L
 
 #include "platform.h"
 
@@ -15,9 +20,9 @@
 #include <X11/Xutil.h>
 #include <X11/keysym.h>
 
-#include <cstdio>
 #include <poll.h>
 #include <signal.h>
+#include <time.h>
 
 namespace platform {
 
@@ -200,6 +205,13 @@ bool HasFocus(const Window *window)
     return window && window->focused;
 }
 
+double Now(void)
+{
+    struct timespec ts;
+    clock_gettime(CLOCK_MONOTONIC, &ts);
+    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
+}
+
 void PumpEvents(Window *window)
 {
     if (!window || !window->display)
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The diagonal is too fast *(fix-the-crash)*

Hold two arrows and watch the marker move 41% faster than it does on the
axes: each axis adds `speed × dt` on its own. Fix it so the marker moves at
`MARKER_SPEED` in *every* direction — treat the pressed keys as one
direction vector and normalize it before applying speed. Prove it: hold one
key for exactly one second, then hold the diagonal for exactly one second,
and compare the distances traveled (a helper that presses, sleeps the exact
time, and releases keeps the comparison honest).

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-035/ex1.md)

### Exercise 2 — The clock that lies *(explain-in-prose)*

Lesson 020's "two clocks" drill, revisited behind the seam. Add an
instrumenting second reading — the wall clock, `CLOCK_REALTIME` — beside
the monotonic one in the platform implementation, and print both in the
startup check. Then explain, in prose, what the frame step `dt = now −
last` would do if the engine timed its frames with *that* clock: what
happens to the marker when the system clock steps backwards, when it steps
forwards, and why the monotonic clock is immune to both.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-035/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 034 — the first interactive frame](lesson-034-first-frame.md) ·
**Next:** [Lesson 036 — frame time as measured data](lesson-036-frame-time.md) ·
**Code tag:** [`lesson-035`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-035)
