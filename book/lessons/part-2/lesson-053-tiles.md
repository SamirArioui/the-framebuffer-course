# Lesson 053 — tilemap drawing

{{#include ../../stability-horizon.md}}

## Prose

The map is data (lesson 052) and the tiles are sprites (lesson 050's
promise, second installment). Today the two meet: the **tilemap walk** —
one blit per cell, at computed origins — which turns the file's
characters into a world you can stand in. This is the drawing half of
the MVD's world obligation, and it is deliberately unglamorous: a loop
of `BlitSprite` calls and nothing else. Every rule it needs — clipping,
transparency, byte-exact copies — the one blitter already owns.

### Tiles are sprites

The art is `assets/tiles.ppm`: a sheet of `TILE_SIZE` (16) × 16 cells in
one row, **one cell per tile kind, in the map's kind-table order**. The
three kinds of lesson 052's map become three cells — floor (a quiet
checkered green), wall (gray brick), water (two blues with a wave). The
loader is the font sheet's loader, adapted: `LoadTileSheet` checks the
sheet is exactly `kind_count × TILE_SIZE` pixels wide and `TILE_SIZE`
tall, then cuts each cell into its own `Sprite`, copying the pixels and
rolling the sheet's bytes back to the arena (lesson 041's mark, lesson
050's pattern).

The kind-table order is the contract between two files: `assets/map.txt`
says kind 1 is `#` and solid; `assets/tiles.ppm` says kind 1 looks like
brick. The map owns *what* a cell is; the sheet owns *how it looks*. A
map with four kinds needs a sheet with four cells — the loader refuses
anything else rather than guess.

### The walk

```c++
void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
                 int x, int y)
{
    for (int cy = 0; cy < map.height; ++cy)
        for (int cx = 0; cx < map.width; ++cx) {
            int kind = map.cells[cy * map.width + cx];
            BlitSprite(fb, sheet.kinds[kind],
                       x + cx * TILE_SIZE, y + cy * TILE_SIZE);
        }
}
```

Cell `(cx, cy)` is drawn at `(x + cx × 16, y + cy × 16)` — world
position, straight from the indices. Everything about a cell landing
outside the framebuffer is the blitter's clipping: the walk computes
positions, the blit drops what falls outside. That division is why this
function can be four lines — the hard rules live in exactly one place.

It also means **a map bigger than the framebuffer is the ordinary case**,
not an edge case. The demo's world is 48 × 32 cells = 768 × 512 pixels
against a 640 × 480 frame; drawn at any origin, the frame shows the
rectangle of the world that lands inside it.

### The claim, checked

The check draws the whole map at two different origins and compares the
overlapping pixels — the same world pixels must appear at
world-position minus offset, everywhere both draws cover:

```
engine: tiles assets/tiles.ppm: 3 tiles of 16x16
engine: tilemap check: map 768x512 px over frame 640x480 — 274365 pixels compared at offset 37,25, 0 mismatches
```

274,365 pixels compared — every screen pixel where the two draws show
the same world content — and zero mismatches. The comparison is the
lesson-045 camera check at world scale: draw, move, draw, and hold the
two up against each other. The parts of the map that fall outside the
frame in the moved draw were simply dropped by the blitter's clip; no
wrap, no crash, no wrong pixel in the overlap.

### The third named phase

The map walk is 1,536 blits, and the frame record says what they cost —
the third subsystem named inside `render`, after `sprites` and `text`:

```
frame 1: update 0.000 ms, render 1.288 ms (sprites 0.001, text 0.002, tilemap 0.923), present 1.584 ms, total 2.872 ms
```

`tilemap 0.923 ms` is the honest first number for the world's cost: at
16×16 pixels a tile, the walk copies up to 768 × 512 = 393,216 pixels
per frame — a third of the frame's render, and the biggest named phase
so far. The frame-budget table (lesson 058) will carry this row, and
Part 5's profiler will look at it first: this is a full redraw of a
world every frame, and the exercise below measures exactly what the
blitter's clip does and does not save you.

## Code step

One change for this lesson: `assets/map.txt` grows from lesson 052's
room into the demo's world (48×32 cells — bigger than the frame),
`assets/tiles.ppm` is authored (three 16×16 tile cells), `src/tiles.h` /
`src/tiles.cpp` bring the sheet loader and the walk, `frame.h` /
`frame.cpp` grow the named `tilemap` phase, and `main.cpp` loads the
sheet, checks the map at two offsets, and draws the world every frame.
Its end state is tagged `lesson-053`.

```diff
diff --git a/.gitattributes b/.gitattributes
new file mode 100644
index 0000000..f7e6e0e
--- /dev/null
+++ b/.gitattributes
@@ -0,0 +1,4 @@
+# Asset files are binary data, whatever their bytes happen to look like.
+# (Lesson 053: the tile art's pixel bytes contained no NULs and git
+# cheerfully diffed it as text — a one-line fact worth writing down.)
+*.ppm binary
diff --git a/assets/map.txt b/assets/map.txt
index f142cb1..fcd9ce5 100644
--- a/assets/map.txt
+++ b/assets/map.txt
@@ -1,16 +1,36 @@
-20 12 3
+48 32 3
 . 0
 # 1
 w 0
-####################
-#..................#
-#..................#
-#....##......##....#
-#....##......##....#
-#..................#
-#..................#
-#...w....##....w...#
-#...w....##....w...#
-#..................#
-#..................#
-####################
+################################################
+#..............................................#
+#..............................................#
+#..............................................#
+#..............................................#
+#..............................................#
+#.......##..........##..........##.............#
+#.......##..........##..........##.............#
+#..............................................#
+#..............................................#
+#..............................................#
+#..............................................#
+#.......##..........##..........##.............#
+#.......##..........##..........##.............#
+#..............................................#
+#..............................................#
+#.............##........##..............##.....#
+#.............##........##..............##.....#
+#.......................##.....................#
+#.......................##.....................#
+#..............................................#
+#..............................................#
+#.......##..........##..##......##.............#
+#.......##..........##..##......##.............#
+#.......................##....wwwwwwwww........#
+#.......................##....wwwwwwwww........#
+#.......................##....wwwwwwwww........#
+#.......................##....wwwwwwwww........#
+#.............................wwwwwwwww........#
+#..............................................#
+#..............................................#
+################################################
diff --git a/assets/tiles.ppm b/assets/tiles.ppm
new file mode 100644
index 0000000..2913240
Binary files /dev/null and b/assets/tiles.ppm differ
diff --git a/src/frame.cpp b/src/frame.cpp
index 36a2827..22cb968 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -15,6 +15,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.total_sum += frame.total;
     stats.sprites_sum += frame.sprites;
     stats.text_sum += frame.text;
+    stats.tilemap_sum += frame.tilemap;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
diff --git a/src/frame.h b/src/frame.h
index 3a68029..8f42a7d 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -24,6 +24,7 @@ struct FrameRecord {
        instead of it: render stays the phase, these say where it went. */
     double sprites; /* sprite draws through the blit */
     double text;    /* lesson 051: text drawing — glyphs through the blit */
+    double tilemap; /* lesson 053: the map's walk — tiles through the blit */
 };
 
 /* The running account: every frame measured so far. */
@@ -35,6 +36,7 @@ struct FrameStats {
     double total_sum;
     double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
     double text_sum;
+    double tilemap_sum;
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
 };
diff --git a/src/main.cpp b/src/main.cpp
index b63c353..0e77458 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -16,6 +16,7 @@
 #include "sprite.h"
 #include "text.h"
 #include "tilemap.h"
+#include "tiles.h"
 
 namespace engine {
 
@@ -382,6 +383,72 @@ int Run(void)
     std::printf("engine: map check: %d cells, %d unknown, %d of 2 corners solid\n",
                 map.width * map.height, unknown, solid_corners);
 
+    /* Lesson 053: tiles are sprites — the sheet, cut per kind. */
+    const char *tiles_path = "assets/tiles.ppm";
+    TileSheetResult tiles_loaded = LoadTileSheet(arena, tiles_path,
+                                                 map.kind_count);
+    if (tiles_loaded.error != TILES_OK) {
+        switch (tiles_loaded.error) {
+        case TILES_MISSING:
+            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
+                         tiles_path);
+            break;
+        case TILES_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: %s: not one %dx%d cell per map kind\n",
+                         tiles_path, TILE_SIZE, TILE_SIZE);
+            break;
+        default:
+            std::fprintf(stderr, "engine: %s: no room in the arena\n",
+                         tiles_path);
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    TileSheet &sheet = tiles_loaded.sheet;
+    std::printf("engine: tiles %s: %d tiles of %dx%d\n", tiles_path,
+                map.kind_count, TILE_SIZE, TILE_SIZE);
+
+    /* The map bigger than the frame, drawn at two offsets: the same
+       world pixels at world-position minus offset, everywhere the two
+       draws overlap. */
+    unsigned char *snap = (unsigned char *)ArenaAlloc(
+        arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 3, 4);
+    if (!snap) {
+        std::fprintf(stderr, "engine: no room for the tilemap check\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawTileMap(*fb, map, sheet, 0, 0);
+    for (int y = 0; y < FRAME_HEIGHT; ++y)
+        for (int x = 0; x < FRAME_WIDTH; ++x) {
+            unsigned char r, g, b;
+            GetPixel(*fb, x, y, r, g, b);
+            unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
+            p[0] = r;
+            p[1] = g;
+            p[2] = b;
+        }
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawTileMap(*fb, map, sheet, -37, -25);
+    int compared = 0, moved_mismatches = 0;
+    for (int y = 25; y < FRAME_HEIGHT; ++y)
+        for (int x = 37; x < FRAME_WIDTH; ++x) {
+            unsigned char r, g, b;
+            GetPixel(*fb, x - 37, y - 25, r, g, b);
+            const unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
+            ++compared;
+            if (r != p[0] || g != p[1] || b != p[2])
+                ++moved_mismatches;
+        }
+    std::printf("engine: tilemap check: map %dx%d px over frame %dx%d — %d pixels compared at offset 37,25, %d mismatches\n",
+                map.width * TILE_SIZE, map.height * TILE_SIZE, FRAME_WIDTH,
+                FRAME_HEIGHT, compared, moved_mismatches);
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
@@ -438,9 +505,12 @@ int Run(void)
                         (int)sprite_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then the
-           sprite and the text, each timed as its own named phase: the
-           subsystems the frame record can name. */
+           map, the sprite, and the text, each timed as its own named
+           phase: the subsystems the frame record can name. */
         ClearBuffer(*fb, 32, 32, 64);
+        double t_tilemap = platform::Now();
+        DrawTileMap(*fb, map, sheet, 0, 0);
+        frame.tilemap = platform::Now() - t_tilemap;
         double t_sprites = platform::Now();
         BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
         frame.sprites = platform::Now() - t_sprites;
@@ -469,21 +539,22 @@ int Run(void)
 
         /* The frame log: one line per record — the format grows its named
            fields, one per subsystem, as the parts name them. */
-        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f), present %.3f ms, total %.3f ms\n",
+        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.update * 1e3, frame.render * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
-                    frame.present * 1e3, frame.total * 1e3);
+                    frame.tilemap * 1e3, frame.present * 1e3,
+                    frame.total * 1e3);
     }
 
     /* The account: what the frames actually cost, including the honest
        price of the presentation copy. */
     if (stats.frames) {
         double n = (double)stats.frames;
-        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, present %.3f)\n",
+        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, tilemap %.3f, present %.3f)\n",
                     stats.frames, stats.total_sum / n * 1e3,
                     stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
                     stats.sprites_sum / n * 1e3, stats.text_sum / n * 1e3,
-                    stats.present_sum / n * 1e3);
+                    stats.tilemap_sum / n * 1e3, stats.present_sum / n * 1e3);
         std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
                     stats.worst * 1e3, stats.worst_number,
                     100.0 * stats.present_sum / stats.total_sum);
diff --git a/src/tiles.cpp b/src/tiles.cpp
new file mode 100644
index 0000000..66698af
--- /dev/null
+++ b/src/tiles.cpp
@@ -0,0 +1,85 @@
+// tiles.cpp — cutting the tile sheet, and the map's walk.
+//
+// Lesson 053: the cut is lesson 050's, the walk is lesson 045's blit in
+// a loop at computed origins. Nothing here draws a pixel by any other
+// path.
+
+#include "tiles.h"
+
+#include "blit.h"
+
+namespace engine {
+
+TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
+{
+    TileSheetResult result = { { { { 0, 0, 0, 0, 0, 0 } } }, TILES_OK };
+
+    if (kind_count <= 0 || kind_count > TILE_MAX_KINDS) {
+        result.error = TILES_MALFORMED;
+        return result;
+    }
+
+    /* One allocation for every kind's tile pixels. */
+    size_t tile_bytes = (size_t)kind_count * TILE_SIZE * TILE_SIZE * 3;
+    unsigned char *pixels = (unsigned char *)ArenaAlloc(arena, tile_bytes, 4);
+    if (!pixels) {
+        result.error = TILES_NO_ROOM;
+        return result;
+    }
+
+    size_t mark = ArenaMark(arena);
+    SpriteResult sheet = LoadSprite(arena, path);
+    if (sheet.error == SPRITE_MISSING) {
+        result.error = TILES_MISSING;
+        return result;
+    }
+    if (sheet.error != SPRITE_OK) {
+        result.error = TILES_MALFORMED;
+        return result;
+    }
+
+    /* The sheet is exactly kind_count cells in one row. */
+    if (sheet.sprite.width != kind_count * TILE_SIZE ||
+        sheet.sprite.height != TILE_SIZE) {
+        ArenaRollback(arena, mark);
+        result.error = TILES_MALFORMED;
+        return result;
+    }
+
+    for (int k = 0; k < kind_count; ++k) {
+        Sprite &tile = result.sheet.kinds[k];
+        tile.pixels = pixels + (size_t)k * TILE_SIZE * TILE_SIZE * 3;
+        tile.width = TILE_SIZE;
+        tile.height = TILE_SIZE;
+        tile.key_r = sheet.sprite.key_r;
+        tile.key_g = sheet.sprite.key_g;
+        tile.key_b = sheet.sprite.key_b;
+        for (int r = 0; r < TILE_SIZE; ++r)
+            for (int c = 0; c < TILE_SIZE; ++c) {
+                const unsigned char *src =
+                    &sheet.sprite.pixels[(((size_t)r * sheet.sprite.width) +
+                                          (k * TILE_SIZE + c)) * 3];
+                unsigned char *dst =
+                    &tile.pixels[(((size_t)r * TILE_SIZE) + c) * 3];
+                dst[0] = src[0];
+                dst[1] = src[1];
+                dst[2] = src[2];
+            }
+    }
+    ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
+    result.error = TILES_OK;
+    return result;
+}
+
+void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
+                 int x, int y)
+{
+    for (int cy = 0; cy < map.height; ++cy)
+        for (int cx = 0; cx < map.width; ++cx) {
+            int kind = map.cells[cy * map.width + cx];
+            BlitSprite(fb, sheet.kinds[kind],
+                       x + cx * TILE_SIZE, y + cy * TILE_SIZE);
+        }
+}
+
+} /* namespace engine */
diff --git a/src/tiles.h b/src/tiles.h
new file mode 100644
index 0000000..21ceb95
--- /dev/null
+++ b/src/tiles.h
@@ -0,0 +1,51 @@
+// tiles.h — the tile sheet, and the walk that draws a map with it.
+//
+// Lesson 053: tiles are sprites (lesson 045's promise, second
+// installment). The sheet is a PPM image of TILE_SIZE cells in one row —
+// one cell per tile kind, in the map's kind-table order — and the map's
+// walk is a loop of blits at computed origins.
+#ifndef TILES_H
+#define TILES_H
+
+#include "framebuffer.h"
+#include "sprite.h"
+#include "tilemap.h"
+
+namespace engine {
+
+/* The format's cell size: every tile is TILE_SIZE x TILE_SIZE pixels. */
+constexpr int TILE_SIZE = 16;
+
+/* A tile sheet: one sprite per kind, cut from the sheet at load. */
+struct TileSheet {
+    Sprite kinds[TILE_MAX_KINDS];
+};
+
+/* A load either hands over the sheet or names what went wrong. */
+enum TileSheetError {
+    TILES_OK = 0,
+    TILES_MISSING,   /* the sheet is not there or cannot be read */
+    TILES_MALFORMED, /* the sheet is not exactly kind_count cells wide */
+    TILES_NO_ROOM,   /* the arena had no room for the tiles */
+};
+
+struct TileSheetResult {
+    TileSheet sheet;
+    TileSheetError error;
+};
+
+/* Loads a sheet of exactly kind_count cells of TILE_SIZE, cutting each
+   cell into its own sprite — the map's kinds, as art. */
+TileSheetResult LoadTileSheet(Arena &arena, const char *path,
+                              int kind_count);
+
+/* Draws a map with its top-left cell at world origin (x, y): one blit
+   per cell, at (x + cell_x * TILE_SIZE, y + cell_y * TILE_SIZE). The
+   blitter's clipping drops the cells that fall outside the framebuffer —
+   a map bigger than the screen is an ordinary case. */
+void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
+                 int x, int y);
+
+} /* namespace engine */
+
+#endif
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The tile under the sprite *(extend-the-code)*

The walk maps cell to world; write the inverse — world to cell. At
startup, report which cell the sprite stands on (its position divided by
the tile size) and what that cell's kind is; then draw a one-pixel
outline around that cell every frame, following the sprite as it moves.
Verify with scripted input and a pixel readback at the outline's corner.
What does this mapping give lesson 055 for free?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-053/ex1.md)

### Exercise 2 — The empty draw *(predict-the-output)*

The clip drops off-screen tiles, but the walk still visits every cell.
Measure it: time the map walk at three origins — fully on screen
`(0, 0)`, mostly off `(-400, -300)`, and entirely off
`(-4096, -4096)` — and predict the three numbers before you run. How
much of the on-screen cost is *copying* and how much is *walking*, and
what would it take to skip cells instead of clipping them?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-053/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 052 — the tilemap asset format](lesson-052-tilemap.md) ·
**Next:** [Lesson 054 — the camera](lesson-054-camera.md) ·
**Code tag:** [`lesson-053`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-053)
