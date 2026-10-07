# Lesson 083 — the tilemap and camera

{{#include ../../stability-horizon.md}}

## Prose

The skeleton has a shape now — five states, one at a time. But play's
screen is still the world the *loop* happens to draw. This lesson makes
it the game's: **the game's world is one scrolling map, and the game
looks at it through its own camera.** The map and the camera are not
new services — Part 2 built the tilemap and lesson 054 defined the
camera — but until now the loop owned them and moved them. From here
the game owns its world and its view of it (design D2): the camera
lives in `Game`, the game draws its map and its entities through it, and
the loop keeps only the timing and the frame record.

### The camera has two offsets, and the game owns them

The camera is one struct and two offsets (lesson 054):

```cpp
struct Camera {
    int base_x, base_y; /* where the view sits over the world */
    int add_x, add_y;   /* the juice hook: added on top, zero at rest */
};
```

The **base** is the view's origin over the map — it scrolls the world.
The **additive** offset is the hook the juice toolkit will drive for
screenshake (lesson 092); it rests at exactly zero. Every scene draw
uses the sum, once. The camera is `Game::camera` now — the game's
world-view — and two things happen to it each frame of play: the base
follows the hero, and the additive stays at rest.

### The base follows the hero, clamped to the map

The base aims to keep the hero centered: the hero's position, plus half
its sprite, minus half the frame. That is the view's origin — *if* it
is inside the map. It is clamped so the window never shows past the
world's edge:

```cpp
int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2;
if (base_x < 0) base_x = 0;
if (base_x > map.width * TILE_SIZE - FRAME_WIDTH)
    base_x = map.width * TILE_SIZE - FRAME_WIDTH;
```

The map is `48×32` cells of `TILE_SIZE` 16 — `768×512` pixels — and the
frame is `640×480`. So the view's origin can sit anywhere in
`x ∈ [0, 128]`, `y ∈ [0, 32]`: `768 − 640 = 128`, `512 − 480 = 32`.
Inside that rectangle the camera tracks the hero; at the edges it stops,
and the world's edge lines up with the window's.

From a real run of this lesson's end state, driven by scripted input
under the headless display — the hero walked right, then back left:

```
engine: hero at 312,232
engine: hero at 389,232 (t=3.792)
engine: camera base 77,0
engine: hero at 290,232 (t=9.224)
engine: camera base 0,0
```

and, in a run where the hero reached the far right:

```
engine: hero at 437,232 (t=3.548)
engine: camera base 125,0
engine: hero at 441,235 (t=3.562)
engine: camera base 128,3
```

Read the base against the arithmetic. At hero `389`, the base is
`389 + 8 − 320 = 77` — the camera is following, centered on the hero.
At hero `441`, `441 + 8 − 320 = 129`, but the base reads `128`: the
clamp held it at the map's right edge (`768 − 640`). And at hero `290`,
`290 + 8 − 320 = −22`, but the base reads `0`: clamped at the left edge.
The camera follows the hero wherever it goes and stops at the world's
bounds — it never shows blank space past the map.

### The map is drawn through the game's camera

The world's draw is the game's now: `GameDrawMap` draws the single
scrolling map and `GameDrawSprites` draws the live entities, both
through the game's camera (the summed offset, once). The loop no longer
reaches for the map or a camera of its own — it times the two draws as
the frame record's named sub-phases and moves on. The HUD stays over the
world and does *not* scroll (lesson 054's rule); it is drawn outside
the camera and is the next lessons' concern.

### What this run verified, and what it did not

- **The map draws** — the `tilemap` sub-phase costs real time in play
  (the world is rendered) and nothing on the panels.
- **The camera follows the hero** — the base tracks `hero + half sprite −
  half frame`, observed moving with the hero across the run.
- **The camera clamps to the map's bounds** — the base reads `0` and
  `128` where the arithmetic would go negative or past the edge: the
  view never shows past the world.

What this lesson does **not** do is resolve the hero against the map —
the hero still walks into and through walls, and the runs above show it
stopping in places it should not. That is tile collision, and it is the
next lesson. This lesson is only the view: the game owns its world and
the camera it looks through.

## Code step

One change: the camera becomes the game's, and the game draws its world
through it. `src/game.h` grows `Game::camera` and declares three things
the loop asks of the game — `GameFollow` (the base follows the hero,
clamped), and `GameDrawMap` / `GameDrawSprites` (the world through the
game's camera, split so the frame record keeps timing the two
sub-phases). `src/game.cpp` implements them — the follow's clamp moves
here from the loop, and the two draw loops move here too. `src/main.cpp`
drops its own `Camera`, the inline follow, and the inline world draw;
it calls the game's three functions and keeps only the timing. The
camera's additive offset rests at zero in `GameFollow`, the hook waiting
for lesson 092. Its end state is tagged `lesson-083`.

```diff
diff --git a/src/game.cpp b/src/game.cpp
index f66443a..4a6656f 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -11,7 +11,10 @@
 
 #include <cstdio>
 
+#include "blit.h"
 #include "text.h"
+#include "tilemap.h"
+#include "tiles.h"
 
 namespace engine {
 
@@ -57,6 +60,7 @@ void GameInit(Game &game, int hero_health_full)
     game.waves_remaining = GAME_WAVES;
     game.hero_health_full = hero_health_full;
     game.play_clock = 0.0;
+    game.camera = { 0, 0, 0, 0 };
     std::printf("engine: game: %d state%s, starting on %s\n", 5, "s",
                 GameStateName(game.state));
 }
@@ -190,4 +194,52 @@ void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font)
     }
 }
 
+void GameFollow(Game &game, const Entity &hero, const TileMap &map)
+{
+    /* Lesson 083: the camera's base follows the hero — the world scrolls
+       under the movement — clamped to the map's bounds so the view never
+       shows past the world's edge. The base is the view's origin over the
+       map; the additive offset rests at exactly zero, the hook the juice
+       toolkit will drive (lesson 092). */
+    int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2;
+    int base_y = (int)hero.y + hero.sprite->height / 2 - FRAME_HEIGHT / 2;
+    if (base_x < 0)
+        base_x = 0;
+    if (base_y < 0)
+        base_y = 0;
+    if (base_x > map.width * TILE_SIZE - FRAME_WIDTH)
+        base_x = map.width * TILE_SIZE - FRAME_WIDTH;
+    if (base_y > map.height * TILE_SIZE - FRAME_HEIGHT)
+        base_y = map.height * TILE_SIZE - FRAME_HEIGHT;
+    if (base_x != game.camera.base_x || base_y != game.camera.base_y) {
+        game.camera.base_x = base_x;
+        game.camera.base_y = base_y;
+        std::printf("engine: camera base %d,%d\n", base_x, base_y);
+    }
+    game.camera.add_x = 0;
+    game.camera.add_y = 0;
+}
+
+void GameDrawMap(const Game &game, Framebuffer &fb, const TileMap &map,
+                 const TileSheet &sheet)
+{
+    /* The single scrolling map, drawn through the game's camera — the
+       world is the game's, and this is the game drawing it. */
+    DrawTileMap(fb, map, sheet, -CameraX(game.camera), -CameraY(game.camera));
+}
+
+void GameDrawSprites(const Game &game, Framebuffer &fb,
+                     const EntityStore &store)
+{
+    /* Every live entity, its art at its position, through the camera's
+       summed offset — the draw walk, once, for the game's whole world. */
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        if (!store.slots[i].live)
+            continue;
+        const Entity &e = store.slots[i];
+        BlitSprite(fb, *e.sprite, (int)e.x - CameraX(game.camera),
+                   (int)e.y - CameraY(game.camera));
+    }
+}
+
 } /* namespace engine */
diff --git a/src/game.h b/src/game.h
index eccc9de..327b088 100644
--- a/src/game.h
+++ b/src/game.h
@@ -20,11 +20,13 @@
 #ifndef GAME_H
 #define GAME_H
 
+#include "camera.h"
 #include "entity.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "gametime.h"
 #include "platform.h"
+#include "tiles.h"
 
 namespace engine {
 
@@ -50,6 +52,10 @@ struct Game {
     int waves_remaining;  /* the named condition for victory */
     int hero_health_full; /* the health a fresh game starts the hero at */
     double play_clock;    /* wall seconds spent in play this game */
+    Camera camera;        /* lesson 083: the game's world-view — one
+                            camera over the single scrolling map. Its
+                            base follows the hero; its additive offset
+                            rests at zero (the juice hook, lesson 092). */
 };
 
 /* The game begins on the title screen. The hero's starting health is the
@@ -75,10 +81,25 @@ double GameScale(const Game &game);
 
 /* The current state's screen, for the four states whose screen is a
    panel over a still world — title, pause, death, victory. Play's screen
-   is the world the frame draws; the loop draws it and calls this for the
-   rest. */
+   is the world the game draws below; the loop draws it and calls this
+   for the rest. */
 void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font);
 
+/* Lesson 083: the game's world-view. The camera's base follows the hero
+   — the world scrolls under the movement — clamped to the map's bounds,
+   and its additive offset rests at exactly zero. The game owns the
+   camera now; the loop no longer keeps one. */
+void GameFollow(Game &game, const Entity &hero, const TileMap &map);
+
+/* The game's world, drawn through the game's camera: the single
+   scrolling map, and every live entity at its position. Split so the
+   frame record can time the map and the sprites as the two named
+   sub-phases it already has. */
+void GameDrawMap(const Game &game, Framebuffer &fb, const TileMap &map,
+                 const TileSheet &sheet);
+void GameDrawSprites(const Game &game, Framebuffer &fb,
+                     const EntityStore &store);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index 7f72983..60b4442 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -14,7 +14,6 @@
 #include "arena.h"
 #include "audio.h"
 #include "blit.h"
-#include "camera.h"
 #include "entity.h"
 #include "font.h"
 #include "framebuffer.h"
@@ -368,7 +367,6 @@ int Run(void)
     int exit_code = 0;
     long frame_number = 0;
     FrameStats stats = {};
-    Camera camera = { 0, 0, 0, 0 };
 
     /* Lesson 060: the run's own feeding schedule. The device consumes at
        the engine's rate, so the next buffer is due one horizon from the
@@ -460,31 +458,11 @@ int Run(void)
             std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
                         (int)hero.y, platform::Now() - started);
 
-        /* Lesson 054: the camera's base follows the hero — the world
-           scrolls under the movement — clamped to the map's bounds. */
-        int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2;
-        int base_y = (int)hero.y + hero.sprite->height / 2 - FRAME_HEIGHT / 2;
-        if (base_x < 0)
-            base_x = 0;
-        if (base_y < 0)
-            base_y = 0;
-        if (base_x > map.width * TILE_SIZE - FRAME_WIDTH)
-            base_x = map.width * TILE_SIZE - FRAME_WIDTH;
-        if (base_y > map.height * TILE_SIZE - FRAME_HEIGHT)
-            base_y = map.height * TILE_SIZE - FRAME_HEIGHT;
-        if (base_x != camera.base_x || base_y != camera.base_y) {
-            camera.base_x = base_x;
-            camera.base_y = base_y;
-            std::printf("engine: camera base %d,%d (t=%.3f)\n", base_x,
-                        base_y, platform::Now() - started);
-        }
-
-        /* Lesson 082: the camera's additive offset is the juice hook
-           lesson 054 defined, and the game skeleton keeps it at exactly
-           zero — the screenshake that will drive it arrives in lesson
-           092. At rest the sum every scene draw uses is the base alone. */
-        camera.add_x = 0;
-        camera.add_y = 0;
+        /* Lesson 083: the game's world-view — the camera's base follows
+           the hero, clamped to the map's bounds, and its additive offset
+           rests at exactly zero. The game owns the camera now (GameFollow,
+           in game.cpp); the loop keeps no camera of its own. */
+        GameFollow(game, hero, map);
 
         frame.update = platform::Now() - t0;
 
@@ -574,21 +552,15 @@ int Run(void)
         else
             ClearBuffer(*fb, 24, 24, 40);
         if (game.state == GAME_PLAY) {
+            /* Lesson 083: the game draws its own world — the scrolling
+               map and the live entities, through the game's camera. The
+               loop times the two the way it always has, as the frame
+               record's named sub-phases. */
             double t_tilemap = platform::Now();
-            DrawTileMap(*fb, map, sheet, -CameraX(camera), -CameraY(camera));
+            GameDrawMap(game, *fb, map, sheet);
             frame.tilemap = platform::Now() - t_tilemap;
             double t_sprites = platform::Now();
-
-            /* Lesson 076: the draw walk — every live entity, its art at its
-               position, through the camera's summed offset. Per-entity work
-               expressed once, in one loop, like the update's walk. */
-            for (int i = 0; i < ENTITY_CAP; ++i) {
-                if (!store.slots[i].live)
-                    continue;
-                const Entity &e = store.slots[i];
-                BlitSprite(*fb, *e.sprite, (int)e.x - CameraX(camera),
-                           (int)e.y - CameraY(camera));
-            }
+            GameDrawSprites(game, *fb, store);
             frame.sprites = platform::Now() - t_sprites;
             double t_text = platform::Now();
             char score_line[32];
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The camera that leads *(extend-the-code)*

Today the camera centers the hero, so the hero is always in the middle
of the screen and the player sees as much behind as ahead. Make the
camera *lead*: aim the base a little in the direction the hero is going,
so the player sees where they are heading. The hero's movement request
(`move_x`, `move_y`) or its facing tells you the direction; pick a
look-ahead distance in pixels and offset the base toward it. Keep the
clamp to the map's bounds (the camera must still never show past the
world) and keep the additive offset at rest. Then judge it by running:
does the hero sit off-center in the direction of travel, and does the
clamp still hold at the edges?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-083/ex1.md)

### Exercise 2 — The base, at the corners *(predict-the-output)*

The clamp is arithmetic, so it is predictable. Before you run anything,
work out the camera's base when the hero sits at each of the four
corners of the map (world pixels `(0,0)`, `(752,0)`, `(0,496)`,
`(752,496)` — the sprite's top-left at each corner) and when the hero is
at its starting position `(312,232)`. Remember the sprite is `16×16`,
the frame is `640×480`, and the map is `768×512`. Then write a tiny
probe (a print in `GameFollow` when the base changes) and drive the hero
toward each corner to check your five answers against the run. Which
corners clamp on both axes, and which only on one?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-083/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 082 — the game skeleton](lesson-082-skeleton.md) ·
**Next:** [the course home](../../index.md) ·
**Code tag:** [`lesson-083`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-083)
