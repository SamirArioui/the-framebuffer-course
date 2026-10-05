# Lesson 014 — endianness and image-header layout

{{#include ../../stability-horizon.md}}

## Prose

Lesson 013 made the pixel buffer nothing but bytes. This lesson does the
same for the image *file*: a BMP is bytes too, and the format — not the
machine that wrote them — decides what every byte means. The two ideas
today are endianness, concretely, and laying a header out field by field.

Start with the machine. A 32-bit integer is four bytes, but the bytes have
no order of their own — the CPU does. On a **little-endian** machine the
least significant byte lives at the lowest address; on a **big-endian**
machine, the most significant. The probe in `main` asks the machine
directly: it stores `0x01020304` and prints the four bytes through an
`unsigned char *` view of the same memory. Here is the real output on this
machine:

```
0x01020304 in memory: 04 03 02 01
```

`04` first — least significant byte first — so this is a little-endian
machine. Two details deserve a pause. First, the cast from `unsigned int *`
to `unsigned char *` is the *one* type-punning cast C always permits: a
`char` pointer may inspect any object's bytes. Every other "reinterpret"
cast is a trap that lesson 018 will visit. Second, the value `0x01020304`
is portable; the *byte sequence* `04 03 02 01` is a fact about this CPU.
Run the same probe on a big-endian machine and it prints `01 02 03 04`.

Now the problem this creates for file formats. If a BMP stored its width as
"the four bytes of an `int`, in memory order", the file would mean different
things depending on which machine wrote it — the same file would decode as
width 8 on one computer and width 134217728 on another. So every serious
file format **pins the byte order**. BMP pins little-endian for every
multi-byte header field. Our encoders, `PutU16LE` and `PutU32LE`, write
that order by hand: masks and shifts, least significant byte first, no
matter what the host thinks. `GetU16LE` and `GetU32LE` read it back the
same way — a decode is just the encode run backwards. Because the encoders
deal only in bytes and shifts, they are correct on *every* machine: a BMP
written on a big-endian box is byte-identical to one written here.

With the encoders in place, `BuildBmpHeader` assembles the 54-byte header
the format prescribes: a 14-byte file header ("BM", file size, reserved
zeros, data offset) followed by a 40-byte information header (size, width,
height, planes, bits per pixel, compression, image size, resolution,
colors). Every field gets its exact offset — width at bytes 18 through 21,
height at 22 through 25, and so on. `main` dumps the resulting bytes and
then decodes each field back out; on this machine the dump reads

```
42 4D C6 00 00 00 00 00 00 00 36 00 00 00 28 00
00 00 08 00 00 00 06 00 00 00 01 00 18 00 00 00
...
```

Read it slowly: `42 4D` is "BM", `C6 00 00 00` is file size 198
little-endian, `36 00 00 00` is data offset 54, `28 00 00 00` is info
header size 40, then `08 00 00 00 06 00 00 00` — width 8, height 6, both
in little-endian, exactly where the format says they must be.

One question remains, and exercise 3 makes you measure the answer: why
build this with byte stores instead of declaring a `struct BmpHeader` and
`fwrite`-ing it? Short version: the C compiler is free to insert padding
between struct fields and reorder nothing but guarantee little — the struct
lives by the *compiler's* layout rules, the file lives by the *format's*.
Only one of those can be written to disk safely, and it is not the struct.

The build command is unchanged from lesson 013:

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Code step

One change for this lesson: `paint.c` gains the little-endian encoders and
decoders, `BuildBmpHeader`, a byte dumper, and the endianness probe in
`main`. Committed together with this prose; its end state is tagged
`lesson-014`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index cfbe8bd..2c95a68 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,9 +1,9 @@
-// paint.c — Lesson 013: raw bytes and pixel formats.
+// paint.c — Lesson 014: endianness and image-header layout.
 //
-// A pixel buffer is nothing but bytes.  Ours is width x height pixels at
-// 3 bytes each (RGB888), laid out row by row.  PutPixel and GetPixel map
-// (x, y, color) onto byte offsets; HexDump shows the buffer as it really
-// sits in memory.
+// A pixel buffer is bytes (lesson 013); an image FILE is bytes too, with a
+// header whose field order and byte order the format pins down.  PutU16LE
+// and PutU32LE write integers byte by byte in little-endian order, and
+// BuildBmpHeader lays out the 54-byte BMP header field by field.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -15,7 +15,7 @@ static unsigned char *pixels;
 static void PutPixel(unsigned char *px, int w, int h, int x, int y,
                      unsigned char r, unsigned char g, unsigned char b)
 {
-    (void)h; /* unused so far — bounds arrive in the exercises below */
+    (void)h; /* unused for now — no bounds checks yet */
     int off = y * (w * 3) + x * 3;
     px[off + 0] = r;
     px[off + 1] = g;
@@ -42,6 +42,71 @@ static void HexDump(const unsigned char *px, int w, int h)
     }
 }
 
+static void DumpBytes(const unsigned char *p, int n)
+{
+    for (int i = 0; i < n; i++) {
+        printf("%02X", p[i]);
+        if (i % 16 == 15 || i == n - 1) putchar('\n');
+        else putchar(' ');
+    }
+}
+
+static void PutU16LE(unsigned char *dst, unsigned int v)
+{
+    dst[0] = (unsigned char)(v & 0xFF);
+    dst[1] = (unsigned char)((v >> 8) & 0xFF);
+}
+
+static void PutU32LE(unsigned char *dst, unsigned int v)
+{
+    dst[0] = (unsigned char)(v & 0xFF);
+    dst[1] = (unsigned char)((v >> 8) & 0xFF);
+    dst[2] = (unsigned char)((v >> 16) & 0xFF);
+    dst[3] = (unsigned char)((v >> 24) & 0xFF);
+}
+
+static unsigned int GetU16LE(const unsigned char *src)
+{
+    return (unsigned int)src[0] | ((unsigned int)src[1] << 8);
+}
+
+static unsigned int GetU32LE(const unsigned char *src)
+{
+    return (unsigned int)src[0]
+         | ((unsigned int)src[1] << 8)
+         | ((unsigned int)src[2] << 16)
+         | ((unsigned int)src[3] << 24);
+}
+
+// BuildBmpHeader — lay out the 54-byte BMP header field by field.
+// 14-byte file header:  "BM", file size, reserved, data offset.
+// 40-byte info header: size, width, height, planes, bpp, compression,
+//                      image size, resolution, colors.
+static void BuildBmpHeader(unsigned char *header, int w, int h)
+{
+    unsigned int row_size = (unsigned int)w * 3;
+    unsigned int pad = (4 - row_size % 4) % 4;
+    unsigned int image_size = (row_size + pad) * (unsigned int)h;
+
+    header[0] = 'B';
+    header[1] = 'M';
+    PutU32LE(header + 2, 54 + image_size);      /* file size */
+    PutU32LE(header + 6, 0);                    /* reserved */
+    PutU32LE(header + 10, 54);                  /* data offset */
+
+    PutU32LE(header + 14, 40);                  /* info header size */
+    PutU32LE(header + 18, (unsigned int)w);     /* width */
+    PutU32LE(header + 22, (unsigned int)h);     /* height */
+    PutU16LE(header + 26, 1);                   /* planes */
+    PutU16LE(header + 28, 24);                  /* bits per pixel */
+    PutU32LE(header + 30, 0);                   /* compression: none */
+    PutU32LE(header + 34, image_size);          /* image size */
+    PutU32LE(header + 38, 2835);                /* x pixels per meter */
+    PutU32LE(header + 42, 2835);                /* y pixels per meter */
+    PutU32LE(header + 46, 0);                   /* colors used */
+    PutU32LE(header + 50, 0);                   /* important colors */
+}
+
 int main(void)
 {
     pixels = calloc((size_t)W * H * 3, 1);
@@ -59,6 +124,28 @@ int main(void)
     printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);
 
     HexDump(pixels, W, H);
+
+    /* What does a multi-byte integer look like in memory? */
+    unsigned int probe = 0x01020304;
+    printf("0x01020304 in memory:");
+    for (int i = 0; i < 4; i++)
+        printf(" %02X", ((unsigned char *)&probe)[i]);
+    putchar('\n');
+
+    unsigned char header[54];
+    BuildBmpHeader(header, W, H);
+    printf("BMP header:\n");
+    DumpBytes(header, 54);
+
+    printf("file size   %u\n", GetU32LE(header + 2));
+    printf("data offset %u\n", GetU32LE(header + 10));
+    printf("header size %u\n", GetU32LE(header + 14));
+    printf("width       %u\n", GetU32LE(header + 18));
+    printf("height      %u\n", GetU32LE(header + 22));
+    printf("planes      %u\n", GetU16LE(header + 26));
+    printf("bpp         %u\n", GetU16LE(header + 28));
+    printf("image size  %u\n", GetU32LE(header + 34));
+
     free(pixels);
     return 0;
 }
```

## Exercises

Four short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Width 258 *(predict-the-output)*

In `main`, build the header for a different image: change the
`BuildBmpHeader` call to `BuildBmpHeader(header, 258, 4);`. Before you
compile, predict two things in writing: (a) the exact eight bytes the dump
will show at header offsets 18 through 25 — width and height fields — and
(b) the two lines the decoded-field printout will show for width and
height. Then run and reconcile. Why is 258 a better test value here than
8 would have been?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-014/ex1.md)

### Exercise 2 — The wrong way around *(extend-the-code)*

Add a `PutU32BE` encoder (big-endian: most significant byte first) next to
`PutU32LE`, and use it for the width field in `BuildBmpHeader` — one line
changed. Run the program and look at what the decoder now reports as the
width. Where exactly does that number come from, and which part of the
lesson's argument does this little experiment confirm?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-014/ex2.md)

### Exercise 3 — Structs do not make files *(explain-in-prose)*

Instead of byte stores, one could declare a `struct BmpHeader` holding
every field in order and write it out with one `fwrite`. Measure what the
compiler actually does with that idea: declare such a struct (an
`unsigned short` for the two-byte fields, `unsigned int`/`int` for the
four-byte fields, all in BMP's field order) and print `sizeof` the struct
and `offsetof` for at least the `type`, `file_size`, `data_offset`,
`info_size`, `planes`, and `bpp` fields. You will need `#include
<stddef.h>` for `offsetof`. Compare your numbers against the byte offsets
`BuildBmpHeader` uses, then explain in your own words why writing the
struct to disk would produce a file no BMP decoder accepts — and why
`#pragma pack` or `__attribute__((packed))` patches the symptom rather
than the disease. *(Extra credit if you figure out, and verify, what the
two missing bytes would do to a decoder.)*

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-014/ex3.md)

### Exercise 4 — Ask your own machine *(port-to-your-own-machine)*

Make the program report its host's byte order at runtime — one line, no
compiler macros or predefined symbols allowed: decide by inspecting the
bytes of an `int` the way the probe does. Run it on your machine and record
what it says. Then find out what your machine *thinks* it is (`lscpu`
prints a `Byte order:` line on Linux; `uname -m` is a hint) and confirm the
two agree. Finally: which lines of the header dump would change if you ran
this program on a big-endian machine, and which lines would stay identical?
Argue from the code, not from memory.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-014/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 013 — raw bytes and pixel formats](lesson-013-raw-bytes.md) ·
**Next:** [Lesson 015 — fill-rect onto a memory buffer](lesson-015-fill-rect.md) ·
**Code tag:** [`lesson-014`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-014)
