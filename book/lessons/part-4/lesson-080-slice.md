# Lesson 080 — the vertical slice

{{#include ../../stability-horizon.md}}

## Prose

This is the lesson Part 4 has been walking toward, and it is deliberately
small: **a hero walks the tilemap with the camera following, using only
finished services.** Not a feature — a *gate*. The MVD names it L0\*,
and its rule is the whole lesson: the run introduces no new behavior, and
if it is not trivial, a service is missing and Part 4 is not done. Every
part so far proved its own subsystem. This one is the proof that they
compose.

### What the game is

The run is the game's shape. Read one frame of it and every line points
back at a lesson that was already finished:

| The frame does | The service | Since |
| -------------- | ----------- | ----- |
| the world is the table's rows, one entity per definition | the archetype table, `EntityCreate` | 071-074 |
| the player's intent is polled input state | the seam's keys | 032 |
| every live entity is walked once, in slot order | the store | 075 |
| each entity's request becomes motion through the mover | `MoveEntity` | 056, 077 |
| the step is `request × speed × dt` | the entity's fields, game time | 076-078 |
| the camera's base follows the hero, clamped to the map | the camera, the map's bounds | 052-054 |
| every entity is drawn at its position through the camera | the blit, the draw walk | 045, 076 |
| the record measures the phases and the step | the frame record | 036, 079 |

The startup is the same story: the table loads (complete or named), the
definitions' art loads and is handed to them, the game asks for its hero
by name and spawns the world from the same rows, and the knob is at
play. Two entities in the store — the hero and the enemy type the file
names — and a game that would grow by *rows*, not by code.

### What the slice invents: nothing

The code step for this lesson is mostly deletions. The demo scaffolding
that earlier lessons needed — the creation script that filled the store
to test the refusal, the kill check that made retirement visible, the
reuse script that proved slot order — is not the game's, and the slice
takes it out. What is left is the walk's per-entity work as the game
has it: the entity's step, resolved against the world. No per-kind
branches, no special case for the hero (the player's intent is written
to the hero's request before the walk, exactly like Part 5's AI will
write to its entities'), and no new engine function.

That is the gate's test, and it is worth being precise about what
"trivial" means here. The slice's *composition* is trivial: the frame is
the same frame lesson 077 left, with the same mover, the same walk, the
same camera. If today's code step had needed a new engine function — a
"camera follow" helper, a "spawn the hero" path, a movement special case
— that function would be the missing service, and the lesson would have
named it instead of shipping it. The closing review records this: the
slice invented **nothing**, and what it removed is the demo's scaffolding
(see `plan/part4-review.md`).

### What this run verified, and what it did not

From a real run of this lesson's end state on this machine — the
headless display, scripted input, and the arithmetic checked against the
run's own reports:

- **The slice runs.** `engine: part 4 done — the vertical slice: a hero
  walks the tilemap, the camera follows` — and the world it starts from
  is the table's: `engine: world: 2 entities from the table's rows, live
  2 of 64`.
- **The hero's position and the camera's base reconcile against the
  map's bounds.** Every `camera base` report in the run was checked
  against the hero's position at that frame through the clamp lesson 054
  defines — `clamp(hero + art/2 − frame/2, 0, map·TILE − frame)` — and
  every one agrees. The run's own numbers show both halves of the rule:
  `camera base 120,0`, `123,0` as the hero walks right (the view
  following), and `128,0` — the map's own scroll limit, `48·16 − 640` —
  where the clamp holds the view while the hero keeps walking. The y
  column clamps at `32` (`32·16 − 480`) the same way.
- **Every entity is walked and drawn once per frame.** The account:
  `engine: walk: 372 visits over 186 frames` — 2 entities × 186 frames,
  to the digit.
- **Game time is in the frame.** The step runs through `GameTimeStep`
  and the demo's script turns the knob (lesson 078's three settings) —
  the slice composes the service; it does not re-implement it.

What this lesson does **not** verify is whether the slice *feels* right
— that is not checkable headlessly, and the design routes it where it
belongs: your hands, on your machine (the exercise below). What *is*
checkable is that the hero walks, the camera follows, the numbers
reconcile, and nothing new was invented.

## Code step

One change for this lesson, from demo to game: `src/main.cpp` composes
the finished services into the slice. The world spawns from the table's
rows — the hero by name, the other kinds one entity per row — and the
walk's per-entity work is the entity's step, the demo's kill check and
lifetime scripts retired with the scaffolding they were. The identity
line names the gate this lesson closes. The engine's files are untouched
— `entity.*`, `table.*`, `gametime.*`, `frame.*`, and the platform seam
stand exactly as the earlier lessons left them. Its end state is tagged
`lesson-080`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 1904f32..edb88b1 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -239,24 +239,27 @@ int Run(void)
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
-    /* Lesson 074: the creation policy — the first free slot, and never
-       a live entity's. Lesson 075: the demo keeps a small world — eight
-       entities, three of them killed before the first walk — so the
-       store has free slots and the report can tell a freed slot from one
-       that has never been used. */
-    int created = 0;
-    while (store.live < 8) {
-        EntityResult made =
-            EntityCreate(store, table.rows[store.live % table.count]);
-        if (made.error != ENTITY_OK)
-            break;
+    /* Lesson 080: the vertical slice — the game's shape, and nothing
+       else. The hero is the row the game asks for by name (it is the
+       one the player controls); the world's other kinds come from the
+       same table, one entity per row. A new row is a new entity; the
+       run has no per-kind code to grow. */
+    int created = 1;
+    for (int i = 0; i < table.count; ++i) {
+        if (&table.rows[i] == hero_def.def)
+            continue;
+        EntityResult made = EntityCreate(store, table.rows[i]);
+        if (made.error != ENTITY_OK) {
+            std::fprintf(stderr, "engine: the store refused %s\n",
+                         table.rows[i].name);
+            platform::CloseWindow(opened.window);
+            ArenaRelease(arena);
+            return 1;
+        }
         created += 1;
     }
-    store.slots[2].health = 0;
-    store.slots[4].health = 0;
-    store.slots[6].health = 0;
-    std::printf("engine: store: live %d of %d — the hero and %d from the script; slots 2, 4, 6 killed\n",
-                store.live, ENTITY_CAP, created);
+    std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
+                created, store.live, ENTITY_CAP);
 
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
@@ -310,9 +313,10 @@ int Run(void)
     int scale_phase = 0;  /* lesson 078: the demo's script, by wall seconds */
     bool was_blocked = false; /* lesson 077: the mover's state report */
 
-    /* The demo's identity: what the run is, named at once — the hero,
-       an entity the game moves, over the world the map draws. */
-    std::printf("engine: the hero, as an entity — a row the game moves, a camera that follows\n");
+    /* The slice's identity: what the run is, named at once — L0*, the
+       gate this part closes on. Every service it uses was finished
+       before this lesson; the lesson is the fit. */
+    std::printf("engine: part 4 done — the vertical slice: a hero walks the tilemap, the camera follows\n");
     std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; hero %dx%d\n",
                 map.width, map.height, map.width * TILE_SIZE,
                 map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
@@ -437,26 +441,16 @@ int Run(void)
 
         /* Lesson 075: the walk — every live entity, once per frame, in
            slot order. The per-entity work is expressed here, once, and
-           not per type; lesson 076 makes it the entity's step — its
-           movement request becomes motion, and its facing follows where
-           it is going. An entity retired in passing is not visited again
-           and no other is skipped — the slots do not move under the
-           walk. */
+           not per type: the entity's step — its movement request becomes
+           motion through the mover, and its facing follows where it is
+           going. Lesson 080: the walk is the game's now, and its work is
+           the world's — no demo scaffolding, no per-kind branches. */
         int visited = 0;
         for (int i = 0; i < ENTITY_CAP; ++i) {
             if (!store.slots[i].live)
                 continue;
             visited += 1;
-            if (store.slots[i].health <= 0) {
-                std::printf("engine: walk (frame %ld): retiring slot %d in passing\n",
-                            frame.number, i);
-                EntityRetire(store, store.slots[i]);
-                continue;
-            }
             Entity &e = store.slots[i];
-
-            /* Lesson 077: the mover, on the entity — the request
-               becomes motion only where the map allows it. */
             MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
             if (e.move_x > 0.0)
                 e.facing = 0;
@@ -468,24 +462,6 @@ int Run(void)
                 e.facing = 3;
         }
         walk_visits += visited;
-        if (frame_number == 1)
-            std::printf("engine: walk (frame 1): visited %d live entities, once each, in slot order; live %d of %d\n",
-                        visited, store.live, ENTITY_CAP);
-
-        /* Lesson 075: the reuse — three requests once the walk has
-           freed three slots. Each lands in a freed slot, before any
-           slot that has never been used. */
-        if (frame_number == 2) {
-            for (int k = 0; k < 3; ++k) {
-                EntityResult made =
-                    EntityCreate(store, table.rows[k % table.count]);
-                if (made.error != ENTITY_OK)
-                    break;
-                std::printf("engine: store: created in slot %d (freed before never-used)\n",
-                            (int)(made.entity - store.slots));
-            }
-            std::printf("engine: store: live %d of %d\n", store.live, ENTITY_CAP);
-        }
 
         /* The score, and the hero's own report: where the entity the
            game moves has got to. */
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The camera at the map's edges *(predict-the-output)*

The camera's base is `clamp(hero + art/2 − frame/2, 0, map·TILE −
frame)` — and this map scrolls 128 pixels horizontally and 32
vertically. Before running anything, write down the base for each of
these hero positions: `0,0`; `400,232`; `734,480`; `312,232`; and
`734,232`. Which of your answers is the clamp talking and which is the
view following? Then run a scratch probe that sets the hero to each
position (and moves the camera the way the update does) and reconcile.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-080/ex1.md)

### Exercise 2 — The gate, answered *(explain-in-prose)*

The gate's question is "did the slice invent anything?" Answer it in
your own words, in two directions. First: name every line of the
slice's frame that is *not* a finished service — and if you find none,
say why that is evidence rather than politeness. Second: the
counterfactual — if the slice *had* needed one new piece of engine code
to fit (a helper, a special case, a new field), what would that tell
you about the service it touched, and what would you do about it before
Part 5 starts? To make the first half checkable, have the run name the
services it composes — one line per group, the way lesson 069's demo
named its capabilities — and fill the acceptance list beneath it.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-080/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 079 — measurement is not scaled](lesson-079-wall-clock.md) ·
**Next:** [Lesson 081 — the slice's cost in the frame budget](lesson-081-entities-row.md) ·
**Code tag:** [`lesson-080`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-080)
