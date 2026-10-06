# Lesson 057 — the closing demo: the world, drawn

{{#include ../../stability-horizon.md}}

## Prose

Thirteen lessons ago the engine could draw a sprite and nothing else.
This is what the promise looks like running: **one measured frame loop
drawing a world** — a scrolling map, a sprite that moves through it and
stops at walls, text laid out over the scene, every phase timed. Nothing
new is invented today; today the parts *fit*, and the fit is the point.
This is "software renderer done".

### The demo

```
$ DISPLAY=:99 ./build/game
engine: part 2 done — the software renderer draws the world
engine: world 48x32 cells (768x512 px), 3 kinds; 96 glyphs; sprite 16x16
engine: arrow keys move the sprite, space shakes the camera; close the window to stop
engine: sprite at 312,232
frame 1: update 0.000 ms, render 1.829 ms (sprites 0.001, text 0.007, tilemap 1.015), present 1.279 ms, total 3.109 ms
...
engine: sprite at 471,232 (t=3.668)
engine: camera base 151,0 (t=3.668)
...
engine: 27 frames — avg 2.093 ms (update 0.014, render 1.501 incl. sprites 0.001, text 0.008, tilemap 1.007, present 0.578)
engine: worst frame 2.632 ms (frame 1); present is 28% of the frame
engine: arena: 1251840 of 33554432 bytes used
engine: close reported
engine: closed
```

Read the run as a tour of the part:

- **The world** is data first (lessons 052-053): 48×32 cells of a
  hand-authored map, its three kinds, and their art — loaded whole at
  startup, drawn cell by cell through the one blit. The demo's map is
  bigger than the window, and the window shows the corner of it the
  camera points at.
- **The camera** (lesson 054) follows the sprite — `camera base 151,0`
  is the view sliding across the world — and its additive offset shakes
  the screen on the space key, always returning to zero.
- **The mover** (lesson 056) is what makes the sprite a thing that lives
  in the world rather than on the screen: intent from the polled keys,
  permission from the map's collision queries, and the `blocked` /
  `unblocked` reports whenever the wall wins.
- **The text** (lessons 050-051) draws the HUD — `SCORE` counting the
  world the sprite has walked, and its position — through the same blit
  as everything else, in screen space, over the scrolling scene.
- **The measurement** (lessons 036, 046, 051, 053) is the frame record's
  named phases, one line per frame, and the account at the end:
  `render 1.501 incl. sprites 0.001, text 0.008, tilemap 1.007` — the
  render attributed to its subsystems, exactly the data lesson 058
  prints as a table.

The readback check confirms what the demo claims. Driving the sprite
into the world's corner — where the camera's clamp pins the view at the
origin, so screen and world coordinates coincide — the reported position
is `22,21`, and the window holds the sprite's own center color at
exactly `(30, 29)`:

```
winread: 30,29 -> r=220 g=40 b=40     <- the sprite's (8,8) pixel
winread: 100,100 -> r=58 g=92 b=58    <- the floor tile's art
```

The engine is not moving variables; it is moving a thing the window
shows, at a position it reports, over a world it loaded from a file.

### What "done" means

Done is not finished — the engine has no game in it. Done is the MVD's
Part 2 obligations **delivered and demonstrable**:

| Obligation | Delivered in | Demonstrated by |
| ---------- | ------------ | --------------- |
| O1 — frame-timing instrumentation | lessons 036, 046, 051, 053 | every frame line, the account's per-subsystem attribution |
| O2 — camera offsets | lesson 054 | the base following the sprite, the additive shaking and restoring |
| O3 — bitmap text | lessons 050-051 | the HUD, built from runtime data |
| O4 — tilemap + collision | lessons 052-053, 055-056 | the world loaded from a file; the mover stopped by its walls |

and the limits are named, as they were at Part 1's close:

- **the tilemap walk is the render's cost** — 1.0 ms of the 1.5 ms
  render is the 1,536-cell redraw every frame (lesson 053's exercise
  measured what clipping saves and what it does not);
- **the mover's steps are all-or-nothing** (lesson 056's honest wart) —
  a long frame refuses its whole step;
- **no optimization landed in Part 2** — the deep dives measured and
  named the costs; the fixing is Part 5's three-pass menu.

### The shape the rest inherits

The loop is the same four moves lesson 043 settled — pump, update,
render, present — and Part 3's sound and Part 4's services live beside
it, not inside it. What Part 5 inherits is deliberately already in
place: the frame record's named subsystem phases (the profiler's raw
data and the frame-budget table's rows), one drawing path to optimize
instead of three (lesson 045's decision), asset formats that cannot
churn, and two deep dives whose measurements Part 5 re-runs before it
changes anything. The next lesson closes the part the way Part 1 closed:
with a table of what the frames actually cost.

## Code step

One change for this lesson: `main.cpp` becomes the closing demo — the
startup check blocks of lessons 044-055 retire in favor of the demo
(lesson 043's pattern: the experiments were for their lessons; the demo
is for the part), and the HUD becomes the demo's own: `SCORE` and the
sprite's position, built at runtime and drawn through `DrawText`. The
engine's libraries — sprite, blit, font, text, tilemap, tiles, camera —
are untouched. Its end state is tagged `lesson-057`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 9bfc720..bc94247 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,9 +1,11 @@
-// main.cpp — the engine: one measured frame loop.
+// main.cpp — the engine: one measured frame loop, drawing the world.
 //
-// Lesson 043: the closing demo. The platform layer does its whole job at
-// once — a window kept alive, input polled, an arena-backed framebuffer
-// presented, every frame measured — and this loop is the shape Part 2
-// draws into. The language law of lesson 026 still holds over all of it.
+// Lesson 057: the Part 2 closing demo. Every capability of the software
+// renderer at once — the map drawn through the camera, the sprite moved
+// by polled input and stopped by the map, text laid out over it all —
+// and every phase measured, one record per frame. Nothing is invented
+// here; today the parts fit, and the fit is what the demo shows. The
+// language law of lesson 026 still holds over all of it.
 
 #include <cstdio>
 
@@ -26,11 +28,6 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
-/* Lesson 050: the label the demo lays out by hand — one glyph per
-   blit, one position per glyph. Lesson 051 replaces the hand with a
-   layout loop. */
-constexpr char HUD_LABEL[] = "SCORE";
-
 /* Lesson 054: the scene, drawn through the camera. The camera's summed
    offset is applied once, at each draw's origin — the map's and the
    sprite's. The HUD is not scene and does not pass through here. */
@@ -44,111 +41,6 @@ static void DrawScene(Framebuffer &fb, const TileMap &map,
     BlitSprite(fb, sprite, sprite_x - x, sprite_y - y);
 }
 
-/* The framebuffer's whole content, copied out — the check's reference. */
-static void Snapshot(Framebuffer &fb, unsigned char *snap)
-{
-    for (int y = 0; y < FRAME_HEIGHT; ++y)
-        for (int x = 0; x < FRAME_WIDTH; ++x) {
-            unsigned char r, g, b;
-            GetPixel(fb, x, y, r, g, b);
-            unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
-            p[0] = r;
-            p[1] = g;
-            p[2] = b;
-        }
-}
-
-/* Compares the framebuffer against a snapshot shifted by (dx, dy): the
-   pixel at (x, y) now must be the snapshot's pixel at (x + dx, y + dy). */
-static void CompareShift(Framebuffer &fb, const unsigned char *snap, int dx,
-                         int dy, int &compared, int &mismatches)
-{
-    compared = 0;
-    mismatches = 0;
-    for (int y = 0; y < FRAME_HEIGHT; ++y) {
-        if (y + dy < 0 || y + dy >= FRAME_HEIGHT)
-            continue;
-        for (int x = 0; x < FRAME_WIDTH; ++x) {
-            if (x + dx < 0 || x + dx >= FRAME_WIDTH)
-                continue;
-            unsigned char r, g, b;
-            GetPixel(fb, x, y, r, g, b);
-            const unsigned char *p =
-                &snap[(((y + dy) * FRAME_WIDTH) + (x + dx)) * 3];
-            ++compared;
-            if (r != p[0] || g != p[1] || b != p[2])
-                ++mismatches;
-        }
-    }
-}
-
-/* Lesson 047: the caches deep dive's evidence — a copy walk over arena
-   memory at two strides, timed at working-set sizes that cross this
-   machine's caches. The walk is the blit's inner copy with the
-   bookkeeping removed: source bytes into destination bytes, nothing
-   else — so what it costs is what the blitter's copy costs.
-   Lesson 049: the walkers lose their `static` so the compiler must emit
-   each one as a named function — the SIMD lens needs a listing to read. */
-
-void CopySequential(unsigned char *dst, const unsigned char *src, size_t n)
-{
-    for (size_t i = 0; i < n; ++i)
-        dst[i] = src[i];
-}
-
-void CopyStrided(unsigned char *dst, const unsigned char *src, size_t n,
-                 size_t stride)
-{
-    for (size_t i = 0; i < n; i += stride)
-        dst[i] = src[i];
-}
-
-static void CacheProbe(Arena &arena)
-{
-    const size_t sizes[] = { 4096, 65536, 524288, 4194304, 8388608,
-                             12582912 };
-    const size_t biggest = 12582912;
-
-    /* Two allocations the compiler cannot connect: source and destination
-       are separate arena blocks — 2 × size bytes of working set per walk,
-       and the copy loop is free to run wide. */
-    unsigned char *src = (unsigned char *)ArenaAlloc(arena, biggest, 64);
-    unsigned char *dst = (unsigned char *)ArenaAlloc(arena, biggest, 64);
-    if (!src || !dst) {
-        std::printf("engine: cache probe: no room in the arena\n");
-        return;
-    }
-    for (size_t i = 0; i < biggest; i += 4096) {
-        src[i] = (unsigned char)(i * 7); /* touch every page first */
-        dst[i] = 0;
-    }
-
-    std::printf("engine: cache probe — copy walk, useful GB/s per stride\n");
-    std::printf("engine: %10s %12s %12s\n", "working set", "sequential",
-                "stride 64");
-    for (unsigned s = 0; s < sizeof sizes / sizeof sizes[0]; ++s) {
-        double gbs[2];
-        for (unsigned mode = 0; mode < 2; ++mode) {
-            size_t stride = mode == 0 ? 1 : 64;
-            size_t per_rep = sizes[s] / stride;
-            long reps = (long)(8000000 / per_rep);
-            if (reps < 1)
-                reps = 1;
-            double t0 = platform::Now();
-            for (long r = 0; r < reps; ++r) {
-                if (mode == 0)
-                    CopySequential(dst, src, sizes[s]);
-                else
-                    CopyStrided(dst, src, sizes[s], 64);
-            }
-            double seconds = platform::Now() - t0;
-            gbs[mode] = (double)per_rep * reps / seconds / 1e9;
-        }
-        std::printf("engine: %7zu KB %12.1f %12.1f\n", sizes[s] / 1024,
-                    gbs[0], gbs[1]);
-    }
-}
-
 int Run(void)
 {
     platform::WindowResult opened =
@@ -172,430 +64,65 @@ int Run(void)
     }
 
     /* The engine's memory: one arena over one reservation. Everything the
-       engine allocates lives in here and is released together. Lesson 047
-       grows it past every cache this machine has, so the cache probe can
-       walk working sets bigger than all of them. */
+       engine allocates lives in here and is released together. */
     Arena arena;
     ArenaInit(arena, 32 * 1024 * 1024);
     Framebuffer *fb = GetFramebuffer(arena);
 
-    /* Lesson 044: the sprite is a file's bytes. It is loaded once, at
-       startup, through the seam's whole-file read into the arena — and
-       then inspected like Part 0 inspected everything: by byte. */
-    const char *sprite_path = "assets/sprite.ppm";
-    SpriteResult loaded = LoadSprite(arena, sprite_path);
+    /* The world's assets, loaded whole at startup (lessons 044-053):
+       a sprite, a font, a map, and the map's tile art. Every load is a
+       typed failure or a complete asset — and a failure ends the run by
+       name. */
+    SpriteResult loaded = LoadSprite(arena, "assets/sprite.ppm");
     if (loaded.error != SPRITE_OK) {
-        switch (loaded.error) {
-        case SPRITE_MISSING:
-            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
-                         sprite_path);
-            break;
-        case SPRITE_MALFORMED:
-            std::fprintf(stderr,
-                         "engine: %s: not a complete P6 image\n",
-                         sprite_path);
-            break;
-        default:
-            std::fprintf(stderr, "engine: %s: no room in the arena\n",
-                         sprite_path);
-            break;
-        }
+        std::fprintf(stderr, "engine: assets/sprite.ppm: could not load\n");
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
     Sprite &sprite = loaded.sprite;
-    long pixel_bytes = (long)sprite.width * sprite.height * 3;
-    long byte_sum = 0;
-    for (long i = 0; i < pixel_bytes; ++i)
-        byte_sum += sprite.pixels[i];
 
-    std::printf("engine: sprite %s: %dx%d, %ld pixel bytes\n", sprite_path,
-                sprite.width, sprite.height, pixel_bytes);
-    std::printf("engine: pixel 0,0 = %d,%d,%d\n", sprite.pixels[0],
-                sprite.pixels[1], sprite.pixels[2]);
-    std::printf("engine: pixel %d,%d = %d,%d,%d\n", sprite.width / 2,
-                sprite.height / 2,
-                sprite.pixels[(sprite.height / 2 * sprite.width +
-                               sprite.width / 2) * 3 + 0],
-                sprite.pixels[(sprite.height / 2 * sprite.width +
-                               sprite.width / 2) * 3 + 1],
-                sprite.pixels[(sprite.height / 2 * sprite.width +
-                               sprite.width / 2) * 3 + 2]);
-    std::printf("engine: pixel bytes sum to %ld\n", byte_sum);
-
-    /* Lesson 045: the blitter's three claims, checked against the
-       framebuffer's own bytes before anything depends on them. */
-    ClearBuffer(*fb, 32, 32, 64);
-    BlitSprite(*fb, sprite, 100, 100);
-    int opaque = 0, key_pixels = 0, mismatches = 0;
-    for (int j = 0; j < sprite.height; ++j)
-        for (int i = 0; i < sprite.width; ++i) {
-            const unsigned char *p =
-                &sprite.pixels[(j * sprite.width + i) * 3];
-            unsigned char r, g, b;
-            GetPixel(*fb, 100 + i, 100 + j, r, g, b);
-            bool is_key = p[0] == sprite.key_r && p[1] == sprite.key_g &&
-                          p[2] == sprite.key_b;
-            if (is_key) {
-                ++key_pixels;
-                if (r != 32 || g != 32 || b != 64)
-                    ++mismatches; /* the key must have written nothing */
-            } else {
-                ++opaque;
-                if (r != p[0] || g != p[1] || b != p[2])
-                    ++mismatches;
-            }
-        }
-    std::printf("engine: blit check: %d opaque pixels drawn unchanged, %d mismatches\n",
-                opaque, mismatches);
-    std::printf("engine: blit check: %d key pixels wrote nothing over the background\n",
-                key_pixels);
-
-    ClearBuffer(*fb, 32, 32, 64);
-    BlitSprite(*fb, sprite, -4, -4);
-    int landed = 0, wrong = 0, wrapped = 0;
-    for (int j = 0; j < sprite.height; ++j)
-        for (int i = 0; i < sprite.width; ++i) {
-            if (i < 4 || j < 4)
-                continue; /* these pixels landed outside and were dropped */
-            const unsigned char *p =
-                &sprite.pixels[(j * sprite.width + i) * 3];
-            unsigned char r, g, b;
-            GetPixel(*fb, i - 4, j - 4, r, g, b);
-            ++landed;
-            bool is_key = p[0] == sprite.key_r && p[1] == sprite.key_g &&
-                          p[2] == sprite.key_b;
-            if (is_key ? (r != 32 || g != 32 || b != 64)
-                       : (r != p[0] || g != p[1] || b != p[2]))
-                ++wrong;
-        }
-    for (int y = 0; y < FRAME_HEIGHT; ++y)
-        for (int x = 0; x < FRAME_WIDTH; ++x) {
-            if (x < sprite.width - 4 && y < sprite.height - 4)
-                continue; /* the landed region, checked above */
-            unsigned char r, g, b;
-            GetPixel(*fb, x, y, r, g, b);
-            if (r != 32 || g != 32 || b != 64)
-                ++wrapped;
-        }
-    std::printf("engine: blit check: clip at -4,-4 landed %d pixels, %d wrong, %d touched outside\n",
-                landed, wrong, wrapped);
-
-    /* Lesson 047's evidence, measured before the loop starts. */
-    CacheProbe(arena);
-
-    /* Lesson 050: the font is an asset too — a glyph sheet the loader
-       cuts into sprites. */
-    const char *font_path = "assets/font.ppm";
-    FontResult font_loaded = LoadFont(arena, font_path);
+    FontResult font_loaded = LoadFont(arena, "assets/font.ppm");
     if (font_loaded.error != FONT_OK) {
-        switch (font_loaded.error) {
-        case FONT_MISSING:
-            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
-                         font_path);
-            break;
-        case FONT_MALFORMED:
-            std::fprintf(stderr,
-                         "engine: %s: not a 16x6 sheet of 8x8 glyphs\n",
-                         font_path);
-            break;
-        default:
-            std::fprintf(stderr, "engine: %s: no room in the arena\n",
-                         font_path);
-            break;
-        }
+        std::fprintf(stderr, "engine: assets/font.ppm: could not load\n");
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
     Font &font = font_loaded.font;
-    std::printf("engine: font %s: %d glyphs of %dx%d from a %dx%d sheet\n",
-                font_path, FONT_COUNT, FONT_CELL, FONT_CELL,
-                FONT_COLS * FONT_CELL, FONT_ROWS * FONT_CELL);
 
-    /* The glyph claim, checked against the framebuffer's bytes: a glyph
-       drawn through the blit is the sheet's cell, pixel for pixel. */
-    const char checked[2] = { 'A', 'g' };
-    for (int gi = 0; gi < 2; ++gi) {
-        const Sprite *glyph = FontGlyph(font, checked[gi]);
-        ClearBuffer(*fb, 32, 32, 64);
-        BlitSprite(*fb, *glyph, 200 + gi * 16, 64);
-        int ink = 0, key = 0, mismatches = 0;
-        for (int j = 0; j < glyph->height; ++j)
-            for (int i = 0; i < glyph->width; ++i) {
-                const unsigned char *p = &glyph->pixels[(j * glyph->width + i) * 3];
-                unsigned char r, g, b;
-                GetPixel(*fb, 200 + gi * 16 + i, 64 + j, r, g, b);
-                bool is_key = p[0] == glyph->key_r && p[1] == glyph->key_g &&
-                              p[2] == glyph->key_b;
-                if (is_key) {
-                    ++key;
-                    if (r != 32 || g != 32 || b != 64)
-                        ++mismatches;
-                } else {
-                    ++ink;
-                    if (r != p[0] || g != p[1] || b != p[2])
-                        ++mismatches;
-                }
-            }
-        std::printf("engine: font check: glyph '%c' — %d pixels read back, %d mismatches (%d ink, %d key)\n",
-                    checked[gi], glyph->width * glyph->height, mismatches,
-                    ink, key);
-    }
-
-    /* Lesson 051: the text claims — glyphs laid out in order, and the
-       missing-glyph rule: the character the font lacks draws nothing and
-       the characters after it keep their slots. */
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawText(*fb, font, "AB", 200, 120);
-    {
-        int slots_with_ink = 0;
-        for (int s = 0; s < 2; ++s) {
-            bool ink = false;
-            for (int j = 0; j < FONT_CELL && !ink; ++j)
-                for (int i = 0; i < FONT_CELL && !ink; ++i) {
-                    unsigned char r, g, b;
-                    GetPixel(*fb, 200 + s * FONT_CELL + i, 120 + j, r, g, b);
-                    if (r != 32 || g != 32 || b != 64)
-                        ink = true;
-                }
-            if (ink)
-                ++slots_with_ink;
-        }
-        std::printf("engine: text check: \"AB\" at 200,120 — %d of 2 slots have ink, width %d\n",
-                    slots_with_ink, TextWidth("AB"));
-    }
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawText(*fb, font, "A\xC2\xB5" "B", 200, 140); /* "AµB": µ is not in the sheet */
-    {
-        /* Four bytes, four slots: the µ is two bytes, and each keeps its
-           slot — the layout follows the bytes, and B lands where the
-           layout says. */
-        int slot_state[4] = { 0, 0, 0, 0 };
-        for (int s = 0; s < 4; ++s) {
-            for (int j = 0; j < FONT_CELL; ++j)
-                for (int i = 0; i < FONT_CELL; ++i) {
-                    unsigned char r, g, b;
-                    GetPixel(*fb, 200 + s * FONT_CELL + i, 140 + j, r, g, b);
-                    if (r != 32 || g != 32 || b != 64)
-                        ++slot_state[s];
-                }
-        }
-        std::printf("engine: text check: \"A?B\" with a missing character — ink pixels per slot: %d, %d, %d, %d\n",
-                    slot_state[0], slot_state[1], slot_state[2], slot_state[3]);
-    }
-
-    /* Lesson 052: the world as data — the map file, loaded whole, its
-       cells counted against the file's own rows. */
-    const char *map_path = "assets/map.txt";
-    TileResult map_loaded = LoadTileMap(arena, map_path);
+    TileResult map_loaded = LoadTileMap(arena, "assets/map.txt");
     if (map_loaded.error != TILE_OK) {
-        switch (map_loaded.error) {
-        case TILE_MISSING:
-            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
-                         map_path);
-            break;
-        case TILE_MALFORMED:
-            std::fprintf(stderr,
-                         "engine: %s: not a complete map\n", map_path);
-            break;
-        default:
-            std::fprintf(stderr, "engine: %s: no room in the arena\n",
-                         map_path);
-            break;
-        }
+        std::fprintf(stderr, "engine: assets/map.txt: could not load\n");
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
     TileMap &map = map_loaded.map;
-    std::printf("engine: map %s: %dx%d, %d kinds\n", map_path, map.width,
-                map.height, map.kind_count);
-    for (int k = 0; k < map.kind_count; ++k) {
-        int count = 0;
-        for (int y = 0; y < map.height; ++y)
-            for (int x = 0; x < map.width; ++x)
-                if (TileAt(map, x, y) == k)
-                    ++count;
-        std::printf("engine: map kind '%c' (solid %d): %d cells\n",
-                    map.kinds[k].cell, map.kinds[k].solid, count);
-    }
-    int unknown = 0, solid_corners = 0;
-    for (int y = 0; y < map.height; ++y)
-        for (int x = 0; x < map.width; ++x) {
-            int kind = TileAt(map, x, y);
-            if (kind < 0 || kind >= map.kind_count)
-                ++unknown;
-        }
-    if (map.kinds[TileAt(map, 0, 0)].solid)
-        ++solid_corners;
-    if (map.kinds[TileAt(map, map.width - 1, map.height - 1)].solid)
-        ++solid_corners;
-    std::printf("engine: map check: %d cells, %d unknown, %d of 2 corners solid\n",
-                map.width * map.height, unknown, solid_corners);
 
-    /* Lesson 053: tiles are sprites — the sheet, cut per kind. */
-    const char *tiles_path = "assets/tiles.ppm";
-    TileSheetResult tiles_loaded = LoadTileSheet(arena, tiles_path,
+    TileSheetResult tiles_loaded = LoadTileSheet(arena, "assets/tiles.ppm",
                                                  map.kind_count);
     if (tiles_loaded.error != TILES_OK) {
-        switch (tiles_loaded.error) {
-        case TILES_MISSING:
-            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
-                         tiles_path);
-            break;
-        case TILES_MALFORMED:
-            std::fprintf(stderr,
-                         "engine: %s: not one %dx%d cell per map kind\n",
-                         tiles_path, TILE_SIZE, TILE_SIZE);
-            break;
-        default:
-            std::fprintf(stderr, "engine: %s: no room in the arena\n",
-                         tiles_path);
-            break;
-        }
+        std::fprintf(stderr, "engine: assets/tiles.ppm: could not load\n");
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
     TileSheet &sheet = tiles_loaded.sheet;
-    std::printf("engine: tiles %s: %d tiles of %dx%d\n", tiles_path,
-                map.kind_count, TILE_SIZE, TILE_SIZE);
 
-    /* The map bigger than the frame, drawn at two offsets: the same
-       world pixels at world-position minus offset, everywhere the two
-       draws overlap. */
-    unsigned char *snap = (unsigned char *)ArenaAlloc(
-        arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 3, 4);
-    if (!snap) {
-        std::fprintf(stderr, "engine: no room for the tilemap check\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawTileMap(*fb, map, sheet, 0, 0);
-    for (int y = 0; y < FRAME_HEIGHT; ++y)
-        for (int x = 0; x < FRAME_WIDTH; ++x) {
-            unsigned char r, g, b;
-            GetPixel(*fb, x, y, r, g, b);
-            unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
-            p[0] = r;
-            p[1] = g;
-            p[2] = b;
-        }
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawTileMap(*fb, map, sheet, -37, -25);
-    int compared = 0, moved_mismatches = 0;
-    for (int y = 25; y < FRAME_HEIGHT; ++y)
-        for (int x = 37; x < FRAME_WIDTH; ++x) {
-            unsigned char r, g, b;
-            GetPixel(*fb, x - 37, y - 25, r, g, b);
-            const unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
-            ++compared;
-            if (r != p[0] || g != p[1] || b != p[2])
-                ++moved_mismatches;
-        }
-    std::printf("engine: tilemap check: map %dx%d px over frame %dx%d — %d pixels compared at offset 37,25, %d mismatches\n",
-                map.width * TILE_SIZE, map.height * TILE_SIZE, FRAME_WIDTH,
-                FRAME_HEIGHT, compared, moved_mismatches);
-
-    /* Lesson 054: the camera's three claims, each checked against the
-       framebuffer's pixels: the base scrolls the scene, the additive
-       offset stacks over it, and clearing the additive restores the
-       base view exactly. */
-    unsigned char *snap2 = (unsigned char *)ArenaAlloc(
-        arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 3, 4);
-    Camera camera = { 0, 0, 0, 0 };
-    if (!snap2) {
-        std::fprintf(stderr, "engine: no room for the camera check\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
-    Snapshot(*fb, snap);
-
-    camera.base_x = 100;
-    camera.base_y = 50;
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
-    int cam_cmp = 0, cam_bad = 0;
-    CompareShift(*fb, snap, 100, 50, cam_cmp, cam_bad);
-    std::printf("engine: camera check: base (100,50) scrolls the scene — %d pixels compared, %d mismatches\n",
-                cam_cmp, cam_bad);
-    Snapshot(*fb, snap2); /* the base view, for the restore check below */
-
-    camera.add_x = 7;
-    camera.add_y = -3;
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawScene(*fb, map, sheet, sprite, 200, 150, camera); /* base + add */
-    Snapshot(*fb, snap);
-    Camera summed = { 107, 47, 0, 0 }; /* the same sum, written out */
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawScene(*fb, map, sheet, sprite, 200, 150, summed);
-    CompareShift(*fb, snap, 0, 0, cam_cmp, cam_bad);
-    std::printf("engine: camera check: additive (7,-3) stacks over base — %d pixels compared, %d mismatches\n",
-                cam_cmp, cam_bad);
-
-    camera.add_x = 0;
-    camera.add_y = 0;
-    ClearBuffer(*fb, 32, 32, 64);
-    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
-    CompareShift(*fb, snap2, 0, 0, cam_cmp, cam_bad);
-    std::printf("engine: camera check: additive cleared restores the base view — %d pixels compared, %d mismatches\n",
-                cam_cmp, cam_bad);
-
-    /* Lesson 055: the collision queries — every documented answer
-       checked against the map's own data, out-of-bounds included. */
-    struct CollisionCase {
-        const char *what;
-        bool point; /* true: a point query; false: a rectangle */
-        int x, y, w, h;
-        bool expected;
-    };
-    const CollisionCase cases[] = {
-        { "point over floor", true, 256, 224, 0, 0, false },
-        { "point in the border wall", true, 8, 8, 0, 0, true },
-        { "point in a pillar", true, 128, 96, 0, 0, true },
-        { "point in the water", true, 528, 416, 0, 0, false },
-        { "point left of the map", true, -1, 100, 0, 0, true },
-        { "point past the right edge", true, 768, 100, 0, 0, true },
-        { "point below the map", true, 100, 512, 0, 0, true },
-        { "rect over floor", false, 240, 216, 32, 32, false },
-        { "rect reaching a pillar", false, 120, 88, 32, 32, true },
-        { "rect leaving the map", false, -8, 100, 16, 16, true },
-        { "rect past the right edge", false, 760, 100, 16, 16, true },
-        { "empty rect", false, 100, 100, 0, 0, false },
-    };
-    int passed = 0;
-    for (unsigned ci = 0; ci < sizeof cases / sizeof cases[0]; ++ci) {
-        const CollisionCase &c = cases[ci];
-        bool answer = c.point ? TilePointSolid(map, c.x, c.y)
-                              : TileRectSolid(map, c.x, c.y, c.w, c.h);
-        if (answer == c.expected) {
-            ++passed;
-        } else {
-            std::printf("engine: collision check: %s — expected %s, got %s\n",
-                        c.what, c.expected ? "solid" : "free",
-                        answer ? "solid" : "free");
-        }
-    }
-    std::printf("engine: collision check: %d of %d answers as documented (out-of-bounds is solid)\n",
-                passed, (int)(sizeof cases / sizeof cases[0]));
-
-    double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
-    double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
+    double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
     double last = started;
+    double distance = 0.0; /* the score: the world the sprite has walked */
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
     bool was_blocked = false; /* lesson 056: the mover's state report */
 
-    std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
+    std::printf("engine: part 2 done — the software renderer draws the world\n");
+    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; sprite %dx%d\n",
+                map.width, map.height, map.width * TILE_SIZE,
+                map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
+                sprite.width, sprite.height);
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
-    std::printf("engine: the sprite stops at walls (lesson 056's mover)\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
     /* The frame step: read news, update from polled state, draw, present —
@@ -603,6 +130,7 @@ int Run(void)
     int exit_code = 0;
     long frame_number = 0;
     FrameStats stats = {};
+    Camera camera = { 0, 0, 0, 0 };
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -617,7 +145,6 @@ int Run(void)
         double dt = now - last;
         last = now;
 
-        int old_x = (int)sprite_x, old_y = (int)sprite_y;
         double was_x = sprite_x, was_y = sprite_y;
         double move_x = 0.0, move_y = 0.0;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
@@ -631,9 +158,7 @@ int Run(void)
 
         /* Lesson 056: the mover — intent becomes motion only where the
            map allows it. One axis at a time, so a wall blocks the
-           movement into it and the movement along it still works. The
-           clamp of lesson 054 retires: the world's edge is solid, and
-           the queries are the boundary now. */
+           movement into it and the movement along it still works. */
         double next_x = sprite_x + move_x;
         if (!TileRectSolid(map, (int)next_x, (int)sprite_y, sprite.width,
                            sprite.height))
@@ -642,10 +167,11 @@ int Run(void)
         if (!TileRectSolid(map, (int)sprite_x, (int)next_y, sprite.width,
                            sprite.height))
             sprite_y = next_y;
+        distance += (sprite_x > was_x ? sprite_x - was_x : was_x - sprite_x) +
+                    (sprite_y > was_y ? sprite_y - was_y : was_y - sprite_y);
 
         /* The mover reports its state on transitions: moving, or pushed
-           against something that will not move. The comparison is on the
-           exact positions — a sub-pixel step is movement, not a wall. */
+           against something that will not move. */
         bool blocked = (move_x != 0.0 || move_y != 0.0) &&
                        sprite_x == was_x && sprite_y == was_y;
         if (blocked != was_blocked) {
@@ -654,6 +180,9 @@ int Run(void)
                         (int)sprite_y, platform::Now() - started);
             was_blocked = blocked;
         }
+        if ((int)sprite_x != (int)was_x || (int)sprite_y != (int)was_y)
+            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
+                        (int)sprite_y, platform::Now() - started);
 
         /* Lesson 054: the camera's base follows the sprite — the world
            scrolls under the movement — clamped to the map's bounds. */
@@ -696,25 +225,26 @@ int Run(void)
         frame.update = platform::Now() - t0;
         double t1 = platform::Now();
 
-        if ((int)sprite_x != old_x || (int)sprite_y != old_y)
-            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
-                        (int)sprite_y, platform::Now() - started);
-
-        /* Render: every frame draws the whole scene — clear, then the
-           world through the camera, then the HUD — each timed as its own
-           named phase: the subsystems the frame record can name. The
-           camera's summed offset is applied once, at each draw's origin. */
-        int cam_x = CameraX(camera);
-        int cam_y = CameraY(camera);
+        /* Render: every frame draws the whole scene — clear, the world
+           through the camera, and the HUD over it — each timed as its own
+           named phase: the subsystems the frame record can name. */
         ClearBuffer(*fb, 32, 32, 64);
         double t_tilemap = platform::Now();
-        DrawTileMap(*fb, map, sheet, -cam_x, -cam_y);
+        DrawTileMap(*fb, map, sheet, -CameraX(camera), -CameraY(camera));
         frame.tilemap = platform::Now() - t_tilemap;
         double t_sprites = platform::Now();
-        BlitSprite(*fb, sprite, (int)sprite_x - cam_x, (int)sprite_y - cam_y);
+        BlitSprite(*fb, sprite, (int)sprite_x - CameraX(camera),
+                   (int)sprite_y - CameraY(camera));
         frame.sprites = platform::Now() - t_sprites;
         double t_text = platform::Now();
-        DrawText(*fb, font, HUD_LABEL, 8, 8);
+        char score_line[32];
+        std::snprintf(score_line, sizeof score_line, "SCORE %06d",
+                      (int)distance);
+        DrawText(*fb, font, score_line, 8, 8);
+        char pos_line[32];
+        std::snprintf(pos_line, sizeof pos_line, "X %3d Y %3d",
+                      (int)sprite_x, (int)sprite_y);
+        DrawText(*fb, font, pos_line, 8, 8 + FONT_CELL + 4);
         frame.text = platform::Now() - t_text;
 
         frame.render = platform::Now() - t1;
@@ -745,8 +275,8 @@ int Run(void)
                     frame.total * 1e3);
     }
 
-    /* The account: what the frames actually cost, including the honest
-       price of the presentation copy. */
+    /* The account: what the frames actually cost, subsystem by subsystem —
+       the frame-budget table's first data (lesson 058 prints the table). */
     if (stats.frames) {
         double n = (double)stats.frames;
         std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, tilemap %.3f, present %.3f)\n",
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The Part 2 demo on your machine *(port-to-your-own-machine)*

The book's numbers came from Xvfb on the authoring machine; yours should
come from your desktop. Grow the account with one line — how long the
run measured and how many frames per second that gave — then run the
demo with your own hands and compare shapes with the book's: which
subsystem dominates your render? where does your present sit as a share
of the frame? what does the tilemap row do when you resize the world?
Record the machine with the numbers; a measurement without its machine
is a rumor.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-057/ex1.md)

### Exercise 2 — The acceptance table *(explain-in-prose)*

"Software renderer done" is a claim, and a claim wants a table. Make the
demo name the capabilities it uses — one line per group: drawing, text,
world, input, measurement — then fill the acceptance table: for every
scenario in the software-rendering and tilemap specs, the lesson that
built it and the *evidence* from your own run (the command and what it
printed). Finish with the question that makes the table worth keeping:
which rows break first when the engine changes, and how would you
notice?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-057/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 056 — the mover that stops at walls](lesson-056-mover.md) ·
**Next:** [Lesson 058 — the frame-budget table](lesson-058-budget.md) ·
**Code tag:** [`lesson-057`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-057)
