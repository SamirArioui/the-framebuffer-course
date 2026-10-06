# Lesson 055 — tile kinds and solidity

{{#include ../../stability-horizon.md}}

## Prose

The world knows what it is made of — lesson 052's kind table has carried
solidity since the format was defined. This lesson makes the map *answer
questions* with it: **does this point overlap a wall? does this rectangle
collide with anything solid?** The MVD's fourth obligation completes
here: tile collision, as pure queries against loaded data — no pixels,
no screen, no frame loop. The spec's purpose statement says why that
matters: worlds must be testable without a window, and a collision
system that can only be checked by walking a character into a wall is a
system you cannot test at all.

### The policy first

Every query meets coordinates outside the map eventually, and "what
happens there" is a decision, not an accident. This engine's answer,
documented in the code and the lesson: **outside the map counts as
solid** — the world's edge blocks like a wall. The hero never leaves the
world because every query that pokes past the edge says *collision*.

The spec's requirement is not *which* answer but *that* there is one:
positions outside the map have a defined answer and the engine never
reads outside the map's data. Exercise 1 takes the opposite policy for a
walk — both are legitimate; the sin is not choosing.

### The queries

Three functions, all answering from the map's data alone:

```c++
bool TileSolid(const TileMap &map, int x, int y);        /* one cell */
bool TilePointSolid(const TileMap &map, int world_x, int world_y);
bool TileRectSolid(const TileMap &map, int x, int y, int w, int h);
```

- **`TileSolid`** is the primitive: the cell's kind, the kind's `solid`
  flag, one `!= 0`. Cells outside the map answer solid (the policy).
- **`TilePointSolid`** is the world-coordinate wrapper: bounds-check the
  point against the map's pixel size, then divide by `TILE_SIZE` — the
  world-to-cell mapping lesson 053's exercise wrote — and ask the cell.
  The bounds check comes first so a point at `(-1, 100)` is answered by
  the policy and not by a division that would happily call it cell 0.
- **`TileRectSolid`** covers the cells the rectangle spans: from
  `(x, y)` to `(x + w − 1, y + h − 1)` in pixels, converted to cells
  `[x/TILE_SIZE .. (x+w−1)/TILE_SIZE]`. The `− 1` is the rectangle's
  right/bottom edge being *exclusive*: a rectangle at `(16, 0, 16, 16)`
  covers pixels 16..31 and not one more — cell 1, not cell 0. Get that
  wrong and rectangles report collisions with walls they never touch.

The queries are *pure functions of the map and the coordinates*: the
same question gets the same answer every time, on any machine, with no
window involved. That is what makes them testable — and the check block
tests them like data.

### The answers, checked

Twelve cases, each with the answer the documentation promises, run at
startup and compared:

```
engine: collision check: 12 of 12 answers as documented (out-of-bounds is solid)
```

The table behind that line (each row's expected answer written *before*
the run):

| Query | Position | Answer |
| ----- | -------- | ------ |
| point over floor | (256, 224) | free |
| point in the border wall | (8, 8) | solid |
| point in a pillar | (128, 96) | solid |
| point in the water | (528, 416) | free — water is drawn but walkable |
| point left of the map | (−1, 100) | solid — the policy |
| point past the right edge | (768, 100) | solid — the policy |
| point below the map | (100, 512) | solid — the policy |
| rect over floor | (240, 216, 32, 32) | free |
| rect reaching a pillar | (120, 88, 32, 32) | solid |
| rect leaving the map | (−8, 100, 16, 16) | solid — the policy |
| rect past the right edge | (760, 100, 16, 16) | solid — the policy |
| empty rect | (100, 100, 0, 0) | free — nothing overlaps nothing |

The floor/water/pillar rows are the spec's solidity scenario in person:
three kinds, two solidnesses, and the queries distinguish cells **by the
kinds' solidity alone** — the drawing code is not consulted anywhere in
`tilemap.cpp`. Rename the wall's art, recolor the water, draw the map
upside down: the collision answers do not change, because they were
never about the pictures.

### What the queries are for

Lesson 056 builds the mover: a sprite with intent (the arrow keys) that
asks `TileRectSolid` before it moves and stops when the answer is
*solid*. The mover is the first piece of the game proper — the hero's
precursor — and every rule it obeys is one of this lesson's twelve
answers. Part 5 turns the same queries into knockback, into enemies
that path around pillars, into the capstone's collision resolution; the
data has been ready since lesson 052, and the questions have been
answerable since right now.

## Code step

One change for this lesson: `tilemap.h` / `tilemap.cpp` grow the
collision queries (`TileSolid`, `TilePointSolid`, `TileRectSolid` —
world coordinates in, the kinds' solidity out, the out-of-bounds policy
defined and documented), `TILE_SIZE` moves to the map's header where the
world geometry lives, and `main.cpp` checks twelve answers against the
map's own data. Nothing about drawing changes. Its end state is tagged
`lesson-055`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 3c0c5bf..180c505 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -548,6 +548,44 @@ int Run(void)
     std::printf("engine: camera check: additive cleared restores the base view — %d pixels compared, %d mismatches\n",
                 cam_cmp, cam_bad);
 
+    /* Lesson 055: the collision queries — every documented answer
+       checked against the map's own data, out-of-bounds included. */
+    struct CollisionCase {
+        const char *what;
+        bool point; /* true: a point query; false: a rectangle */
+        int x, y, w, h;
+        bool expected;
+    };
+    const CollisionCase cases[] = {
+        { "point over floor", true, 256, 224, 0, 0, false },
+        { "point in the border wall", true, 8, 8, 0, 0, true },
+        { "point in a pillar", true, 128, 96, 0, 0, true },
+        { "point in the water", true, 528, 416, 0, 0, false },
+        { "point left of the map", true, -1, 100, 0, 0, true },
+        { "point past the right edge", true, 768, 100, 0, 0, true },
+        { "point below the map", true, 100, 512, 0, 0, true },
+        { "rect over floor", false, 240, 216, 32, 32, false },
+        { "rect reaching a pillar", false, 120, 88, 32, 32, true },
+        { "rect leaving the map", false, -8, 100, 16, 16, true },
+        { "rect past the right edge", false, 760, 100, 16, 16, true },
+        { "empty rect", false, 100, 100, 0, 0, false },
+    };
+    int passed = 0;
+    for (unsigned ci = 0; ci < sizeof cases / sizeof cases[0]; ++ci) {
+        const CollisionCase &c = cases[ci];
+        bool answer = c.point ? TilePointSolid(map, c.x, c.y)
+                              : TileRectSolid(map, c.x, c.y, c.w, c.h);
+        if (answer == c.expected) {
+            ++passed;
+        } else {
+            std::printf("engine: collision check: %s — expected %s, got %s\n",
+                        c.what, c.expected ? "solid" : "free",
+                        answer ? "solid" : "free");
+        }
+    }
+    std::printf("engine: collision check: %d of %d answers as documented (out-of-bounds is solid)\n",
+                passed, (int)(sizeof cases / sizeof cases[0]));
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
diff --git a/src/tilemap.cpp b/src/tilemap.cpp
index fadf9fe..b65ec21 100644
--- a/src/tilemap.cpp
+++ b/src/tilemap.cpp
@@ -161,4 +161,44 @@ int TileAt(const TileMap &map, int x, int y)
     return map.cells[y * map.width + x];
 }
 
+bool TileSolid(const TileMap &map, int x, int y)
+{
+    int kind = TileAt(map, x, y);
+    if (kind < 0)
+        return true; /* outside the map: the edge blocks like a wall */
+    return map.kinds[kind].solid != 0;
+}
+
+bool TilePointSolid(const TileMap &map, int world_x, int world_y)
+{
+    if (world_x < 0 || world_y < 0 ||
+        world_x >= map.width * TILE_SIZE ||
+        world_y >= map.height * TILE_SIZE)
+        return true; /* out of bounds: the policy's answer, no cell read */
+    return TileSolid(map, world_x / TILE_SIZE, world_y / TILE_SIZE);
+}
+
+bool TileRectSolid(const TileMap &map, int x, int y, int w, int h)
+{
+    if (w <= 0 || h <= 0)
+        return false; /* an empty rectangle overlaps nothing */
+
+    /* The map's edge is solid: a rectangle that leaves the map answers
+       without reading a single cell. */
+    if (x < 0 || y < 0 || x + w > map.width * TILE_SIZE ||
+        y + h > map.height * TILE_SIZE)
+        return true;
+
+    /* Otherwise only the cells the rectangle covers can say yes. */
+    int cx0 = x / TILE_SIZE;
+    int cy0 = y / TILE_SIZE;
+    int cx1 = (x + w - 1) / TILE_SIZE;
+    int cy1 = (y + h - 1) / TILE_SIZE;
+    for (int cy = cy0; cy <= cy1; ++cy)
+        for (int cx = cx0; cx <= cx1; ++cx)
+            if (map.kinds[map.cells[cy * map.width + cx]].solid)
+                return true;
+    return false;
+}
+
 } /* namespace engine */
diff --git a/src/tilemap.h b/src/tilemap.h
index 1c076cf..d34fa8d 100644
--- a/src/tilemap.h
+++ b/src/tilemap.h
@@ -22,6 +22,11 @@ namespace engine {
 constexpr int TILE_MAX_DIM = 256;
 constexpr int TILE_MAX_KINDS = 8;
 
+/* The cell's size in pixels: the geometry every world coordinate walks
+   on. Lesson 053 draws cells at this size; lesson 055's queries read
+   world positions through it. */
+constexpr int TILE_SIZE = 16;
+
 /* One tile kind: the character that names it in the file, and whether it
    is solid for collision (lesson 055 reads this; the format carries it
    from the first day). */
@@ -62,6 +67,22 @@ TileResult LoadTileMap(Arena &arena, const char *path);
 /* The kind of a cell, or -1 outside the map. */
 int TileAt(const TileMap &map, int x, int y);
 
+/* Lesson 055: the collision queries — answered from the map data alone
+   (the kinds' solidity), never from drawing code. The out-of-bounds
+   policy is defined, not accidental: a position or rectangle outside the
+   map counts as solid, so the world's edge blocks like a wall and no
+   query ever reads outside the map's cells. */
+
+/* Is the cell at (x, y) solid? Cells outside the map answer solid. */
+bool TileSolid(const TileMap &map, int x, int y);
+
+/* Point query: does this world position overlap a solid tile? */
+bool TilePointSolid(const TileMap &map, int world_x, int world_y);
+
+/* Rectangle query: does this world rectangle (w, h > 0) overlap any
+   solid tile — or the world's edge, which counts as solid? */
+bool TileRectSolid(const TileMap &map, int x, int y, int w, int h);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/tiles.h b/src/tiles.h
index 21ceb95..6474146 100644
--- a/src/tiles.h
+++ b/src/tiles.h
@@ -13,9 +13,6 @@
 
 namespace engine {
 
-/* The format's cell size: every tile is TILE_SIZE x TILE_SIZE pixels. */
-constexpr int TILE_SIZE = 16;
-
 /* A tile sheet: one sprite per kind, cut from the sheet at load. */
 struct TileSheet {
     Sprite kinds[TILE_MAX_KINDS];
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The policy is yours *(extend-the-code)*

The out-of-bounds answer is a choice; make it a visible one. Write the
same two queries with the opposite policy — outside the map is *free* —
beside the solid ones, and run the four out-of-bounds cases under both.
Predict first which cases will flip and which will not; then reconcile,
and answer the question the map asks: why does `rect leaving the map`
report *solid* under **both** policies?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-055/ex1.md)

### Exercise 2 — The rectangle at the boundary *(predict-the-output)*

Rectangle edges are where collision bugs live. Before you run anything,
write down the answer for six rectangles against the pillar at cells
(8,6)-(9,7) — the pillar's exact cell, the cell beside it, one pixel
*into* it, diagonal to it, one pixel touching its corner pixel, and one
pixel past that corner. Then run the table and reconcile every answer
with the rectangle's coverage arithmetic. Which single character in
`TileRectSolid` decides the one-pixel cases?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-055/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 054 — the camera](lesson-054-camera.md) ·
**Next:** [Lesson 056 — the mover that stops at walls](lesson-056-mover.md) ·
**Code tag:** [`lesson-055`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-055)
