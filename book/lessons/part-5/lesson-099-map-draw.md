# Lesson 099 — pass 2a: fix the map's draw

{{#include ../../stability-horizon.md}}

## Prose

Pass 1 named the game's two hottest pieces of work with numbers from
real frames, and the frozen menu (design D11) fixes them in order —
exactly two, one lesson each, and nothing else. **Hotspot #1 is the
map's draw**: `DrawTileMap`'s walk, one `BlitSprite` per cell — `tilemap
0.981 ms` of a play frame's `1.944 ms`, `BlitSprite` at `2.11 s` of the
`3.53 s` the profile sampled (60.9%). This lesson fixes that, with the
levers the deep dives named and priced: **copy less, copy closer
together, copy wider** (lesson 049's closing: "the three levers are
already named"). And it fixes nothing else.

### What the hotspot actually is

1,536 tiles a frame, every one through the sprite blit's inner loop:
per pixel, recompute the source index (a multiply and two adds), test
three bytes for the transparent color, recompute the destination
index, store four bytes. On a play frame that loop runs over ~393,000
pixels — and, measured, it is half the frame.

Two facts about the work make the levers obvious. The per-pixel
addressing is *recomputation* — the row's pointers are the same for
every pixel in it. And the per-pixel transparency test is *wasted* on
map tiles almost everywhere it runs: it is a decision with no decision
in it. Culling is the third fact: the walk visits all 1,536 cells of
the map whether or not the frame can show them.

### The fix, three moves

**Copy closer together** — `BlitSprite`'s rows now compute their source
and destination pointers once and step them (`src += 3`, `dst += 4`),
instead of rebuilding both indices per pixel. Sequential access, the
shape lesson 047 walks cache lines with.

**Copy wider** — a sprite with **no** transparent pixel takes a
straight expand: no per-pixel test at all. What knows whether a sprite
has one? The load does: `Sprite` gains `key_count`, counted once where
a sprite is born — `LoadSprite`, the tile sheet's cut in
`LoadTileSheet`, the font's cut in `LoadFont` — so the frame pays no
decision the load already made (`CountKeyPixels`, beside the loader).
The map's tiles carry zero key pixels and go the straight path; the
hero, the shots, and the glyphs keep the per-pixel test, because for
them the decision is real — lesson 049's question stands: what would
"vectorized transparency" mean?

**Copy less** — `DrawTileMap` walks only the cells the frame can show:
the visible window computed once from the map's offset — the first
cell whose right edge passes the frame's left, through the last whose
left edge is inside it. On this map (48×32 cells, a 640×480 frame)
that is about 1,240 cells of the 1,536 — the rest write nothing
through the blit's clipping anyway, so the pixels drawn are the same
pixels.

Every pixel lands where it always landed; that claim is measured at
the end of this lesson's run, not argued.

### The measured cost falls

The same measurement run as pass 1 — the same scenario (play legs with
restarts), the same split by the record's `step` field (1,551 frames,
1,415 of them play, in both runs):

```
play frames:                     before (lesson-098)   after (lesson-099)
  tilemap                          0.981 ms              0.559 ms
  clear                            0.456                 0.449
  sprites                          0.006                 0.006
  text                             0.012                 0.010
  present                          0.444                 0.434
  total                            1.944 ms              1.505 ms
```

The named hotspot **falls 43%** and takes a fifth of the frame with
it. The profile from the same scenario (2,583 frames each run) agrees
at function level: `BlitSprite`'s self time `2.11 s → 1.14 s` (**−46%**
— `0.957 ms` per map draw to `0.466 ms`), its calls `3,501,190 →
3,045,953` (the culling, visible in the count), and the run's whole
profiled CPU `3.53 s → 2.28 s` (−35%).

### The percentage trap, read from the same profile

Look what happened to hotspot #2 in that profile: `ClearBuffer` reads
`37.68% → 46.05%`. Did the clear get *slower*? Its self time fell
`1.33 s → 1.05 s` (sampling noise and the frame's cache state), and
the precise instrument — the account's `clear` row — reads `0.456 →
0.449 ms`: **unchanged**, exactly as the menu requires. The share rose
because the *total* fell: percentages are fractions of what remains,
and a fix moves every line that is not its own. This is why the lesson
quotes milliseconds beside every percent, and why "the cost falls" is
checked against the account, never against the profile's share column.

### What this run verified, and what it did not

- **The change addresses the named hotspot and the measured cost
  falls** — `tilemap 0.981 → 0.559 ms` on 1,415 real play frames;
  `BlitSprite` self `2.11 s → 1.14 s` on 2,583 real frames of the
  profiled build.
- **The game's behavior is unchanged** — two ways. Byte-level: a
  scratch harness drew 2,000 randomized sprites (opaque and
  transparent, clipped at every edge) through both the old and the new
  loops into separate buffers: **zero pixels differ**. Report-level:
  the scripted run's transcript reduces to the same shape as the
  previous state's — 106 report templates, identical sets — and the
  demonstration lines (the states, the screens, the waves, the combat)
  re-run as documented.
- **The menu held** — the sprites' twin loop (`BlitSpriteFrame`, the
  `sprites` row, `0.006 ms`) is **not** in this diff. It was not named,
  so it was not fixed; its medicine is exercise 1's, if the learner
  wants to spend it there.

What this run did **not** verify: that these levers are all the levers
there are. The census (lesson 049's lens, `-O3`) still reads **zero
vector instructions for `BlitSprite`** — the straight expand is
branch-free and sequential, but it moves 3 bytes in and 4 bytes out,
and GCC declines to widen *that* at these settings. The listing shows
the levers' shape — the `key_count` guard, the folded addressing, the
scalar loops — and what widening would take is a *layout* change, not a
loop change: the tiles' pixels living in the framebuffer's own order
(one more arena allocation at load) so the map's copy moves words, not
bytes. That breaks the engine's one-copy-loop honesty ("every drawn
pixel comes through this loop", `blit.h`) and it is recorded as
future work — the kind of change measurement must ask for, not a
lesson smuggle in. And all numbers remain this machine's and this
build's (WSL2, Xvfb `:99`, `-O0`): the fall is real here; the sizes
are this rig's.

## Code step

One change: hotspot #1. `src/blit.cpp`'s `BlitSprite` gains the two
paths — the straight expand for sprites with no transparent pixels,
the key path with hoisted row pointers for the rest — writing exactly
the pixels the old loop wrote. `src/sprite.h/.cpp` grow `key_count` and
`CountKeyPixels`, counted at every site a sprite is born (`sprite.cpp`'s
loader, `tiles.cpp`'s sheet cut, `font.cpp`'s glyph cut). `src/tiles.cpp`'s
`DrawTileMap` walks only the visible window of cells. `BlitSpriteFrame`
is untouched — it was not named. Its end state is tagged `lesson-099`.

```diff
diff --git a/src/blit.cpp b/src/blit.cpp
index 0d417f4..4b70b5d 100644
--- a/src/blit.cpp
+++ b/src/blit.cpp
@@ -19,18 +19,54 @@ void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
     int right = x + s.width < fb.width ? x + s.width : fb.width;
     int bottom = y + s.height < fb.height ? y + s.height : fb.height;
 
-    for (int j = top; j < bottom; ++j) {
-        for (int i = left; i < right; ++i) {
+    /* Lesson 099: this loop is the map's draw, and the measure pass
+       named it the game's hottest work — so it gets the deep dives'
+       levers, and every pixel lands exactly where it always landed.
+
+       Copy closer together: each row's source and destination pointers
+       are computed once and stepped, never recomputed per pixel.
+
+       Copy wider: a sprite with no transparent pixel — `key_count`
+       zero, counted at load — takes the straight expand below, with no
+       per-pixel decision: the shape lesson 049's lens reads (a guard, a
+       wide loop, the tails folded into the row).
+
+       And the key path keeps its per-pixel decision because it must:
+       what would "vectorized transparency" mean? (lesson 049). */
+    if (s.key_count == 0) {
+        for (int j = top; j < bottom; ++j) {
             const unsigned char *src =
-                &s.pixels[(((size_t)(j - y) * s.width) + (i - x)) * 3];
-            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
-                continue; /* the transparent color writes nothing */
+                s.pixels + (((size_t)(j - y) * s.width) + (left - x)) * 3;
             unsigned char *dst =
-                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
-            dst[0] = src[2]; /* blue */
-            dst[1] = src[1]; /* green */
-            dst[2] = src[0]; /* red */
-            dst[3] = 0;
+                fb.pixels + (((size_t)j * fb.width) + left) * 4;
+            for (int i = left; i < right; ++i) {
+                dst[0] = src[2]; /* blue */
+                dst[1] = src[1]; /* green */
+                dst[2] = src[0]; /* red */
+                dst[3] = 0;
+                src += 3;
+                dst += 4;
+            }
+        }
+        return;
+    }
+
+    for (int j = top; j < bottom; ++j) {
+        const unsigned char *src =
+            s.pixels + (((size_t)(j - y) * s.width) + (left - x)) * 3;
+        unsigned char *dst =
+            fb.pixels + (((size_t)j * fb.width) + left) * 4;
+        for (int i = left; i < right; ++i) {
+            if (!(src[0] == s.key_r && src[1] == s.key_g &&
+                  src[2] == s.key_b)) { /* the transparent color writes
+                                           nothing */
+                dst[0] = src[2]; /* blue */
+                dst[1] = src[1]; /* green */
+                dst[2] = src[0]; /* red */
+                dst[3] = 0;
+            }
+            src += 3;
+            dst += 4;
         }
     }
 }
diff --git a/src/blit.h b/src/blit.h
index 3832e8b..8faf327 100644
--- a/src/blit.h
+++ b/src/blit.h
@@ -17,7 +17,11 @@ namespace engine {
    becomes one framebuffer pixel carrying the exact color the sprite has,
    except the sprite's transparent color, which writes nothing at all.
    Pixels whose destination falls outside the framebuffer are dropped —
-   lesson 015's fold at rectangle scale, never a wrap into other pixels. */
+   lesson 015's fold at rectangle scale, never a wrap into other pixels.
+   Lesson 099: a sprite with no transparent pixel (`key_count` zero —
+   counted where sprites are born) draws through a straight expand with
+   no per-pixel decision; a sprite with one keeps the decision per
+   pixel. Both paths write the same pixels. */
 void BlitSprite(Framebuffer &fb, const Sprite &sprite, int x, int y);
 
 /* Lesson 086: one frame of a sprite sheet — the `frame_w`-wide column of
diff --git a/src/font.cpp b/src/font.cpp
index 75ff026..a3586ab 100644
--- a/src/font.cpp
+++ b/src/font.cpp
@@ -68,6 +68,7 @@ FontResult LoadFont(Arena &arena, const char *path)
                 dst[1] = src[1];
                 dst[2] = src[2];
             }
+        glyph.key_count = CountKeyPixels(glyph); /* lesson 099 */
     }
     ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
     result.error = FONT_OK;
diff --git a/src/sprite.cpp b/src/sprite.cpp
index 4d04e96..fd92f3e 100644
--- a/src/sprite.cpp
+++ b/src/sprite.cpp
@@ -64,7 +64,7 @@ bool ReadNumber(const unsigned char *data, size_t size, size_t &at, long &out)
 
 SpriteResult LoadSprite(Arena &arena, const char *path)
 {
-    SpriteResult result = { { 0, 0, 0, 0, 0, 0 }, SPRITE_OK };
+    SpriteResult result = { { 0, 0, 0, 0, 0, 0, 0 }, SPRITE_OK };
 
     platform::FileData file = platform::ReadFile(path);
     if (file.error != platform::FILE_OK) {
@@ -119,8 +119,21 @@ SpriteResult LoadSprite(Arena &arena, const char *path)
     result.sprite.key_r = SPRITE_KEY_R;
     result.sprite.key_g = SPRITE_KEY_G;
     result.sprite.key_b = SPRITE_KEY_B;
+    result.sprite.key_count = CountKeyPixels(result.sprite); /* 099 */
     result.error = SPRITE_OK;
     return result;
 }
 
+int CountKeyPixels(const Sprite &sprite)
+{
+    int count = 0;
+    for (int i = 0; i < sprite.width * sprite.height; ++i) {
+        const unsigned char *p = &sprite.pixels[(size_t)i * 3];
+        if (p[0] == sprite.key_r && p[1] == sprite.key_g &&
+            p[2] == sprite.key_b)
+            count += 1;
+    }
+    return count;
+}
+
 } /* namespace engine */
diff --git a/src/sprite.h b/src/sprite.h
index b9ef087..5529c9f 100644
--- a/src/sprite.h
+++ b/src/sprite.h
@@ -27,6 +27,9 @@ struct Sprite {
     int width;
     int height;
     unsigned char key_r, key_g, key_b; /* the transparent color */
+    int key_count; /* lesson 099: how many pixels are that color, counted
+                      once at load. Zero says the sprite is opaque — the
+                      draw can skip its per-pixel decision entirely. */
 };
 
 /* A load either hands over a complete sprite or names what went wrong —
@@ -48,6 +51,12 @@ struct SpriteResult {
    back to the OS — what the engine keeps is its copy. */
 SpriteResult LoadSprite(Arena &arena, const char *path);
 
+/* Lesson 099: the sprite's transparent pixels, counted — the fact the
+   draw's fast path is keyed on. Counted once where a sprite is born
+   (the loader, the tile sheet's cut, the font's cut), never at draw
+   time: the frame pays no decision the load already made. */
+int CountKeyPixels(const Sprite &sprite);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/tiles.cpp b/src/tiles.cpp
index 66698af..0a25834 100644
--- a/src/tiles.cpp
+++ b/src/tiles.cpp
@@ -12,7 +12,7 @@ namespace engine {
 
 TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
 {
-    TileSheetResult result = { { { { 0, 0, 0, 0, 0, 0 } } }, TILES_OK };
+    TileSheetResult result = { { { { 0, 0, 0, 0, 0, 0, 0 } } }, TILES_OK };
 
     if (kind_count <= 0 || kind_count > TILE_MAX_KINDS) {
         result.error = TILES_MALFORMED;
@@ -65,6 +65,7 @@ TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
                 dst[1] = src[1];
                 dst[2] = src[2];
             }
+        tile.key_count = CountKeyPixels(tile); /* lesson 099 */
     }
     ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
     result.error = TILES_OK;
@@ -74,8 +75,25 @@ TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
 void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
                  int x, int y)
 {
-    for (int cy = 0; cy < map.height; ++cy)
-        for (int cx = 0; cx < map.width; ++cx) {
+    /* Lesson 099: copy less — the walk visits only the cells the frame
+       can show. The window is the visible span of cells computed once
+       from the map's offset: the first cell whose right edge passes the
+       frame's left (at offset x, cell -x/TILE_SIZE is the first one
+       with a pixel on screen), through the last whose left edge is
+       inside the frame (the fold's exclusive bound). A cell outside
+       writes nothing through the blit's clipping either way — the
+       pixels drawn are the same pixels; the walk is shorter. */
+    int cx0 = x < 0 ? -x / TILE_SIZE : 0;
+    int cy0 = y < 0 ? -y / TILE_SIZE : 0;
+    int cx1 = (fb.width - x + TILE_SIZE - 1) / TILE_SIZE;
+    int cy1 = (fb.height - y + TILE_SIZE - 1) / TILE_SIZE;
+    if (cx1 > map.width)
+        cx1 = map.width;
+    if (cy1 > map.height)
+        cy1 = map.height;
+
+    for (int cy = cy0; cy < cy1; ++cy)
+        for (int cx = cx0; cx < cx1; ++cx) {
             int kind = map.cells[cy * map.width + cx];
             BlitSprite(fb, sheet.kinds[kind],
                        x + cx * TILE_SIZE, y + cy * TILE_SIZE);
```

## Exercises

Two challenges that take the lesson's own medicine further — one where
the menu forbade it here, one into the measurement. Each ends with its
solution — a diff against this lesson's end state plus a walkthrough —
after the prompt.

### Exercise 1 — the sprites' draw gets the same medicine *(extend-the-code)*

`BlitSpriteFrame` is `BlitSprite`'s twin — the same per-pixel loop with
the sheet's frame column folded in — and this lesson deliberately left
it alone because the menu fixes only what pass 1 named. Spend the next
lever where you choose: give it the same treatment (hoisted rows, the
straight expand keyed on `key_count`), then measure the `sprites` row
before and after on a busy frame and report honestly what it was ever
worth. The interesting answer is the honest one.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-099/ex1.md)

### Exercise 2 — the cost per tile *(measure-the-performance)*

The map's draw is a count of tiles and a cost per tile, and the fix
touched both numbers — the culling cut the count (about 1,536 cells to
about 1,200–1,271 on this map, moving as the camera clamps along it),
the loop rewrite cut the cost each one pays. Make the count visible: a
probe that reports how many cells the walk actually drew, and at which
map offset. Then measure the pair (the probe's count beside the frame
log's `tilemap` milliseconds) over a full walk of the map, and give the
per-tile cost — before and after this lesson's step — in nanoseconds.
Say which lever each number belongs to.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-099/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 098 — pass 1: measure](lesson-098-measure.md) ·
**Next:** [Lesson 100 — pass 2b: fix the clear](lesson-100-clear.md) ·
**Code tag:** [`lesson-099`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-099)
