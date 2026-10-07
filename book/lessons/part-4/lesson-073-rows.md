# Lesson 073 — entities as rows

{{#include ../../stability-horizon.md}}

## Prose

Lesson 072 left the game holding a table of definitions and nothing else:
two rows of facts, loaded whole, complete or named. Facts are not things.
The hero on screen is still the demo's sprite moved by the demo's own
`double`s, and the table's row for it is data nothing has read. So this
lesson's idea is the bridge between them: **an entity is a row the game
created from a definition** — a live thing carrying that definition's
identity and attributes in named fields the game reads directly. And
writes directly, which is the half that matters.

### A definition carries its facts

Creating an entity has to answer every question about it from the
definition alone — never from assumptions about the file, and never from
a second source of truth. So a definition carries what creation needs:

```cpp
struct EntityDef {
    char name[TABLE_NAME_MAX];   /* the definition's identity */
    int x, y;                    /* where it starts, in world pixels */
    int facing;                  /* 0 right, 1 down, 2 left, 3 up */
    int speed;                   /* world pixels per second */
    int health;                  /* points */
    char sprite[TABLE_PATH_MAX]; /* the art file it draws */
    const Sprite *image;         /* that art, loaded at startup */
};
```

Seven of those fields are the file's, filled by the parse. The eighth —
`image` — is the one fact the file states as a *name*: the sprite column
nobody can draw with. The run resolves it at startup, loading each
definition's art through the loader lesson 044 wrote and handing the
definition its image, so from there on a definition knows its own art the
same way it knows its own speed. That split is deliberate and it is the
asset habit again: the file names what it wants; the engine loads it once
and keeps it (in the arena, like every other asset); the data carries the
result.

### The entity is a copy, not a view

`EntityFromDef` takes a definition and answers with an entity:

```cpp
struct Entity {
    char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
    int x, y;                  /* where it is, in world pixels */
    int facing;                /* 0 right, 1 down, 2 left, 3 up */
    int speed;                 /* world pixels per second */
    int health;                /* points */
    const Sprite *sprite;      /* the art it draws, from its row */
};
```

The same facts, one struct deeper — and *copies* of them, every one. That
is not bookkeeping. The game writes these fields: the hero's position
changes every frame its player holds a key, its health changes when
something hits it. If the entity were a view onto the definition —
pointers into the table's rows — then moving one hero would rewrite the
row, and every other hero created from that row would move with it, and
the file's stated values would stop being the file's stated values before
the run was over. The definition is where an entity *comes from*; the
entity is what it *lives in*. Two entities of one kind start identical
and diverge from their first frame.

The fields are named and plain on purpose (design's call, kept): the game
reads `hero.speed` and writes `hero.x` like any other values — no key/value
bag, no lookup by string per attribute per frame. When Part 5 gives
enemies behavior, the behavior reads these same fields; there is no
per-type struct hierarchy to grow.

### Asking for a definition

The game does not know which rows a file has — that is the file's
business — so asking for one is a request with a typed answer:

```cpp
DefResult TableFind(const EntityTable &table, const char *name);
```

`DEF_OK` and the definition, or `DEF_UNKNOWN` and nothing. A definition
the table does not hold is **reported as a value**, never answered with
"the first row" or "something close": an entity with assumed attributes
is worse than a failure, because it runs. The run checks that failure on
purpose — asking for `"dragon"` — and reports `engine: table: "dragon" ->
unknown`. The same shape as every load in this engine: the answer or the
name of what is missing.

### What this run verified, and what it did not

From a real run of this lesson's end state on this machine:

- **An entity created from a definition carries the table's values.**
  The run prints the definition's row and the entity side by side:
  `engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite
  assets/sprite.ppm` and `engine: entity hero: x 312 y 232 facing 0 speed
  240 health 3 sprite 16x16`. Field for field, the same values — with
  the sprite column's *name* become the art the game can draw (16×16,
  the file's own dimensions).
- **A definition the table does not hold is a typed failure.** The run's
  `"dragon"` request answers `unknown`, and the run's own `hero` request
  — made the same way — would end by name if the row were missing.
- **The definition keeps the file's values.** The entity is a copy; the
  row's values are what the file stated, before and after an entity is
  created from them (the exercise below watches exactly this).

What this lesson does **not** do is keep the entity anywhere. `hero` is
one local in the run: one entity, no room for a second, and nothing that
walks them. That is the next lesson's job — one fixed store of live
entities, so the game holds many of them without inventing storage per
feature — and nothing here moves yet either (lesson 076 gives the hero
its player).

## Code step

One change for this lesson, from definitions to entities: `src/entity.h`
/ `src/entity.cpp` grow `Entity` and `EntityFromDef` — the row copied
into a life, named fields the game reads and writes. `src/table.h` /
`src/table.cpp` grow `TableFind`, the typed lookup a game makes when it
wants a kind of entity, and `EntityDef` grows the one field the run
fills: the definition's image, loaded at startup from the sprite column's
file. `src/main.cpp` grows the run's startup accordingly — each
definition's art loaded and handed to its definition, the hero's entity
created from its row and reported field for field, and the unknown-name
request answered as a value. The demo's own sprite, the map, and the
loop are untouched. Its end state is tagged `lesson-073`.

```diff
diff --git a/src/entity.cpp b/src/entity.cpp
new file mode 100644
index 0000000..3bef96c
--- /dev/null
+++ b/src/entity.cpp
@@ -0,0 +1,26 @@
+// entity.cpp — entities from definitions: the row, copied into a life.
+//
+// Lesson 073: creating an entity is a copy. The definition keeps the
+// values the file stated — every entity of a kind starts from the same
+// row — and the entity carries its own, which the game is then free to
+// change.
+
+#include "entity.h"
+
+namespace engine {
+
+Entity EntityFromDef(const EntityDef &def)
+{
+    Entity entity = {};
+    for (int i = 0; i < TABLE_NAME_MAX; ++i)
+        entity.name[i] = def.name[i];
+    entity.x = def.x;
+    entity.y = def.y;
+    entity.facing = def.facing;
+    entity.speed = def.speed;
+    entity.health = def.health;
+    entity.sprite = def.image;
+    return entity;
+}
+
+} /* namespace engine */
diff --git a/src/entity.h b/src/entity.h
new file mode 100644
index 0000000..edcb991
--- /dev/null
+++ b/src/entity.h
@@ -0,0 +1,35 @@
+// entity.h — a live entity: the facts the game acts on.
+//
+// Lesson 073: an entity is a row the game created from a definition. It
+// carries that definition's identity and attributes in named fields the
+// game reads directly — and writes directly. The hero's position changes
+// every frame and its health changes when it is hit; what the game writes
+// is the entity's own copy of the facts, never the table's row. The
+// definition is where an entity comes from, not what it lives in.
+#ifndef ENTITY_H
+#define ENTITY_H
+
+#include "sprite.h"
+#include "table.h"
+
+namespace engine {
+
+/* One entity: the facts the game acts on, one struct of named fields.
+   No key/value bag, no lookup by string — the game reads entity.speed
+   and writes entity.x like any other values. */
+struct Entity {
+    char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
+    int x, y;                  /* where it is, in world pixels */
+    int facing;                /* 0 right, 1 down, 2 left, 3 up */
+    int speed;                 /* world pixels per second */
+    int health;                /* points */
+    const Sprite *sprite;      /* the art it draws, from its row */
+};
+
+/* An entity created from a definition: every attribute its row states,
+   answered from the definition alone. */
+Entity EntityFromDef(const EntityDef &def);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 3f07b8b..abb8e6e 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -15,6 +15,7 @@
 #include "audio.h"
 #include "blit.h"
 #include "camera.h"
+#include "entity.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
@@ -201,6 +202,54 @@ int Run(void)
                     def.sprite);
     }
 
+    /* Lesson 073: the definitions' art, loaded at startup. The table's
+       sprite column names the file; the run loads each one and hands the
+       definition its image, so an entity created from a definition is
+       answered from the definition alone. */
+    Sprite *images = (Sprite *)ArenaAlloc(
+        arena, (size_t)table.count * sizeof(Sprite), 4);
+    if (!images) {
+        std::fprintf(stderr, "engine: no room for the definitions' art\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    for (int i = 0; i < table.count; ++i) {
+        EntityDef &def = table.rows[i];
+        SpriteResult art = LoadSprite(arena, def.sprite);
+        if (art.error != SPRITE_OK) {
+            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
+            platform::CloseWindow(opened.window);
+            ArenaRelease(arena);
+            return 1;
+        }
+        images[i] = art.sprite;
+        def.image = &images[i];
+    }
+
+    /* Lesson 073: the game's first entity — created from the hero's
+       definition, carrying the values its row states in named fields the
+       game reads directly. */
+    DefResult hero_def = TableFind(table, "hero");
+    if (hero_def.error != DEF_OK) {
+        std::fprintf(stderr,
+                     "engine: assets/entities.txt: no definition named \"hero\"\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Entity hero = EntityFromDef(*hero_def.def);
+    std::printf("engine: entity %s: x %d y %d facing %d speed %d health %d sprite %dx%d\n",
+                hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
+                hero.sprite->width, hero.sprite->height);
+
+    /* The lookup's typed failure, checked on purpose: a definition the
+       table does not hold is a value — never an entity with assumed
+       attributes. */
+    DefResult unknown = TableFind(table, "dragon");
+    std::printf("engine: table: \"dragon\" -> %s\n",
+                unknown.error == DEF_OK ? "found" : "unknown");
+
     /* Lesson 066: the run's two sounds as files' bytes — the music that
        loops and the effect that plays once. Lesson 061's tone leaves the
        run here (it stays on disk: the file lessons 059-065 were built
diff --git a/src/table.cpp b/src/table.cpp
index c589b8d..ec1b59e 100644
--- a/src/table.cpp
+++ b/src/table.cpp
@@ -228,6 +228,7 @@ TableResult LoadTable(Arena &arena, const char *path)
         }
 
         EntityDef &def = defs[result.table.count];
+        def.image = 0; /* the run hands the definition its art, not the file */
         at = 0;
         for (int i = 0; ok && i < COL_COUNT; ++i) {
             switch (order[i]) {
@@ -289,4 +290,16 @@ TableResult LoadTable(Arena &arena, const char *path)
     return result;
 }
 
+DefResult TableFind(const EntityTable &table, const char *name)
+{
+    DefResult result = { 0, DEF_OK };
+    for (int i = 0; i < table.count; ++i)
+        if (SameText(table.rows[i].name, name)) {
+            result.def = &table.rows[i];
+            return result;
+        }
+    result.error = DEF_UNKNOWN;
+    return result;
+}
+
 } /* namespace engine */
diff --git a/src/table.h b/src/table.h
index c570c17..fae166d 100644
--- a/src/table.h
+++ b/src/table.h
@@ -19,6 +19,7 @@
 #define TABLE_H
 
 #include "arena.h"
+#include "sprite.h"
 
 namespace engine {
 
@@ -31,7 +32,11 @@ constexpr int TABLE_NAME_MAX = 16;
 constexpr int TABLE_PATH_MAX = 64;
 
 /* One definition: a row of the table, carrying every value its row
-   states — the identity and the attributes an entity is created from. */
+   states — the identity and the attributes an entity is created from.
+   The image is the one fact the file states as a name: the run loads the
+   art the sprite column names and hands the definition its image, so an
+   entity created from the definition is answered from the definition
+   alone. */
 struct EntityDef {
     char name[TABLE_NAME_MAX];   /* the definition's identity */
     int x, y;                    /* where it starts, in world pixels */
@@ -39,6 +44,7 @@ struct EntityDef {
     int speed;                   /* world pixels per second */
     int health;                  /* points */
     char sprite[TABLE_PATH_MAX]; /* the art file it draws */
+    const Sprite *image;         /* that art, loaded at startup */
 };
 
 /* A loaded table: one definition per row, in the arena — as many rows as
@@ -72,6 +78,24 @@ struct TableResult {
    to it: a load that refuses leaves nothing behind. */
 TableResult LoadTable(Arena &arena, const char *path);
 
+/* Lesson 073: a definition lookup — the request the game makes when it
+   wants an entity of a kind. The table either answers with the
+   definition or names what is missing: a definition the table does not
+   hold is a typed failure, never a row with assumed attributes. */
+enum DefError {
+    DEF_OK = 0,
+    DEF_UNKNOWN, /* the table holds no definition by that name */
+};
+
+struct DefResult {
+    const EntityDef *def; /* the definition, or 0 */
+    DefError error;       /* DEF_OK exactly when def is non-0 */
+};
+
+/* The definition named `name`, or the failure that says the table does
+   not hold one. */
+DefResult TableFind(const EntityTable &table, const char *name);
+
 } /* namespace engine */
 
 #endif
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Two heroes, one row *(predict-the-output)*

Create a second entity from the same `hero` definition beside the first
one. Then, before you report anything, change the *first* entity in the
run: set its `health` to 0 and its `x` to 0. Before running anything,
write down what the report will say for the second entity's `x` and
`health`, and what the *definition's* `x` and `health` are afterwards —
and one sentence on why. Run it and reconcile.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-073/ex1.md)

### Exercise 2 — Every definition, an entity *(extend-the-code)*

One entity per definition is the game's natural shape: the table lists
the kinds, the run brings one of each to life. Make the run create an
entity from *every* definition in the table — reporting each one's fields
the way the hero's are reported — instead of asking for `hero` by name.
Then add a third row to `assets/entities.txt` (your own kind: a `bat`, a
`turret`, whatever your game wants) and run again **without rebuilding**.
What appears in the report, and what in the run's code changed to make it
appear? Report both runs.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-073/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 072 — the load, complete or named](lesson-072-load.md) ·
**Next:** [Lesson 074 — one fixed store](lesson-074-store.md) ·
**Code tag:** [`lesson-073`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-073)
