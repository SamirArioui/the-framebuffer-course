# Lesson 046 — the sprite moves

{{#include ../../stability-horizon.md}}

## Prose

The marker retires today. It was a square drawn out of `PutPixel` calls —
a placeholder with a job: hold the input-driven part of the loop until
something real could stand there. That something is the sprite, and this
lesson moves it with the same polled input the marker used. Nothing about
the loop changes: pump, update, render, present — the shape lesson 043
settled, kept honest by lesson 036's record. The change is what the loop
*draws*, and what the record can *say* about it: for the first time, a
frame's render phase names where its time went.

### The marker retires

The update phase is unchanged in structure: the arrow keys' polled state
adds `speed × dt` to the sprite's position, and the sprite stays on screen
by lesson 015's fold at frame scale. What changed is the object:

- `DrawMarker` and `MARKER_SIZE` are **gone**. The 24×24 square that was
  redrawn every frame from four nested `PutPixel` loops is replaced by one
  call: `BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y)`.
- The sprite's size comes from the asset (`sprite.width`, `sprite.height`),
  not from a constant. The clamp is `FRAME_WIDTH - sprite.width` — the
  same arithmetic, now reading the thing it is keeping on screen.
- The reports say `sprite` where they said `marker`, and the position they
  report is the position the blit draws at.

The run under scripted input (a held Right arrow, then Down):

```
$ DISPLAY=:99 ./build/game
engine: sprite assets/sprite.ppm: 16x16, 768 pixel bytes
...
engine: arrow keys move the sprite; close the window to stop
engine: sprite at 312,232
frame 1: update 0.000 ms, render 0.372 ms (sprites 0.001), present 0.565 ms, total 0.938 ms
frame 2: update 0.000 ms, render 0.400 ms (sprites 0.001), present 0.928 ms, total 1.329 ms
engine: sprite at 470,232 (t=2.661)
engine: sprite at 480,232 (t=2.701)
engine: sprite at 490,232 (t=2.742)
...
engine: sprite at 547,235 (t=3.031)
...
engine: 15 frames — avg 0.913 ms (update 0.000, render 0.435 incl. sprites 0.001, present 0.478)
engine: worst frame 1.329 ms (frame 2); present is 52% of the frame
```

The readback check confirms the pixels and the report agree: at the
reported position `547,235`, the window holds the background at the
sprite's key-colored corner `(547,235)` and the sprite's own center color
`(220, 40, 40)` at `(555, 243)` — the sprite's `(8, 8)` pixel, exactly
where the report put the sprite. The engine is not moving a variable; it
is moving a thing the window shows.

### The record names its phases

`frame.h`'s record grows one field, and it is the beginning of the
frame-budget report:

```c++
    /* Lesson 046: the render phase starts naming what is inside it — one
       field per subsystem, the attribution the frame-budget table
       (lesson 058) grows from. The named times are inside render, never
       instead of it: render stays the phase, these say where it went. */
    double sprites; /* sprite draws through the blit */
```

The measurement is the same two-clock-reads discipline as every phase:
read `platform::Now()` before the blit, read it after, subtract. The log
line grows the named field where it belongs — inside the render phase it
belongs to:

```
frame 1: update 0.000 ms, render 0.372 ms (sprites 0.001), present 0.565 ms, total 0.938 ms
```

and the account sums it like it sums everything else, so the run's summary
can attribute the average frame:

```
engine: 15 frames — avg 0.913 ms (update 0.000, render 0.435 incl. sprites 0.001, present 0.478)
```

Two rules keep the attribution honest as it grows (they are what makes the
final table trustworthy rather than decorative):

1. **Named times live inside their phase, never instead of it.** `render`
   stays in the record exactly as lesson 036 published it. The sub-fields
   say where the render went; they do not replace what it measured.
2. **The format grows, the shape does not.** One line per record, fields
   named — lesson 036's contract was a stable *text* format, not a frozen
   one. Text and tilemap phases will join `sprites` the same way, in the
   lessons that introduce them.

### And the first number is a surprise

Look at the account again: `render 0.435 incl. sprites 0.001`. The frame
spends 435 microseconds rendering and **one** of them drawing the sprite.
Where did the other 434 go? Into `ClearBuffer` — painting all 640 × 480 =
307,200 pixels of background before anything is drawn on it. The sprite is
256 pixels of real drawing work; the clear is twelve hundred times more
pixels of "nothing to see here".

This is the first honest cost of a software renderer, and it is exactly
what the next lesson is about. Not "how do we make it faster" — that is
Part 5's job — but *why is copying bytes this expensive*, which turns out
to be a question about the machine, not about the code. The frame record's
named phases earned their keep on their first outing: without `sprites` in
the log, "render" would have looked like one opaque number and the story
inside it would have been invisible.

## Code step

One change for this lesson: `main.cpp` retires `DrawMarker` and moves the
sprite with the same polled input, `frame.h` / `frame.cpp` grow the
record's first named render sub-phase (`sprites`) and its sum in the
account, and the log line and summary carry the name. The blit, the
sprite, and the asset are untouched. Its end state is tagged
`lesson-046`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index d5e2920..e104e47 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -13,6 +13,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.render_sum += frame.render;
     stats.present_sum += frame.present;
     stats.total_sum += frame.total;
+    stats.sprites_sum += frame.sprites;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
diff --git a/src/frame.h b/src/frame.h
index 3ca1cdb..bea2fbf 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -17,6 +17,12 @@ struct FrameRecord {
     double render; /* drawing the scene into the framebuffer */
     double present;/* the copy to the window, sync included */
     double total;  /* the whole frame step */
+
+    /* Lesson 046: the render phase starts naming what is inside it — one
+       field per subsystem, the attribution the frame-budget table
+       (lesson 058) grows from. The named times are inside render, never
+       instead of it: render stays the phase, these say where it went. */
+    double sprites; /* sprite draws through the blit */
 };
 
 /* The running account: every frame measured so far. */
@@ -26,6 +32,7 @@ struct FrameStats {
     double render_sum;
     double present_sum;
     double total_sum;
+    double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
 };
diff --git a/src/main.cpp b/src/main.cpp
index 7b750ac..1f8ad5c 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -16,17 +16,10 @@
 
 namespace engine {
 
-/* The marker: one square the arrow keys move. Its speed is the engine's —
-   pixels per second — and the clock's dt turns it into a per-frame step. */
-constexpr int MARKER_SIZE = 24;
-constexpr double MARKER_SPEED = 240.0; /* pixels per second */
-
-static void DrawMarker(Framebuffer &fb, int x, int y)
-{
-    for (int j = 0; j < MARKER_SIZE; ++j)
-        for (int i = 0; i < MARKER_SIZE; ++i)
-            PutPixel(fb, x + i, y + j, 240, 220, 80);
-}
+/* The scene's one object: the sprite the arrow keys move. Its speed is
+   the engine's — pixels per second — and the clock's dt turns it into a
+   per-frame step. */
+constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
 int Run(void)
 {
@@ -159,14 +152,14 @@ int Run(void)
     std::printf("engine: blit check: clip at -4,-4 landed %d pixels, %d wrong, %d touched outside\n",
                 landed, wrong, wrapped);
 
-    double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
-    double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
+    double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
+    double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
     double last = started;
 
     std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
-    std::printf("engine: arrow keys move the marker; close the window to stop\n");
-    std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
+    std::printf("engine: arrow keys move the sprite; close the window to stop\n");
+    std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
     /* The frame step: read news, update from polled state, draw, present —
        every phase measured, one record per frame. */
@@ -187,38 +180,40 @@ int Run(void)
         double dt = now - last;
         last = now;
 
-        int old_x = (int)marker_x, old_y = (int)marker_y;
+        int old_x = (int)sprite_x, old_y = (int)sprite_y;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            marker_x -= MARKER_SPEED * dt;
+            sprite_x -= SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            marker_x += MARKER_SPEED * dt;
+            sprite_x += SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_UP))
-            marker_y -= MARKER_SPEED * dt;
+            sprite_y -= SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            marker_y += MARKER_SPEED * dt;
+            sprite_y += SPRITE_SPEED * dt;
 
-        /* The marker stays on screen — lesson 015's fold at frame scale. */
-        if (marker_x < 0)
-            marker_x = 0;
-        if (marker_x > FRAME_WIDTH - MARKER_SIZE)
-            marker_x = FRAME_WIDTH - MARKER_SIZE;
-        if (marker_y < 0)
-            marker_y = 0;
-        if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
-            marker_y = FRAME_HEIGHT - MARKER_SIZE;
+        /* The sprite stays on screen — lesson 015's fold at frame scale. */
+        if (sprite_x < 0)
+            sprite_x = 0;
+        if (sprite_x > FRAME_WIDTH - sprite.width)
+            sprite_x = FRAME_WIDTH - sprite.width;
+        if (sprite_y < 0)
+            sprite_y = 0;
+        if (sprite_y > FRAME_HEIGHT - sprite.height)
+            sprite_y = FRAME_HEIGHT - sprite.height;
 
         frame.update = platform::Now() - t0;
         double t1 = platform::Now();
 
-        if ((int)marker_x != old_x || (int)marker_y != old_y)
-            std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
-                        (int)marker_y, platform::Now() - started);
+        if ((int)sprite_x != old_x || (int)sprite_y != old_y)
+            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
+                        (int)sprite_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then the
-           sprite through the one blit. */
+           sprite through the one blit. The sprite draw is timed as its own
+           named phase: the first subsystem the frame record can name. */
         ClearBuffer(*fb, 32, 32, 64);
-        BlitSprite(*fb, sprite, 32, 32);
-        DrawMarker(*fb, (int)marker_x, (int)marker_y);
+        double t_sprites = platform::Now();
+        BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
+        frame.sprites = platform::Now() - t_sprites;
 
         frame.render = platform::Now() - t1;
         double t2 = platform::Now();
@@ -239,20 +234,22 @@ int Run(void)
         frame.total = platform::Now() - t0;
         AccountFrame(stats, frame);
 
-        /* The frame log: one line per record — the format Part 2 grows. */
-        std::printf("frame %ld: update %.3f ms, render %.3f ms, present %.3f ms, total %.3f ms\n",
+        /* The frame log: one line per record — the format grows its named
+           fields, one per subsystem, as the parts name them. */
+        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.update * 1e3, frame.render * 1e3,
-                    frame.present * 1e3, frame.total * 1e3);
+                    frame.sprites * 1e3, frame.present * 1e3,
+                    frame.total * 1e3);
     }
 
     /* The account: what the frames actually cost, including the honest
        price of the presentation copy. */
     if (stats.frames) {
         double n = (double)stats.frames;
-        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f, present %.3f)\n",
+        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, present %.3f)\n",
                     stats.frames, stats.total_sum / n * 1e3,
                     stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
-                    stats.present_sum / n * 1e3);
+                    stats.sprites_sum / n * 1e3, stats.present_sum / n * 1e3);
         std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
                     stats.worst * 1e3, stats.worst_number,
                     100.0 * stats.present_sum / stats.total_sum);
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Where does render go? *(measure-the-performance)*

The account says `render 0.435 incl. sprites 0.001` — so name the missing
phase and measure it: give `ClearBuffer` its own named field in the record
(`clear`), time it the same way, and carry both names through the log line
and the account's summary. Before you run: predict how the two named
phases will split the render time, and say what the split means for a
renderer that draws one 16×16 sprite per frame. Then run and reconcile —
and note which phase a second sprite would change.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-046/ex1.md)

### Exercise 2 — The sprite that wraps *(extend-the-code)*

The clamp is a policy, not a law. Make the sprite wrap: leave one edge of
the frame and reappear at the opposite one, moving in the same direction —
and let it travel fully outside the frame on its way, with the blit's
clipping keeping the pixels honest in the crossing frames. Verify with
scripted input and a readback at the reported position on both sides of an
edge. What did the clamp buy you that the wrap now costs?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-046/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 045 — the clipped, transparent blit](lesson-045-blit.md) ·
**Next:** [Lesson 047 — the caches deep dive](lesson-047-caches.md) ·
**Code tag:** [`lesson-046`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-046)
