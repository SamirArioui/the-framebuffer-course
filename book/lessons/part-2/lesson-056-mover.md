# Lesson 056 — the mover that stops at walls

{{#include ../../stability-horizon.md}}

## Prose

Two lessons ago the map started answering questions; today something
asks them: **the mover** — a sprite with intent, arrow keys, and a rule
that motion only happens where the map allows it. The hero of Part 4
starts here: this is the first piece of code where *wanting* to move and
*being able* to move are different things. The queries of lesson 055 are
the wall; the mover is the thing that stops.

### Intent, then permission

The update phase separates the two:

```c++
        double move_x = 0.0, move_y = 0.0;
        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
            move_x -= SPRITE_SPEED * dt;
        ... /* intent: where the player wants to go this frame */

        double next_x = sprite_x + move_x;
        if (!TileRectSolid(map, (int)next_x, (int)sprite_y, sprite.width,
                           sprite.height))
            sprite_x = next_x;
        double next_y = sprite_y + move_y;
        if (!TileRectSolid(map, (int)sprite_x, (int)next_y, sprite.width,
                           sprite.height))
            sprite_y = next_y;
```

Three decisions in those lines:

- **Intent is not motion.** The keys compute where the sprite *wants* to
  be; the query decides where it *gets* to be. A rejected move simply
  does not happen — the position stays, no slide, no bounce, no error.
- **One axis at a time.** The x move is tested and applied, then the y
  move. That is what makes wall-sliding work: pushing diagonally into a
  wall, the movement *along* it survives while the movement *into* it is
  refused. Test both axes together instead and a diagonal press against
  a wall sticks the player to it.
- **The rectangle is the sprite.** The query gets the sprite's size —
  its 16×16 box in world coordinates — so "can I be there?" is asked
  with the sprite's actual footprint, not its center point.

And with the queries live, the lesson-054 clamp retires: the map's
border is solid, the queries treat the world's edge as solid, so the
mover cannot leave the map and no separate bounds check is needed. The
boundary is data now.

### The run

Scripted input — hold Left into the border, then Up along the left
wall, then Right along the top:

```
engine: the sprite stops at walls (lesson 056's mover)
engine: sprite at 312,232
engine: camera base 0,0 (t=0.000)
engine: sprite at 27,232 (t=2.718)
engine: sprite at 18,232 (t=2.758)
engine: sprite blocked at 18,232 (t=2.799)
engine: sprite unblocked at 18,232 (t=3.552)
engine: sprite at 18,156 (t=3.867)
engine: sprite blocked at 18,156 (t=4.528)
engine: sprite unblocked at 18,146 (t=4.568)
engine: sprite at 18,146 (t=4.568)
engine: sprite at 18,136 (t=4.609)
...
engine: sprite at 18,21 (t=5.090)
engine: sprite blocked at 18,21 (t=5.130)
engine: sprite unblocked at 21,18 (t=5.383)
engine: sprite at 21,18 (t=5.383)
engine: sprite at 179,18 (t=6.044)
```

Read it against the map: the sprite starts on floor at world `(312,
232)`, walks left, and **stops at x = 18** — the border wall occupies
pixels 0..15 and the sprite's 16-wide box can never overlap them. The
report line `sprite blocked at 18,232` is the mover noticing it wanted
to move and did not. Then Up along the wall: the sprite climbs at
x = 18 — the same wall blocking its leftward half-step would have
taken — reaches the top wall (y stops at 21), and when Right is pressed
it **slides** along the top wall: `unblocked at 21,18` and onward to
`179,18`. Blocking one axis never blocks the other.

### The step that jumps

One pair of lines in that run is not about walls at all:

```
engine: sprite at 18,156 (t=3.867)
engine: sprite blocked at 18,156 (t=4.528)
engine: sprite unblocked at 18,146 (t=4.568)
```

The sprite was moving up, at 156 — nowhere near a wall — and the mover
reported *blocked*. Then it moved again, ten pixels at once. The cause
is lesson 034's old friend: **frames happen when news happens.** Look at
the timestamps: the frame at t=3.867 came 0.315 s after the one before
it, and the frame at t=4.528 came 0.66 s later. The mover moves
`speed × dt` per frame — so that single frame tried to move the sprite
**158 pixels in one step**, from y=156 down past the top of the map. The
query did its job: a 16×16 box at the destination is outside the map,
the edge is solid, the step is refused — *the whole step*, including the
140 pixels of it that were perfectly fine.

That is the honest behavior of the code as written, and it names the
mover's real limitation: **the step is all or nothing.** A long frame
does not move the player part-way to the wall; it moves them not at all,
and the next ordinary frame picks up from there. The fix — clamp `dt` to
a frame's worth, or move in fixed-size substeps until the query refuses —
is exercise 2's, and Part 4's hero will want it. The check to keep: the
report said *blocked* and the sprite truly did not move. The mover never
lies about what it did; it just does less than it could.

### What the mover is the beginning of

The rule of the mover — *want, ask, move or don't* — is the whole shape
of game logic in this engine. Part 4's hero adds acceleration and
knockback to the intent, the enemies add their own intents, and the
capstone adds resolution when two movers want the same cell; the asking
is always these queries, and the answers always come from map data. The
report transitions (`blocked` / `unblocked`) are the seed of the kind of
instrumented game logic the frame-budget report will one day account
for.

## Code step

One change for this lesson: `main.cpp` replaces the mover's clamp with
collision-gated motion — intent computed from the keys, the x and y
steps tested separately against `TileRectSolid`, and the `blocked` /
`unblocked` transitions reported. The map, the queries, and the drawing
path are untouched. Its end state is tagged `lesson-056`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 180c505..9bfc720 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -591,9 +591,11 @@ int Run(void)
     double started = platform::Now();
     double last = started;
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
+    bool was_blocked = false; /* lesson 056: the mover's state report */
 
     std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
+    std::printf("engine: the sprite stops at walls (lesson 056's mover)\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
     /* The frame step: read news, update from polled state, draw, present —
@@ -616,25 +618,42 @@ int Run(void)
         last = now;
 
         int old_x = (int)sprite_x, old_y = (int)sprite_y;
+        double was_x = sprite_x, was_y = sprite_y;
+        double move_x = 0.0, move_y = 0.0;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            sprite_x -= SPRITE_SPEED * dt;
+            move_x -= SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            sprite_x += SPRITE_SPEED * dt;
+            move_x += SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_UP))
-            sprite_y -= SPRITE_SPEED * dt;
+            move_y -= SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            sprite_y += SPRITE_SPEED * dt;
-
-        /* The sprite stays in the world — the map's bounds now, not the
-           screen's: the camera moves the view, the world is bigger. */
-        if (sprite_x < 0)
-            sprite_x = 0;
-        if (sprite_x > map.width * TILE_SIZE - sprite.width)
-            sprite_x = map.width * TILE_SIZE - sprite.width;
-        if (sprite_y < 0)
-            sprite_y = 0;
-        if (sprite_y > map.height * TILE_SIZE - sprite.height)
-            sprite_y = map.height * TILE_SIZE - sprite.height;
+            move_y += SPRITE_SPEED * dt;
+
+        /* Lesson 056: the mover — intent becomes motion only where the
+           map allows it. One axis at a time, so a wall blocks the
+           movement into it and the movement along it still works. The
+           clamp of lesson 054 retires: the world's edge is solid, and
+           the queries are the boundary now. */
+        double next_x = sprite_x + move_x;
+        if (!TileRectSolid(map, (int)next_x, (int)sprite_y, sprite.width,
+                           sprite.height))
+            sprite_x = next_x;
+        double next_y = sprite_y + move_y;
+        if (!TileRectSolid(map, (int)sprite_x, (int)next_y, sprite.width,
+                           sprite.height))
+            sprite_y = next_y;
+
+        /* The mover reports its state on transitions: moving, or pushed
+           against something that will not move. The comparison is on the
+           exact positions — a sub-pixel step is movement, not a wall. */
+        bool blocked = (move_x != 0.0 || move_y != 0.0) &&
+                       sprite_x == was_x && sprite_y == was_y;
+        if (blocked != was_blocked) {
+            std::printf("engine: sprite %s at %d,%d (t=%.3f)\n",
+                        blocked ? "blocked" : "unblocked", (int)sprite_x,
+                        (int)sprite_y, platform::Now() - started);
+            was_blocked = blocked;
+        }
 
         /* Lesson 054: the camera's base follows the sprite — the world
            scrolls under the movement — clamped to the map's bounds. */
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The hitbox *(extend-the-code)*

The sprite's art and its collision box are the same rectangle today.
Shrink the collision box two pixels on every side — the classic hitbox
inside the art — and drive the mover into the wall it could not reach
before. Read back the pixels where the art meets the wall and say what
the player sees, then answer the design question: why do games keep the
hitbox smaller than the art, and when is that the wrong choice?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-056/ex1.md)

### Exercise 2 — The step and the wall *(predict-the-output)*

The lesson's run contains a `blocked` report at y=156 with no wall in
sight. Before you change anything, write down what happened to that
frame — the timestamps in the log are the evidence — and predict what
the fractional position report (printed to two decimals) will show for
the ordinary case: driving left into the border wall, how close to x=16
does the sprite actually stop, and why does it not land exactly on the
boundary? Then extend the report to two decimals, re-run both cases,
and reconcile. The question to carry: what should the mover do about
steps too large to take?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-056/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 055 — tile kinds and solidity](lesson-055-collision.md) ·
**Next:** [Lesson 057 — the closing demo](lesson-057-demo.md) ·
**Code tag:** [`lesson-056`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-056)
