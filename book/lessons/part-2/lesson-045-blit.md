# Lesson 045 — the clipped, transparent blit

{{#include ../../stability-horizon.md}}

## Prose

Last lesson the sprite's bytes arrived; today they become pixels. This is
the renderer: **one copy loop** — the *blitter* — that takes source pixels
into the framebuffer at an origin, obeying exactly two rules. A pixel of
the sprite's transparent color writes **nothing** (transparency), and a
pixel whose destination falls outside the framebuffer is **dropped**
(clipping). Everything Part 2 draws from here — glyphs, tiles, the closing
demo's world — rides this one loop. That is deliberate: the deep dives of
lessons 047-049 measure and read *one honest unit of work*, and Part 5's
optimizer fixes the same unit.

### Two rules, one loop

**Transparency** is the sprite naming one color as *nothing*. Ours is
magenta — `255, 0, 255` — and it is the first rule the copy loop applies:
if the source pixel is the key color, skip it, leaving whatever was in the
framebuffer exactly as it was. Not black, not blended: *nothing*. The key
color is data on the sprite (`key_r, key_g, key_b` in `Sprite`), because
the same sprite can be drawn by anything later and "which color means
nothing" belongs to the image, not to the draw call. PPM has no field for
it, so the course fixes the convention at load: magenta is the key.

**Clipping** is lesson 015's fold at rectangle scale. A sprite drawn at
`(-4, -4)` has its first four rows and columns hanging outside the frame;
those pixels are dropped — never wrapped into other rows, never written
to memory that is not the framebuffer. The fold was per-pixel in
`PutPixel`; the blitter makes the same decision *once per edge*: compute
the rectangle where sprite and framebuffer overlap, and let the copy loop
run only there.

### The loop

```c++
void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
{
    /* The fold, at rectangle scale: the in-bounds region computed once,
       so the copy loop below never checks bounds again. */
    int left = x < 0 ? 0 : x;
    int top = y < 0 ? 0 : y;
    int right = x + s.width < fb.width ? x + s.width : fb.width;
    int bottom = y + s.height < fb.height ? y + s.height : fb.height;

    for (int j = top; j < bottom; ++j) {
        for (int i = left; i < right; ++i) {
            const unsigned char *src =
                &s.pixels[(((size_t)(j - y) * s.width) + (i - x)) * 3];
            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
                continue; /* the transparent color writes nothing */
            unsigned char *dst =
                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
            dst[0] = src[2]; /* blue */
            dst[1] = src[1]; /* green */
            dst[2] = src[0]; /* red */
            dst[3] = 0;
        }
    }
}
```

Read it in four moves:

- **The clip rectangle.** Four clamps turn the requested origin into the
  rectangle `left, top .. right, bottom` — the intersection of the
  sprite's box with the frame. Drawn fully inside? The clamps change
  nothing. Hanging off an edge? The rectangle shrinks to what survives.
  The inner loop then has no bounds check at all: every pixel it touches
  is known to be inside. One decision per edge instead of one per pixel —
  and the loop is cheaper for it, which lesson 047 will make concrete.
- **The source index.** `(j - y, i - x)` maps a destination pixel back to
  the sprite pixel that belongs there. The sprite's `(0, 0)` lands at
  `(x, y)`; the clip never moved that origin, it only refused to draw
  outside it.
- **The key check.** Three byte comparisons and a `continue`. This is the
  whole of transparency: the pixel writes nothing, and the copy moves on.
- **The copy itself.** Three bytes out of the sprite, four bytes into the
  framebuffer — and *swapped*: the framebuffer's pixel is blue, green,
  red, one unused byte (lesson 030's format, what `Present` carries),
  while the PPM's pixel is red, green, blue. The copy writes `dst[0] =
  src[2]` and friends; the sprite's colors arrive on screen unchanged.
  The bytes are written directly — not through `PutPixel` — because this
  inner loop is the one the next four lessons measure, read, and vectorize.

### The claims, checked

The code step's check block draws the sprite through the blit and reads the
framebuffer back with `GetPixel`, one pixel at a time — the same readback
habit lesson 044 used on the file:

```
$ DISPLAY=:99 ./build/game
engine: sprite assets/sprite.ppm: 16x16, 768 pixel bytes
engine: pixel 0,0 = 255,0,255
engine: pixel 8,8 = 220,40,40
engine: pixel bytes sum to 125580
engine: blit check: 130 opaque pixels drawn unchanged, 0 mismatches
engine: blit check: 126 key pixels wrote nothing over the background
engine: blit check: clip at -4,-4 landed 144 pixels, 0 wrong, 0 touched outside
engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames
...
frame 1: update 0.000 ms, render 0.381 ms, present 0.602 ms, total 0.984 ms
```

Three claims, three verdicts:

- **Drawn unchanged** — all 130 opaque pixels read back with exactly the
  color the file holds, and zero mismatches. The copy is faithful:
  source bytes to framebuffer pixels, order swapped, values preserved.
- **Transparency writes nothing** — the sprite's 126 key-colored pixels
  drew *over* a cleared background and left it untouched at every one of
  them. The count matters: 130 + 126 = 256 = 16 × 16. Every pixel is
  accounted for — either copied or deliberately not.
- **Clipping drops** — drawn at `(-4, -4)`, exactly `12 × 12 = 144` pixels
  landed (the rows and columns that survive), 0 landed wrong, and 0 pixels
  *outside* the landed rectangle were touched. That last column is the
  wrap detector: a loop that wrote dropped pixels at wrapped addresses
  would light it up.

The window shows the same story from the OS side (lesson 031's readback,
sampled at the sprite's on-screen position `(32, 32)`):

```
32,32 -> r=32 g=32 b=64      <- sprite pixel (0,0): the key — background shows through
36,36 -> r=32 g=32 b=64      <- sprite pixel (4,4): also key
44,44 -> r=220 g=40 b=40     <- sprite pixel (12,12): red, copied unchanged
100,100 -> r=32 g=32 b=64    <- outside the sprite: untouched
```

The first two lines are the transparency rule, visible through the whole
presentation path: the sprite's magenta corners are not on screen. The
pixels behind them are.

### One loop to rule the drawing

The sprite sits at a fixed spot today and the marker still moves. The
point of the lesson is not the picture — it is the funnel. Lesson 050's
glyphs will be sprites cut out of a font sheet; lesson 053's tiles will be
sprites cut from a tile sheet; both will draw through `BlitSprite` with
nothing but bookkeeping around the call. One clip, one key check, one copy
— the renderer's whole behavior is auditable in thirty-odd lines, and the
deep dives that follow have exactly one loop to hold up to the light.

## Code step

One change for this lesson: `src/blit.h` / `src/blit.cpp` bring the
blitter (the clipped, transparent copy loop), `Sprite` grows the
transparent color the loader fills from the course convention, and
`main.cpp` checks the blit's three claims against the framebuffer at
startup and draws the sprite through it every frame. The marker is
untouched. Its end state is tagged `lesson-045`.

```diff
diff --git a/src/blit.cpp b/src/blit.cpp
new file mode 100644
index 0000000..1c52f0d
--- /dev/null
+++ b/src/blit.cpp
@@ -0,0 +1,38 @@
+// blit.cpp — the copy loop.
+//
+// Lesson 045: the engine's one drawing loop. It clips first — the
+// intersection of the sprite's rectangle with the framebuffer is the only
+// region that can be drawn — then copies bytes: three of the sprite's
+// bytes into four of the framebuffer's, in the framebuffer's order,
+// skipping pixels of the transparent color.
+
+#include "blit.h"
+
+namespace engine {
+
+void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
+{
+    /* The fold, at rectangle scale: the in-bounds region computed once,
+       so the copy loop below never checks bounds again. */
+    int left = x < 0 ? 0 : x;
+    int top = y < 0 ? 0 : y;
+    int right = x + s.width < fb.width ? x + s.width : fb.width;
+    int bottom = y + s.height < fb.height ? y + s.height : fb.height;
+
+    for (int j = top; j < bottom; ++j) {
+        for (int i = left; i < right; ++i) {
+            const unsigned char *src =
+                &s.pixels[(((size_t)(j - y) * s.width) + (i - x)) * 3];
+            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
+                continue; /* the transparent color writes nothing */
+            unsigned char *dst =
+                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
+            dst[0] = src[2]; /* blue */
+            dst[1] = src[1]; /* green */
+            dst[2] = src[0]; /* red */
+            dst[3] = 0;
+        }
+    }
+}
+
+} /* namespace engine */
diff --git a/src/blit.h b/src/blit.h
new file mode 100644
index 0000000..259ddf3
--- /dev/null
+++ b/src/blit.h
@@ -0,0 +1,25 @@
+// blit.h — the blitter: one clipped, transparent copy from a sprite's
+// bytes into the framebuffer.
+//
+// Lesson 045: every drawn pixel in the game comes through this loop —
+// sprites now, glyphs and tiles later. One copy, one place where clipping
+// and transparency live, and one honest unit of work for the deep dives of
+// lessons 047-049 to measure and read.
+#ifndef BLIT_H
+#define BLIT_H
+
+#include "framebuffer.h"
+#include "sprite.h"
+
+namespace engine {
+
+/* Draws a sprite with its top-left corner at (x, y): each source pixel
+   becomes one framebuffer pixel carrying the exact color the sprite has,
+   except the sprite's transparent color, which writes nothing at all.
+   Pixels whose destination falls outside the framebuffer are dropped —
+   lesson 015's fold at rectangle scale, never a wrap into other pixels. */
+void BlitSprite(Framebuffer &fb, const Sprite &sprite, int x, int y);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 8febd4a..7b750ac 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -8,6 +8,7 @@
 #include <cstdio>
 
 #include "arena.h"
+#include "blit.h"
 #include "framebuffer.h"
 #include "frame.h"
 #include "platform.h"
@@ -100,6 +101,64 @@ int Run(void)
                                 sprite.width / 2) * 3 + 2]);
     std::printf("engine: pixel bytes sum to %ld\n", byte_sum);
 
+    /* Lesson 045: the blitter's three claims, checked against the
+       framebuffer's own bytes before anything depends on them. */
+    ClearBuffer(*fb, 32, 32, 64);
+    BlitSprite(*fb, sprite, 100, 100);
+    int opaque = 0, key_pixels = 0, mismatches = 0;
+    for (int j = 0; j < sprite.height; ++j)
+        for (int i = 0; i < sprite.width; ++i) {
+            const unsigned char *p =
+                &sprite.pixels[(j * sprite.width + i) * 3];
+            unsigned char r, g, b;
+            GetPixel(*fb, 100 + i, 100 + j, r, g, b);
+            bool is_key = p[0] == sprite.key_r && p[1] == sprite.key_g &&
+                          p[2] == sprite.key_b;
+            if (is_key) {
+                ++key_pixels;
+                if (r != 32 || g != 32 || b != 64)
+                    ++mismatches; /* the key must have written nothing */
+            } else {
+                ++opaque;
+                if (r != p[0] || g != p[1] || b != p[2])
+                    ++mismatches;
+            }
+        }
+    std::printf("engine: blit check: %d opaque pixels drawn unchanged, %d mismatches\n",
+                opaque, mismatches);
+    std::printf("engine: blit check: %d key pixels wrote nothing over the background\n",
+                key_pixels);
+
+    ClearBuffer(*fb, 32, 32, 64);
+    BlitSprite(*fb, sprite, -4, -4);
+    int landed = 0, wrong = 0, wrapped = 0;
+    for (int j = 0; j < sprite.height; ++j)
+        for (int i = 0; i < sprite.width; ++i) {
+            if (i < 4 || j < 4)
+                continue; /* these pixels landed outside and were dropped */
+            const unsigned char *p =
+                &sprite.pixels[(j * sprite.width + i) * 3];
+            unsigned char r, g, b;
+            GetPixel(*fb, i - 4, j - 4, r, g, b);
+            ++landed;
+            bool is_key = p[0] == sprite.key_r && p[1] == sprite.key_g &&
+                          p[2] == sprite.key_b;
+            if (is_key ? (r != 32 || g != 32 || b != 64)
+                       : (r != p[0] || g != p[1] || b != p[2]))
+                ++wrong;
+        }
+    for (int y = 0; y < FRAME_HEIGHT; ++y)
+        for (int x = 0; x < FRAME_WIDTH; ++x) {
+            if (x < sprite.width - 4 && y < sprite.height - 4)
+                continue; /* the landed region, checked above */
+            unsigned char r, g, b;
+            GetPixel(*fb, x, y, r, g, b);
+            if (r != 32 || g != 32 || b != 64)
+                ++wrapped;
+        }
+    std::printf("engine: blit check: clip at -4,-4 landed %d pixels, %d wrong, %d touched outside\n",
+                landed, wrong, wrapped);
+
     double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
     double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
     double started = platform::Now();
@@ -155,8 +214,10 @@ int Run(void)
             std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
                         (int)marker_y, platform::Now() - started);
 
-        /* Render: every frame draws the whole scene — clear, then marker. */
+        /* Render: every frame draws the whole scene — clear, then the
+           sprite through the one blit. */
         ClearBuffer(*fb, 32, 32, 64);
+        BlitSprite(*fb, sprite, 32, 32);
         DrawMarker(*fb, (int)marker_x, (int)marker_y);
 
         frame.render = platform::Now() - t1;
diff --git a/src/sprite.cpp b/src/sprite.cpp
index 449b4dc..4d04e96 100644
--- a/src/sprite.cpp
+++ b/src/sprite.cpp
@@ -64,7 +64,7 @@ bool ReadNumber(const unsigned char *data, size_t size, size_t &at, long &out)
 
 SpriteResult LoadSprite(Arena &arena, const char *path)
 {
-    SpriteResult result = { { 0, 0, 0 }, SPRITE_OK };
+    SpriteResult result = { { 0, 0, 0, 0, 0, 0 }, SPRITE_OK };
 
     platform::FileData file = platform::ReadFile(path);
     if (file.error != platform::FILE_OK) {
@@ -116,6 +116,9 @@ SpriteResult LoadSprite(Arena &arena, const char *path)
     result.sprite.pixels = pixels;
     result.sprite.width = (int)width;
     result.sprite.height = (int)height;
+    result.sprite.key_r = SPRITE_KEY_R;
+    result.sprite.key_g = SPRITE_KEY_G;
+    result.sprite.key_b = SPRITE_KEY_B;
     result.error = SPRITE_OK;
     return result;
 }
diff --git a/src/sprite.h b/src/sprite.h
index 5be7140..b9ef087 100644
--- a/src/sprite.h
+++ b/src/sprite.h
@@ -12,12 +12,21 @@
 
 namespace engine {
 
+/* The transparent color of the course's sprites: magenta. PPM carries no
+   key field, so the format's convention is the loader's job — every sprite
+   loaded here names 255,0,255 as "draw nothing". */
+constexpr unsigned char SPRITE_KEY_R = 255;
+constexpr unsigned char SPRITE_KEY_G = 0;
+constexpr unsigned char SPRITE_KEY_B = 255;
+
 /* A sprite: one image's pixels in the engine's memory — row after row,
-   three bytes each (red, green, blue), exactly the file's pixel section. */
+   three bytes each (red, green, blue), exactly the file's pixel section —
+   and the color that means "nothing" when it is drawn. */
 struct Sprite {
     unsigned char *pixels; /* width * height * 3 bytes */
     int width;
     int height;
+    unsigned char key_r, key_g, key_b; /* the transparent color */
 };
 
 /* A load either hands over a complete sprite or names what went wrong —
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Your own key *(extend-the-code)*

Magenta is a convention the loader invented — PPM has no field that could
name the key. Make the sprite's own bytes name it instead: the image's
top-left pixel *is* the transparent color, whatever color it is, and the
inspection lines print the key the loader chose. Then recolor a copy of the
sprite so its corner is a color you picked — lime, cyan, anything but
magenta — with magenta pixels as real art, and run the blit's three checks
against it. What does this convention cost an image that genuinely needs
its corner color as art?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-045/ex1.md)

### Exercise 2 — The four corners *(predict-the-output)*

The clip check draws at one position. Extend it to four: `(-4, -4)`,
`(632, 472)`, `(-100, 0)`, and `(640, 480)`. For each, write down the
number of sprite pixels you expect to land *before you run anything* — the
sprite is 16×16, the frame is 640×480, and the arithmetic is the exercise.
Then run, reconcile every count, and say what the `0 touched outside`
column would have reported if the copy loop wrapped instead of dropped —
why is that column the one that catches the bug?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-045/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 044 — a sprite as loaded bytes](lesson-044-sprite-bytes.md) ·
**Next:** [Lesson 046 — the sprite moves](lesson-046-movable-sprite.md) ·
**Code tag:** [`lesson-045`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-045)
