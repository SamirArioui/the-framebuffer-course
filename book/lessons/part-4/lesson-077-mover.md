# Lesson 077 — the mover on an entity

{{#include ../../stability-horizon.md}}

## Prose

Lesson 076 ended with a hero that moves and a world that does not
answer: the scripted hero walked through walls and off the map's edge,
and the lesson said so out loud. Motion without the world's opinion is
half a movement model. So this lesson's idea is the habit lesson 056
started, finally applied where it belongs: **the mover on an entity —
intent becomes motion only where the map allows it**, one function
every entity's movement goes through, so the hero stops at walls and
slides along them.

### One axis at a time

```cpp
void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy);
```

The whole mover is four lines of shape:

```cpp
double next_x = entity.x + dx;
if (!TileRectSolid(map, (int)next_x, (int)entity.y,
                   entity.sprite->width, entity.sprite->height))
    entity.x = next_x;
double next_y = entity.y + dy;
if (!TileRectSolid(map, (int)entity.x, (int)next_y,
                   entity.sprite->width, entity.sprite->height))
    entity.y = next_y;
```

The request arrives as a step in pixels; each axis is tried separately;
an axis whose step would put the entity in a solid tile simply does not
happen. That is the mover habit of lesson 056, unchanged — what is new
is that it is *one function* now, taking an entity, and the walk's
per-entity step is its call. Every entity resolves against the same
queries; the hero has no private mover.

Why one axis at a time is worth its own sentence: it is what makes the
**slide** work. A player pushing a hero diagonally into a wall expects
to move *along* the wall, not to stop dead — and with the axes tried
separately that is exactly what happens: the axis into the wall is
refused, the axis along it lands. A mover that tested the whole diagonal
at once would refuse both and the hero would stick at every wall corner.

The query the mover asks is `TileRectSolid` — lesson 055's rectangle
query, answered from the map's own data (the kinds' solidity), never
from drawing code. The rectangle is the entity's **art**: its sprite's
width and height at its position. That is deliberate and simple — what
an entity draws is what it collides as — and it is the same rectangle
the old demo sprite was checked with. The map's edge policy comes with
it (lesson 055 defined it): a rectangle outside the map counts as
solid, so the world's edge blocks like a wall and the query never reads
past the map's cells.

### The step, end to end

The walk's per-entity work is one call now:

```cpp
MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
```

read it right to left: the frame's request × the row's speed × the
frame's `dt` is the step the entity *wants*; the mover hands over the
step the world *allows*. Nothing else in the engine moves an entity —
the game writes the request, the walk calls the mover, the map has the
last word.

### What this run verified, and what it did not

From real runs of this lesson's end state on this machine, driven by
scripted input:

- **The hero stops at solid tiles.** Driving down the map's lower
  reach: `engine: hero at 706,480 (t=6.366)` and then
  `engine: hero blocked at 706,480 (t=6.406)` — the request kept
  coming and the position held. 480 + 16 reaches the border row, which
  is a `#` kind — solid in the map's own data — and the step that would
  enter it does not happen. (The mover's state report came back with
  the mover: `blocked` / `unblocked` on transitions, as lesson 056's
  did.)
- **The movement along the wall still does.** The same run's slide:
  `hero at 648,477`, `hero at 653,477`, `hero at 658,477`, then
  `hero blocked at 658,477` (the Down requests, refused at the bottom
  row) and `hero at 663,477`, `hero at 668,477` (the Right requests,
  landing): x advances while y holds. The hero slid along the wall
  instead of sticking to it — the one-axis shape, visible in two
  columns of a report.
- **Movement is free where nothing is solid.** The long runs in
  between: `hero at 419,232` … `hero at 706,232`, one step per pressed
  frame, nothing consulted but the map's open cells.

What this lesson does **not** change is anything about the map: the
queries are used, not altered (the modified-capabilities line of this
change's proposal). And nothing here is *game* behavior — the mover is
geometry. What a hero does when it hits a wall (turn, slide, get hurt)
is the game's; that it cannot walk through the wall is the mover's.

## Code step

One change for this lesson, from arithmetic to resolution:
`src/entity.h` / `src/entity.cpp` grow `MoveEntity` — the mover habit of
lesson 056 as one function every entity's motion goes through, one axis
at a time against the map's collision queries. `src/main.cpp` grows the
walk's per-entity step to call it (the bare arithmetic it replaces is
one line) and brings back the mover's state report — `blocked` /
`unblocked` on transitions — now about the hero's entity. The store, the
table, the camera, the sound, and the loop's measurements are untouched.
Its end state is tagged `lesson-077`.

```diff
diff --git a/src/entity.cpp b/src/entity.cpp
index 04ff1f3..d3c7d3b 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -56,4 +56,20 @@ void EntityRetire(EntityStore &store, Entity &entity)
     store.live -= 1;
 }
 
+void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy)
+{
+    /* One axis at a time: a wall blocks the movement into it and the
+       movement along it still works — the slide is this shape, not a
+       special case. The rectangle the map is asked about is the
+       entity's art: what it draws is what it collides as. */
+    double next_x = entity.x + dx;
+    if (!TileRectSolid(map, (int)next_x, (int)entity.y,
+                       entity.sprite->width, entity.sprite->height))
+        entity.x = next_x;
+    double next_y = entity.y + dy;
+    if (!TileRectSolid(map, (int)entity.x, (int)next_y,
+                       entity.sprite->width, entity.sprite->height))
+        entity.y = next_y;
+}
+
 } /* namespace engine */
diff --git a/src/entity.h b/src/entity.h
index bcda6b1..c0ec0e7 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -11,6 +11,7 @@
 
 #include "sprite.h"
 #include "table.h"
+#include "tilemap.h"
 
 namespace engine {
 
@@ -78,6 +79,15 @@ EntityResult EntityCreate(EntityStore &store, const EntityDef &def);
    been used. Retiring an entity that is already gone is nothing. */
 void EntityRetire(EntityStore &store, Entity &entity);
 
+/* Lesson 077: the mover, applied to entities — the habit lesson 056
+   started, as one function every entity's motion goes through. Intent
+   becomes motion only where the map allows it: the move that would put
+   the entity in a solid tile does not happen, and the movement along the
+   wall still does (one axis at a time, which is what makes the slide
+   work). Empty space is free: the entity arrives at the requested
+   position. */
+void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy);
+
 /* Lesson 075: the walk — the shape the game's per-entity work takes:
 
      for (int i = 0; i < ENTITY_CAP; ++i) {
diff --git a/src/main.cpp b/src/main.cpp
index 2bdcaa7..dbc83e7 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -305,6 +305,7 @@ int Run(void)
     double last = started;
     double distance = 0.0; /* the score: the world the hero has walked */
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
+    bool was_blocked = false; /* lesson 077: the mover's state report */
 
     /* The demo's identity: what the run is, named at once — the hero,
        an entity the game moves, over the world the map draws. */
@@ -415,8 +416,10 @@ int Run(void)
                 continue;
             }
             Entity &e = store.slots[i];
-            e.x += e.move_x * e.speed * dt;
-            e.y += e.move_y * e.speed * dt;
+
+            /* Lesson 077: the mover, on the entity — the request
+               becomes motion only where the map allows it. */
+            MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
             if (e.move_x > 0.0)
                 e.facing = 0;
             else if (e.move_y > 0.0)
@@ -450,6 +453,17 @@ int Run(void)
            game moves has got to. */
         distance += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
                     (hero.y > was_y ? hero.y - was_y : was_y - hero.y);
+
+        /* Lesson 077: the mover's state report, on transitions — the
+           hero moving, or pushed against something that will not move. */
+        bool blocked = (hero.move_x != 0.0 || hero.move_y != 0.0) &&
+                       hero.x == was_x && hero.y == was_y;
+        if (blocked != was_blocked) {
+            std::printf("engine: hero %s at %d,%d (t=%.3f)\n",
+                        blocked ? "blocked" : "unblocked", (int)hero.x,
+                        (int)hero.y, platform::Now() - started);
+            was_blocked = blocked;
+        }
         if ((int)hero.x != (int)was_x || (int)hero.y != (int)was_y)
             std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
                         (int)hero.y, platform::Now() - started);
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The corner, predicted *(predict-the-output)*

The mover tries one axis at a time, and the order is x first. Take the
hero in a corner: solid tiles to its right and below it, open space up
and left — and one frame whose request is **Down-Right** at a step that
would move it 20 pixels on each axis. Before running anything, write
down the hero's position after that frame and which of the two
components moved. Then write down the same two answers for a frame whose
request is **Down-Left** in the same corner, and for **Up-Right**. Run
all three (a scratch probe that calls `MoveEntity` directly is the
cleanest) and reconcile — and say in one sentence what the x-first order
means for a hero sliding around a corner.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-077/ex1.md)

### Exercise 2 — Which wall said no *(extend-the-code)*

`blocked` says the hero did not move; it does not say which axis the
world refused. Make the mover *report* its resolution — a typed result
saying which components of the request landed and which were refused —
and make the run's state report use it, so a hero pushed left into a
wall and a hero pushed down into the floor read differently. Verify
with scripted input against a wall on one side only. Why is a typed
answer worth more here than the position comparison the run uses now?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-077/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 076 — the hero as an entity](lesson-076-hero.md) ·
**Next:** [Lesson 078 — the game-time scale](lesson-078-game-time.md) ·
**Code tag:** [`lesson-077`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-077)
