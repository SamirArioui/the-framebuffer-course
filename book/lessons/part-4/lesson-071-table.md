# Lesson 071 — the archetype table

{{#include ../../stability-horizon.md}}

## Prose

Part 3 closed on an engine that holds one object at a time — the demo's
sprite, one map, one font, two sounds — and Part 5 opens on a game that
has to hold many things of several kinds: three enemy types, a boss,
projectiles, particle bursts. Every one of those is a set of *facts*
before it is anything else: where it starts, which way it faces, how fast
it moves, how much health it has. If the facts live in the engine's code,
every balance change is a rebuild and every new type is a copy of the last
type's numbers. So the idea of this lesson is one, and the next three
lessons stand on it: **an entity's facts are authored as data — a table of
definitions, one row each, in a format this course defines by hand.**

### The file, and the format

`assets/entities.txt` is the course's first table, and its whole grammar
fits on the screen:

```
name x y facing speed health sprite
hero 312 232 0 240 3 assets/sprite.ppm
slime 400 320 2 96 1 assets/sprite.ppm
```

The first line names the columns. Every line after it is one definition:
its values, separated by whitespace, in the order the header names them.
That is the entire grammar — no key/value blocks, no binary framing, no
quoting rules. This is the habit lesson 052's map file started: a text
format small enough that every rule of it can be checked against the file
with your own eyes, parsed byte by byte, because no library reads a file
in student-visible engine code.

The two rows are the two definitions the game has so far: `hero`, whose
row is the one Part 4 will move, and `slime`, one enemy type for the game
Part 5 will assemble. Both name `assets/sprite.ppm` as their art because
the course ships one sprite — the table is where that would change, and
it would change without a rebuild.

### The columns name the fields

A row's values are never "the third number is the speed". The header says
which value is which, and the loader fills the fields the header
declares. What it fills is a struct of named fields the game reads
directly:

```cpp
struct EntityDef {
    char name[TABLE_NAME_MAX];   /* the definition's identity */
    int x, y;                    /* where it starts, in world pixels */
    int facing;                  /* 0 right, 1 down, 2 left, 3 up */
    int speed;                   /* world pixels per second */
    int health;                  /* points */
    char sprite[TABLE_PATH_MAX]; /* the art file it draws */
};
```

Not a bag of keys read by name at runtime — a definition's speed is
`def.speed`, one field of one struct, and the loader decided which byte
range fills it by reading the header. The columns may come in any order:
a file whose header reads `speed name sprite health y facing x` is the
same table with its facts listed differently, and the exercise below
makes you predict what that does before you run it.

The columns have types, and the type is the column's, not the file's
politeness:

- `name` and `sprite` are **text** — a run of non-space bytes. The name
  is the definition's identity; the sprite is the art file it draws.
- `x`, `y`, `facing`, `speed`, and `health` are **whole numbers** —
  digits, separated from their neighbors by spaces. `facing` is one of
  the four the format defines: 0 right, 1 down, 2 left, 3 up.

And the fields have bounds, because a field is a field and not a
suggestion: a name longer than `TABLE_NAME_MAX` or a path longer than
`TABLE_PATH_MAX` is refused, never truncated into one — a truncated name
is a *different name*, and a truncated path is a different file. A number
is refused before it can grow past what the reader's arithmetic holds,
the same bound `map.txt`'s number reader carries.

### What the parse refuses

"A load yields a complete table or a typed failure — never partial data
presented as success" is the rule lesson 044 gave sprites and lesson 061
gave samples, kept exactly. The failures are named values, as they are
everywhere in this engine:

| Failure | The file that gets it |
| ------- | --------------------- |
| `TABLE_MISSING` | the file is not there, or the OS will not read it |
| `TABLE_MALFORMED` | the bytes are not a complete table in the engine's format |
| `TABLE_FULL` | more rows than the parse's array holds |

and `TABLE_MALFORMED` covers every row that disagrees with the format: a
column the format does not know, a column named twice or a column not
named at all, a row with one value too few or one too many, a value where
a number is required or a number where text is, a facing that is not one
of the four, a name the table already holds (one name, one definition —
the map's kind table has the same rule), a row where the header is
expected, or a header with no rows after it. Anything that is not
described is refused; nothing is read "as best it can".

Why this strict? Because the failures it refuses are worse than the ones
it makes. A row whose values sit one column off from the header is a hero
with an enemy's health and an enemy with the hero's speed — *and it runs
fine*: the wrong game that works is the failure no report ever catches. A
name truncated to sixteen bytes can collide with another definition and
the loader would answer the wrong one. A typed failure is the cheap,
honest answer: the run names the file, names the failure, and no
definition is ever used that the loader could not vouch for.

### Why the data is worth this

The point of the format is the requirement it carries: **changing a
table's value changes the game without recompiling the engine**, and the
engine's own code holds no per-type copy of these values. Look for the
hero's speed in `src/` and it is not there — it is in a file, next to the
other row's speed, editable in the same editor. That is what makes
balance a data change and a new enemy type a new row rather than a new
branch.

The header is part of the same promise. Because the file names its own
columns, a table stays readable months after the code that parses it was
written, and a value that lands in the wrong field lands there *by the
file's statement* — which is checkable against the file — rather than by
the loader's assumption, which is checkable against nothing.

### What this run verified, and what it did not

All of the numbers below come from real runs of this lesson's end state on
this machine:

- **The table loads completely.** The run reports
  `engine: table: 2 definitions` and then one line per definition —
  `engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite
  assets/sprite.ppm` and `engine: def slime: x 400 y 320 facing 2 speed
  96 health 1 sprite assets/sprite.ppm`. Every value on those lines is
  the value its row states; the line is printed so the comparison with
  the file is the check, and it is exact.
- **A missing file fails typed.** Point the run at a directory where
  `assets/entities.txt` is not and it says
  `engine: assets/entities.txt: could not load (missing)` and ends, the
  file named and the failure named.
- **A wrong-shaped file fails typed.** Four corruptions were cut from the
  real file and every one refuses by name: a row with a value where a
  number is required (`hero 312 232 0 fast 3 …`) and a row one value
  short both report `(malformed)`; two rows sharing the name `hero`
  report `(malformed)`; seventeen rows report `(too many rows)` — the
  parse's array holds sixteen.
- **The header's order is the file's.** The same two definitions with
  the columns reordered and the values written to match load into exactly
  the same fields: the report is identical, character for character.

What this lesson does **not** do is create anything. The table is data;
nothing in the run moves, draws, or lives yet. Lesson 072 finishes the
load's half of the story — where the rows are kept and what a refused load
leaves behind — and lesson 073 turns a row into an entity the game acts
on.

## Code step

One change for this lesson, from file to definitions: `src/table.h` /
`src/table.cpp` grow `EntityDef`, `EntityTable`, the typed failures, and
`LoadTable` — the header and the rows walked byte by byte, every value
checked against the column that claims it, the file's own bytes going
back to the OS when the walk is done. `src/main.cpp` grows the run's
startup: `assets/entities.txt` is loaded beside the other assets, a
successful load reports every definition's values as the byte-level
check, and a failed one ends the run by name like every other asset. The
new asset is the code step's other half: `assets/entities.txt`, the
hero's row and one enemy's, in the format this lesson defines. The
stream, the loop, and the seam are untouched. Its end state is tagged
`lesson-071`.

```diff
diff --git a/assets/entities.txt b/assets/entities.txt
new file mode 100644
index 0000000..0683f17
--- /dev/null
+++ b/assets/entities.txt
@@ -0,0 +1,3 @@
+name x y facing speed health sprite
+hero 312 232 0 240 3 assets/sprite.ppm
+slime 400 320 2 96 1 assets/sprite.ppm
diff --git a/src/main.cpp b/src/main.cpp
index 455b331..7ddd340 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -20,6 +20,7 @@
 #include "frame.h"
 #include "platform.h"
 #include "sprite.h"
+#include "table.h"
 #include "text.h"
 #include "tilemap.h"
 #include "tiles.h"
@@ -162,6 +163,43 @@ int Run(void)
     }
     TileSheet &sheet = tiles_loaded.sheet;
 
+    /* Lesson 071: the run's entities are data. The table file holds one
+       row per definition — its columns named by its header — and the load
+       either hands over every definition or names what went wrong, like
+       every asset above. */
+    TableResult table_loaded = LoadTable("assets/entities.txt");
+    if (table_loaded.error != TABLE_OK) {
+        switch (table_loaded.error) {
+        case TABLE_MISSING:
+            std::fprintf(stderr,
+                         "engine: assets/entities.txt: could not load (missing)\n");
+            break;
+        case TABLE_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: assets/entities.txt: could not load (malformed)\n");
+            break;
+        default:
+            std::fprintf(stderr,
+                         "engine: assets/entities.txt: could not load (too many rows)\n");
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    EntityTable &table = table_loaded.table;
+
+    /* The byte-level check, before anything uses the table: every
+       definition, carrying the values its row states. */
+    std::printf("engine: table: %d definition%s\n", table.count,
+                table.count == 1 ? "" : "s");
+    for (int i = 0; i < table.count; ++i) {
+        const EntityDef &def = table.rows[i];
+        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s\n",
+                    def.name, def.x, def.y, def.facing, def.speed, def.health,
+                    def.sprite);
+    }
+
     /* Lesson 066: the run's two sounds as files' bytes — the music that
        loops and the effect that plays once. Lesson 061's tone leaves the
        run here (it stays on disk: the file lessons 059-065 were built
diff --git a/src/table.cpp b/src/table.cpp
new file mode 100644
index 0000000..76e6d98
--- /dev/null
+++ b/src/table.cpp
@@ -0,0 +1,263 @@
+// table.cpp — the table file's reader: rows of definitions, complete or
+// nothing.
+//
+// Lesson 071: the format's grammar is a paragraph — a header naming the
+// columns, then one row per definition — and this file walks it byte by
+// byte like map.txt's reader does. Every value is checked against the
+// column that claims it: a whole number where a number belongs, a run of
+// non-space bytes where text belongs, and exactly as many values as the
+// header names.
+
+#include "table.h"
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
+/* One whitespace-separated value, exactly as long as the file has it. */
+bool NextToken(const unsigned char *line, int len, int &at,
+               const unsigned char *&token, int &token_len)
+{
+    while (at < len && (line[at] == ' ' || line[at] == '\t'))
+        ++at;
+    if (at >= len)
+        return false;
+    token = line + at;
+    while (at < len && line[at] != ' ' && line[at] != '\t')
+        ++at;
+    token_len = (int)(line + at - token);
+    return true;
+}
+
+/* One whole number: digits, separated from its neighbors by spaces. The
+   bound is the number reader's, not the format's — a number is refused
+   before it can grow past what this arithmetic holds. */
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
+/* One text value: a run of non-space bytes, copied into a field no wider
+   than out_max. A value that does not fit is refused, never truncated —
+   a truncated name is a different name, and a truncated path is a
+   different file. */
+bool ReadText(const unsigned char *line, int len, int &at, char *out,
+              int out_max)
+{
+    while (at < len && (line[at] == ' ' || line[at] == '\t'))
+        ++at;
+    size_t start = (size_t)at;
+    while (at < len && line[at] != ' ' && line[at] != '\t')
+        ++at;
+    int width = (int)((size_t)at - start);
+    if (width == 0 || width >= out_max)
+        return false;
+    for (int i = 0; i < width; ++i)
+        out[i] = (char)line[start + i];
+    out[width] = 0;
+    return true;
+}
+
+/* The columns the format knows. */
+enum Column {
+    COL_NAME,
+    COL_X,
+    COL_Y,
+    COL_FACING,
+    COL_SPEED,
+    COL_HEALTH,
+    COL_SPRITE,
+    COL_COUNT
+};
+
+const char *const COLUMN_NAMES[COL_COUNT] = {
+    "name", "x", "y", "facing", "speed", "health", "sprite"
+};
+
+bool TokenIs(const unsigned char *token, int token_len, const char *name)
+{
+    int n = 0;
+    while (name[n])
+        ++n;
+    if (n != token_len)
+        return false;
+    for (int i = 0; i < n; ++i)
+        if (name[i] != (char)token[i])
+            return false;
+    return true;
+}
+
+/* A text field against a text field: the same bytes, or not. */
+bool SameText(const char *a, const char *b)
+{
+    int i = 0;
+    while (a[i] && a[i] == b[i])
+        ++i;
+    return a[i] == b[i];
+}
+
+int FindColumn(const unsigned char *token, int token_len)
+{
+    for (int c = 0; c < COL_COUNT; ++c)
+        if (TokenIs(token, token_len, COLUMN_NAMES[c]))
+            return c;
+    return -1;
+}
+
+} /* namespace */
+
+TableResult LoadTable(const char *path)
+{
+    TableResult result = {};
+
+    platform::FileData file = platform::ReadFile(path);
+    if (file.error != platform::FILE_OK) {
+        result.error = TABLE_MISSING;
+        return result;
+    }
+
+    Lines lines = { file.data, file.size, 0 };
+    const unsigned char *line = 0;
+    int len = 0;
+    bool ok = true;
+    TableError failure = TABLE_MALFORMED;
+
+    /* The header: the columns this file's rows carry, named one after
+       another. The order is the file's — the loader fills the fields the
+       header declares — but every column the format knows is named, and
+       named once. A name the format does not know is refused here rather
+       than read as something else later. */
+    int order[COL_COUNT];
+    for (int c = 0; c < COL_COUNT; ++c)
+        order[c] = -1;
+    ok = ok && NextLine(lines, line, len);
+    int at = 0;
+    for (int i = 0; ok && i < COL_COUNT; ++i) {
+        const unsigned char *token = 0;
+        int token_len = 0;
+        ok = ok && NextToken(line, len, at, token, token_len);
+        if (ok) {
+            int column = FindColumn(token, token_len);
+            ok = ok && column >= 0;
+            for (int prev = 0; ok && prev < i; ++prev)
+                ok = ok && order[prev] != column; /* one name, one column */
+            order[i] = column;
+        }
+    }
+    const unsigned char *extra = 0;
+    int extra_len = 0;
+    ok = ok && !NextToken(line, len, at, extra, extra_len);
+
+    /* The rows: one definition each, every value landing in the field
+       its column names. The row is refused — the whole file is — when a
+       value is missing or one too many, when a value is not what its
+       column requires, when the facing is not one of the four the format
+       defines, or when the name is one the table already holds (one
+       name, one definition — the map's kind table has the same rule). */
+    while (ok) {
+        if (!NextLine(lines, line, len))
+            break;
+        if (len == 0)
+            break; /* the rows end here; the tail is checked below */
+        if (result.table.count >= TABLE_MAX_ROWS) {
+            failure = TABLE_FULL;
+            ok = false;
+            break;
+        }
+
+        EntityDef &def = result.table.rows[result.table.count];
+        at = 0;
+        for (int i = 0; ok && i < COL_COUNT; ++i) {
+            switch (order[i]) {
+            case COL_NAME:
+                ok = ReadText(line, len, at, def.name, TABLE_NAME_MAX);
+                break;
+            case COL_X:
+                ok = ReadInt(line, len, at, def.x);
+                break;
+            case COL_Y:
+                ok = ReadInt(line, len, at, def.y);
+                break;
+            case COL_FACING:
+                ok = ReadInt(line, len, at, def.facing);
+                ok = ok && def.facing <= 3;
+                break;
+            case COL_SPEED:
+                ok = ReadInt(line, len, at, def.speed);
+                break;
+            case COL_HEALTH:
+                ok = ReadInt(line, len, at, def.health);
+                break;
+            case COL_SPRITE:
+                ok = ReadText(line, len, at, def.sprite, TABLE_PATH_MAX);
+                break;
+            default:
+                ok = false;
+                break;
+            }
+        }
+        while (ok && at < len && (line[at] == ' ' || line[at] == '\t'))
+            ++at;
+        ok = ok && at == len; /* nothing else on the line */
+        for (int prev = 0; ok && prev < result.table.count; ++prev)
+            ok = ok && !SameText(result.table.rows[prev].name, def.name);
+        if (ok)
+            result.table.count += 1;
+    }
+
+    /* After the last row: trailing blank lines, and nothing else. A file
+       that holds a header and no rows is not a table — the load is a
+       complete table or a typed failure. */
+    while (ok && lines.at < lines.size) {
+        ok = ok && NextLine(lines, line, len);
+        ok = ok && len == 0;
+    }
+    ok = ok && result.table.count > 0;
+
+    platform::ReleaseFile(file);
+    if (!ok) {
+        result.table.count = 0;
+        result.error = failure;
+        return result;
+    }
+
+    result.error = TABLE_OK;
+    return result;
+}
+
+} /* namespace engine */
diff --git a/src/table.h b/src/table.h
new file mode 100644
index 0000000..06e0a89
--- /dev/null
+++ b/src/table.h
@@ -0,0 +1,72 @@
+// table.h — the archetype table as loadable data.
+//
+// Lesson 071: an entity's facts are authored as data. A table file names
+// its columns in a header and holds one row per definition, every value
+// whitespace-separated — the format is ours, defined by hand like
+// map.txt's and the WAV loader's:
+//
+//   name x y facing speed health sprite
+//   hero 312 232 0 240 3 assets/sprite.ppm
+//   slime 400 320 2 96 1 assets/sprite.ppm
+//
+// The header names the columns, and the loader fills the fields the
+// header declares — so the columns may come in any order, but every
+// column the format knows comes exactly once. `name` and `sprite` are
+// text (a run of non-space bytes); x, y, facing, speed, and health are
+// whole numbers, and facing is one of the four the format defines:
+// 0 right, 1 down, 2 left, 3 up.
+#ifndef TABLE_H
+#define TABLE_H
+
+namespace engine {
+
+/* The parse's destination is a fixed array of rows, like the map's kinds
+   are. How many rows a table holds is the file's fact and not this
+   constant's — lesson 072 moves the rows where the file's count is what
+   lands. The name and path widths are the fields' own bounds: a value
+   longer than its field is refused, never truncated into one. */
+constexpr int TABLE_MAX_ROWS = 16;
+constexpr int TABLE_NAME_MAX = 16;
+constexpr int TABLE_PATH_MAX = 64;
+
+/* One definition: a row of the table, carrying every value its row
+   states — the identity and the attributes an entity is created from. */
+struct EntityDef {
+    char name[TABLE_NAME_MAX];   /* the definition's identity */
+    int x, y;                    /* where it starts, in world pixels */
+    int facing;                  /* 0 right, 1 down, 2 left, 3 up */
+    int speed;                   /* world pixels per second */
+    int health;                  /* points */
+    char sprite[TABLE_PATH_MAX]; /* the art file it draws */
+};
+
+/* A loaded table: one definition per row. */
+struct EntityTable {
+    EntityDef rows[TABLE_MAX_ROWS];
+    int count;
+};
+
+/* A load either hands over a complete table or names what went wrong —
+   never a partial table presented as success. */
+enum TableError {
+    TABLE_OK = 0,
+    TABLE_MISSING,   /* the file is not there or cannot be read */
+    TABLE_MALFORMED, /* the bytes are not a complete table in the format */
+    TABLE_FULL,      /* more rows than the parse's array holds */
+};
+
+struct TableResult {
+    EntityTable table;
+    TableError error; /* TABLE_OK exactly when the table is complete */
+};
+
+/* Loads an entity table from a file read whole. The header and the rows
+   are parsed byte by byte — no library reads it — and anything the format
+   does not describe is refused typed: a column it does not know, a row
+   with the wrong number of values, a value where a number is required, a
+   value where text is, a name the table already holds. */
+TableResult LoadTable(const char *path);
+
+} /* namespace engine */
+
+#endif
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The header, reordered *(predict-the-output)*

The loader fills the fields the header declares, so a table file may name
its columns in any order — the rows follow their own header. Here are the
same two definitions as `assets/entities.txt`, the columns in a different
order and the values written to match:

```
speed name sprite health y facing x
240 hero assets/sprite.ppm 3 232 0 312
96 slime assets/sprite.ppm 1 320 2 400
```

Before running anything, write down what the run's report will say about
`hero` — every field, in the report's own order — and one sentence on why
each value lands where it does. Then make one change to the hero row and
nothing else: swap its `3` and `240`, so the row reads `3 hero
assets/sprite.ppm 240 232 0 312`. Predict again, before running: what
does the report say now, and what does the loader say about the row — if
it says anything at all? Run both files and reconcile your two
predictions with the two runs.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-071/ex1.md)

### Exercise 2 — The table written back *(extend-the-code)*

The loader reads this lesson's format; teach the engine to write one. Add
a writer beside the loader — this lesson's header, then one row per
definition, every value assembled by hand into bytes and written through
the seam's whole-file write — in the engine's format and no other. Then
make the run a round trip: load the table, write it to a file of your
own, load that file back with `LoadTable`, and compare every field of
every definition against the table that went out — report the first field
that disagrees, or that every field agrees. Compare the file you wrote
against `assets/entities.txt` outside the run. What does the round trip
prove about the format that reading the file once cannot?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-071/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 070 — the mix's cost in the frame budget](../part-3/lesson-070-audio-row.md) ·
**Next:** [Lesson 072 — the load, complete or named](lesson-072-load.md) ·
**Code tag:** [`lesson-071`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-071)
