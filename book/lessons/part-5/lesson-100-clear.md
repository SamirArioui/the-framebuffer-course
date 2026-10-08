# Lesson 100 — pass 2b: fix the clear

{{#include ../../stability-horizon.md}}

## Prose

The menu's second and last fix. Pass 1 named two hotspots; pass 2a
took the map's draw down 43%; this lesson takes the other one —
**the frame's clear**: `ClearBuffer`, one call a frame, every frame,
painting all 307,200 pixels of the framebuffer — `clear 0.456 ms` of a
play frame (23%) and `0.473 ms` of a panel frame (**49% of every frame
that draws no map**), `1.33 s` (37.7%) of the profile's CPU. One
function, one loop, and the deep dives' third lever: **copy wider**.

### What the clear costs today

The loop writes each pixel as four byte stores through a byte pointer:
`p[0] = b; p[1] = g; p[2] = r; p[3] = 0;` — four stores, an address
computed per pixel, 1.2 million byte stores per frame. Lesson 049
priced this exact loop in its table of outcomes: "`ClearBuffer` fares
the same: four byte stores per pixel even at `-O3`, because a pixel is
four *different* bytes … and the loop is a pattern fill, not a copy."

But the lesson's own lens also says what to do about it: a fill of *one
repeated value* is the most vectorizable shape there is — if the loop
gives the machine one value to repeat.

### The fix: one word per pixel

A pixel is four bytes, and four bytes are a word. The fix writes one
`unsigned int` per pixel instead of four bytes — and composes the word
*from the framebuffer's own bytes*: a four-byte array `{b, g, r, 0}`
and one `memcpy` into the word. Whatever order this machine stores
words in, the four bytes land exactly where the byte stores put them;
the loop never thinks about byte order again. Same pixels, one store
in four.

And the fill loop is now the shape lesson 049 reads: one value, one
contiguous region, no decisions. The census (`tools/disasm.sh
--census` at `-O3`) says what the compiler made of it — `ClearBuffer`
went from **0 vector instructions to 5**, and the listing is the
lesson's four parts in miniature:

```
    5891:	movd   %ecx,%xmm1          ; the color, in a register
    589b:	pshufd $0x0,%xmm1,%xmm0    ; splat: four pixels per word-load
    ...
    58c0:	movups %xmm0,(%rax)        ; the wide loop: 32 bytes a turn
    58c3:	add    $0x20,%rax
    58c7:	movups %xmm0,-0x10(%rax)   ; two stores, eight pixels
    58cb:	cmp    %rdi,%rax
    58ce:	jne    58c0
```

— a guard for the tiny counts, the wide loop moving **eight pixels per
turn**, and the tails folded after it. The course never writes SIMD;
it shapes the loop and reads what the compiler reached for (049's
rule). Here the compiler reached.

### The measured cost falls

Same measurement run, same `step` split (1,551 frames, 1,415 play):

```
                                 before (lesson-099)   after (lesson-100)
  play frames, clear               0.449 ms              0.239 ms
  panel frames, clear              0.446                 0.246
  play frames, total               1.505 ms              1.269 ms
```

The named hotspot **falls 47%** on the frames that matter most — a
panel frame is now `0.769 ms`, half of which used to be the clear. The
profiled run (2,583 frames) agrees: `ClearBuffer`'s self time `1.05 s
→ 0.70 s`, the whole run's CPU `2.28 s → 1.98 s`. Not the fourfold
the store count suggests at `-O0` — the loop overhead that remains is
the compiler's bookkeeping, and at `-O3` the wide loop moves eight
pixels a turn as quoted above. Both numbers are this build's; both
fell.

### Two fixes, one menu, and what the frame looks like now

The three-pass menu is spent: measure named two, the two are fixed,
and nothing else was touched. The play frame's account, from the first
measurement to this one:

```
play frames:            lesson-098   lesson-099   lesson-100
  tilemap                 0.981        0.559        0.539
  clear                   0.456        0.449        0.239
  sprites                 0.006        0.006        0.006
  text                    0.012        0.010        0.010
  update + audio          0.045        0.046        0.046
  present                 0.444        0.434        0.428
  total                   1.944        1.505        1.269
```

The two named rows fell `1.437 → 0.778 ms` together and the frame
fell with them (−35%); the rows the menu did not name — the seam's
`present` above all — are now the frame's largest single line. That
is not a failure of the menu; it *is* the menu: anything the passes
did not name is future work, and pass 3's report will say exactly
that, measured.

### What this run verified, and what it did not

- **The change addresses the named hotspot and the measured cost
  falls** — `clear 0.449 → 0.239 ms` on 1,415 real play frames (and
  `0.446 → 0.246 ms` on the panel frames); `ClearBuffer` self `1.05 s
  → 0.70 s` on 2,583 real frames of the profiled build.
- **The game's behavior is unchanged** — byte-level: a scratch harness
  ran 400 clears (random colors, random sizes, odd widths included)
  through both loops: **zero differing bytes**. Report-level: the
  scripted run's transcript reduces to the same report set as the
  previous state's — the same 106 templates (one probe line landing a
  few positions earlier in the fight's sequence — the noise floor
  lesson 097 measured).
- **The compiler's answer is read, not assumed** — the census's 5
  vector instructions and the listing above are the evidence that the
  widened loop exists at all at `-O3`; at `-O0` (this course's build)
  the win is the store count, and it is measured, not argued.

What this run did **not** verify is anything about *your* machine's
byte order or alignment — the word fill's safety argument (why the
bytes land where the byte stores put them; why the word pointer is
allowed) is exercise 2's, and it is worth making with your own eyes,
because a fast fill that lies about its pixels is worse than a slow
one. And the numbers stay this rig's (WSL2, Xvfb `:99`, `-O0`): the
fall is real here, the size is this machine's.

## Code step

One change: hotspot #2. `src/framebuffer.cpp`'s `ClearBuffer` writes
one word per pixel — the word composed from the framebuffer's own four
bytes (`{b, g, r, 0}` through `memcpy`, so the byte order is the
machine's and the pixels are the engine's) — in a contiguous fill the
compiler can widen. The bytes written are the bytes the old loop
wrote. `BlitSpriteFrame`, `PutPixel`, and the rest of the framebuffer
are untouched. Its end state is tagged `lesson-100`.

```diff
diff --git a/src/framebuffer.cpp b/src/framebuffer.cpp
index 88080d5..b36981f 100644
--- a/src/framebuffer.cpp
+++ b/src/framebuffer.cpp
@@ -9,6 +9,8 @@
 
 #include "framebuffer.h"
 
+#include <cstring>
+
 namespace engine {
 
 static Framebuffer framebuffer = { 0, FRAME_WIDTH, FRAME_HEIGHT };
@@ -25,14 +27,23 @@ Framebuffer *GetFramebuffer(Arena &arena)
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                  unsigned char b)
 {
+    /* Lesson 100: the clear is the frame's second-hottest work (the
+       measure pass, lesson 098) — 307,200 pixels every frame — and its
+       loop writes four bytes per pixel through a byte pointer. The deep
+       dives' lever is "copy wider": one *word* per pixel, in a
+       contiguous fill the compiler can widen (lesson 049's lens reads
+       the answer below). The word is composed from the framebuffer's
+       own bytes — bytes through memcpy, so whatever order this machine
+       stores words in, the four bytes land exactly as the byte stores
+       put them. Same pixels, one store in four. */
+    const unsigned char bytes[4] = { b, g, r, 0 };
+    unsigned int color;
+    std::memcpy(&color, bytes, sizeof color);
+
+    unsigned int *pixels = (unsigned int *)fb.pixels;
     int count = fb.width * fb.height;
-    for (int i = 0; i < count; ++i) {
-        unsigned char *p = fb.pixels + i * 4;
-        p[0] = b;
-        p[1] = g;
-        p[2] = r;
-        p[3] = 0;
-    }
+    for (int i = 0; i < count; ++i)
+        pixels[i] = color;
 }
 
 void PutPixel(Framebuffer &fb, int x, int y, unsigned char r,
```

## Exercises

Two challenges that finish the fix's education — the measurement
before/after with your own instrument, and the safety argument a fast
fill owes its pixels. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the price of a pixel *(measure-the-performance)*

The lesson quotes the `clear` row's fall; the row is a frame's cost,
not a pixel's. Price the pixel: a bench at startup that times
`ClearBuffer` on the real buffer at several sizes (the frame's own
640×480 down to a small region) and prints nanoseconds per pixel. The
bench's patch applies at `lesson-099` as well as here — the files it
touches are untouched by this lesson's step — so run it at both tags
and report the per-pixel price before and after, beside the
instruction-count prediction the lesson's argument makes (four byte
stores and an address each → one word store each). Which of your
numbers belongs to the store count, and which to the loop the compiler
leaves behind at `-O0`?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-100/ex1.md)

### Exercise 2 — what the word knows *(explain-in-prose)*

A fast fill that lies about its pixels is worse than a slow one, and
the word fill makes three claims it owes an argument for: the word
pointer may write this memory (alignment), the word's bytes land where
the byte stores put them (byte order), and the write may go through a
different type than the buffer was written through before (aliasing).
Make the argument — in your own words, each claim, what the engine
guarantees and what the machine guarantees — and make it checkable: a
probe that verifies each claim on the real buffer at startup (the
pointer's alignment, and a clear read back through `GetPixel` and
through raw bytes at the corners and the middle), printing what it
checked. Then say what you would change for a machine where one of the
claims does not hold.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-100/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 099 — pass 2a: fix the map's draw](lesson-099-map-draw.md) ·
**Next:** [Lesson 101 — pass 3: the frame-budget report](lesson-101-frame-budget.md) ·
**Code tag:** [`lesson-100`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-100)
