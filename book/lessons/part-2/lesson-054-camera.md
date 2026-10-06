# Lesson 054 — the camera

{{#include ../../stability-horizon.md}}

## Prose

The world is bigger than the window — lesson 053 made sure of that. So
the view must move over it: this lesson delivers the MVD's second
obligation, **camera offsets**, as one struct with two offsets and one
rule. The base offset scrolls the world (the game's intent: where the
player is looking). The additive offset stacks on top of it and is the
hook the juice toolkit will drive for screenshake — zero at rest,
always. O2 is a contract more than a feature: the engine must be able to
move everything on screen *without the game's view moving at all*, and
that distinction is exactly what this lesson builds and checks.

### One struct, two offsets, one rule

```c++
struct Camera {
    int base_x, base_y; /* where the view sits over the world */
    int add_x, add_y;   /* the juice hook: added on top, zero at rest */
};
```

and the rule lives in one function, so no draw ever sums the two its own
way:

```c++
int CameraX(const Camera &camera) { return camera.base_x + camera.add_x; }
int CameraY(const Camera &camera) { return camera.base_y + camera.add_y; }
```

Every scene draw uses the sum **once, at its origin**:

```c++
    int cam_x = CameraX(camera);
    int cam_y = CameraY(camera);
    DrawTileMap(*fb, map, sheet, -cam_x, -cam_y);
    BlitSprite(*fb, sprite, (int)sprite_x - cam_x, (int)sprite_y - cam_y);
```

World position minus the offset — the rule lesson 053's walk already
follows, now fed by the camera. Note who is *not* in the list: the HUD.
`DrawText` draws in screen space, on purpose. The world scrolls; the
player's score line belongs to the player and does not wander off with
the scenery. "Scene drawing" means the world — the map and what lives in
it.

One consequence runs through the code: **the sprite now lives in world
space.** Its clamp moved from the screen's bounds to the map's (`0` to
`map_width × TILE_SIZE − sprite_width`) — the hero walks the world, not
the window, and the camera decides what the window shows.

### The three scenarios, checked

The spec behind this lesson names three behaviors; the check block turns
each into a pixel comparison — draw the scene through one camera state,
draw it through another, hold the two up pixel by pixel:

```
engine: camera check: base (100,50) scrolls the scene — 232200 pixels compared, 0 mismatches
engine: camera check: additive (7,-3) stacks over base — 307200 pixels compared, 0 mismatches
engine: camera check: additive cleared restores the base view — 307200 pixels compared, 0 mismatches
```

- **The base camera scrolls the scene** — the view at base (100, 50) is
  the view at (0, 0) with every pixel moved by exactly (−100, −50):
  232,200 overlapping pixels compared, all identical.
- **The additive offset stacks** — base (100, 50) with additive (7, −3)
  draws *exactly* what base (107, 47) draws with no additive: the sum is
  the only thing the draws see, verified across the whole frame.
- **Clearing the additive restores the base view** — the scene after the
  additive returns to zero is byte-identical to the scene at the base
  alone. Not "close enough": the same pixels, 307,200 of them.

### The hook, driven

The demo makes both offsets move. The camera's base follows the sprite —
clamped to the map, so the view stops at the world's edges — and the
sprite walks the world while the window follows:

```
engine: arrow keys move the sprite, space shakes the camera; close the window to stop
engine: sprite at 312,232
engine: camera base 0,0 (t=0.000)
engine: camera base 128,0 (t=2.473)
engine: sprite at 471,232 (t=2.473)
...
```

`camera base 128,0` is the clamp doing its job: the world is 768 pixels
wide and the frame is 640, so `base_x` tops out at 128 — the view shows
the world's right edge and no further.

The additive hook is on the space key:

```
engine: camera additive 6,0 (shake starts)
...
engine: camera additive 0,0 (at rest)
```

This is a *demonstration* of the hook, not the toolkit: a fixed shake
that ends at zero. Part 4's juice toolkit owns the real thing — hitstop,
screenshake, particle bursts, easing — and it will drive exactly this
field. The contract it needs is what this lesson delivers: the additive
can move every pixel on screen without disturbing the base, and clearing
it restores the view exactly.

### Why two offsets at all

The alternative — one offset, with the shake added into the base and
subtracted back out — works until the two things disagree: the game
wants to look at the player while the shake wants to look at nothing in
particular. Separate fields mean the game's intent (the base) is never
corrupted by feedback (the additive), and a shake of any size — or a
hitstop's screen nudge, or an easing bounce — composes over any view
without the world logic knowing it exists. The additive is a hook *for
the renderer*; the base is a statement *about the game*.

## Code step

One change for this lesson: `src/camera.h` / `src/camera.cpp` bring the
camera (the struct and the sum), and `main.cpp` draws the scene through
it (`DrawScene` — map and sprite at world position minus the summed
offset), checks the spec's three scenarios pixel by pixel, follows the
sprite with the base camera, and drives the additive hook from the space
key. The sprite's clamp moves from screen bounds to the map's. The HUD,
the font, and the blitter are untouched. Its end state is tagged
`lesson-054`.

```diff
diff --git a/src/camera.cpp b/src/camera.cpp
new file mode 100644
index 0000000..b17626b
--- /dev/null
+++ b/src/camera.cpp
@@ -0,0 +1,20 @@
+// camera.cpp — the sum.
+//
+// Lesson 054: base plus additive, in one place, so no draw ever sums the
+// two its own way.
+
+#include "camera.h"
+
+namespace engine {
+
+int CameraX(const Camera &camera)
+{
+    return camera.base_x + camera.add_x;
+}
+
+int CameraY(const Camera &camera)
+{
+    return camera.base_y + camera.add_y;
+}
+
+} /* namespace engine */
diff --git a/src/camera.h b/src/camera.h
new file mode 100644
index 0000000..9736cbb
--- /dev/null
+++ b/src/camera.h
@@ -0,0 +1,25 @@
+// camera.h — the camera: one struct, summed at draw time.
+//
+// Lesson 054: the camera is two offsets and one rule. The base scrolls
+// the world (the view's origin over the map); the additive offset is the
+// hook the juice toolkit will drive for screenshake (O2) — zero at rest.
+// Every scene draw uses the sum, once, at its origin. The HUD is not
+// scene: text on screen belongs to the player and does not move with the
+// world.
+#ifndef CAMERA_H
+#define CAMERA_H
+
+namespace engine {
+
+struct Camera {
+    int base_x, base_y; /* where the view sits over the world */
+    int add_x, add_y;   /* the juice hook: added on top, zero at rest */
+};
+
+/* The summed offset every scene draw applies to its origin. */
+int CameraX(const Camera &camera);
+int CameraY(const Camera &camera);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 0e77458..3c0c5bf 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -9,6 +9,7 @@
 
 #include "arena.h"
 #include "blit.h"
+#include "camera.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
@@ -30,6 +31,57 @@ constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
    layout loop. */
 constexpr char HUD_LABEL[] = "SCORE";
 
+/* Lesson 054: the scene, drawn through the camera. The camera's summed
+   offset is applied once, at each draw's origin — the map's and the
+   sprite's. The HUD is not scene and does not pass through here. */
+static void DrawScene(Framebuffer &fb, const TileMap &map,
+                      const TileSheet &sheet, const Sprite &sprite,
+                      int sprite_x, int sprite_y, const Camera &camera)
+{
+    int x = CameraX(camera);
+    int y = CameraY(camera);
+    DrawTileMap(fb, map, sheet, -x, -y);
+    BlitSprite(fb, sprite, sprite_x - x, sprite_y - y);
+}
+
+/* The framebuffer's whole content, copied out — the check's reference. */
+static void Snapshot(Framebuffer &fb, unsigned char *snap)
+{
+    for (int y = 0; y < FRAME_HEIGHT; ++y)
+        for (int x = 0; x < FRAME_WIDTH; ++x) {
+            unsigned char r, g, b;
+            GetPixel(fb, x, y, r, g, b);
+            unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
+            p[0] = r;
+            p[1] = g;
+            p[2] = b;
+        }
+}
+
+/* Compares the framebuffer against a snapshot shifted by (dx, dy): the
+   pixel at (x, y) now must be the snapshot's pixel at (x + dx, y + dy). */
+static void CompareShift(Framebuffer &fb, const unsigned char *snap, int dx,
+                         int dy, int &compared, int &mismatches)
+{
+    compared = 0;
+    mismatches = 0;
+    for (int y = 0; y < FRAME_HEIGHT; ++y) {
+        if (y + dy < 0 || y + dy >= FRAME_HEIGHT)
+            continue;
+        for (int x = 0; x < FRAME_WIDTH; ++x) {
+            if (x + dx < 0 || x + dx >= FRAME_WIDTH)
+                continue;
+            unsigned char r, g, b;
+            GetPixel(fb, x, y, r, g, b);
+            const unsigned char *p =
+                &snap[(((y + dy) * FRAME_WIDTH) + (x + dx)) * 3];
+            ++compared;
+            if (r != p[0] || g != p[1] || b != p[2])
+                ++mismatches;
+        }
+    }
+}
+
 /* Lesson 047: the caches deep dive's evidence — a copy walk over arena
    memory at two strides, timed at working-set sizes that cross this
    machine's caches. The walk is the blit's inner copy with the
@@ -449,13 +501,61 @@ int Run(void)
                 map.width * TILE_SIZE, map.height * TILE_SIZE, FRAME_WIDTH,
                 FRAME_HEIGHT, compared, moved_mismatches);
 
+    /* Lesson 054: the camera's three claims, each checked against the
+       framebuffer's pixels: the base scrolls the scene, the additive
+       offset stacks over it, and clearing the additive restores the
+       base view exactly. */
+    unsigned char *snap2 = (unsigned char *)ArenaAlloc(
+        arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 3, 4);
+    Camera camera = { 0, 0, 0, 0 };
+    if (!snap2) {
+        std::fprintf(stderr, "engine: no room for the camera check\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
+    Snapshot(*fb, snap);
+
+    camera.base_x = 100;
+    camera.base_y = 50;
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
+    int cam_cmp = 0, cam_bad = 0;
+    CompareShift(*fb, snap, 100, 50, cam_cmp, cam_bad);
+    std::printf("engine: camera check: base (100,50) scrolls the scene — %d pixels compared, %d mismatches\n",
+                cam_cmp, cam_bad);
+    Snapshot(*fb, snap2); /* the base view, for the restore check below */
+
+    camera.add_x = 7;
+    camera.add_y = -3;
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawScene(*fb, map, sheet, sprite, 200, 150, camera); /* base + add */
+    Snapshot(*fb, snap);
+    Camera summed = { 107, 47, 0, 0 }; /* the same sum, written out */
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawScene(*fb, map, sheet, sprite, 200, 150, summed);
+    CompareShift(*fb, snap, 0, 0, cam_cmp, cam_bad);
+    std::printf("engine: camera check: additive (7,-3) stacks over base — %d pixels compared, %d mismatches\n",
+                cam_cmp, cam_bad);
+
+    camera.add_x = 0;
+    camera.add_y = 0;
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
+    CompareShift(*fb, snap2, 0, 0, cam_cmp, cam_bad);
+    std::printf("engine: camera check: additive cleared restores the base view — %d pixels compared, %d mismatches\n",
+                cam_cmp, cam_bad);
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
     double last = started;
+    int shake_frames = 0; /* lesson 054: the additive hook's demo */
 
     std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
-    std::printf("engine: arrow keys move the sprite; close the window to stop\n");
+    std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
     /* The frame step: read news, update from polled state, draw, present —
@@ -487,15 +587,54 @@ int Run(void)
         if (platform::KeyDown(opened.window, platform::KEY_DOWN))
             sprite_y += SPRITE_SPEED * dt;
 
-        /* The sprite stays on screen — lesson 015's fold at frame scale. */
+        /* The sprite stays in the world — the map's bounds now, not the
+           screen's: the camera moves the view, the world is bigger. */
         if (sprite_x < 0)
             sprite_x = 0;
-        if (sprite_x > FRAME_WIDTH - sprite.width)
-            sprite_x = FRAME_WIDTH - sprite.width;
+        if (sprite_x > map.width * TILE_SIZE - sprite.width)
+            sprite_x = map.width * TILE_SIZE - sprite.width;
         if (sprite_y < 0)
             sprite_y = 0;
-        if (sprite_y > FRAME_HEIGHT - sprite.height)
-            sprite_y = FRAME_HEIGHT - sprite.height;
+        if (sprite_y > map.height * TILE_SIZE - sprite.height)
+            sprite_y = map.height * TILE_SIZE - sprite.height;
+
+        /* Lesson 054: the camera's base follows the sprite — the world
+           scrolls under the movement — clamped to the map's bounds. */
+        int base_x = (int)sprite_x + sprite.width / 2 - FRAME_WIDTH / 2;
+        int base_y = (int)sprite_y + sprite.height / 2 - FRAME_HEIGHT / 2;
+        if (base_x < 0)
+            base_x = 0;
+        if (base_y < 0)
+            base_y = 0;
+        if (base_x > map.width * TILE_SIZE - FRAME_WIDTH)
+            base_x = map.width * TILE_SIZE - FRAME_WIDTH;
+        if (base_y > map.height * TILE_SIZE - FRAME_HEIGHT)
+            base_y = map.height * TILE_SIZE - FRAME_HEIGHT;
+        if (base_x != camera.base_x || base_y != camera.base_y) {
+            camera.base_x = base_x;
+            camera.base_y = base_y;
+            std::printf("engine: camera base %d,%d (t=%.3f)\n", base_x,
+                        base_y, platform::Now() - started);
+        }
+
+        /* The additive offset: the hook the juice toolkit will drive.
+           Here SPACE demonstrates it — a shake that ends at zero, which
+           is where it lives at rest. */
+        if (platform::KeyPressed(opened.window, platform::KEY_SPACE) &&
+            shake_frames <= 0) {
+            shake_frames = 30;
+            std::printf("engine: camera additive 6,0 (shake starts)\n");
+        }
+        if (shake_frames > 0) {
+            --shake_frames;
+            camera.add_x = (shake_frames % 2) ? 6 : -6;
+            camera.add_y = 0;
+            if (shake_frames == 0) {
+                camera.add_x = 0;
+                camera.add_y = 0;
+                std::printf("engine: camera additive 0,0 (at rest)\n");
+            }
+        }
 
         frame.update = platform::Now() - t0;
         double t1 = platform::Now();
@@ -505,14 +644,17 @@ int Run(void)
                         (int)sprite_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then the
-           map, the sprite, and the text, each timed as its own named
-           phase: the subsystems the frame record can name. */
+           world through the camera, then the HUD — each timed as its own
+           named phase: the subsystems the frame record can name. The
+           camera's summed offset is applied once, at each draw's origin. */
+        int cam_x = CameraX(camera);
+        int cam_y = CameraY(camera);
         ClearBuffer(*fb, 32, 32, 64);
         double t_tilemap = platform::Now();
-        DrawTileMap(*fb, map, sheet, 0, 0);
+        DrawTileMap(*fb, map, sheet, -cam_x, -cam_y);
         frame.tilemap = platform::Now() - t_tilemap;
         double t_sprites = platform::Now();
-        BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
+        BlitSprite(*fb, sprite, (int)sprite_x - cam_x, (int)sprite_y - cam_y);
         frame.sprites = platform::Now() - t_sprites;
         double t_text = platform::Now();
         DrawText(*fb, font, HUD_LABEL, 8, 8);
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The shake that decays *(extend-the-code)*

The demo's shake is a square wave: ±6 for thirty frames, then zero. Make
it decay — the amplitude stepping down (6, 4, 2, 0) as the shake runs
out — and report every change of the additive as it happens. Verify the
path in the log and that the run ends at exactly `0,0`. What does
"decaying" buy the feel of the effect, and what does the *report* buy
the next person who reads your run?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-054/ex1.md)

### Exercise 2 — The additive that cancels *(predict-the-output)*

The sum is the only thing the draws see — so the additive can do things
to the base that look like camera work and are not. Before you run
anything, predict what the scene looks like with base `(100, 50)` and
additive `(−100, −50)`, and say which lesson-045 check this resembles.
Then add the case to the camera checks — draw it, compare it against the
origin view — and reconcile. The question to keep: if the juice toolkit
can cancel the base, what must it promise never to do?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-054/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 053 — tilemap drawing](lesson-053-tiles.md) ·
**Next:** [Lesson 055 — tile kinds and solidity](lesson-055-collision.md) ·
**Code tag:** [`lesson-054`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-054)
