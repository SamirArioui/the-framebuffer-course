# Lesson 034 — the first interactive frame

{{#include ../../stability-horizon.md}}

## Prose

Three lessons' worth of machinery meets today: polled input, an owned
framebuffer, and presentation through the seam. The result is the smallest
complete thing an engine does — **the interactive frame**: read the input,
update the world, draw it, show it. One yellow square, moved by the arrow
keys. Everything Part 2 and beyond will do to this loop is an expansion of
it, never a replacement.

### The frame step

The loop is four moves with one rule each:

```c++
while (!platform::CloseRequested(opened.window)) {
    platform::PumpEvents(opened.window);   /* news -> state */
    ...                                     /* react to close first */
    ...                                     /* update: read state */
    ...                                     /* render: draw the scene */
    platform::Present(...);                 /* show it */
}
```

- **Pump** folds news into state and returns. It is the only place events
  exist (lesson 028's rule).
- **Update** reads *state* — four `KeyDown` calls, one per arrow — and
  moves the marker. A frame never handles events; it asks questions.
- **Render** draws the whole scene into the framebuffer: clear, then the
  marker. Every frame, the whole scene. Partial redraws are an
  optimization with a bookkeeping cost, and the honest version is a full
  redraw of our own buffer — 307,200 pixels of `ClearBuffer` is nothing to
  a machine and everything to a reader.
- **Present** copies the buffer to the window (lesson 030's XImage,
  lesson 031's contract).

The update reads state, so two things fall out of the shape instead of
being special-cased: a held key keeps moving the marker (the state stays
down), and a tap moves it exactly as far as its frames say (lesson 033's
latch is what makes sure the tap is seen at all).

### Keeping the marker honest

One piece of game logic lives in the update besides the movement: the
marker stays on the screen. The clamp is lesson 015's fold at frame scale —
the position is checked after the move, and the marker stops at the edge
instead of walking off into coordinates that have no pixels. Every `PutPixel`
in `DrawMarker` is *also* clipped (the fold again), so the two layers of
honesty do not depend on each other: the clamp is about the game's rules,
the clip is about memory.

### How fast is a frame?

Here is the honest edge of this lesson: **frames arrive when news
arrives**. The pump blocks between batches, so the loop runs when events
run — and while a key is held, the news that keeps frames coming is the
keyboard's auto-repeat. The marker's step is the engine's (`MARKER_STEP`,
eight pixels); the marker's *rate* is still the keyboard's.

That is not a design decision to keep; it is the boundary of what exists
before the clock does. Lesson 035 gives the engine a monotonic clock and
the frame its `dt`, and from then on the step is `speed × elapsed` —
pixels per second, no matter who wakes the loop. (The spin-loop alternative
— run frames as fast as the CPU allows — is worse in every way and dies
here without ever being born.)

### The marker, verified

Scripted input, ten repeats of the right arrow at fifty milliseconds apart:

```
$ DISPLAY=:99 ./build/game &
$ DISPLAY=:99 xdotool key --delay 50 --repeat 10 --window <id> Right
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
engine: marker at 316,228
engine: marker at 324,228
...
engine: marker at 388,228
engine: close reported
engine: closed
```

Ten presses, ten frames, ten steps of eight pixels — the report says the
marker is at `388,228`, and the window agrees: locating the marker's color
in the presented pixels finds it exactly there.

```
locate: marker pixels span 308,228 .. 330,250 (size 23x23)
locate: marker pixels span 388,228 .. 410,250 (size 23x23)
```

(Sampled every other pixel, hence 23 of the 24 rows and columns.) The
pixels the engine wrote are the pixels the window shows, at the position
the engine says — the loop is not just moving a variable, it is moving a
thing you can see.

## Code step

One change for this lesson: `main.cpp` grows the interactive frame — a
marker moved by polled arrow keys, clamped to the screen, redrawn in full
each frame and presented — and the one-shot test pattern of lesson 030 is
replaced by the loop every later part builds on. Its end state is tagged
`lesson-034`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 0d0d64b..47d7360 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -12,10 +12,18 @@
 
 namespace engine {
 
-/* The seam's keys, by name — for the report below. */
-static const char *const key_names[platform::KEY_COUNT] = {
-    "up", "down", "left", "right", "space", "enter", "escape",
-};
+/* The marker: one square the arrow keys move. Its position is whole
+   pixels and its speed is pixels-per-frame — lesson 035's clock turns
+   that into pixels-per-second. */
+constexpr int MARKER_SIZE = 24;
+constexpr int MARKER_STEP = 8;
+
+static void DrawMarker(Framebuffer &fb, int x, int y)
+{
+    for (int j = 0; j < MARKER_SIZE; ++j)
+        for (int i = 0; i < MARKER_SIZE; ++i)
+            PutPixel(fb, x + i, y + j, 240, 220, 80);
+}
 
 int Run(void)
 {
@@ -39,69 +47,50 @@ int Run(void)
         return 1;
     }
 
-    /* Paint: clear, then pixels — the two instincts Part 0's paint taught,
-       now onto the engine's own buffer. */
+    /* The scene: a marker the arrow keys move. The report below is its
+       position — the interactive frame makes itself observable. */
     Framebuffer *fb = GetFramebuffer();
-    ClearBuffer(*fb, 32, 32, 64);
-    PutPixel(*fb, 0, 0, 255, 0, 0);
-    PutPixel(*fb, 639, 479, 0, 255, 0);
-    PutPixel(*fb, 700, 100, 0, 0, 255); /* out of bounds: dropped */
-
-    /* The byte-level report: what is actually in the buffer. */
-    unsigned char r, g, b;
-    std::printf("engine: framebuffer %dx%d, %d bytes, stride %d\n",
-                fb->width, fb->height, fb->width * fb->height * 4,
-                fb->width * 4);
-    GetPixel(*fb, 0, 0, r, g, b);
-    std::printf("engine: pixel (0,0) = %d %d %d\n", r, g, b);
-    GetPixel(*fb, 639, 479, r, g, b);
-    std::printf("engine: pixel (639,479) = %d %d %d\n", r, g, b);
-    GetPixel(*fb, 60, 101, r, g, b);
-    std::printf("engine: pixel (60,101) = %d %d %d\n", r, g, b);
-
-    /* First light: the bytes go to the window through the seam. */
-    if (!platform::Present(opened.window, fb->pixels, fb->width, fb->height)) {
-        std::fprintf(stderr, "engine: presentation failed\n");
-        platform::CloseWindow(opened.window);
-        return 1;
-    }
-    std::printf("engine: presented\n");
-
-    /* The frame step: read news, react, present our pixels, repeat.
-       Presentation is not an event — it is the engine's answer to every
-       event: the pixels the engine wrote are the pixels the window shows,
-       and re-presenting is what repairs the window when the OS damaged it.
-       React first: if the news was "the window is gone", there is nothing
-       left to present to. */
+    int marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2;
+    int marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2;
+    std::printf("engine: arrow keys move the marker; close the window to stop\n");
+    std::printf("engine: marker at %d,%d\n", marker_x, marker_y);
+
+    /* The frame step: read news, update from polled state, draw, present.
+       This is the shape every later part fills in — Part 2 draws into it,
+       Part 5 measures it. */
     int exit_code = 0;
-    bool had_focus = platform::HasFocus(opened.window);
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
             break;
 
-        /* The polled state: what is down right now, as of this poll. */
-        std::printf("engine: polled:");
-        bool any = false;
-        for (int k = 0; k < platform::KEY_COUNT; ++k) {
-            if (platform::KeyDown(opened.window, (platform::Key)k)) {
-                std::printf(" %s", key_names[k]);
-                any = true;
-            }
-        }
-        std::printf(any ? "\n" : " -\n");
-
-        /* The latches: presses that ended before this poll are not lost. */
-        for (int k = 0; k < platform::KEY_COUNT; ++k)
-            if (platform::KeyPressed(opened.window, (platform::Key)k))
-                std::printf("engine: pressed %s\n", key_names[k]);
-
-        /* Focus: reported when it changes. */
-        bool focus = platform::HasFocus(opened.window);
-        if (focus != had_focus) {
-            std::printf("engine: focus %s\n", focus ? "gained" : "lost");
-            had_focus = focus;
-        }
+        /* Update: a frame reads state — it never handles events. */
+        int old_x = marker_x, old_y = marker_y;
+        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
+            marker_x -= MARKER_STEP;
+        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
+            marker_x += MARKER_STEP;
+        if (platform::KeyDown(opened.window, platform::KEY_UP))
+            marker_y -= MARKER_STEP;
+        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
+            marker_y += MARKER_STEP;
+
+        /* The marker stays on screen — lesson 015's fold at frame scale. */
+        if (marker_x < 0)
+            marker_x = 0;
+        if (marker_x > FRAME_WIDTH - MARKER_SIZE)
+            marker_x = FRAME_WIDTH - MARKER_SIZE;
+        if (marker_y < 0)
+            marker_y = 0;
+        if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
+            marker_y = FRAME_HEIGHT - MARKER_SIZE;
+
+        if (marker_x != old_x || marker_y != old_y)
+            std::printf("engine: marker at %d,%d\n", marker_x, marker_y);
+
+        /* Render: every frame draws the whole scene — clear, then marker. */
+        ClearBuffer(*fb, 32, 32, 64);
+        DrawMarker(*fb, marker_x, marker_y);
 
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Eight directions *(extend-the-code)*

Four `if`s give four directions; a marker wants eight. Rework the update so
the two axes are read independently — one step by the summed direction —
and show the diagonal working with a chord of two held arrows
(`xdotool key --delay 50 --repeat 5 --window <id> Right+Down`). While you
are in there: what does the marker do when left *and* right are both down,
and why is that the right answer?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-034/ex1.md)

### Exercise 2 — The speed that belongs to the keyboard *(port-to-your-own-machine)*

The marker moves when a key is held — but *how fast* is not the engine's
answer yet. Make the frame countable: one instrumenting line that reports
the frame number alongside each movement. Then hold an arrow key on your
own desktop for one second and count: how many steps, and how many frames?
Explain what the numbers say about who currently owns the marker's speed —
and what lesson 035 will have to give the engine for that to change.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-034/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 033 — latching brief presses and tracking focus](lesson-033-latching.md) ·
**Next:** [Lesson 035 — the platform clock](lesson-035-clock.md) ·
**Code tag:** [`lesson-034`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-034)
