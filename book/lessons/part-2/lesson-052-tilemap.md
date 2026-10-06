# Lesson 052 — the tilemap asset format

{{#include ../../stability-horizon.md}}

## Prose

A game's world is data before it is pixels. This lesson defines the
course's **tilemap format** — the file that says what the world is made
of — and gets it loaded: dimensions, tile kinds with their solidity, one
character per cell, all of it small enough to define by hand and read by
eye. The MVD's fourth obligation (*world: single scrolling map, tile
collision*) starts here: lesson 053 draws this data, lesson 055 queries
it, and the closing demo runs on it. The format is fixed in this lesson,
like every other contract in the course.

### The format, defined by hand

`assets/map.txt` — a text file in three parts:

```
20 12 3
. 0
# 1
w 0
####################
#..................#
#..................#
#....##......##....#
#....##......##....#
#..................#
#..................#
#...w....##....w...#
#...w....##....w...#
#..................#
#..................#
####################
```

1. **The first line: three numbers** — width, height, and how many tile
   kinds the file defines. Every later line is checked against these
   counts.
2. **The kind table: one line per kind** — the character that names the
   kind, then its solidity (`0` or `1`). Solidity is collision data
   (lesson 055 reads it), and it rides in the *format* from day one
   because the map is the world's data: which tiles block movement is a
   fact about the world, not about the renderer.
3. **The rows: exactly `height` lines of exactly `width` characters**,
   each character naming a kind from the table. The rows *are* the map —
   this file is readable as ASCII art, which is the whole reason to
   define a text format instead of packing bytes.

The example is a 20×12 room: `#` walls (solid) around the edge and in
four pillars, `.` floor, `w` shallow water (drawn but walkable — the
kind table says so, and lesson 055 will believe it).

### Complete, or nothing

`LoadTileMap` reads the file whole through the seam (lesson 037) and
parses it line by line. Its contract is the spec's: *a load yields either
a complete map or a typed failure — never a partial map presented as
success.* The rules it enforces, each with its own reason:

- the first line is three numbers, in range (`≤ 256` per dimension,
  `≤ 8` kinds) and nothing else;
- every kind line is one character plus `0` or `1` — and no character
  names two kinds;
- every row is exactly `width` characters — not one shorter, not one
  longer;
- every character in every row names a kind in the table;
- after the last row: trailing blank lines are tolerated, **content is
  not**.

The cells land in an arena block of `width × height` bytes, one kind
index each; on any failure the block is rolled back (lesson 041's mark)
and the map struct stays empty. The typed failure is the answer — the
caller sees `TILE_MALFORMED` and no map, exactly like the sprite loader's
contract of lesson 044.

The run, on the file above:

```
engine: map assets/map.txt: 20x12, 3 kinds
engine: map kind '.' (solid 0): 164 cells
engine: map kind '#' (solid 1): 72 cells
engine: map kind 'w' (solid 0): 4 cells
engine: map check: 240 cells, 0 unknown, 2 of 2 corners solid
```

The load is complete in the only sense that matters: **164 + 72 + 4 =
240 = 20 × 12** — every cell of the file is a cell of the map, counted.
The kind table came through with its solidity (`#` is solid; the corners
of this map are walls and the check says so), and no cell names a kind
outside the table (the loader would have refused the whole file rather
than let that happen).

### The typed failures, named

A corrupt copy of the file fails like this. The end state has one
message for every kind of wrong — each run below is a copy with exactly
one mistake:

```
$ ./build/game    # the header claims "0 12 3", a row is 19 characters,
$ ./build/game    # a cell is '?' no kind claims, two rows are missing,
$ ./build/game    # or a stray line follows the last row — every time:
engine: assets/map.txt: not a complete map
```

The report does not yet say *which* rule the file broke (the exercise at
the end of this lesson splits it into named cases), but the contract is
already doing its real job: the run reports a typed failure and hands
over **no map at all** — no half-loaded cells, no wrong cell count, no
crash. The failure paths are as tested as the success path; that is what
"a typed failure" buys.

### Why the map is data

The spec behind this lesson says the purpose out loud: *worlds can be
authored as files and tested without a screen.* Everything the engine
knows about the world comes from this file — its size, its kinds, which
kinds block movement — and every answer about it is computable in a test
harness with no window, no drawing, and no player. When lesson 055
answers "is this rectangle inside a wall?", the answer comes from these
bytes and nothing else. When the closing demo (lesson 057) scrolls a
world, it scrolls *this* data. Authors change the world by editing a
text file; the engine re-reads it at startup; nothing else moves.

## Code step

One change for this lesson: `assets/map.txt` is authored (a 20×12 map in
the format above), `src/tilemap.h` / `src/tilemap.cpp` bring the reader
(line-oriented, complete-or-nothing, solidity carried per kind), and
`main.cpp` loads the map, reports its kinds and cell counts, and checks
the load against the file's own arithmetic. The font, the text, and the
blitter are untouched. Its end state is tagged `lesson-052`.

```diff
diff --git a/assets/map.txt b/assets/map.txt
new file mode 100644
index 0000000..f142cb1
--- /dev/null
+++ b/assets/map.txt
@@ -0,0 +1,16 @@
+20 12 3
+. 0
+# 1
+w 0
+####################
+#..................#
+#..................#
+#....##......##....#
+#....##......##....#
+#..................#
+#..................#
+#...w....##....w...#
+#...w....##....w...#
+#..................#
+#..................#
+####################
diff --git a/src/main.cpp b/src/main.cpp
index c60358b..b63c353 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -15,6 +15,7 @@
 #include "platform.h"
 #include "sprite.h"
 #include "text.h"
+#include "tilemap.h"
 
 namespace engine {
 
@@ -332,6 +333,55 @@ int Run(void)
                     slot_state[0], slot_state[1], slot_state[2], slot_state[3]);
     }
 
+    /* Lesson 052: the world as data — the map file, loaded whole, its
+       cells counted against the file's own rows. */
+    const char *map_path = "assets/map.txt";
+    TileResult map_loaded = LoadTileMap(arena, map_path);
+    if (map_loaded.error != TILE_OK) {
+        switch (map_loaded.error) {
+        case TILE_MISSING:
+            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
+                         map_path);
+            break;
+        case TILE_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: %s: not a complete map\n", map_path);
+            break;
+        default:
+            std::fprintf(stderr, "engine: %s: no room in the arena\n",
+                         map_path);
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    TileMap &map = map_loaded.map;
+    std::printf("engine: map %s: %dx%d, %d kinds\n", map_path, map.width,
+                map.height, map.kind_count);
+    for (int k = 0; k < map.kind_count; ++k) {
+        int count = 0;
+        for (int y = 0; y < map.height; ++y)
+            for (int x = 0; x < map.width; ++x)
+                if (TileAt(map, x, y) == k)
+                    ++count;
+        std::printf("engine: map kind '%c' (solid %d): %d cells\n",
+                    map.kinds[k].cell, map.kinds[k].solid, count);
+    }
+    int unknown = 0, solid_corners = 0;
+    for (int y = 0; y < map.height; ++y)
+        for (int x = 0; x < map.width; ++x) {
+            int kind = TileAt(map, x, y);
+            if (kind < 0 || kind >= map.kind_count)
+                ++unknown;
+        }
+    if (map.kinds[TileAt(map, 0, 0)].solid)
+        ++solid_corners;
+    if (map.kinds[TileAt(map, map.width - 1, map.height - 1)].solid)
+        ++solid_corners;
+    std::printf("engine: map check: %d cells, %d unknown, %d of 2 corners solid\n",
+                map.width * map.height, unknown, solid_corners);
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
diff --git a/src/tilemap.cpp b/src/tilemap.cpp
new file mode 100644
index 0000000..fadf9fe
--- /dev/null
+++ b/src/tilemap.cpp
@@ -0,0 +1,164 @@
+// tilemap.cpp — the map file's reader: line by line, complete or nothing.
+//
+// Lesson 052: the format is three counts, a kind table, and one character
+// per cell — small enough that every rule here can be checked against the
+// file with your own eyes.
+
+#include "tilemap.h"
+
+#include "platform.h"
+
+namespace engine {
+namespace {
+
+/* Line-oriented parsing over the file's bytes: the format is lines, so
+   the reader is lines. */
+struct Lines {
+    const unsigned char *data;
+    size_t size;
+    size_t at; /* the start of the current line */
+};
+
+bool NextLine(Lines &lines, const unsigned char *&line, int &len)
+{
+    if (lines.at >= lines.size)
+        return false;
+    size_t start = lines.at;
+    while (lines.at < lines.size && lines.data[lines.at] != '\n')
+        ++lines.at;
+    len = (int)(lines.at - start);
+    line = lines.data + start;
+    if (lines.at < lines.size)
+        ++lines.at; /* consume the newline */
+    return true;
+}
+
+/* One decimal number, separated from its neighbors by spaces. */
+bool ReadInt(const unsigned char *line, int len, int &at, int &out)
+{
+    while (at < len && (line[at] == ' ' || line[at] == '\t'))
+        ++at;
+    if (at >= len || line[at] < '0' || line[at] > '9')
+        return false;
+    int value = 0;
+    while (at < len && line[at] >= '0' && line[at] <= '9') {
+        value = value * 10 + (line[at] - '0');
+        if (value > 1000000)
+            return false;
+        ++at;
+    }
+    out = value;
+    return true;
+}
+
+} /* namespace */
+
+TileResult LoadTileMap(Arena &arena, const char *path)
+{
+    TileResult result = { { 0, 0, 0, { { 0, 0 } }, 0 }, TILE_OK };
+
+    platform::FileData file = platform::ReadFile(path);
+    if (file.error != platform::FILE_OK) {
+        result.error = TILE_MISSING;
+        return result;
+    }
+
+    Lines lines = { file.data, file.size, 0 };
+    const unsigned char *line = 0;
+    int len = 0;
+    bool ok = true;
+
+    /* The first line: width height kind-count — the file's only numbers,
+       and the counts every later line is checked against. */
+    int width = 0, height = 0, kind_count = 0;
+    ok = ok && NextLine(lines, line, len);
+    int at = 0;
+    ok = ok && ReadInt(line, len, at, width);
+    ok = ok && ReadInt(line, len, at, height);
+    ok = ok && ReadInt(line, len, at, kind_count);
+    ok = ok && at == len; /* nothing else on the line */
+    ok = ok && width > 0 && width <= TILE_MAX_DIM;
+    ok = ok && height > 0 && height <= TILE_MAX_DIM;
+    ok = ok && kind_count > 0 && kind_count <= TILE_MAX_KINDS;
+
+    /* The kind table: one character and its solidity per kind, in order. */
+    TileKind kinds[TILE_MAX_KINDS];
+    for (int k = 0; ok && k < kind_count; ++k) {
+        ok = ok && NextLine(lines, line, len);
+        ok = ok && len >= 3;
+        if (ok) {
+            kinds[k].cell = (char)line[0];
+            int solid = -1;
+            int at2 = 1;
+            ok = ok && ReadInt(line, len, at2, solid);
+            ok = ok && (solid == 0 || solid == 1);
+            ok = ok && at2 == len;
+            kinds[k].solid = (unsigned char)solid;
+        }
+        for (int prev = 0; ok && prev < k; ++prev)
+            if (kinds[prev].cell == kinds[k].cell)
+                ok = false; /* one character, one kind */
+    }
+
+    /* The cells: exactly height rows of exactly width characters, every
+       character one the kind table names. */
+    size_t mark = ArenaMark(arena);
+    unsigned char *cells = 0;
+    if (ok) {
+        cells = (unsigned char *)ArenaAlloc(arena, (size_t)width * height, 1);
+        if (!cells) {
+            ArenaRollback(arena, mark);
+            platform::ReleaseFile(file);
+            result.error = TILE_NO_ROOM;
+            return result;
+        }
+    }
+    for (int y = 0; ok && y < height; ++y) {
+        ok = ok && NextLine(lines, line, len);
+        ok = ok && len == width;
+        for (int x = 0; ok && x < width; ++x) {
+            int kind = -1;
+            for (int k = 0; k < kind_count; ++k)
+                if (kinds[k].cell == (char)line[x]) {
+                    kind = k;
+                    break;
+                }
+            if (kind < 0)
+                ok = false; /* a character no kind claims */
+            else
+                cells[y * width + x] = (unsigned char)kind;
+        }
+    }
+
+    /* After the last row: trailing blank lines, and nothing else. */
+    while (ok && lines.at < lines.size) {
+        ok = ok && NextLine(lines, line, len);
+        ok = ok && len == 0;
+    }
+
+    if (!ok) {
+        ArenaRollback(arena, mark);
+        platform::ReleaseFile(file);
+        result.error = TILE_MALFORMED;
+        return result;
+    }
+
+    platform::ReleaseFile(file);
+    result.map.width = width;
+    result.map.height = height;
+    result.map.kind_count = kind_count;
+    for (int k = 0; k < kind_count; ++k)
+        result.map.kinds[k] = kinds[k];
+    result.map.cells = cells;
+    result.error = TILE_OK;
+    return result;
+}
+
+int TileAt(const TileMap &map, int x, int y)
+{
+    if (x < 0 || x >= map.width || y < 0 || y >= map.height)
+        return -1; /* outside the map: the defined answer, not a read */
+    return map.cells[y * map.width + x];
+}
+
+} /* namespace engine */
diff --git a/src/tilemap.h b/src/tilemap.h
new file mode 100644
index 0000000..1c076cf
--- /dev/null
+++ b/src/tilemap.h
@@ -0,0 +1,67 @@
+// tilemap.h — the tilemap as loadable world data.
+//
+// Lesson 052: the map is a file, the format is ours, and the load is
+// complete or nothing. The format, defined by hand:
+//
+//   <width> <height> <kind-count>       one line, three numbers
+//   <cell-char> <solid: 0|1>            one line per tile kind
+//   <width> characters                  one line per map row
+//
+// Every row is exactly <width> characters; every character names a kind
+// from the table; the table carries each kind's solidity, so collision
+// queries (lesson 055) are answered from the map data alone.
+#ifndef TILEMAP_H
+#define TILEMAP_H
+
+#include "arena.h"
+
+namespace engine {
+
+/* The bounds the format fixes — a map bigger than this is a different
+   format's file. */
+constexpr int TILE_MAX_DIM = 256;
+constexpr int TILE_MAX_KINDS = 8;
+
+/* One tile kind: the character that names it in the file, and whether it
+   is solid for collision (lesson 055 reads this; the format carries it
+   from the first day). */
+struct TileKind {
+    char cell;
+    unsigned char solid;
+};
+
+/* A loaded map: its dimensions, its kinds, and one kind index per cell. */
+struct TileMap {
+    int width;
+    int height;
+    int kind_count;
+    TileKind kinds[TILE_MAX_KINDS];
+    unsigned char *cells; /* width * height kind indices, in the arena */
+};
+
+/* A load either hands over a complete map or names what went wrong —
+   never a partial map presented as success. */
+enum TileError {
+    TILE_OK = 0,
+    TILE_MISSING,   /* the file is not there or cannot be read */
+    TILE_MALFORMED, /* the bytes do not form a complete map */
+    TILE_NO_ROOM,   /* the arena had no room for the cells */
+};
+
+struct TileResult {
+    TileMap map;
+    TileError error;
+};
+
+/* Loads a map from a file read whole: the first line's three numbers, the
+   kind table, then exactly height rows of exactly width characters. Any
+   deviation — a short row, an extra line, a character no kind claims —
+   is a typed failure, and nothing is handed over. */
+TileResult LoadTileMap(Arena &arena, const char *path);
+
+/* The kind of a cell, or -1 outside the map. */
+int TileAt(const TileMap &map, int x, int y);
+
+} /* namespace engine */
+
+#endif
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The failure that names itself *(extend-the-code)*

The loader's single `TILE_MALFORMED` covers five different mistakes, and
a report that says "malformed" sends you reading the file by hand. Split
the failure into named cases — the header, the kind table, the rows, the
trailing content — carry them through the enum and the run's report, and
re-run every corrupt copy from the lesson to check each report names its
own mistake. Where is the boundary between "the rows are wrong" and "the
kind table is wrong" if a row uses a character the table never defined?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-052/ex1.md)

### Exercise 2 — The map as characters *(extend-the-code)*

You have a font and a map; draw the map as a debug view — each cell as
its kind's character through `DrawText`, one row of glyphs per row of
cells. Then read two rows back and compare their ink against the file's
own arithmetic (count the characters per row and the ink each glyph
carries). Which is the sharper check — slots with ink, or ink pixels —
and what does the answer say about checking drawings against data?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-052/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 051 — text on screen](lesson-051-text.md) ·
**Next:** [Lesson 053 — tilemap drawing](lesson-053-tiles.md) ·
**Code tag:** [`lesson-052`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-052)
