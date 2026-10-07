# Lesson 084 — tile collision

{{#include ../../stability-horizon.md}}

## Prose

The world scrolls and the camera follows it. But the hero still walks
through walls like they were painted on. This lesson is about the other
half of movement: **the game resolves every entity's motion against the
tilemap — an entity stops at a solid tile and slides along a wall.**

The collision query is not new: lesson 055 gave the tilemap
`TileRectSolid` (is this rectangle over a solid tile — or off the map's
edge, which counts as solid?), and lesson 077 put it to work in the
mover, `MoveEntity`. What is new is that the *game* resolves its
movement through it — the walk is the game's now (`GameWalk`, in
`game.cpp`), and it turns every live entity's movement request into
motion against the map. The hero and every other entity resolve the same
way.

### One axis at a time — that is the slide

`MoveEntity` is small, and its whole trick is the order it works in:

```cpp
double next_x = entity.x + dx;
if (!TileRectSolid(map, (int)next_x, (int)entity.y, w, h))
    entity.x = next_x;              // the x step, on its own
double next_y = entity.y + dy;
if (!TileRectSolid(map, (int)entity.x, (int)next_y, w, h))
    entity.y = next_y;              // then the y step, on its own
```

It resolves **one axis at a time**. Take the x step alone: if the entity
would land on a solid tile moving in x, it simply does not move in x —
but that says nothing about y. Then take the y step alone, against the
entity's *original* x. If y is free, the entity moves in y even though x
was refused.

That is the slide. An entity pushed diagonally into a wall has one axis
refused and the other free — so it glides *along* the wall instead of
stopping dead. And an entity pushed into a corner has both axes refused
— so it stops. Stopping and sliding are not two behaviors; they are the
same one-axis rule at a corner versus at a wall.

### The hero and every entity, resolved alike

The walk (`GameWalk`) is the game's movement resolution: every live
entity, once per frame, its request `move_x` / `move_y` turned into
`dx` / `dy` through the mover. There is no per-kind collision code — the
hero and a walking slime go through the same `MoveEntity`. From a real
run of this lesson's end state, the hero driven up-and-left by scripted
input under the headless display:

```
engine: hero at 312,232
engine: hero at 210,232 (t=3.447)
engine: hero blocked at 206,228 (t=6.477)
```

Read the middle line: the hero moved from `312` to `210` in x while its
`y` stayed at `232` — **it slid left along a wall**, the y step refused
(the tile above was solid) while the x step was free. Then `hero blocked
at 206,228`: pushed where both axes meet a solid tile, it stopped — it
did not enter the wall. Stop and slide, both from the one-axis rule.

The non-hero entity in the run — the slime, given a walk (down-right) as
a stand-in for the AI lesson 089 brings — resolves through the same
`GameWalk` / `MoveEntity`: it slides along the walls it meets and stops
at the solid tiles it cannot pass. The walk reports one visit per live
entity per frame, and there is no branch that treats the hero any
differently.

### What this run verified, and what it did not

- **The hero stops at solid tiles** — `hero blocked at 206,228`: pushed
  against walls on both axes, it did not move into them.
- **The hero slides along walls** — `312 → 210` at a frozen `y = 232`:
  one axis refused, the other free, motion along the wall.
- **An entity resolves the same way** — the walk turns every entity's
  request into motion through the same mover; the walking slime slides
  and stops by the identical code.

What this lesson does **not** do is make the movement *feel* right — the
hero's velocity still jumps from zero to full in a step, and a very fast
hero on a very slow frame could in principle step a long way in one
frame. That is hero feel (lesson 085) and the mover's robustness; this
lesson is only the resolution: the map is solid, and the game treats it
that way.

## Code step

One change: the movement resolution becomes the game's. `src/game.h`
declares `GameWalk`; `src/game.cpp` implements it — the per-entity walk
that turns each live entity's movement request into motion through the
mover (`MoveEntity`), one axis at a time, with the facing following where
the entity is going. `src/main.cpp` drops its inline walk loop and calls
the game's `GameWalk` (still timed as the frame record's `entities`
sub-phase), and gives a non-hero entity a walk (down-right) at spawn — a
stand-in for the AI lesson 089 brings, so a non-hero entity is resolved
against the map here too. Its end state is tagged `lesson-084`.

```diff
diff --git a/src/game.cpp b/src/game.cpp
index a4371f7..0a78d42 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -243,4 +243,32 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
     }
 }
 
+int GameWalk(EntityStore &store, const TileMap &map, double dt)
+{
+    /* Lesson 084: the walk — every live entity, once per frame, in slot
+       order, its movement resolved against the tilemap. The per-entity
+       work is expressed once here, not per type: the mover (MoveEntity)
+       turns the request into motion one axis at a time, so an entity
+       that meets a solid tile stops on that axis and slides along the
+       wall on the other — and the facing follows where it is going.
+       The hero and every other entity resolve the same way. */
+    int visited = 0;
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        if (!store.slots[i].live)
+            continue;
+        visited += 1;
+        Entity &e = store.slots[i];
+        MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
+        if (e.move_x > 0.0)
+            e.facing = 0;
+        else if (e.move_y > 0.0)
+            e.facing = 1;
+        else if (e.move_x < 0.0)
+            e.facing = 2;
+        else if (e.move_y < 0.0)
+            e.facing = 3;
+    }
+    return visited;
+}
+
 } /* namespace engine */
diff --git a/src/game.h b/src/game.h
index 327b088..a031ef7 100644
--- a/src/game.h
+++ b/src/game.h
@@ -100,6 +100,12 @@ void GameDrawMap(const Game &game, Framebuffer &fb, const TileMap &map,
 void GameDrawSprites(const Game &game, Framebuffer &fb,
                      const EntityStore &store);
 
+/* Lesson 084: the walk — the game resolves every live entity's movement
+   against the tilemap. Each entity's movement request becomes motion
+   through the mover (MoveEntity), one axis at a time, so it stops at a
+   solid tile and slides along a wall. Returns the visit count. */
+int GameWalk(EntityStore &store, const TileMap &map, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index 60b4442..fda43b0 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -264,6 +264,12 @@ int Run(void)
             ArenaRelease(arena);
             return 1;
         }
+        /* Lesson 084: a non-hero entity walks (down-right) so its
+           movement is resolved against the map like the hero's — a stand
+           in for the AI lesson 089 brings. The walk slides it along
+           walls and stops it at solid tiles. */
+        made.entity->move_x = 1.0;
+        made.entity->move_y = 1.0;
         created += 1;
     }
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
@@ -413,29 +419,12 @@ int Run(void)
 
         double was_x = hero.x, was_y = hero.y;
 
-        /* Lesson 075: the walk — every live entity, once per frame, in
-           slot order. The per-entity work is expressed here, once, and
-           not per type: the entity's step — its movement request becomes
-           motion through the mover, and its facing follows where it is
-           going. Lesson 080: the walk is the game's now, and its work is
-           the world's — no demo scaffolding, no per-kind branches. */
-        int visited = 0;
+        /* Lesson 084: the game resolves its movement against its map —
+           the walk is the game's now (GameWalk, in game.cpp), turning
+           every live entity's request into motion through the mover.
+           The loop times it as the frame record's entity sub-phase. */
         double t_entities = platform::Now();
-        for (int i = 0; i < ENTITY_CAP; ++i) {
-            if (!store.slots[i].live)
-                continue;
-            visited += 1;
-            Entity &e = store.slots[i];
-            MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
-            if (e.move_x > 0.0)
-                e.facing = 0;
-            else if (e.move_y > 0.0)
-                e.facing = 1;
-            else if (e.move_x < 0.0)
-                e.facing = 2;
-            else if (e.move_y < 0.0)
-                e.facing = 3;
-        }
+        int visited = GameWalk(store, map, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The corner and the wall *(predict-the-output)*

`MoveEntity` resolves x first, then y. Use that to predict, before you
run anything, what the hero does in two cases: (a) driven diagonally
into a flat wall (say, up-and-left into a wall that blocks only x) —
does it stop or slide, and along which axis?; (b) driven diagonally into
a corner where both axes are solid — does it stop entirely, or does one
axis still win? Then write a tiny probe (print the `dx`, `dy` requested
and the `x`, `y` after the move) and drive the hero into a wall and into
a corner to check both predictions. Which axis wins in a corner, and
why does the order matter?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-084/ex1.md)

### Exercise 2 — No tunneling *(extend-the-code)*

A fast hero on a slow frame takes one big step — `speed × dt`, which on a
long frame can be tens of pixels. `MoveEntity` checks only the
destination of that step, so a big enough step could jump clean over a
thin wall (one tile wide, like the map's border) without ever landing on
a solid tile — the hero would tunnel through. Fix it: resolve a large
move in small sub-steps (say, at most a few pixels each), keeping the
one-axis rule in each, so the entity stops at the first solid tile it
would cross rather than skipping past it. Keep the slide working. Then
test the edge: raise the hero's speed in its table row and drive it at
the thin border wall — it should stop, not pass through.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-084/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 083 — the tilemap and camera](lesson-083-tilemap-camera.md) ·
**Next:** [the course home](../../index.md) ·
**Code tag:** [`lesson-084`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-084)
