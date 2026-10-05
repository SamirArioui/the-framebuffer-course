<!-- Filled-in copy of the lesson template (plan/lesson-template.md) with
     illustrative content. This is a demo of a lesson's shape, not curriculum;
     the code below is fictional and lives nowhere in src/. -->

# Sample lesson — filling a rectangle

{{#include ../stability-horizon.md}}

## Prose

Every pixel this engine shows is written by code we own. The first building
block is a solid rectangle: given a position, a size, and a color, write that
color into the framebuffer.

Two details do all the work. The arithmetic in `FillRectangle` computes the
pixel bounds from the rectangle's corner and size. The clipping branch of
`FillRectangle` then folds those bounds back into the framebuffer before any
pixel is touched, so a rectangle hanging off the edge of the screen writes only
the pixels that are actually visible. Without that branch, a rectangle outside
the screen would walk off the end of the pixel buffer.

> *Sample page: prose references symbols and structural locations — "the
> clipping branch of `FillRectangle`" — and never line numbers.*

## Code step

One change for this lesson: `FillRectangle`, committed together with this
prose. (The real lesson would tag this end state `lesson-NNN`.)

```diff
diff --git a/src/draw.c b/src/draw.c
--- a/src/draw.c
+++ b/src/draw.c
@@ -1,3 +1,22 @@
+// FillRectangle: write a solid rectangle into the framebuffer.
+void FillRectangle(Framebuffer *fb, Rectangle r, Color color)
+{
+    int x0 = r.x;
+    int y0 = r.y;
+    int x1 = r.x + r.w;
+    int y1 = r.y + r.h;
+
+    // The clipping branch: fold the bounds back into the framebuffer.
+    if (x0 < 0) x0 = 0;
+    if (y0 < 0) y0 = 0;
+    if (x1 > fb->width)  x1 = fb->width;
+    if (y1 > fb->height) y1 = fb->height;
+
+    for (int y = y0; y < y1; ++y)
+        for (int x = x0; x < x1; ++x)
+            fb->pixels[y * fb->width + x] = color;
+}
```

## Exercises

Two exercises, as a Part 1-2 lesson would carry. Each prompt ends with its
solution link and nothing else.

### Exercise 1 — Draw an outline *(extend-the-code)*

Add `FillRectangleOutline(Framebuffer *fb, Rectangle r, int thickness, Color
color)` next to `FillRectangle`: it draws only the border of the rectangle, at
the given thickness. Keep the behavior of the clipping branch: an outline
hanging off an edge draws only the pixels that are visible.

> **Solution:** [ex1 — diff + walkthrough](../solutions/lesson-000/ex1.md)

### Exercise 2 — The crashing rectangle *(fix-the-crash)*

A teammate sent a variant that skips bookkeeping and writes pixels straight
from the rectangle:

```c
void FillRectangleFast(Framebuffer *fb, Rectangle r, Color color)
{
    for (int y = r.y; y < r.y + r.h; ++y)
        for (int x = r.x; x < r.x + r.w; ++x)
            fb->pixels[y * fb->width + x] = color;
}
```

It is fast, and it crashes on rectangles that lie partly or fully outside the
framebuffer. Find out exactly why it crashes, and fix it so it keeps as much of
its speed as it can.

> **Solution:** [ex2 — diff + walkthrough](../solutions/lesson-000/ex2.md)

---

**Part:** Sample ·
**Previous:** — ·
**Next:** — ·
**Code tag:** `lesson-000` *(sample)*
