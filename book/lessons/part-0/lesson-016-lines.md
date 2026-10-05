# Lesson 016 — drawing lines onto the buffer

{{#include ../../stability-horizon.md}}

## Prose

A rectangle is easy: two nested loops. A line is the first shape that
requires an *algorithm*, and the algorithm is old enough to be famous:
Bresenham's, from 1962, when "computer" meant a machine where floating
point was expensive enough that avoiding it was a design goal.

Start with what people write first. The naive line computes a slope once —
`slope = (y1 - y0) / (x1 - x0)` — and then, for every `x` from `x0` to
`x1`, computes `y = y0 + slope * (x - x0)` in floating point, truncates to
an integer pixel, and plots. It reads like the math it came from. It is
also unstable in three separate ways, all of which you can see without
taking anyone's word for it. First, the truncation picks *different
pixels* than the true line: for the diagonal `(0, 0)` to `(7, 5)` the
float loop lands on `(1, 0)`, `(4, 2)`, `(5, 3)` where the exact line
goes through `(1, 1)`, `(4, 3)`, `(5, 4)` — six of the pixels the two
rasterizers touch disagree. Second, it is *asymmetric*: run the same line
backwards and truncation rounds the other way — the float line lights a
different set of pixels going down than going up. Third, a vertical line
has `x0 == x1` and the slope divides by zero, so the naive loop needs a
special case just to survive. And the folklore that it is slower: it used
to be, decisively — a floating multiply and an int conversion per pixel on
machines that had neither cheap. Measure it yourself in exercise 3; on a
modern CPU the gap has largely closed, and the pixels are the real cost.

Bresenham's answer is to not compute `y` at all. The algorithm tracks an
**error term** `err` — the scaled distance from the ideal line to the
center of the current pixel — and each step asks one question: is the next
pixel to the right closer to the line, or the one above it? If the error
says "up", `y` steps; either way `x` steps and the error is adjusted by
plain integer addition. Two adds, two compares, zero multiplies, zero
division, and the pixel choices are *exactly* the nearest-pixel
approximation of the line — the same pixels in both directions. `err` is
exact because everything it tracks is an integer difference of integers:
there is no rounding to drift, at any coordinate size. That is what
Bresenham computes: not the line, but the *pixels the line passes closest
to*, decided one integer step at a time.

Clipping is the same fold-before-write discipline as lesson 015, applied
to a segment. `ClipLine` is the classic Cohen–Sutherland: each endpoint
gets an out-code saying which side(s) of the buffer it is outside of
(`OutCode` builds one from four tests). If both endpoints share an
outside edge the segment misses the buffer entirely; if both codes are
zero it is fully inside; otherwise the outside end is moved to its
intersection with the buffer boundary and the codes are recomputed. The
loop converges after at most a couple of rounds, and only then does
`DrawLine` start plotting — every pixel is known in-bounds in advance, so
the inner loop, like `FillRect`'s, writes without checking. The divisions
in the clip run once or twice per line; the per-pixel loop is integers
only. One honest detail: the intersections are computed in `double` and
truncated toward zero when converted back to `int`, which is why the
clipped cyan line in the scene lands where the dump shows it and not a
pixel higher.

The scene draws three lines: a yellow diagonal across the whole buffer, a
cyan one from off-screen top-left to off-screen right, and a magenta
vertical through the middle that hangs off both edges and gets clipped to
`(4, 0)`–`(4, 5)`. The magenta line's column cuts through everything the
rectangles drew — last write wins, as always.

Build command, unchanged:

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Code step

One change for this lesson: `paint.c` gains `OutCode`, `ClipLine`
(Cohen–Sutherland), and `DrawLine` (Bresenham), and the scene draws three
lines. Committed together with this prose; its end state is tagged
`lesson-016`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index e42ce65..4bc0fdc 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,9 +1,9 @@
-// paint.c — Lesson 015: fill-rect onto a memory buffer.
+// paint.c — Lesson 016: drawing lines onto the buffer.
 //
-// A pixel buffer is bytes (lesson 013) and a file header is pinned bytes
-// (lesson 014).  Now we draw: FillRect fills a rectangle of the buffer,
-// clipping it to the buffer first — fold the rectangle to the visible
-// region BEFORE writing, never pixel by pixel.
+// A pixel buffer is bytes (lesson 013), a file header is pinned bytes
+// (lesson 014), and rectangles fold-clip before they write (lesson 015).
+// Now lines: DrawLine rasterizes with Bresenham's integer error term and
+// clips the segment to the buffer before stepping a single pixel.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -96,6 +96,61 @@ static void FillRect(unsigned char *px, int w, int h,
             PutPixel(px, w, h, x + i, y + j, r, g, b);
 }
 
+// OutCode — which side(s) of the buffer a point is outside of.
+static int OutCode(int x, int y, int w, int h)
+{
+    int code = 0;
+    if (x < 0) code |= 1;
+    else if (x >= w) code |= 2;
+    if (y < 0) code |= 4;
+    else if (y >= h) code |= 8;
+    return code;
+}
+
+// ClipLine — Cohen-Sutherland clipping: shrink the segment to the part
+// inside the buffer, in place.  Returns 0 if it misses the buffer.
+// The one division per intersection happens per segment end, never per
+// pixel; the rasterizer afterwards is pure integers.
+static int ClipLine(int *x0, int *y0, int *x1, int *y1, int w, int h)
+{
+    int c0 = OutCode(*x0, *y0, w, h), c1 = OutCode(*x1, *y1, w, h);
+    for (;;) {
+        if (!(c0 | c1)) return 1;   /* both ends inside */
+        if (c0 & c1) return 0;      /* both outside the same edge */
+        int c = c0 ? c0 : c1;
+        int x = 0, y = 0;
+        double dx = (double)(*x1 - *x0), dy = (double)(*y1 - *y0);
+        if (c & 8)      { x = *x0 + (int)(dx * (h - 1 - *y0) / dy); y = h - 1; }
+        else if (c & 4) { x = *x0 + (int)(dx * (0 - *y0) / dy);     y = 0; }
+        else if (c & 2) { y = *y0 + (int)(dy * (w - 1 - *x0) / dx); x = w - 1; }
+        else            { y = *y0 + (int)(dy * (0 - *x0) / dx);     x = 0; }
+        if (c == c0) { *x0 = x; *y0 = y; c0 = OutCode(x, y, w, h); }
+        else         { *x1 = x; *y1 = y; c1 = OutCode(x, y, w, h); }
+    }
+}
+
+// DrawLine — Bresenham's line.  After clipping, step from (x0, y0) to
+// (x1, y1) one pixel at a time; `err` tracks the doubled distance from
+// the ideal line, so the pixel choice is exact and entirely integer.
+static void DrawLine(unsigned char *px, int w, int h,
+                     int x0, int y0, int x1, int y1,
+                     unsigned char r, unsigned char g, unsigned char b)
+{
+    if (!ClipLine(&x0, &y0, &x1, &y1, w, h))
+        return;
+
+    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
+    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
+    int err = dx + dy;
+    for (;;) {
+        PutPixel(px, w, h, x0, y0, r, g, b);
+        if (x0 == x1 && y0 == y1) break;
+        int e2 = 2 * err;
+        if (e2 >= dy) { err += dy; x0 += sx; }
+        if (e2 <= dx) { err += dx; y0 += sy; }
+    }
+}
+
 // BuildBmpHeader — lay out the 54-byte BMP header field by field.
 // 14-byte file header:  "BM", file size, reserved, data offset.
 // 40-byte info header: size, width, height, planes, bpp, compression,
@@ -143,6 +198,11 @@ int main(void)
     FillRect(pixels, W, H, 5, 4, 10, 10, 128, 0, 255); /* off the bottom-right */
     FillRect(pixels, W, H, 2, 2, 3, 2, 255, 255, 255); /* fully inside */
 
+    /* Lines: a diagonal, one drawn from off-screen, one straight through. */
+    DrawLine(pixels, W, H, 0, 0, 7, 5, 255, 255, 0);   /* yellow diagonal */
+    DrawLine(pixels, W, H, -5, -3, 12, 2, 0, 255, 255); /* cyan, clipped */
+    DrawLine(pixels, W, H, 4, -2, 4, 9, 255, 0, 255);   /* magenta vertical */
+
     unsigned char r, g, b;
     GetPixel(pixels, W, H, 1, 0, &r, &g, &b);
     printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);
```

## Exercises

Four short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The vertical line through everything *(predict-the-output)*

Look only at the magenta line, `(4, -2)` to `(4, 9)`. Predict, before
running anything: (a) the exact endpoints `ClipLine` computes for it, and
(b) the exact three bytes at offset 12 of every one of the six rows of the
dump afterwards, including the rows where the line overwrites something
the rectangles drew — name those stolen pixels. Then add one `fprintf` to
`DrawLine` printing the clipped endpoints, run, and reconcile.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-016/ex1.md)

### Exercise 2 — Rectangle outlines *(extend-the-code)*

Add `DrawRect`, the outline counterpart of `FillRect`, built from exactly
four `DrawLine` calls (top, bottom, left, right edges), and draw one
outline that hangs off the left edge of the buffer — `(−2, 1, 6, 4)` in
`(x, y, w, h)` form is a good test. The lines must clip; nothing may be
written outside the buffer. Check the dump against which parts of the
outline are actually visible.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-016/ex2.md)

### Exercise 3 — Float versus integer *(measure-the-performance)*

Implement `DrawLineFloat` — the naive line from the prose: clip, compute
`slope` as a `double`, then one `slope * (x - x0)` per pixel — and
benchmark both rasterizers on the same long diagonal (a 512×512 scratch
buffer and `clock()` around each loop; the small scene is too noisy to
time). Report microseconds per line for both. Then draw the small
diagonal `(0, 0)`–`(7, 5)` with both into separate buffers and count the
pixels they disagree on. Which of the two results — the timing or the
pixels — actually settles the question of which rasterizer to use, and
why?

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-016/ex3.md)

### Exercise 4 — The error term, watched *(explain-in-prose)*

Add one `fprintf(stderr, "step (%d,%d) err=%d\n", ...)` inside
`DrawLine`'s loop, plotting position and error term, and run the program.
Then explain in your own words, following the printed trace of the yellow
diagonal: (a) what one `e2 >= dy` / `e2 <= dx` round decides about the
next pixel; (b) why `err` is always an integer and never drifts — where,
in the trace, would a floating-point version have rounded differently?
(c) why the trace's first `err` is `dx + dy` and not zero.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-016/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 015 — fill-rect onto a memory buffer](lesson-015-fill-rect.md) ·
**Next:** [Lesson 017 — writing a real image file by hand](lesson-017-image-file.md) ·
**Code tag:** [`lesson-016`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-016)
