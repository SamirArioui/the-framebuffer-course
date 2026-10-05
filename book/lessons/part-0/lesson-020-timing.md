# Lesson 020 — timing with `clock_gettime`

{{#include ../../stability-horizon.md}}

## Prose

Lesson 019's loop had no notion of speed: frames went by as fast as the CPU
could print them, and nothing in the program knew how long one took. A game
is different — the snake must cross the field in the same number of seconds on
a fast machine and a slow one. This lesson gives the loop a clock: how C
measures time, what *delta time* is and why game logic integrates over it,
and the fixed-timestep discipline that keeps a game's speed stable.

Measuring time starts with `clock_gettime`, which fills a `struct timespec`
— two integers, `tv_sec` and `tv_nsec`, seconds plus nanoseconds. Two details
of that call are curriculum in their own right. First, a wrinkle from the
toolchain: `clock_gettime` is POSIX, not ISO C, and `-std=c11` deliberately
hides everything the standard does not describe. The fix is a *feature test
macro* at the very top of the file, before any `#include`:

```c
#define _POSIX_C_SOURCE 200809L
```

This tells the C library headers "expose the POSIX.1-2008 declarations
too". Without it the build is a wall of implicit-declaration warnings and an
implicit `int` return that silently corrupts the timing. (The alternative is
`-std=gnu11`, which opens everything GNU as well; pinning the standard and
declaring what we need keeps the language honest.) Second, `clock_gettime`
takes a *clock id*, and the choice matters: `CLOCK_REALTIME` is civil time —
seconds since the epoch, which NTP, an admin, or a dual-boot clock repair can
step forwards **or backwards** at any moment. `CLOCK_MONOTONIC` is time since
an arbitrary fixed point and is guaranteed to move only forwards at a steady
rate. Game timing measures durations, so it uses the monotonic clock — a
clock that can jump backwards can hand you a negative frame.

A frame's duration is *delta time*, `dt`: the wall-clock gap between one
frame's start and the next. Game logic integrates over dt — position is
advanced by `speed * dt`, not by a fixed amount per frame — so that the world
evolves at the same rate regardless of how many frames per second the machine
produces. Advance by a fixed amount per frame and the snake moves twice as
fast on a 120 Hz display as on a 60 Hz one; advance by `speed * dt` and both
take the same crossing time. Integrate raw dt per frame, though, and a
two-second stall — a breakpoint in `gdb`, say — hands the snake a two-second
jump straight through a wall.

The fix for that is the **fixed timestep**: game logic does not see real dt at
all. It runs in quanta of a fixed step — here `TICK_LEN`, 100 ms, giving ten
*updates per second* — and a small accumulator converts real elapsed time
into ticks. Each frame adds its dt to `tick_accum`; every full `TICK_LEN` in
there is drained away as one tick. Real time is now quarantined in the
accumulator, where a stall simply means several ticks fire back-to-back
(catching up, bounded), and the simulation's speed is a constant of the
program: `TICK_LEN` is game law, not hardware behavior. The trace you print
per frame reports both counters — `frame` for loop iterations, `tick` for
game updates — plus the measured dt, and the two counts drift apart on
purpose.

Last piece: the **frame cap**. Nothing in the loop above slows it down; a
busy loop would spin at thousands of frames per second and burn a core to
draw the same picture repeatedly. `FRAME_LEN` sets a minimum frame duration
(here 1/30 s) and the loop sleeps out the remainder of each frame's budget
with `nanosleep` — the argument is a `struct timespec`, and sleeping the
*remainder* (never a fixed amount) means the rate stays right even though
frames themselves take real work. The cap is why a bounded run is also a
timer: with 30 frames per second and 30 frames to run, `./snek 30` is
finished in about a second, and `time ./snek 30` proves it. Predict the
details before you run it — exercise 1 is waiting.

The build command is unchanged from lesson 019:

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

## Code step

One change for this lesson: `snek.c` grows the monotonic clock, the fixed
timestep accumulator, and the frame cap. Its end state is tagged
`lesson-020`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.c
index a5bb4a5..7c77efd 100644
--- a/sandbox/snek/snek.c
+++ b/sandbox/snek/snek.c
@@ -1,21 +1,49 @@
 // snek.c — a terminal snake game, grown lesson by lesson.
 //
-// Lesson 019: the game loop — ProcessInput, Update, Render, frame by frame.
+// Lesson 020: timing — clock_gettime, a fixed timestep, and a frame cap.
+#define _POSIX_C_SOURCE 200809L /* clock_gettime and nanosleep are POSIX, not ISO C */
 #include <errno.h>
 #include <stdio.h>
 #include <stdlib.h>
+#include <time.h>
+
+static const double TICK_LEN = 1.0 / 10.0; /* fixed timestep: 10 updates per second */
+static const double FRAME_LEN = 1.0 / 30.0; /* frame cap: 30 frames per second */
 
 static int running;              /* the loop runs while this is true */
 static unsigned long frame;      /* frames since the loop started */
+static unsigned long tick;       /* game updates since the loop started */
+static double tick_accum;        /* seconds of game time not yet ticked away */
+static double frame_dt;          /* measured length of the current frame */
 static unsigned long max_frames; /* stop after this many frames */
 
+static double Now(void)
+{
+    struct timespec ts;
+    clock_gettime(CLOCK_MONOTONIC, &ts);
+    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
+}
+
+static void SleepSec(double sec)
+{
+    struct timespec ts;
+    ts.tv_sec = (time_t)sec;
+    ts.tv_nsec = (long)((sec - (double)ts.tv_sec) * 1e9);
+    nanosleep(&ts, NULL);
+}
+
 static void ProcessInput(void)
 {
     // No keyboard yet — lesson 021 teaches the terminal.
 }
 
-static void Update(void)
+static void Update(double dt)
 {
+    tick_accum += dt;
+    while (tick_accum >= TICK_LEN) {
+        tick_accum -= TICK_LEN;
+        ++tick;
+    }
     ++frame;
     if (frame >= max_frames)
         running = 0;
@@ -23,7 +51,7 @@ static void Update(void)
 
 static void Render(void)
 {
-    fprintf(stderr, "frame=%lu\n", frame);
+    fprintf(stderr, "frame=%lu tick=%lu dt=%.4f\n", frame, tick, frame_dt);
 }
 
 int main(int argc, char **argv)
@@ -43,12 +71,21 @@ int main(int argc, char **argv)
     }
 
     running = 1;
+    double prev = Now();
     while (running) {
+        double frame_start = Now();
+        frame_dt = frame_start - prev;
+        prev = frame_start;
+
         ProcessInput();
-        Update();
+        Update(frame_dt);
         Render();
+
+        double rem = FRAME_LEN - (Now() - frame_start);
+        if (rem > 0)
+            SleepSec(rem);
     }
 
-    fprintf(stderr, "done after %lu frames\n", frame);
+    fprintf(stderr, "done after %lu frames, %lu ticks\n", frame, tick);
     return 0;
 }
```

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — Predicting the ticks *(predict-the-output)*

Before running anything, write down two numbers for `./snek 30`: the
wall-clock duration you expect (check it with `time ./snek 30`), and the
final `tick` value in the `done after …` line. Then run it and account for
every frame of difference between your prediction and the machine. To see the
mechanism frame by frame, add one instrumenting line inside the tick loop —
`fprintf(stderr, "tick=%lu accum=%.4f\n", tick, tick_accum);` after `++tick;`
— and use its output to explain exactly when ticks fire and what the first
frame's contribution looks like.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-020/ex1.md)

### Exercise 2 — What the frame cap costs *(measure-the-performance)*

Measure what `FRAME_LEN` actually does. Time `./snek 100` with the cap in
place; then disable the cap — setting the frame budget to zero is enough —
and time it again. Also run `./snek 100000` uncapped and look at both the
elapsed time and the `tick` count. How much wall time did the cap add, why
does `dt` change so drastically, and why does the tick count *not* scale with
the frame count when the cap is off?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-020/ex2.md)

### Exercise 3 — Two clocks *(explain-in-prose)*

Explain in prose why the game measures its dt with `CLOCK_MONOTONIC` and not
`CLOCK_REALTIME`: what each clock guarantees, what an NTP correction does to
each, and what a negative dt would do to the accumulator and the game beyond
it. To compare the clocks side by side before you write, extend `Render` with
two `clock_gettime` calls — one per clock — and print both readings each
frame; run a few seconds of frames and describe what the two numbers actually
count.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-020/ex3.md)

### Exercise 4 — A thing that moves *(extend-the-code)*

Give the game its first piece of motion: a `pos` variable that integrates
over the fixed timestep at one cell per second (`pos += 1.0 * TICK_LEN;` in
the tick loop), printed in the trace. Run `./snek 30` and confirm the
position is a function of ticks alone; then run an uncapped build from
exercise 2 for a few million frames and confirm the same. This is exactly how
the snake will move in lesson 022.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-020/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 019 — the game loop](lesson-019-game-loop.md) ·
**Next:** [Lesson 021 — raw terminal input with escape codes](lesson-021-terminal-input.md) ·
**Code tag:** [`lesson-020`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-020)
