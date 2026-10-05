# Lesson 015 — fill-rect onto a memory buffer

{{#include ../../stability-horizon.md}}

## Prose

Lesson 013 gave us pixels; this lesson gives us *drawing*. A filled
rectangle is the simplest shape worth having, and the interesting part is
not the fill — it is what happens when the rectangle hangs off the edge of
the image. Which, in a game, it always does: sprites cross borders, camera
rects slide past the viewport, damage numbers spawn off-screen. A renderer
that only handles in-bounds rectangles is not a renderer.

`FillRect` takes a rectangle as `(x, y, rw, rh)` — the top-left corner and
the size — and a color. Note that `x` and `y` are signed: a rectangle
starting at `(-3, 1)` is legal and means "a shape whose left three columns
are above the left edge". The naive way to handle that is to bounds-check
every pixel inside the write loop: for each `(i, j)`, ask whether
`(x + i, y + j)` is inside the buffer before calling `PutPixel`. That
works. It is also the wrong shape for a renderer: the check runs once per
pixel instead of once per rectangle, and — worse — it papers over the
geometry instead of computing it.

The way this course draws is **fold before write**: clip the rectangle
down to its visible part *once*, up front, and then write pixels that are
already known to be inside. Look at what the four folds in `FillRect` do.
If `x < 0`, the columns left of the buffer are lost: there are `-x` of
them, so `rw += x` shrinks the width by exactly the lost part and `x = 0`
slides the origin to the edge. If `x + rw > w`, the rectangle spills past
the right edge and the surviving width is `w - x`. The two vertical folds
are the same arithmetic on `y` and `rh`. What remains, `(x, y, rw, rh)` in
local variables with the same names, is the visible part — and the nested
loop then writes with no checks at all.

Two details carry real weight. First, **both folds per axis are
load-bearing**. Delete the `x < 0` fold and the left-hanging rectangle
computes offsets like `1 * (8 * 3) + (-3) * 3 = 15` — a write that sails
into the *previous row's* pixels: in this scene's dump, row 0 grows an
orange pixel at column 5 that nobody asked for, and every drawn row bleeds
upward into its neighbor. Push the same rectangle to `y = 0` and the
offsets go negative — bytes *before* the buffer, which AddressSanitizer
reports as a heap-buffer-overflow. The fold is not one of two options; it
is what makes the write loop's arithmetic honest. Second, the **empty
case**: after folding, a rectangle that is entirely
outside has `rw <= 0` or `rh <= 0` — that is not an error, it is the
normal result of clipping, and the early return is what makes "the sprite
left the screen" a non-event.

The lesson's scene hangs four rectangles off the edges on purpose: one off
the left, one off the top-right corner, one off the bottom-right corner,
and one entirely inside. The dump shows the result — row 1 begins

```
row 1: FF 80 00 FF 80 00 FF 80 00 00 00 00 ...
```

three orange pixels at the left edge: of the rectangle that started at
`x = -3`, the first three columns were folded away and the rest landed at
`x = 0`. Row 5's last three bytes read `80 00 FF` — the bottom-right
rectangle overwrote the blue pixel lesson 013 planted at `(7, 5)`. Last
write wins; nobody warned the blue pixel.

One honest footnote about the arithmetic: the end tests `x + rw > w` can
*overflow* if a caller passes an absurd `x` near `INT_MAX` — the addition
wraps, the test lies, and `PutPixel` writes who-knows-where. Exercise 3
makes that crash on demand and fixes the test so it cannot add its way
into undefined behavior. Remember that one; lesson 018 is a whole lesson
about exactly this class of arithmetic.

This pattern — fold the request to the valid region before touching
memory, then run a check-free loop — is the core of Part 2's renderer. It
will come back for lines, for sprites, and for the framebuffer copy. Learn
it here while the shape is small.

Build command, unchanged:

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Code step

One change for this lesson: `paint.c` gains `FillRect` with its clipping
fold, and `main` hangs four rectangles off the edges. Committed together
with this prose; its end state is tagged `lesson-015`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index 2c95a68..e42ce65 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,9 +1,9 @@
-// paint.c — Lesson 014: endianness and image-header layout.
+// paint.c — Lesson 015: fill-rect onto a memory buffer.
 //
-// A pixel buffer is bytes (lesson 013); an image FILE is bytes too, with a
-// header whose field order and byte order the format pins down.  PutU16LE
-// and PutU32LE write integers byte by byte in little-endian order, and
-// BuildBmpHeader lays out the 54-byte BMP header field by field.
+// A pixel buffer is bytes (lesson 013) and a file header is pinned bytes
+// (lesson 014).  Now we draw: FillRect fills a rectangle of the buffer,
+// clipping it to the buffer first — fold the rectangle to the visible
+// region BEFORE writing, never pixel by pixel.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -78,6 +78,24 @@ static unsigned int GetU32LE(const unsigned char *src)
          | ((unsigned int)src[3] << 24);
 }
 
+// FillRect — fill a rectangle, clipping it to the buffer first.
+// The clip is a fold: shrink (x, y, rw, rh) to the visible part once,
+// up front, and then write only pixels that are known to be inside.
+static void FillRect(unsigned char *px, int w, int h,
+                     int x, int y, int rw, int rh,
+                     unsigned char r, unsigned char g, unsigned char b)
+{
+    if (x < 0) { rw += x; x = 0; }
+    if (y < 0) { rh += y; y = 0; }
+    if (x + rw > w) rw = w - x;
+    if (y + rh > h) rh = h - y;
+    if (rw <= 0 || rh <= 0) return;
+
+    for (int j = 0; j < rh; j++)
+        for (int i = 0; i < rw; i++)
+            PutPixel(px, w, h, x + i, y + j, r, g, b);
+}
+
 // BuildBmpHeader — lay out the 54-byte BMP header field by field.
 // 14-byte file header:  "BM", file size, reserved, data offset.
 // 40-byte info header: size, width, height, planes, bpp, compression,
@@ -119,6 +137,12 @@ int main(void)
     PutPixel(pixels, W, H, 1, 0, 0, 255, 0); /* green, beside it */
     PutPixel(pixels, W, H, 7, 5, 0, 0, 255); /* blue, bottom-right */
 
+    /* Rectangles that hang off the edges — only the visible part lands. */
+    FillRect(pixels, W, H, -3, 1, 6, 3, 255, 128, 0);  /* off the left */
+    FillRect(pixels, W, H, 6, -2, 4, 4, 0, 128, 255);  /* off the top-right */
+    FillRect(pixels, W, H, 5, 4, 10, 10, 128, 0, 255); /* off the bottom-right */
+    FillRect(pixels, W, H, 2, 2, 3, 2, 255, 255, 255); /* fully inside */
+
     unsigned char r, g, b;
     GetPixel(pixels, W, H, 1, 0, &r, &g, &b);
     printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);
```

## Exercises

Four short drills, all about clipping cases. Each ends with its solution —
a diff against this lesson's end state plus a walkthrough — after the
prompt.

### Exercise 1 — The rectangle off the left *(predict-the-output)*

Look only at the first rectangle in the scene, `(-3, 1, 6, 3)` in orange,
and predict the clipping before you trust the dump: (a) what does the fold
compute as the visible part — exactly which `x`, `y`, `w`, `h`? — and (b)
what are the exact first nine bytes of `row 1` in the dump? Then add one
`fprintf` to `FillRect` that prints the visible part after the folds, run,
and reconcile both predictions. One of the scene's rectangles *loses* to a
later one in the dump — say which and why.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-015/ex1.md)

### Exercise 2 — Counting what survived *(extend-the-code)*

Make `FillRect` return the number of pixels it actually wrote (`int`, zero
for the rectangles that clip to nothing), accumulate the count in `main`
over the four calls, and print it. Before running, compute the four counts
by hand from the clipped rectangles and check them against the program —
and make one more call with a rectangle entirely outside the buffer (say
`(20, 20, 4, 4)`) to see the zero case work.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-015/ex2.md)

### Exercise 3 — The addition that ate the clip *(fix-the-crash)*

Add this call to the scene and run the program:

```c
FillRect(pixels, W, H, 2147483640, 0, 100, 1, 255, 0, 0);
```

The program dies with a segmentation fault. The pixels are innocent: the
clip's right-hand test computes `x + rw`, and `2147483640 + 100` overflows
`int` — the wrapped result passes the test, the fold never clamps, and
`PutPixel` writes far outside the buffer. Rewrite the two end tests so
they cannot overflow for any `int` inputs, and verify the same call now
clips to nothing instead of crashing.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-015/ex3.md)

### Exercise 4 — Why fold first *(explain-in-prose)*

The alternative to folding is a per-pixel bounds check in the write loop —
a `PutPixelChecked` that refuses coordinates outside the buffer. Explain
in your own words, for this rectangle shape: (a) what work the fold does
exactly once that a per-pixel check would do `rw * rh` times; (b) why the
start folds are load-bearing — with the `x < 0` fold deleted, work out by
hand where the left-hanging rectangle's first pixel's three bytes land
(`off = y * (w * 3) + x * 3`), then actually delete the fold, rebuild, and
compare the dump against the untouched one before restoring it; and (c)
one situation where the per-pixel check is actually the better engineering
call. To ground (a) and (b), add a `fprintf` to each fold in `FillRect`
that prints the values as the fold fires, run, and quote what the scene's
rectangles do to the clip state.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-015/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 014 — endianness and image-header layout](lesson-014-image-headers.md) ·
**Next:** [Lesson 016 — drawing lines onto the buffer](lesson-016-lines.md) ·
**Code tag:** [`lesson-015`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-015)
