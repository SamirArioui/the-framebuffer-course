# Lesson 013 — raw bytes and pixel formats

{{#include ../../stability-horizon.md}}

## Prose

This lesson starts `paint`, the third Part 0 sandbox program: a BMP painter
that will, by the end of this stretch, write real image files from a pixel
buffer every byte of which is written by our own code. Today is the
foundation: what a pixel buffer *is*. Spoiler: bytes. Nothing but bytes.

Python gives you `bytes`, `bytearray`, and a library ecosystem where "image"
is a type somebody else defined. C gives you memory. An image in C is a
block of `unsigned char` — the same type a text file is made of — and every
notion of "pixel" is arithmetic you do on offsets into that block. There is
no image type to import because there is no import: if you want RGB888, you
decide that three consecutive bytes mean red, green, and blue, and you write
the code that believes it.

Our format is **RGB888**: every pixel is exactly three bytes, red first,
then green, then blue, each channel an 8-bit value from `0` to `255`. It is
the simplest useful pixel format, and it is worth contrasting with the two
other families you will meet in real code. A **packed 32-bit** format (often
called ARGB8888 or XRGB8888) stores each pixel in one 32-bit word: four
channels, one machine word, one load — fast, but the byte order *inside*
that word depends on the machine's endianness, which is exactly the problem
lesson 014 opens up. An **indexed** format stores one byte per pixel plus a
palette table mapping index → color: compact and cheap to recolor, but every
pixel needs a lookup. RGB888 sits in the middle: three bytes per pixel, no
palette, no word-order questions — at the cost of not being a machine word.

Those three bytes per pixel are laid out **row-major**: the whole first row
of the image sits in memory before the second row starts, and within a row
pixels run left to right. So the pixel at `(x, y)` lives at byte offset

```
y * (width * 3) + x * 3
```

The `width * 3` term is the **stride**: the number of bytes one full row
occupies. Memorize that shape — it is the single most-repeated piece of
arithmetic in this entire course. Two traps live in it. First, forgetting
the `* 3` and indexing `y * width + x * 3` works for row 0 and silently
overlaps rows from row 1 on. Second, the stride of a row in an *in-memory*
buffer is `width * 3`, but the stride of a row in a *file* is padded up to a
multiple of 4 bytes — a quirk of the BMP format that lesson 017 pays for
directly.

`PutPixel` and `GetPixel` are that arithmetic plus three stores or three
loads: compute `off`, then touch `px[off]`, `px[off + 1]`, `px[off + 2]`.
Note what is *not* in them: no bounds checking. The caller is trusted, the
way a C function usually trusts its caller, and exercise 4 shows what that
trust costs. (The `h` parameter is accepted but unused so far — the
`(void)h;` line is the standard C idiom for "yes, I know the parameter is
unused; do not warn me about it", which `-Wall -Wextra` otherwise would.)

The buffer itself is `calloc`'d: `width * height * 3` bytes, all zero. In
RGB888 zero is black, so an untouched buffer is a black image. `calloc`
returns zeroed memory or `NULL`, and — lesson 001's discipline — `NULL` is a
value you check, not an exception. `HexDump` then prints the buffer one row
per line so the row-major layout is visible: row 0's 24 bytes first, then
row 1's, and so on. Run the program and row 0 begins

```
row 0: FF 00 00 00 FF 00 00 00 00 00 00 00 ...
```

`FF 00 00` is the red pixel at `(0, 0)`, `00 FF 00` the green one at
`(1, 0)` — three bytes each, exactly where the offset arithmetic says they
should be. Row 5 ends in `... 00 00 00 FF`: the blue pixel at `(7, 5)`, the
last pixel of the last row.

Finally the build command. From inside `sandbox/paint/`:

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

Every flag here has already done its introduction — `-std=c11 -Wall -Wextra`
in lesson 001, `-O0 -g` since the gdb work in lesson 002 — so one line will
do: this is the canonical Part 0 build, no optimizations, debug info,
warnings on. Lesson 018 changes exactly one of these flags and the whole
program changes behavior.

## Code step

One change for this lesson: the whole of `paint.c`, committed together with
this prose. Its end state is tagged `lesson-013`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
new file mode 100644
index 0000000..cfbe8bd
--- /dev/null
+++ b/sandbox/paint/paint.c
@@ -0,0 +1,64 @@
+// paint.c — Lesson 013: raw bytes and pixel formats.
+//
+// A pixel buffer is nothing but bytes.  Ours is width x height pixels at
+// 3 bytes each (RGB888), laid out row by row.  PutPixel and GetPixel map
+// (x, y, color) onto byte offsets; HexDump shows the buffer as it really
+// sits in memory.
+#include <stdio.h>
+#include <stdlib.h>
+
+#define W 8
+#define H 6
+
+static unsigned char *pixels;
+
+static void PutPixel(unsigned char *px, int w, int h, int x, int y,
+                     unsigned char r, unsigned char g, unsigned char b)
+{
+    (void)h; /* unused so far — bounds arrive in the exercises below */
+    int off = y * (w * 3) + x * 3;
+    px[off + 0] = r;
+    px[off + 1] = g;
+    px[off + 2] = b;
+}
+
+static void GetPixel(const unsigned char *px, int w, int h, int x, int y,
+                     unsigned char *r, unsigned char *g, unsigned char *b)
+{
+    (void)h;
+    int off = y * (w * 3) + x * 3;
+    *r = px[off + 0];
+    *g = px[off + 1];
+    *b = px[off + 2];
+}
+
+static void HexDump(const unsigned char *px, int w, int h)
+{
+    for (int y = 0; y < h; y++) {
+        printf("row %d:", y);
+        for (int i = 0; i < w * 3; i++)
+            printf(" %02X", px[y * (w * 3) + i]);
+        putchar('\n');
+    }
+}
+
+int main(void)
+{
+    pixels = calloc((size_t)W * H * 3, 1);
+    if (pixels == NULL) {
+        fprintf(stderr, "out of memory\n");
+        return 1;
+    }
+
+    PutPixel(pixels, W, H, 0, 0, 255, 0, 0); /* red, top-left */
+    PutPixel(pixels, W, H, 1, 0, 0, 255, 0); /* green, beside it */
+    PutPixel(pixels, W, H, 7, 5, 0, 0, 255); /* blue, bottom-right */
+
+    unsigned char r, g, b;
+    GetPixel(pixels, W, H, 1, 0, &r, &g, &b);
+    printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);
+
+    HexDump(pixels, W, H);
+    free(pixels);
+    return 0;
+}
```

## Exercises

Four short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Three pixels, by hand *(predict-the-output)*

Add one more call to the scene in `main`:

```c
PutPixel(pixels, W, H, 2, 0, 0x11, 0x22, 0x33);
```

Before you compile or run anything, write down the exact first twelve bytes
of `row 0` as `HexDump` will print them — every byte, in order. Then build,
run, and reconcile the printed row with your prediction. If they disagree,
say precisely which byte surprised you and where its value comes from.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-013/ex1.md)

### Exercise 2 — The same pixel in packed 32-bit *(extend-the-code)*

Add a second buffer to `main` — an `unsigned int` array of `W * H` words,
which is the packed 32-bit format — and store the green pixel `(1, 0)` in
it as the ARGB word `0x0000FF00u`. Then print, side by side, that word's
four in-memory bytes (via an `unsigned char *` view of the word) and the
RGB888 buffer's three bytes for the same pixel. How many bytes does each
format spend on the same green pixel, and which of the two answers would
change on a machine with the opposite byte order?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-013/ex2.md)

### Exercise 3 — Offsets, by hand *(explain-in-prose)*

Compute the byte offsets `PutPixel` uses for the pixels `(0, 0)`, `(1, 0)`,
`(7, 5)`, and `(3, 2)` in this program's buffer — do it with the formula,
not with the program. Then add one `fprintf(stderr, ...)` to `PutPixel`
printing `x`, `y`, and the offset it computed, run, and check yourself.
Finally, explain in your own words what the classic forgotten-stride bug —
indexing with `y * w + x * 3` instead of `y * (w * 3) + x * 3` — does to
the picture, and why row 0 can look right while everything below it is
garbage.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-013/ex3.md)

### Exercise 4 — The pixel that is not there *(fix-the-crash)*

`PutPixel` trusts its caller. Prove that it shouldn't: add
`PutPixel(pixels, W, H, 0, 6, 1, 2, 3);` at the end of the scene and run
the program. Nothing appears to happen — the write lands just past the
buffer and the dump looks normal. Now rebuild with one extra flag:

```
gcc -std=c11 -O0 -g -Wall -Wextra -fsanitize=address paint.c -o paint
```

`-fsanitize=address` turns on AddressSanitizer, a runtime built into the
compiler that pads every heap allocation with poisoned guard zones and
aborts the program with a stack trace the moment a read or write touches
one. Run it and watch it catch the stray write. Then fix the real problem:
make `PutPixel` refuse coordinates outside `w × h` with a message on
`stderr` instead of writing, and verify the same run is now clean under
AddressSanitizer.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-013/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 012 — multi-file builds: translation units and linking](lesson-012-multi-file.md) ·
**Next:** [Lesson 014 — endianness and image-header layout](lesson-014-image-headers.md) ·
**Code tag:** [`lesson-013`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-013)
