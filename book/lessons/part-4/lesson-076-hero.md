# Lesson 076 — the hero as an entity

{{#include ../../stability-horizon.md}}

## Prose

Lesson 075 ended with a store the frame walks and nothing in it worth
walking: the demo's sprite still moved on its own `double`s beside the
store, and the entities sat in their slots being counted. Two things
about that are wrong, and this lesson fixes both at once: the hero is
not a sprite plus loose variables, and a frame's per-entity work is not
a kill check. So the idea here is: **the hero is the first entity the
game acts on** — a row carrying position, sprite, and a speed, moved by
polled input state, drawn through the camera. The demo's sprite retires;
the entity is what the game sees from now on.

### The row carries the motion

The hero's entity came from its row three lessons ago; what this lesson
changes is who writes to it. The entity's fields are the game's:

```cpp
struct Entity {
    char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
    double x, y;               /* where it is, in world pixels */
    int facing;                /* 0 right, 1 down, 2 left, 3 up */
    int speed;                 /* world pixels per second */
    int health;                /* points */
    const Sprite *sprite;      /* the art it draws, from its row */
    double move_x, move_y;     /* this frame's movement request */
    bool live;
};
```

Two fields deserve their say. **The position is a `double`** — the row
states whole pixels (`312`) because a file should, but motion is not
whole: a step of `240 px/s × 16 ms` is 3.84 pixels, and a position that
truncates every frame loses the fraction and moves the hero slower than
its row says. Pixels are integers; positions are not (lesson 046's
sprite moved the same way, and the draw rounds once). And **the
movement request is the entity's own** — `move_x`, `move_y`, the
direction this frame's work wants to go. The player's input writes it
for the hero; Part 5's AI writes it for enemies. Nothing writes `x` and
`y` directly except the walk.

### The intent is the game's; the step is the walk's

The update reads input once per frame — polled state, never events
(lesson 032) — and writes the hero's request:

```cpp
hero.move_x = 0.0;
hero.move_y = 0.0;
if (platform::KeyDown(opened.window, platform::KEY_LEFT))
    hero.move_x -= 1.0;
...
```

Then the walk turns every entity's request into motion, once, in one
body:

```cpp
e.x += e.move_x * e.speed * dt;
e.y += e.move_y * e.speed * dt;
```

That is the movement model, and it is deliberately plain: the request is
a direction, the row's `speed` is how fast, and the frame's `dt` (lesson
035) turns it into pixels. The hero's row says 240 — so holding a
direction moves it 240 pixels per second of game time, and nothing else
in the engine knows that number. Change the row and the hero changes
speed without a rebuild. The entity's `facing` follows where it is
going (0 right, 1 down, 2 left, 3 up — the format's four), so the fact
the game reads for animation or aim is never stale.

What the walk does *not* do yet is ask the world's permission. The
movement is arithmetic; the map is not consulted. That is deliberate and
it is the next lesson — but it is visible in today's run: the scripted
hero walked straight past the map's right edge, to x 872 in a world 768
pixels wide, while the camera — clamped to the map's bounds as lesson
054 defined — held at 128,32 and showed the void beyond the tiles. The
motion is the entity's; the world's answer to it is lesson 077's.

### Drawn through the camera

The render's sprite phase is a walk now, the mirror of the update's:

```cpp
for (int i = 0; i < ENTITY_CAP; ++i) {
    if (!store.slots[i].live)
        continue;
    const Entity &e = store.slots[i];
    BlitSprite(*fb, *e.sprite, (int)e.x - CameraX(camera),
               (int)e.y - CameraY(camera));
}
```

Every live entity's art at its position, through the camera's summed
offset — the same blit, the same camera, one loop instead of one call.
A new row in the table is a new entity, and it draws with no code
change. The camera's base follows the hero (its art's center, clamped to
the map's bounds) exactly as it followed the demo sprite — the camera
never knew it was following a sprite; it follows a position.

### What this run verified, and what it did not

From a real run of this lesson's end state on this machine, driven by
scripted input (`xdotool key --repeat`, twelve Rights and six Downs):

- **The hero moves under scripted input, at its row's speed.** The run
  reports `engine: hero at 312,232` at rest and then one line per moving
  frame: `engine: hero at 792,232 (t=2.004)` — the first pressed frame
  carried 481 pixels of motion (the row's 240 px/s × the frame's 2.004 s
  of idle `dt`) and the integer position reads 792 — then `800`, `807`,
  `814`, `821` … at the script's frame spacing, and `872,276` after the
  Down presses. The step is the row's speed times the frame's `dt`, every
  frame.
- **The camera follows and clamps.** `engine: camera base 128,8` …
  `128,32` as the hero went right and down: 128 is `map.width * TILE_SIZE
  - FRAME_WIDTH` and 32 is the height's, the map's own bounds — the
  camera reconciled against the world, not against the hero.
- **Every entity is walked and drawn once per frame.** The walk's
  account closes again: `engine: walk: 301 visits over 38 frames` —
  8 + 5 + 36 × 8, the live count of every frame, summed.

What this lesson does **not** verify is the world answering back: the
hero walks through walls and off the map, as documented above. Lesson
077 resolves every entity's movement against the tilemap's collision
queries, and the hero stops at walls and slides along them — the mover
habit of lesson 056, applied to entities.

One honest note on the numbers: on this machine the run has no audio
output, so the loop wakes on input news alone and each scripted press
wakes two frames (the press and the release). Only the frame that finds
the key down moves the hero, which is why the scripted pace steps 7-8
pixels a frame rather than 14. On a real desktop — or with a working
audio output ticking the loop — a held key moves the hero every frame,
and the exercise below asks you to measure exactly that.

## Code step

One change for this lesson, from sprite to entity: `src/entity.h` grows
the position the motion needs (`x`, `y` become doubles) and the frame's
movement request (`move_x`, `move_y`) — the fields the game writes and
the walk reads. `src/main.cpp` retires the demo's sprite and its loose
variables — `SPRITE_SPEED`, `sprite_x`, `sprite_y`, the inline mover —
and grows the run around the hero's entity: the player's intent from
polled input state, the walk's per-entity step (request × speed × dt,
facing following), the draw walk through the camera, and the camera
following the hero. The store, the table, the sound, and the loop's
measurements are untouched. Its end state is tagged `lesson-076`.

```diff
diff --git a/src/entity.h b/src/entity.h
index 785f868..bcda6b1 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -19,11 +19,18 @@ namespace engine {
    and writes entity.x like any other values. */
 struct Entity {
     char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
-    int x, y;                  /* where it is, in world pixels */
+    double x, y;               /* where it is, in world pixels — the
+                                  row's whole pixels, the motion's
+                                  doubles (lesson 076) */
     int facing;                /* 0 right, 1 down, 2 left, 3 up */
     int speed;                 /* world pixels per second */
     int health;                /* points */
     const Sprite *sprite;      /* the art it draws, from its row */
+    double move_x, move_y;     /* lesson 076: this frame's movement
+                                  request — the game sets it (the
+                                  player's input for the hero, Part 5's
+                                  AI for enemies) and the walk turns it
+                                  into motion */
     bool live;                 /* lesson 074: this entity exists — the
                                   slot's state, set by the store */
 };
diff --git a/src/main.cpp b/src/main.cpp
index 51ae3de..2bdcaa7 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -28,11 +28,6 @@
 
 namespace engine {
 
-/* The scene's one object: the sprite the arrow keys move. Its speed is
-   the engine's — pixels per second — and the clock's dt turns it into a
-   per-frame step. */
-constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
-
 /* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
    the horizon the loop keeps queued. Lesson 062: a feed is always
    exactly this much stream — the sample's frames where the sample has
@@ -124,18 +119,9 @@ int Run(void)
     Framebuffer *fb = GetFramebuffer(arena);
 
     /* The world's assets, loaded whole at startup (lessons 044-053):
-       a sprite, a font, a map, and the map's tile art. Every load is a
-       typed failure or a complete asset — and a failure ends the run by
-       name. */
-    SpriteResult loaded = LoadSprite(arena, "assets/sprite.ppm");
-    if (loaded.error != SPRITE_OK) {
-        std::fprintf(stderr, "engine: assets/sprite.ppm: could not load\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    Sprite &sprite = loaded.sprite;
-
+       a font, a map, and the map's tile art — and, since lesson 073,
+       the art each definition names. Every load is a typed failure or a
+       complete asset — and a failure ends the run by name. */
     FontResult font_loaded = LoadFont(arena, "assets/font.ppm");
     if (font_loaded.error != FONT_OK) {
         std::fprintf(stderr, "engine: assets/font.ppm: could not load\n");
@@ -248,7 +234,7 @@ int Run(void)
         return 1;
     }
     Entity &hero = *hero_made.entity;
-    std::printf("engine: entity %s: x %d y %d facing %d speed %d health %d sprite %dx%d\n",
+    std::printf("engine: entity %s: x %.0f y %.0f facing %d speed %d health %d sprite %dx%d\n",
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
@@ -315,25 +301,23 @@ int Run(void)
     int effect_count = 0;
     int music_wraps = 0;
 
-    double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
     double last = started;
-    double distance = 0.0; /* the score: the world the sprite has walked */
+    double distance = 0.0; /* the score: the world the hero has walked */
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
-    bool was_blocked = false; /* lesson 056: the mover's state report */
 
-    /* The demo's identity: what the run is, named at once — the world
-       and its sound, one measured frame loop. */
-    std::printf("engine: part 3 done — the world draws and the sound plays\n");
-    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; sprite %dx%d\n",
+    /* The demo's identity: what the run is, named at once — the hero,
+       an entity the game moves, over the world the map draws. */
+    std::printf("engine: the hero, as an entity — a row the game moves, a camera that follows\n");
+    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; hero %dx%d\n",
                 map.width, map.height, map.width * TILE_SIZE,
                 map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
-                sprite.width, sprite.height);
+                hero.sprite->width, hero.sprite->height);
     std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
                 music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
                 AUDIO_MIXER_CHANNELS);
-    std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
-    std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
+    std::printf("engine: arrow keys move the hero, space shakes the camera; close the window to stop\n");
+    std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);
 
     /* Lesson 059: the run's sound is a run of amplitude at the engine's
        rate, and the seam's audio output is what puts those frames in
@@ -396,11 +380,29 @@ int Run(void)
         double dt = now - last;
         last = now;
 
+        /* Lesson 076: the hero's intent — polled input state, read once
+           per frame and written to the hero's own movement request. The
+           walk turns every entity's request into motion; the game never
+           moves an entity except through it. */
+        hero.move_x = 0.0;
+        hero.move_y = 0.0;
+        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
+            hero.move_x -= 1.0;
+        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
+            hero.move_x += 1.0;
+        if (platform::KeyDown(opened.window, platform::KEY_UP))
+            hero.move_y -= 1.0;
+        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
+            hero.move_y += 1.0;
+        double was_x = hero.x, was_y = hero.y;
+
         /* Lesson 075: the walk — every live entity, once per frame, in
            slot order. The per-entity work is expressed here, once, and
-           not per type; today it is the demo's kill check. An entity
-           retired in passing is not visited again and no other is
-           skipped — the slots do not move under the walk. */
+           not per type; lesson 076 makes it the entity's step — its
+           movement request becomes motion, and its facing follows where
+           it is going. An entity retired in passing is not visited again
+           and no other is skipped — the slots do not move under the
+           walk. */
         int visited = 0;
         for (int i = 0; i < ENTITY_CAP; ++i) {
             if (!store.slots[i].live)
@@ -410,7 +412,19 @@ int Run(void)
                 std::printf("engine: walk (frame %ld): retiring slot %d in passing\n",
                             frame.number, i);
                 EntityRetire(store, store.slots[i]);
+                continue;
             }
+            Entity &e = store.slots[i];
+            e.x += e.move_x * e.speed * dt;
+            e.y += e.move_y * e.speed * dt;
+            if (e.move_x > 0.0)
+                e.facing = 0;
+            else if (e.move_y > 0.0)
+                e.facing = 1;
+            else if (e.move_x < 0.0)
+                e.facing = 2;
+            else if (e.move_y < 0.0)
+                e.facing = 3;
         }
         walk_visits += visited;
         if (frame_number == 1)
@@ -432,49 +446,18 @@ int Run(void)
             std::printf("engine: store: live %d of %d\n", store.live, ENTITY_CAP);
         }
 
-        double was_x = sprite_x, was_y = sprite_y;
-        double move_x = 0.0, move_y = 0.0;
-        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            move_x -= SPRITE_SPEED * dt;
-        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            move_x += SPRITE_SPEED * dt;
-        if (platform::KeyDown(opened.window, platform::KEY_UP))
-            move_y -= SPRITE_SPEED * dt;
-        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            move_y += SPRITE_SPEED * dt;
-
-        /* Lesson 056: the mover — intent becomes motion only where the
-           map allows it. One axis at a time, so a wall blocks the
-           movement into it and the movement along it still works. */
-        double next_x = sprite_x + move_x;
-        if (!TileRectSolid(map, (int)next_x, (int)sprite_y, sprite.width,
-                           sprite.height))
-            sprite_x = next_x;
-        double next_y = sprite_y + move_y;
-        if (!TileRectSolid(map, (int)sprite_x, (int)next_y, sprite.width,
-                           sprite.height))
-            sprite_y = next_y;
-        distance += (sprite_x > was_x ? sprite_x - was_x : was_x - sprite_x) +
-                    (sprite_y > was_y ? sprite_y - was_y : was_y - sprite_y);
-
-        /* The mover reports its state on transitions: moving, or pushed
-           against something that will not move. */
-        bool blocked = (move_x != 0.0 || move_y != 0.0) &&
-                       sprite_x == was_x && sprite_y == was_y;
-        if (blocked != was_blocked) {
-            std::printf("engine: sprite %s at %d,%d (t=%.3f)\n",
-                        blocked ? "blocked" : "unblocked", (int)sprite_x,
-                        (int)sprite_y, platform::Now() - started);
-            was_blocked = blocked;
-        }
-        if ((int)sprite_x != (int)was_x || (int)sprite_y != (int)was_y)
-            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
-                        (int)sprite_y, platform::Now() - started);
+        /* The score, and the hero's own report: where the entity the
+           game moves has got to. */
+        distance += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
+                    (hero.y > was_y ? hero.y - was_y : was_y - hero.y);
+        if ((int)hero.x != (int)was_x || (int)hero.y != (int)was_y)
+            std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
+                        (int)hero.y, platform::Now() - started);
 
-        /* Lesson 054: the camera's base follows the sprite — the world
+        /* Lesson 054: the camera's base follows the hero — the world
            scrolls under the movement — clamped to the map's bounds. */
-        int base_x = (int)sprite_x + sprite.width / 2 - FRAME_WIDTH / 2;
-        int base_y = (int)sprite_y + sprite.height / 2 - FRAME_HEIGHT / 2;
+        int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2;
+        int base_y = (int)hero.y + hero.sprite->height / 2 - FRAME_HEIGHT / 2;
         if (base_x < 0)
             base_x = 0;
         if (base_y < 0)
@@ -596,8 +579,17 @@ int Run(void)
         DrawTileMap(*fb, map, sheet, -CameraX(camera), -CameraY(camera));
         frame.tilemap = platform::Now() - t_tilemap;
         double t_sprites = platform::Now();
-        BlitSprite(*fb, sprite, (int)sprite_x - CameraX(camera),
-                   (int)sprite_y - CameraY(camera));
+
+        /* Lesson 076: the draw walk — every live entity, its art at its
+           position, through the camera's summed offset. Per-entity work
+           expressed once, in one loop, like the update's walk. */
+        for (int i = 0; i < ENTITY_CAP; ++i) {
+            if (!store.slots[i].live)
+                continue;
+            const Entity &e = store.slots[i];
+            BlitSprite(*fb, *e.sprite, (int)e.x - CameraX(camera),
+                       (int)e.y - CameraY(camera));
+        }
         frame.sprites = platform::Now() - t_sprites;
         double t_text = platform::Now();
         char score_line[32];
@@ -606,7 +598,7 @@ int Run(void)
         DrawText(*fb, font, score_line, 8, 8);
         char pos_line[32];
         std::snprintf(pos_line, sizeof pos_line, "X %3d Y %3d",
-                      (int)sprite_x, (int)sprite_y);
+                      (int)hero.x, (int)hero.y);
         DrawText(*fb, font, pos_line, 8, 8 + FONT_CELL + 4);
         frame.text = platform::Now() - t_text;
 
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The step, predicted *(predict-the-output)*

The movement is `request × speed × dt`, and the hero's row says 240.
Before running anything, predict the hero's position after each of these
two frames, starting from its row's `312,232`:

1. one frame of `dt = 0.5 s` with the Right key's request;
2. then one frame of `dt = 0.5 s` with **both** Right and Down.

Write the two positions down, and one sentence on what the second answer
says about moving diagonally. Then write a scratch probe that forces
exactly those two frames (set the hero's `move_x`/`move_y` by hand and
step the same arithmetic the walk steps) and run it. Reconcile — and
answer: if a game wants diagonal movement no faster than straight
movement, what changes, and where?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-076/ex1.md)

### Exercise 2 — The hero on your desktop *(port-to-your-own-machine)*

This machine's numbers come from a headless display, scripted presses,
and no sound device — the hero steps 7-8 pixels a frame because only one
frame in two finds the key down. Run the hero on your own machine: a
window on your desktop, a held key, a working audio output if you have
one. Report the hero's position lines and the frame log beside the
book's, and answer with your numbers: what is the hero's *measured*
speed over ground (distance over time), how does it compare to the
row's 240, and what in the movement model explains any difference? Then
hold two directions and measure the diagonal.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-076/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 075 — the walk and the free slot](lesson-075-lifetime.md) ·
**Next:** [Lesson 077 — the mover on an entity](lesson-077-mover.md) ·
**Code tag:** [`lesson-076`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-076)
