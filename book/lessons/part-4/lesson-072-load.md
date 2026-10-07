# Lesson 072 — the load, complete or named

{{#include ../../stability-horizon.md}}

## Prose

Lesson 071 ended with a loader that keeps its promise on the *format*:
every row checked against the header, every value against its column, a
complete table or a named failure. It kept that promise into a fixed
array of sixteen rows — and that array is a decision the file never
made. How many definitions a table holds is the *file's* fact, like its
columns and its values. So this lesson's idea is the other half of a
load's promise: **the load is complete or named — and a load that refuses
keeps nothing.** The rows move into the arena, where the file's count is
what lands, and the whole load becomes one transaction.

### The count is the file's fact

An arena allocation is a pointer that moves forward (lesson 041): what
you ask for is what you get, and it cannot grow. So a loader that
allocates rows while it parses has to either guess the count up front or
keep re-allocating — and the arena has no re-allocation to offer. This
loader walks the file twice instead, which is the honest shape for a bump
allocator:

1. **the counting walk** — the header's line, then the rows until they
   end, counted. Nothing is checked yet and nothing is kept; the walk's
   only product is the number `rows`;
2. **the fill walk** — the header parsed and the rows filled, into
   exactly `rows` definitions allocated from the arena.

Both walks agree on where the rows end: the first blank line. What comes
after it is the file's tail, and the format allows trailing blank lines
and nothing else — a row after the hole is a malformed file, not a
second table. (The fill also refuses to write past the count the first
walk promised, so the two walks can never disagree about the allocation's
size.)

The arena's own account shows the arithmetic. From the scratch probe
below, a two-row table lands in 200 bytes — `rows × sizeof(EntityDef)`,
one hundred bytes a definition — and a two-hundred-row table lands in
22200: not a capacity picked in advance, and not a byte wasted on rows
the file does not have. And lesson 071's one refusal that is gone:
**seventeen rows are a table now.** The parse's array held sixteen; the
arena holds what the file holds, and the only limit left is the arena's
own room, answered as `TABLE_NO_ROOM`.

### The load is one transaction

The mark goes down before the rows are taken and every refusal rolls back
to it — the same transaction lesson 061's sample loader brackets its
frames with, and for the same reason. A load that refuses halfway must
not leave half a table in the engine's memory: the game would see a used
count that never comes down, and the bytes of a file the loader *said* it
refused. With the rollback, "nothing partial kept" is not a comment — it
is a number you can watch:

```
assets/entities.txt        -> ok        rows 2, arena 0 -> 200  rows kept
assets/seventeen.txt       -> ok        rows 17, arena 200 -> 1900  rows kept
assets/short-row.txt       -> malformed rows 0, arena 1900 -> 1900  nothing kept
assets/not-a-number.txt    -> malformed rows 0, arena 1900 -> 1900  nothing kept
assets/two-heroes.txt      -> malformed rows 0, arena 1900 -> 1900  nothing kept
assets/missing.txt         -> missing   rows 0, arena 1900 -> 1900  nothing kept
assets/entities.txt        -> ok        rows 2, arena 1900 -> 2100  rows kept
```

Those seven lines are a scratch probe — not engine code, compiled
against `table.cpp`, `arena.cpp`, and the platform files to watch
`arena.used` across each load. The first load takes its 200 bytes and
keeps them, as a successful load should. The seventeen-row table takes
1700 more. Then four refusals in a row — a row one value short, a value
where a number is required, two definitions named `hero`, a file that is
not there — and the arena's used count does not move by a single byte
across any of them. The last load takes its rows again. That is the
rollback's promise, checked.

The run's own arena report agrees from inside the engine. Before this
lesson it read `engine: arena: 1534080 of 33554432 bytes used`; after it,
`1534280` — the two definitions' 200 bytes and nothing else.

### The failures are the loaders' failures

Lesson 071's failure set had one more name than the others — `TABLE_FULL`
— because the parse had a capacity to run out of. With the capacity gone,
the table's failures settle on the three every asset loader in this
engine answers with:

| Failure | The file that gets it |
| ------- | --------------------- |
| `TABLE_MISSING` | the file is not there, or the OS will not read it |
| `TABLE_MALFORMED` | the bytes are not a complete table in the engine's format |
| `TABLE_NO_ROOM` | the arena has no room for the rows |

`TABLE_MALFORMED` is still the row-level refusals it was: a column the
format does not know, a column named twice or not at all, a row with one
value too few or one too many, a value where a number is required, a
facing that is not one of the four, a name the table already holds, a
file with no rows in it. What changed is not which files are refused but
what a refusal costs: nothing.

### What this run verified, and what it did not

All of the numbers above come from real runs of this lesson's end state
on this machine:

- **The table loads completely, into the arena.** The run reports the
  same two definitions as lesson 071 — `engine: table: 2 definitions`
  and the values their rows state — and its arena report moved by exactly
  `rows × sizeof(EntityDef)`: 200 bytes for two definitions.
- **A refused load keeps nothing.** The scratch probe's four refusals
  above, each with the arena's used count unchanged across it.
- **Missing and malformed files fail typed, by name.** The run against a
  directory without the file says `engine: assets/entities.txt: could not
  load (missing)`; against the file with `fast` where a number is
  required it says `engine: assets/entities.txt: could not load
  (malformed)`. Both end the run by name, like every asset load above it.
- **The row count is the file's fact.** The seventeen-row table the
  previous lesson refused as `(too many rows)` loads, in 1700 bytes of
  arena.

What this lesson does **not** verify is `TABLE_NO_ROOM`: it is the
arena's answer, and this machine's arena is 32 MB — a table would need
some three hundred thousand rows to hear it. The path is the same shape
every other loader's `NO_ROOM` takes (mark, no rows, name the failure),
and lesson 061 exercised that shape where it could be reached.

Nothing here changes the format or the game: the same file, the same
report, the same two definitions. What the game now has is a load it can
trust at its word — complete, or named, with nothing left behind.

## Code step

One change for this lesson, from the array to the arena: `src/table.h` /
`src/table.cpp` move the rows into the engine's memory — the file walked
once to count them and once to fill them — and `LoadTable` grows the
arena it takes and the mark that makes the load a transaction.
`EntityTable`'s rows become a pointer, `TABLE_MAX_ROWS` retires (the
count is the file's fact now), and the failures settle on the three every
loader answers with: `TABLE_MISSING`, `TABLE_MALFORMED`, `TABLE_NO_ROOM`.
`src/main.cpp` grows the call and names the third failure like the other
two. The format is untouched — same file, same refusals — and the run's
report is unchanged. Its end state is tagged `lesson-072`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 7ddd340..3f07b8b 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -166,8 +166,9 @@ int Run(void)
     /* Lesson 071: the run's entities are data. The table file holds one
        row per definition — its columns named by its header — and the load
        either hands over every definition or names what went wrong, like
-       every asset above. */
-    TableResult table_loaded = LoadTable("assets/entities.txt");
+       every asset above. Lesson 072: the rows are the arena's, and a
+       refused load keeps none of them. */
+    TableResult table_loaded = LoadTable(arena, "assets/entities.txt");
     if (table_loaded.error != TABLE_OK) {
         switch (table_loaded.error) {
         case TABLE_MISSING:
@@ -180,7 +181,7 @@ int Run(void)
             break;
         default:
             std::fprintf(stderr,
-                         "engine: assets/entities.txt: could not load (too many rows)\n");
+                         "engine: assets/entities.txt: could not load (no room)\n");
             break;
         }
         platform::CloseWindow(opened.window);
diff --git a/src/table.cpp b/src/table.cpp
index 76e6d98..c589b8d 100644
--- a/src/table.cpp
+++ b/src/table.cpp
@@ -7,6 +7,11 @@
 // column that claims it: a whole number where a number belongs, a run of
 // non-space bytes where text belongs, and exactly as many values as the
 // header names.
+//
+// Lesson 072: the rows live in the arena. How many there are is the
+// file's fact, so the file is walked once to count them and once to fill
+// them, and the whole load is bracketed by a mark — a refused load rolls
+// the arena back and keeps nothing.
 
 #include "table.h"
 
@@ -141,7 +146,7 @@ int FindColumn(const unsigned char *token, int token_len)
 
 } /* namespace */
 
-TableResult LoadTable(const char *path)
+TableResult LoadTable(Arena &arena, const char *path)
 {
     TableResult result = {};
 
@@ -151,22 +156,51 @@ TableResult LoadTable(const char *path)
         return result;
     }
 
+    /* The first walk: the header's line, then the rows, counted. How many
+       rows a table holds is the file's fact — never a capacity the engine
+       picked — so the count comes first and the arena is asked for
+       exactly that many rows. The rows end at the first blank line; what
+       follows them is checked against the format in the second walk. */
     Lines lines = { file.data, file.size, 0 };
     const unsigned char *line = 0;
     int len = 0;
-    bool ok = true;
-    TableError failure = TABLE_MALFORMED;
+    bool ok = NextLine(lines, line, len); /* the header's line */
+    int rows = 0;
+    while (ok && NextLine(lines, line, len)) {
+        if (len == 0)
+            break;
+        rows += 1;
+    }
 
-    /* The header: the columns this file's rows carry, named one after
-       another. The order is the file's — the loader fills the fields the
-       header declares — but every column the format knows is named, and
-       named once. A name the format does not know is refused here rather
-       than read as something else later. */
+    /* The rows, into the arena. The mark is the load's transaction: from
+       here on a refusal rolls the arena back, and a refused load leaves
+       no partial rows behind — the used count does not move. */
+    size_t mark = ArenaMark(arena);
+    EntityDef *defs = 0;
+    if (ok && rows > 0) {
+        defs = (EntityDef *)ArenaAlloc(arena, (size_t)rows * sizeof(EntityDef),
+                                       4);
+        if (!defs) {
+            ArenaRollback(arena, mark);
+            platform::ReleaseFile(file);
+            result.error = TABLE_NO_ROOM;
+            return result;
+        }
+    }
+
+    /* The second walk: the header and the rows, byte by byte — every
+       value landing in the field its column names. A row is refused — the
+       whole file is — when a value is missing or one too many, when a
+       value is not what its column requires, when the facing is not one
+       of the four the format defines, or when the name is one the table
+       already holds. The fill never writes past the count the first walk
+       promised. */
+    lines.at = 0;
+    ok = ok && NextLine(lines, line, len);
+    int at = 0;
     int order[COL_COUNT];
     for (int c = 0; c < COL_COUNT; ++c)
         order[c] = -1;
-    ok = ok && NextLine(lines, line, len);
-    int at = 0;
     for (int i = 0; ok && i < COL_COUNT; ++i) {
         const unsigned char *token = 0;
         int token_len = 0;
@@ -183,24 +217,17 @@ TableResult LoadTable(const char *path)
     int extra_len = 0;
     ok = ok && !NextToken(line, len, at, extra, extra_len);
 
-    /* The rows: one definition each, every value landing in the field
-       its column names. The row is refused — the whole file is — when a
-       value is missing or one too many, when a value is not what its
-       column requires, when the facing is not one of the four the format
-       defines, or when the name is one the table already holds (one
-       name, one definition — the map's kind table has the same rule). */
     while (ok) {
         if (!NextLine(lines, line, len))
             break;
         if (len == 0)
             break; /* the rows end here; the tail is checked below */
-        if (result.table.count >= TABLE_MAX_ROWS) {
-            failure = TABLE_FULL;
-            ok = false;
+        if (result.table.count >= rows) {
+            ok = false; /* the fill never writes past the count's promise */
             break;
         }
 
-        EntityDef &def = result.table.rows[result.table.count];
+        EntityDef &def = defs[result.table.count];
         at = 0;
         for (int i = 0; ok && i < COL_COUNT; ++i) {
             switch (order[i]) {
@@ -235,7 +262,7 @@ TableResult LoadTable(const char *path)
             ++at;
         ok = ok && at == len; /* nothing else on the line */
         for (int prev = 0; ok && prev < result.table.count; ++prev)
-            ok = ok && !SameText(result.table.rows[prev].name, def.name);
+            ok = ok && !SameText(defs[prev].name, def.name);
         if (ok)
             result.table.count += 1;
     }
@@ -251,11 +278,13 @@ TableResult LoadTable(const char *path)
 
     platform::ReleaseFile(file);
     if (!ok) {
+        ArenaRollback(arena, mark);
         result.table.count = 0;
-        result.error = failure;
+        result.error = TABLE_MALFORMED;
         return result;
     }
 
+    result.table.rows = defs;
     result.error = TABLE_OK;
     return result;
 }
diff --git a/src/table.h b/src/table.h
index 06e0a89..c570c17 100644
--- a/src/table.h
+++ b/src/table.h
@@ -18,14 +18,15 @@
 #ifndef TABLE_H
 #define TABLE_H
 
+#include "arena.h"
+
 namespace engine {
 
-/* The parse's destination is a fixed array of rows, like the map's kinds
-   are. How many rows a table holds is the file's fact and not this
-   constant's — lesson 072 moves the rows where the file's count is what
-   lands. The name and path widths are the fields' own bounds: a value
-   longer than its field is refused, never truncated into one. */
-constexpr int TABLE_MAX_ROWS = 16;
+/* The parse's destination is the arena: a table's rows are the file's
+   fact — how many there are is what the file says, and the arena gives
+   exactly that many. The name and path widths are the fields' own
+   bounds: a value longer than its field is refused, never truncated into
+   one. */
 constexpr int TABLE_NAME_MAX = 16;
 constexpr int TABLE_PATH_MAX = 64;
 
@@ -40,19 +41,21 @@ struct EntityDef {
     char sprite[TABLE_PATH_MAX]; /* the art file it draws */
 };
 
-/* A loaded table: one definition per row. */
+/* A loaded table: one definition per row, in the arena — as many rows as
+   the file has, and not one more. */
 struct EntityTable {
-    EntityDef rows[TABLE_MAX_ROWS];
+    EntityDef *rows;
     int count;
 };
 
 /* A load either hands over a complete table or names what went wrong —
-   never a partial table presented as success. */
+   never a partial table presented as success. These are the loaders'
+   failures, the same three every asset in this engine answers with. */
 enum TableError {
     TABLE_OK = 0,
     TABLE_MISSING,   /* the file is not there or cannot be read */
     TABLE_MALFORMED, /* the bytes are not a complete table in the format */
-    TABLE_FULL,      /* more rows than the parse's array holds */
+    TABLE_NO_ROOM,   /* the arena had no room for the rows */
 };
 
 struct TableResult {
@@ -64,8 +67,10 @@ struct TableResult {
    are parsed byte by byte — no library reads it — and anything the format
    does not describe is refused typed: a column it does not know, a row
    with the wrong number of values, a value where a number is required, a
-   value where text is, a name the table already holds. */
-TableResult LoadTable(const char *path);
+   value where text is, a name the table already holds. The rows are
+   copied into the arena behind a mark, and every refusal path rolls back
+   to it: a load that refuses leaves nothing behind. */
+TableResult LoadTable(Arena &arena, const char *path);
 
 } /* namespace engine */
 
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The table with a hole in it *(predict-the-output)*

A table file whose rows stop in the middle and start again after a blank
line:

```
name x y facing speed health sprite
hero 312 232 0 240 3 assets/sprite.ppm

slime 400 320 2 96 1 assets/sprite.ppm
```

Before running anything, write down three predictions: what the loader
reports — its typed failure, or success; how many definitions it hands
over; and the arena's used count before and after the load, with the
course's own table loaded first. Then run and reconcile all three, and
name which walk decided each answer.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-072/ex1.md)

### Exercise 2 — The load's cost, measured *(measure-the-performance)*

The run's report says the load happened; nothing says what it cost. Put a
number on it: measure `LoadTable` on your machine at four table sizes —
2, 20, 200, and 2000 definitions — with `platform::Now()` around the
call, and report the load's time and the arena's growth at each size.
Where does the time go as the row count grows by tens, and what in the
loader is responsible? Would you change anything for this game's tables —
and what would your answer be if a table ever held ten thousand rows?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-072/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 071 — the archetype table](lesson-071-table.md) ·
**Next:** [Lesson 073 — entities as rows](lesson-073-rows.md) ·
**Code tag:** [`lesson-072`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-072)
