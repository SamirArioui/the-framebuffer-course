# Lesson 050 — the bitmap font as an asset

{{#include ../../stability-horizon.md}}

## Prose

Text on a screen is a lie told in pixels, and this lesson starts telling
it: a **bitmap font** — a sheet of tiny glyph images, one per character —
loaded as an asset and drawn through the one blitter. No new drawing code
appears today. A glyph is a sprite; the font is a sheet of them; the
loader's whole job is to cut the sheet into sprites so the blit path
lesson 045 built draws letters with the same rules it draws the hero.
That is the "text and tiles are sprites with bookkeeping" promise of
lesson 045, paid on its first installment.

### The sheet's format, defined by hand

The asset is `assets/font.ppm` — a PPM (P6) image like the sprite, with
one convention on top: **the sheet is a grid of fixed 8×8 cells, 16
columns by 6 rows, holding ASCII 32 (`space`) through 127 in reading
order.** Cell 0 is `space`, cell 16 is `P`, cell 65 is `a`. The whole
format is those sentences; the loader encodes them:

```c++
constexpr int FONT_CELL = 8;
constexpr int FONT_COLS = 16;
constexpr int FONT_ROWS = 6;
constexpr int FONT_FIRST = 32;
constexpr int FONT_COUNT = FONT_COLS * FONT_ROWS; /* ASCII 32..127 */
```

The sheet is `128 × 48` pixels — 16 × 8 by 6 × 8 — and every pixel is
either ink (white in this asset) or the sprite key color (magenta), which
the loader inherits from the sprite convention of lesson 045. A glyph's
background is transparent; the letters float over whatever is behind
them.

The character→cell rule is arithmetic, not a table:

```
index = (unsigned char)c - 32
cell  = (index % 16, index / 16)        /* column, row of the cell */
```

and `FontGlyph` is that arithmetic with a bounds check:

```c++
const Sprite *FontGlyph(const Font &font, char c)
{
    int index = (int)(unsigned char)c - FONT_FIRST;
    if (index < 0 || index >= FONT_COUNT)
        return 0; /* outside the sheet: the character has no glyph */
    return &font.glyphs[index];
}
```

The `(unsigned char)` cast is there because a `char` on this platform is
*signed*: a byte like `0xC3` (the first byte of a UTF-8 `Ö`) is −61 as a
`char`, and the index math must see the byte's value, not its sign. The
`0` return is the sheet's honest "no such character" — the seed of
lesson 051's missing-glyph rule.

### The cut

`LoadFont` is three moves, all familiar:

1. **Allocate the glyph memory** — one arena block of `96 × 8 × 8 × 3` =
   18,432 bytes, exactly the sheet's pixel count. Each glyph will be a
   contiguous 192-byte sprite in this block.
2. **Load the sheet through the sprite loader** — it is a P6 image;
   `LoadSprite` already knows how to parse one, key color included. The
   arena mark is taken *after* the glyph block, so the rollback in the
   next step frees only the sheet's bytes (lesson 041's mark does exactly
   this job).
3. **Cut, cell by cell** — for each of the 96 cells, copy its 8 rows × 8
   pixels out of the sheet's rows into the glyph's own contiguous sprite.

```c++
    for (int k = 0; k < FONT_COUNT; ++k) {
        int cell_x = (k % FONT_COLS) * FONT_CELL;
        int cell_y = (k / FONT_COLS) * FONT_CELL;
        ...
        for (int r = 0; r < FONT_CELL; ++r)
            for (int c = 0; c < FONT_CELL; ++c) {
                const unsigned char *src = &sheet.sprite.pixels[...];
                unsigned char *dst = &glyph.pixels[...];
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
            }
    }
    ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
```

Why cut at all — why not point glyph sprites into the sheet? Because a
cell's rows are *not* contiguous in the sheet (row 2 of the `A` cell
sits 128 pixels away from row 1, not 8), and `Sprite` has one simple
layout: row after row. The copy makes each glyph a textbook sprite —
one that needs no special case in the blitter, the checks, or anything
drawn later. The 18 KB of sheet that the cut replaces is rolled back
into the arena before the game starts.

### The claim, checked

The run draws two glyphs through `BlitSprite` and reads every pixel back
against the glyph's own bytes — a cap letter and a descender letter, the
two shapes that catch layout bugs:

```
engine: font assets/font.ppm: 96 glyphs of 8x8 from a 128x48 sheet
engine: font check: glyph 'A' — 64 pixels read back, 0 mismatches (18 ink, 46 key)
engine: font check: glyph 'g' — 64 pixels read back, 0 mismatches (24 ink, 40 key)
```

64 pixels per glyph — 8 × 8 — and zero mismatches: every ink pixel is
where the sheet says it is, every key-colored pixel wrote nothing. The
counts are the art's own shape (the `A` in this font is 18 pixels of ink;
the `g` is 24) and they move when the art moves — which is how you know
the check is looking at the pixels and not just nodding.

The window agrees: the label the render phase now draws — the five
hand-placed glyphs of `SCORE`, each its own blit at `8 + i × 8` — shows
up white-on-background through the whole presentation path.

### One blit, five glyphs

The render phase draws the label the crude way, on purpose:

```c++
        for (int li = 0; HUD_LABEL[li]; ++li) {
            const Sprite *glyph = FontGlyph(font, HUD_LABEL[li]);
            if (glyph)
                BlitSprite(*fb, *glyph, 8 + li * FONT_CELL, 8);
        }
```

One glyph per blit, one position per glyph, computed by hand in the
loop's index. That hand is exactly what lesson 051 replaces with a
layout — strings, spacing, and the rule for characters the sheet does
not have. And notice where the glyph draws' time goes: the frame
account's `sprites` phase absorbs them (0.001 ms → 0.003 ms with five
more blits), because a glyph *is* a sprite. When lesson 051 names the
text phase, that time moves to its own row — measured, not assumed.

## Code step

One change for this lesson: `assets/font.ppm` is authored (a 128×48 P6
sheet of 96 hand-drawn 8×8 glyphs), `src/font.h` / `src/font.cpp` bring
the loader (the sheet cut into glyph sprites through the sprite loader
and the arena's mark), and `main.cpp` loads the font, checks two glyphs
pixel-for-pixel, and draws the hand-laid `SCORE` label through the blit.
The sprite, the blitter, and the probe are untouched. Its end state is
tagged `lesson-050`.

```diff
diff --git a/assets/font.ppm b/assets/font.ppm
new file mode 100644
index 0000000..9e78c68
Binary files /dev/null and b/assets/font.ppm differ
diff --git a/src/font.cpp b/src/font.cpp
new file mode 100644
index 0000000..75ff026
--- /dev/null
+++ b/src/font.cpp
@@ -0,0 +1,85 @@
+// font.cpp — cutting the glyph sheet into sprites.
+//
+// Lesson 050: the sheet is one sprite; a glyph is a cell of it. The cut
+// is the same byte-by-byte habit as every asset here — each cell's
+// pixels are copied, in order, into their own sprite.
+
+#include "font.h"
+
+namespace engine {
+
+FontResult LoadFont(Arena &arena, const char *path)
+{
+    FontResult result = { {}, FONT_OK };
+
+    /* One allocation for every glyph's pixels — 96 cells of 8x8x3. */
+    size_t glyph_bytes =
+        (size_t)FONT_COUNT * FONT_CELL * FONT_CELL * 3;
+    unsigned char *pixels =
+        (unsigned char *)ArenaAlloc(arena, glyph_bytes, 4);
+    if (!pixels) {
+        result.error = FONT_NO_ROOM;
+        return result;
+    }
+
+    /* The sheet loads through the sprite loader (it is a P6 image like
+       any other), and its bytes go back to the arena when the cut is
+       done — the mark sits after the glyphs, so only the sheet rolls
+       back. */
+    size_t mark = ArenaMark(arena);
+    SpriteResult sheet = LoadSprite(arena, path);
+    if (sheet.error == SPRITE_MISSING) {
+        result.error = FONT_MISSING;
+        return result;
+    }
+    if (sheet.error != SPRITE_OK) {
+        result.error = FONT_MALFORMED;
+        return result;
+    }
+
+    /* The format is fixed: the sheet is exactly FONT_COLS x FONT_ROWS
+       cells. A sheet of any other size is a different format's file. */
+    if (sheet.sprite.width != FONT_COLS * FONT_CELL ||
+        sheet.sprite.height != FONT_ROWS * FONT_CELL) {
+        ArenaRollback(arena, mark);
+        result.error = FONT_MALFORMED;
+        return result;
+    }
+
+    for (int k = 0; k < FONT_COUNT; ++k) {
+        int cell_x = (k % FONT_COLS) * FONT_CELL;
+        int cell_y = (k / FONT_COLS) * FONT_CELL;
+        Sprite &glyph = result.font.glyphs[k];
+        glyph.pixels =
+            pixels + (size_t)k * FONT_CELL * FONT_CELL * 3;
+        glyph.width = FONT_CELL;
+        glyph.height = FONT_CELL;
+        glyph.key_r = sheet.sprite.key_r;
+        glyph.key_g = sheet.sprite.key_g;
+        glyph.key_b = sheet.sprite.key_b;
+        for (int r = 0; r < FONT_CELL; ++r)
+            for (int c = 0; c < FONT_CELL; ++c) {
+                const unsigned char *src =
+                    &sheet.sprite.pixels[(((cell_y + r) * sheet.sprite.width) +
+                                          (cell_x + c)) * 3];
+                unsigned char *dst =
+                    &glyph.pixels[((r * FONT_CELL) + c) * 3];
+                dst[0] = src[0];
+                dst[1] = src[1];
+                dst[2] = src[2];
+            }
+    }
+    ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
+    result.error = FONT_OK;
+    return result;
+}
+
+const Sprite *FontGlyph(const Font &font, char c)
+{
+    int index = (int)(unsigned char)c - FONT_FIRST;
+    if (index < 0 || index >= FONT_COUNT)
+        return 0; /* outside the sheet: the character has no glyph */
+    return &font.glyphs[index];
+}
+
+} /* namespace engine */
diff --git a/src/font.h b/src/font.h
new file mode 100644
index 0000000..fb95e47
--- /dev/null
+++ b/src/font.h
@@ -0,0 +1,52 @@
+// font.h — the bitmap font: a glyph sheet cut into sprites.
+//
+// Lesson 050: the font is an asset like the sprite — a PPM (P6) sheet of
+// 8x8 glyph cells, 16 columns by 6 rows, covering ASCII 32..127 in
+// reading order. The loader cuts the sheet into one Sprite per glyph, so
+// every glyph draws through the one blitter (lesson 045) with no special
+// case, no second drawing path, and no new rules.
+#ifndef FONT_H
+#define FONT_H
+
+#include "sprite.h"
+
+namespace engine {
+
+/* The sheet's format, defined by hand (lesson 050): fixed 8x8 cells in a
+   16x6 grid, one character per cell starting at ASCII 32. */
+constexpr int FONT_CELL = 8;
+constexpr int FONT_COLS = 16;
+constexpr int FONT_ROWS = 6;
+constexpr int FONT_FIRST = 32;
+constexpr int FONT_COUNT = FONT_COLS * FONT_ROWS; /* ASCII 32..127 */
+
+/* A font: one sprite per glyph, cut from the sheet at load. */
+struct Font {
+    Sprite glyphs[FONT_COUNT];
+};
+
+/* A load either hands over a font or names what went wrong. */
+enum FontError {
+    FONT_OK = 0,
+    FONT_MISSING,   /* the sheet is not there or cannot be read */
+    FONT_MALFORMED, /* the bytes are not the 16x6 sheet the format fixes */
+    FONT_NO_ROOM,   /* the arena had no room for the glyphs */
+};
+
+struct FontResult {
+    Font font;
+    FontError error;
+};
+
+/* Loads a glyph sheet and cuts it into FONT_COUNT glyph sprites — one
+   arena allocation for all the glyph pixels, the sheet's own bytes given
+   back when the copy is done. */
+FontResult LoadFont(Arena &arena, const char *path);
+
+/* The glyph sprite for a character, or 0 for a character the sheet does
+   not cover. Lesson 051's layout rule builds on this answer. */
+const Sprite *FontGlyph(const Font &font, char c);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 887d551..9aba782 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -9,6 +9,7 @@
 
 #include "arena.h"
 #include "blit.h"
+#include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
 #include "platform.h"
@@ -21,6 +22,11 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
+/* Lesson 050: the label the demo lays out by hand — one glyph per
+   blit, one position per glyph. Lesson 051 replaces the hand with a
+   layout loop. */
+constexpr char HUD_LABEL[] = "SCORE";
+
 /* Lesson 047: the caches deep dive's evidence — a copy walk over arena
    memory at two strides, timed at working-set sizes that cross this
    machine's caches. The walk is the blit's inner copy with the
@@ -224,6 +230,65 @@ int Run(void)
     /* Lesson 047's evidence, measured before the loop starts. */
     CacheProbe(arena);
 
+    /* Lesson 050: the font is an asset too — a glyph sheet the loader
+       cuts into sprites. */
+    const char *font_path = "assets/font.ppm";
+    FontResult font_loaded = LoadFont(arena, font_path);
+    if (font_loaded.error != FONT_OK) {
+        switch (font_loaded.error) {
+        case FONT_MISSING:
+            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
+                         font_path);
+            break;
+        case FONT_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: %s: not a 16x6 sheet of 8x8 glyphs\n",
+                         font_path);
+            break;
+        default:
+            std::fprintf(stderr, "engine: %s: no room in the arena\n",
+                         font_path);
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Font &font = font_loaded.font;
+    std::printf("engine: font %s: %d glyphs of %dx%d from a %dx%d sheet\n",
+                font_path, FONT_COUNT, FONT_CELL, FONT_CELL,
+                FONT_COLS * FONT_CELL, FONT_ROWS * FONT_CELL);
+
+    /* The glyph claim, checked against the framebuffer's bytes: a glyph
+       drawn through the blit is the sheet's cell, pixel for pixel. */
+    const char checked[2] = { 'A', 'g' };
+    for (int gi = 0; gi < 2; ++gi) {
+        const Sprite *glyph = FontGlyph(font, checked[gi]);
+        ClearBuffer(*fb, 32, 32, 64);
+        BlitSprite(*fb, *glyph, 200 + gi * 16, 64);
+        int ink = 0, key = 0, mismatches = 0;
+        for (int j = 0; j < glyph->height; ++j)
+            for (int i = 0; i < glyph->width; ++i) {
+                const unsigned char *p = &glyph->pixels[(j * glyph->width + i) * 3];
+                unsigned char r, g, b;
+                GetPixel(*fb, 200 + gi * 16 + i, 64 + j, r, g, b);
+                bool is_key = p[0] == glyph->key_r && p[1] == glyph->key_g &&
+                              p[2] == glyph->key_b;
+                if (is_key) {
+                    ++key;
+                    if (r != 32 || g != 32 || b != 64)
+                        ++mismatches;
+                } else {
+                    ++ink;
+                    if (r != p[0] || g != p[1] || b != p[2])
+                        ++mismatches;
+                }
+            }
+        std::printf("engine: font check: glyph '%c' — %d pixels read back, %d mismatches (%d ink, %d key)\n",
+                    checked[gi], glyph->width * glyph->height, mismatches,
+                    ink, key);
+    }
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
@@ -281,10 +346,17 @@ int Run(void)
 
         /* Render: every frame draws the whole scene — clear, then the
            sprite through the one blit. The sprite draw is timed as its own
-           named phase: the first subsystem the frame record can name. */
+           named phase: the first subsystem the frame record can name.
+           Glyphs are sprites too (lesson 050) — they count here until
+           lesson 051 names the text phase. */
         ClearBuffer(*fb, 32, 32, 64);
         double t_sprites = platform::Now();
         BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
+        for (int li = 0; HUD_LABEL[li]; ++li) {
+            const Sprite *glyph = FontGlyph(font, HUD_LABEL[li]);
+            if (glyph)
+                BlitSprite(*fb, *glyph, 8 + li * FONT_CELL, 8);
+        }
         frame.sprites = platform::Now() - t_sprites;
 
         frame.render = platform::Now() - t1;
```

The font's pixels are binary and so is its diff — git prints `Binary
files … differ` for it. The sheet's format is the four constants in
`font.h`; its art is the file.

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The font dump *(extend-the-code)*

The sheet is 96 glyphs in a grid; prove the cut is lossless by drawing
the grid back: blit every glyph in its sheet position (a 16×6 grid of
8×8 cells, at a fixed corner of the frame), then read every pixel of the
dump back and compare it with the glyphs' own bytes. Report one number:
the mismatch count. What would a nonzero count tell you about the cut —
and which diagnosis matches which bug: mismatches in *every* glyph, or
mismatches in *exactly one*?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-050/ex1.md)

### Exercise 2 — The bytes the sheet never heard of *(predict-the-output)*

`FontGlyph` answers `0` for characters outside ASCII 32..127 — but the
interesting part is the arithmetic that gets it there. Before you run
anything, write down what the function returns for five probes: `A`,
newline, byte `0`, byte `200`, and byte `195` (a UTF-8 lead byte). Then
extend the run to print the five answers and to dry-run a label
containing a `Ö` — whose UTF-8 encoding is *two* bytes — and report how
many of the label's bytes found a glyph. Reconcile every answer with the
index math, including the one case where a signed `char` would have
computed a different index than the byte deserves.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-050/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 049 — the SIMD lens](lesson-049-simd.md) ·
**Next:** [Lesson 051 — text on screen](lesson-051-text.md) ·
**Code tag:** [`lesson-050`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-050)
