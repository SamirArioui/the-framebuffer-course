# Lesson 017 — writing a real image file by hand

{{#include ../../stability-horizon.md}}

## Prose

Everything is in place: bytes for pixels (013), bytes for headers (014),
clipping (015), lines (016). This lesson crosses the last gap — the buffer
becomes a file that other programs can open. `WriteBmp` writes the
54-byte header lesson 014 built, then the pixels, and the file that comes
out is a real BMP: the proof is one command away.

`file paint.bmp` reads magic bytes and header fields, and after this
lesson's run it says

```
paint.bmp: PC bitmap, Windows 3.x format, 8 x 6 x 24, image size 144,
resolution 2835 x 2835 px/m, cbSize 198, bits offset 54
```

Not "data" — a named format with dimensions and a size that matches the
file on disk to the byte. If you want stronger proof, decode it: the
following ten-line Python (standard library only — it is exactly lesson
014's decoders in another language) unpacks the header and samples two
pixels:

```python
import struct
d = open('paint.bmp', 'rb').read()
off, = struct.unpack_from('<I', d, 10)
w, h = struct.unpack_from('<ii', d, 18)
row = w * 3 + (-(w * 3)) % 4
def px(x, y):
    b, g, r = d[off + (h - 1 - y) * row + x * 3:][:3]
    return r, g, b
print("dims", w, h, "size", len(d))
print("pixel (1,0):", px(1, 0))
print("pixel (7,5):", px(7, 5))
```

It prints `dims 8 6 size 198`, then the green pixel at `(1, 0)` as
`(0, 255, 0)` and the yellow end of the diagonal at `(7, 5)` as
`(255, 255, 0)` — our scene, decoded out of the file by code that has
never seen our buffer. (ImageMagick's `identify`, `feh`, GIMP, or any
viewer will open it too.)

Now the three quirks `WriteBmp` pays, one at a time — all three are
requirements of the BMP format, not choices we made.

**Rows are bottom-up.** BMP stores the *last* image row first: the row
loop starts at `y = h - 1` and walks down to 0. Nobody designed this; it
fell out of early Windows' bottom-up coordinate system, and every BMP
reader since expects it. Write rows top-down without also negating the
height field and every viewer shows the image flipped — exercise 3 makes
you watch that happen.

**Rows are padded to 4 bytes.** Each row is padded with zero bytes until
its length is a multiple of 4 — for our 8-wide image, `8 * 3 = 24`, which
already is, so `pad = 0`; at width 5 the row is 15 bytes and grows to 18.
The padding is why `BuildBmpHeader` computes `image_size` from the padded
row size and why the file's total is `54 + (row_size + pad) * h`. Skip the
padding writes but keep the size arithmetic and the rows slide out of
alignment after the first one.

**Channels are blue-green-red.** BMP pixel triples are BGR, the mirror of
our RGB888 buffer. The per-pixel emission in `WriteBmp` swaps `p[2]` and
`p[0]` on the way out. Forgetting the swap produces a file that is
perfectly valid and subtly wrong — every red and blue channel trades
places.

Then there is `ClearBuffer`, which exists to give the scene a background
and to be *the deliberately bad code of this stretch*. It fills the
buffer byte by byte with a loop whose stop condition is the offset turning
negative — a stop that only a wrapping counter can deliver. **This is
written naively on purpose; lesson 018 finds out what the optimizer does
with it.** At `-O0` the machine wraps the counter, the loop ends, and the
first line of the program's output is `clear ended at offset -2147483648`
— a number that should make you suspicious on sight. The file still comes
out correct at `-O0`; correctness under optimization is a separate
question, and it is lesson 018's entire subject.

The output writer itself is naive in the ordinary way: one `putc` per
byte. That is the clear code first and the fast code later — exercise 4
measures what the difference actually is on this machine.

Build command, unchanged:

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Code step

One change for this lesson: `paint.c` gains `ClearBuffer` (the deliberately
naive fill) and `WriteBmp`, and `main` clears the buffer, draws the scene,
and writes `paint.bmp`. Committed together with this prose; its end state
is tagged `lesson-017`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index 4bc0fdc..3aeb7bd 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,9 +1,10 @@
-// paint.c — Lesson 016: drawing lines onto the buffer.
+// paint.c — Lesson 017: writing a real image file by hand.
 //
 // A pixel buffer is bytes (lesson 013), a file header is pinned bytes
-// (lesson 014), and rectangles fold-clip before they write (lesson 015).
-// Now lines: DrawLine rasterizes with Bresenham's integer error term and
-// clips the segment to the buffer before stepping a single pixel.
+// (lesson 014), rectangles fold-clip (lesson 015), lines rasterize
+// (lesson 016).  Now the buffer becomes a real file: WriteBmp emits the
+// 54-byte header plus bottom-up, padded rows — and a small scene lands
+// in paint.bmp.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -151,6 +152,21 @@ static void DrawLine(unsigned char *px, int w, int h,
     }
 }
 
+// ClearBuffer — fill the buffer with one byte value.  This fill is
+// written naively on purpose: its stop condition is the offset turning
+// negative, a stop that only a wrapping counter can deliver.
+// Lesson 018 finds out what the optimizer does with it.
+static void ClearBuffer(unsigned char *px, int nbytes, unsigned char v)
+{
+    int i = 0;
+    while (i >= 0) {          /* keep going while the offset is positive */
+        if (i < nbytes)       /* clip: never write past the buffer */
+            px[i] = v;
+        i++;
+    }
+    printf("clear ended at offset %d\n", i);
+}
+
 // BuildBmpHeader — lay out the 54-byte BMP header field by field.
 // 14-byte file header:  "BM", file size, reserved, data offset.
 // 40-byte info header: size, width, height, planes, bpp, compression,
@@ -180,6 +196,42 @@ static void BuildBmpHeader(unsigned char *header, int w, int h)
     PutU32LE(header + 50, 0);                   /* important colors */
 }
 
+// WriteBmp — write the pixel buffer to `path` as a 24-bit BMP: the
+// header from lesson 014, then the rows bottom-up (BMP stores the last
+// image row first), each row padded to a 4-byte boundary and with the
+// channels in BMP's blue-green-red order.
+static int WriteBmp(const char *path, const unsigned char *px, int w, int h)
+{
+    unsigned char header[54];
+    BuildBmpHeader(header, w, h);
+
+    unsigned int row_size = (unsigned int)w * 3;
+    unsigned int pad = (4 - row_size % 4) % 4;
+
+    FILE *f = fopen(path, "wb");
+    if (f == NULL) {
+        fprintf(stderr, "cannot write %s\n", path);
+        return -1;
+    }
+
+    for (int i = 0; i < 54; i++)
+        putc(header[i], f);
+
+    for (int y = h - 1; y >= 0; y--) {
+        const unsigned char *row = px + (size_t)y * row_size;
+        for (int x = 0; x < w; x++) {
+            const unsigned char *p = row + x * 3;
+            putc(p[2], f); /* blue first */
+            putc(p[1], f); /* then green */
+            putc(p[0], f); /* then red */
+        }
+        for (unsigned int k = 0; k < pad; k++)
+            putc(0, f);
+    }
+    fclose(f);
+    return 0;
+}
+
 int main(void)
 {
     pixels = calloc((size_t)W * H * 3, 1);
@@ -187,6 +239,7 @@ int main(void)
         fprintf(stderr, "out of memory\n");
         return 1;
     }
+    ClearBuffer(pixels, W * H * 3, 0x20); /* dark gray background */
 
     PutPixel(pixels, W, H, 0, 0, 255, 0, 0); /* red, top-left */
     PutPixel(pixels, W, H, 1, 0, 0, 255, 0); /* green, beside it */
@@ -230,6 +283,9 @@ int main(void)
     printf("bpp         %u\n", GetU16LE(header + 28));
     printf("image size  %u\n", GetU32LE(header + 34));
 
+    if (WriteBmp("paint.bmp", pixels, W, H) == 0)
+        printf("wrote paint.bmp (%dx%d, 24 bpp)\n", W, H);
+
     free(pixels);
     return 0;
 }
```

## Exercises

Four short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Rows on disk *(predict-the-output)*

The file is 198 bytes; the dump you already have. Predict the layout
before you touch the code: (a) the file offset at which each of the six
image rows begins, in the order they are written, and (b) which image
pixel lands at file offset 54, and which lands at file offset 75 — with
the exact three bytes at each. Then add one `fprintf` to `WriteBmp`'s row
loop printing each row's starting offset (`ftell`), run, and reconcile
row order and offsets with your prediction.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-017/ex1.md)

### Exercise 2 — Round trip *(extend-the-code)*

Add a `CheckBmp` that re-opens `paint.bmp` after writing and verifies it
against the buffer that produced it: decode the size, width, height, and
bpp fields with `GetU32LE`/`GetU16LE`, then seek to the file offset of
image pixel `(w - 1, h - 1)` — the last pixel of the first row on disk —
read its three bytes, un-swap them, and compare with `GetPixel` for the
same coordinate. Print both sides. Everything must match; make one
deliberate mismatch (compare against `(w - 1, 0)` instead) and watch which
check notices.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-017/ex2.md)

### Exercise 3 — The flipped image *(explain-in-prose)*

BMP's bottom-up row order is a format rule, not a preference. Prove it:
change the row loop to walk top-down (`y = 0` up to `h - 1`) without
touching anything else, rebuild, and decode the file with the Python
sampler from the prose (it assumes the standard bottom-up rule). State
what you see and why the *header* is now lying. Then explain in your own
words: what would a writer have to do to legitimately produce a top-down
BMP (research the height field's sign), and why do you think the format
carries this quirk at all? Restore the loop afterwards.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-017/ex3.md)

### Exercise 4 — Per byte versus per row *(measure-the-performance)*

The writer emits one `putc` per byte. Build the alternative: `WriteBmpFast`
assembles each output row (BGR swap and padding included) into a row
buffer and writes it with one `fwrite`. Then benchmark both writers on a
1024×1024 gradient buffer (a few repetitions each, `clock()` around the
loops) and report milliseconds per write — and verify with `cmp` that the
two files are byte-identical. How big is the gap on your machine, and what
is each loop actually spending its time on?

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-017/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 016 — drawing lines onto the buffer](lesson-016-lines.md) ·
**Next:** [Lesson 018 — the optimizer and undefined behavior](lesson-018-optimizer-ub.md) ·
**Code tag:** [`lesson-017`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-017)
