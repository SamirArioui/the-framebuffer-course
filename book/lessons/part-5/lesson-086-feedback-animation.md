# Lesson 086 — feedback and animation

{{#include ../../stability-horizon.md}}

## Prose

The hero has weight now, but it still slides across the map like a
statue and nothing in the game *reacts*. This lesson gives it two things
the feel work of lessons 092-093 will lean on: **the hero animates as it
walks, and the two feedback hooks — screenshake and hitstop — fire and
rest on their own.**

### The walk cycle

The hero's art is a **sprite sheet** now (`assets/hero.ppm`): two
16×16 frames side by side — a stride, and the same stride one step on.
The animation is the frame index advancing while the hero moves:

```cpp
if (want_x != 0.0 || want_y != 0.0) {
    hero.frame_t += dt;
    if (hero.frame_t >= ANIM_STEP) {
        hero.frame_t -= ANIM_STEP;
        hero.frame = (hero.frame + 1) % count;   /* the sheet's frames */
    }
} else {
    hero.frame = 0;                              /* at rest, frame 0 */
}
```

One frame every `ANIM_STEP`, wrapping at the sheet's end — a walk cycle.
At rest it holds frame 0. Two details keep it honest: an entity collides
as **one frame** (`ANIM_FRAME_W` wide), not as the whole sheet — what it
draws is what it hits as — and the draw walks frame `hero.frame` of the
sheet (`BlitSpriteFrame`) rather than the whole image. From a real run,
the frame advancing as the hero steps:

```
engine: hero frame 1 (t=3.348)
engine: hero frame 0 (t=7.863)
```

The frame leaves 0, steps to 1, and returns to 0 — the cycle advancing
while the hero walks and resting when it stops.

### The two feedback hooks

A **screenshake** and a **hitstop** — the first two of the juice
toolkit's four effects (lessons 092-093 add the burst and the easing and,
more importantly, decide *when* to fire these from the game's events).
Here they are the mechanisms, each a thing that **fires and then rests**:

- **Screenshake** moves the camera's additive offset (lesson 054's juice
  hook) while it lasts, and rests it at **exactly zero**.
- **Hitstop** drops the game-time scale to a fraction and returns it to
  **full speed on its own wall-time deadline** — lesson 078's clock: what
  must *end* while the game is stopped cannot run on game time.

The scale is the state's (play runs, the rest hold) times the hitstop's
factor — one knob, two drivers, multiplied. From a real run, both hooks
fired once and resting on their own:

```
engine: feel: shake fired (6 px, 0.5s), hitstop fired (0.25x, 0.4s)
engine: feel: hitstop rested — full speed again
engine: feel: shake rested at 0,0
```

Fired, and then — with nothing undoing them — at rest. The hitstop came
back to full speed when its deadline passed; the shake settled at the
offset it promised: `0,0`, exactly.

### What this run verified, and what it did not

- **The animation advances** — the walk cycle's frame leaves 0 and steps
  while the hero moves, resting at 0 when it stops.
- **The hooks fire and rest** — the shake rests at exactly `0,0`; the
  hitstop returns to full speed on its own deadline.

What this lesson does **not** do is decide *when* to fire the hooks. A
wall-time demonstration fires them once here; the toolkit (lessons
092-093) fires them from the game's events — a hit lands, a burst goes
off — which is the "feedback starts with the event" rule. Nor does it
bring the burst or the easing: those are the toolkit's other two effects,
and they are next.

## Code step

One change: animation and feedback. `src/feel.h` and `src/feel.cpp` are
new — the two hooks (`FeelShake`, `FeelHitstop`) and their fire-and-rest
life (`FeelUpdate` on the wall clock, `FeelTimeScale` for the scale).
`assets/hero.ppm` is a new sprite sheet (the hero's walk cycle), and the
hero's row names it. `src/entity.h/.cpp` carry the frame (and collide as
one frame, not the sheet); `src/hero.h/.cpp` advance the walk cycle;
`src/blit.h/.cpp` draw one frame of a sheet; `src/game.cpp` draws the
entity's frame and hands the camera's additive offset to the feedback
(the hooks own it now); `src/main.cpp` fires the demonstration and folds
the hitstop into the game-time scale. Its end state is tagged
`lesson-086`.

```diff
diff --git a/assets/entities.txt b/assets/entities.txt
index 0683f17..f0635f1 100644
--- a/assets/entities.txt
+++ b/assets/entities.txt
@@ -1,3 +1,3 @@
 name x y facing speed health sprite
-hero 312 232 0 240 3 assets/sprite.ppm
+hero 312 232 0 240 3 assets/hero.ppm
 slime 400 320 2 96 1 assets/sprite.ppm
diff --git a/src/blit.cpp b/src/blit.cpp
index 1c52f0d..0d417f4 100644
--- a/src/blit.cpp
+++ b/src/blit.cpp
@@ -35,4 +35,31 @@ void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
     }
 }
 
+void BlitSpriteFrame(Framebuffer &fb, const Sprite &s, int src_x,
+                     int frame_w, int x, int y)
+{
+    /* BlitSprite, scoped to one frame_w-wide column of the sheet at
+       source x src_x: the source pixel's column is src_x + (i - x). */
+    int left = x < 0 ? 0 : x;
+    int top = y < 0 ? 0 : y;
+    int right = x + frame_w < fb.width ? x + frame_w : fb.width;
+    int bottom = y + s.height < fb.height ? y + s.height : fb.height;
+
+    for (int j = top; j < bottom; ++j) {
+        for (int i = left; i < right; ++i) {
+            const unsigned char *src =
+                &s.pixels[(((size_t)(j - y) * s.width) +
+                           (src_x + (i - x))) * 3];
+            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
+                continue; /* the transparent color writes nothing */
+            unsigned char *dst =
+                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
+            dst[0] = src[2]; /* blue */
+            dst[1] = src[1]; /* green */
+            dst[2] = src[0]; /* red */
+            dst[3] = 0;
+        }
+    }
+}
+
 } /* namespace engine */
diff --git a/src/blit.h b/src/blit.h
index 259ddf3..3832e8b 100644
--- a/src/blit.h
+++ b/src/blit.h
@@ -20,6 +20,12 @@ namespace engine {
    lesson 015's fold at rectangle scale, never a wrap into other pixels. */
 void BlitSprite(Framebuffer &fb, const Sprite &sprite, int x, int y);
 
+/* Lesson 086: one frame of a sprite sheet — the `frame_w`-wide column of
+   the sheet starting at source x `src_x` — drawn at (x, y) exactly like
+   BlitSprite. A walk cycle is one sheet, and this draws one step. */
+void BlitSpriteFrame(Framebuffer &fb, const Sprite &sprite, int src_x,
+                     int frame_w, int x, int y);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/entity.cpp b/src/entity.cpp
index d3c7d3b..b45cc1f 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -60,15 +60,17 @@ void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy)
 {
     /* One axis at a time: a wall blocks the movement into it and the
        movement along it still works — the slide is this shape, not a
-       special case. The rectangle the map is asked about is the
-       entity's art: what it draws is what it collides as. */
+       special case. The rectangle the map is asked about is one frame of
+       the entity's art (lesson 086: an animated kind's sheet is several
+       frames wide, but what it draws — and so what it collides as — is
+       one ANIM_FRAME_W-wide frame). */
     double next_x = entity.x + dx;
     if (!TileRectSolid(map, (int)next_x, (int)entity.y,
-                       entity.sprite->width, entity.sprite->height))
+                       ANIM_FRAME_W, entity.sprite->height))
         entity.x = next_x;
     double next_y = entity.y + dy;
     if (!TileRectSolid(map, (int)entity.x, (int)next_y,
-                       entity.sprite->width, entity.sprite->height))
+                       ANIM_FRAME_W, entity.sprite->height))
         entity.y = next_y;
 }
 
diff --git a/src/entity.h b/src/entity.h
index c0ec0e7..560296f 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -26,7 +26,11 @@ struct Entity {
     int facing;                /* 0 right, 1 down, 2 left, 3 up */
     int speed;                 /* world pixels per second */
     int health;                /* points */
-    const Sprite *sprite;      /* the art it draws, from its row */
+    const Sprite *sprite;      /* the art it draws, from its row — for an
+                                  animated kind, its sprite sheet */
+    int frame;                 /* lesson 086: which frame of the sheet is
+                                  showing — the walk cycle advances it */
+    double frame_t;            /* and how long this frame has shown */
     double move_x, move_y;     /* lesson 076: this frame's movement
                                   request — the game sets it (the
                                   player's input for the hero, Part 5's
@@ -40,6 +44,11 @@ struct Entity {
    answered from the definition alone. */
 Entity EntityFromDef(const EntityDef &def);
 
+/* Lesson 086: every frame of a sprite sheet is this wide — a walk cycle
+   is a sheet of ANIM_FRAME_W-wide frames. An entity collides as one
+   frame (what it draws), not as the whole sheet. */
+constexpr int ANIM_FRAME_W = 16;
+
 /* Lesson 074: the store's capacity — a decision, made here and named in
    the closing review. Sixty-four live entities: the hero, the enemy
    types, and a screenful of projectiles. What the game does when it is
diff --git a/src/feel.cpp b/src/feel.cpp
new file mode 100644
index 0000000..2651e0c
--- /dev/null
+++ b/src/feel.cpp
@@ -0,0 +1,71 @@
+// feel.cpp — the feedback hooks: fire, decay, rest.
+//
+// Lesson 086: each hook fires, runs down its own wall-time, and returns
+// exactly to rest. Nothing here decides *when* to fire — that is the
+// juice toolkit's job (lessons 092-093), reading the game's events.
+
+#include "feel.h"
+
+#include <cstdio>
+
+namespace engine {
+
+void FeelInit(Feedback &feel)
+{
+    feel.shake = 0.0;
+    feel.shake_mag = 0.0;
+    feel.hitstop = 0.0;
+    feel.hitstop_k = 0.0;
+}
+
+void FeelShake(Feedback &feel, double magnitude, double seconds)
+{
+    feel.shake = seconds;
+    feel.shake_mag = magnitude;
+}
+
+void FeelHitstop(Feedback &feel, double fraction, double seconds)
+{
+    feel.hitstop = seconds;
+    feel.hitstop_k = fraction;
+}
+
+double FeelTimeScale(const Feedback &feel)
+{
+    /* At rest the factor is full speed; during a hitstop it is the
+       fraction the hitstop was fired at. */
+    return feel.hitstop > 0.0 ? feel.hitstop_k : GAMETIME_FULL;
+}
+
+void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera)
+{
+    /* The hitstop runs on its own wall-time and returns to full speed
+       when its deadline passes — the game need not remember to undo it. */
+    if (feel.hitstop > 0.0) {
+        feel.hitstop -= wall_dt;
+        if (feel.hitstop < 0.0) {
+            feel.hitstop = 0.0;
+            std::printf("engine: feel: hitstop rested — full speed again\n");
+        }
+    }
+
+    /* The screenshake drives the camera's additive offset — the juice
+       hook lesson 054 defined. While it lasts the offset alternates;
+       when it ends the offset rests at exactly zero. */
+    if (feel.shake > 0.0) {
+        feel.shake -= wall_dt;
+        if (feel.shake <= 0.0) {
+            feel.shake = 0.0;
+            camera.add_x = 0;
+            camera.add_y = 0;
+            std::printf("engine: feel: shake rested at %d,%d\n", camera.add_x,
+                        camera.add_y);
+        } else {
+            camera.add_x = ((int)(feel.shake * 40.0) & 1) ? (int)feel.shake_mag
+                                                          : -(int)feel.shake_mag;
+            camera.add_y = 0;
+        }
+    }
+}
+
+} /* namespace engine */
diff --git a/src/feel.h b/src/feel.h
new file mode 100644
index 0000000..d8706e2
--- /dev/null
+++ b/src/feel.h
@@ -0,0 +1,48 @@
+// feel.h — the feedback hooks the juice toolkit will drive.
+//
+// Lesson 086: two hooks — a screenshake and a hitstop — each a thing
+// that fires and then rests. These are the hooks the juice toolkit
+// (lessons 092-093) will drive from the game's events: here they are the
+// mechanisms, each with its own fire-and-rest life, and nothing yet says
+// when to fire them. A hook at rest costs nothing and changes nothing.
+#ifndef FEEL_H
+#define FEEL_H
+
+#include "camera.h"
+#include "gametime.h"
+
+namespace engine {
+
+/* The feedback state: what is still firing. Every field is at rest at
+   zero — a hook that has fired and finished leaves itself exactly here. */
+struct Feedback {
+    double shake;     /* seconds of screenshake left; 0 = at rest */
+    double shake_mag; /* the shake's offset while it lasts, pixels */
+    double hitstop;   /* seconds of hitstop left; 0 = full speed */
+    double hitstop_k; /* the fraction of game time during the hitstop */
+};
+
+void FeelInit(Feedback &feel);
+
+/* Fire a screenshake: the camera's additive offset moves for `seconds`,
+   then returns to rest — exactly zero. */
+void FeelShake(Feedback &feel, double magnitude, double seconds);
+
+/* Fire a hitstop: the game-time scale drops to `fraction` and returns to
+   full speed on its own wall-time deadline (lesson 078's clock: what
+   must end while the game is stopped cannot run on game time). */
+void FeelHitstop(Feedback &feel, double fraction, double seconds);
+
+/* The game-time factor the hitstop applies right now — a fraction during
+   a hitstop, full speed at rest. The state's own scale multiplies this,
+   so a pause still freezes and a hitstop only slows play. */
+double FeelTimeScale(const Feedback &feel);
+
+/* Advance the feedback once a frame on the wall clock: each hook decays
+   toward rest and drives what it owns — the shake, the camera's additive
+   offset (resting at exactly zero). */
+void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/game.cpp b/src/game.cpp
index 4db9368..967f072 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -207,8 +207,8 @@ void GameFollow(Game &game, const Entity &hero, const TileMap &map)
         game.camera.base_y = base_y;
         std::printf("engine: camera base %d,%d\n", base_x, base_y);
     }
-    game.camera.add_x = 0;
-    game.camera.add_y = 0;
+    /* The camera's additive offset is the juice hook — lesson 086's
+       feedback (FeelUpdate) drives it now, and rests it at zero. */
 }
 
 void GameDrawMap(const Game &game, Framebuffer &fb, const TileMap &map,
@@ -228,8 +228,9 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
         if (!store.slots[i].live)
             continue;
         const Entity &e = store.slots[i];
-        BlitSprite(fb, *e.sprite, (int)e.x - CameraX(game.camera),
-                   (int)e.y - CameraY(game.camera));
+        BlitSpriteFrame(fb, *e.sprite, e.frame * ANIM_FRAME_W, ANIM_FRAME_W,
+                        (int)e.x - CameraX(game.camera),
+                        (int)e.y - CameraY(game.camera));
     }
 }
 
diff --git a/src/hero.cpp b/src/hero.cpp
index 7c119ae..a2798ab 100644
--- a/src/hero.cpp
+++ b/src/hero.cpp
@@ -42,6 +42,23 @@ void HeroMove(Entity &hero, platform::Window *window, double dt)
         k = 1.0;
     hero.move_x += (intent_x - hero.move_x) * k;
     hero.move_y += (intent_y - hero.move_y) * k;
+
+    /* Lesson 086: the walk cycle — the frame advances while the hero
+       steps, one frame per ANIM_STEP, and holds at frame 0 at rest. The
+       sheet's frame count is its width over one frame's width. */
+    if (want_x != 0.0 || want_y != 0.0) {
+        hero.frame_t += dt;
+        if (hero.frame_t >= ANIM_STEP) {
+            hero.frame_t -= ANIM_STEP;
+            int count = hero.sprite->width / ANIM_FRAME_W;
+            if (count < 1)
+                count = 1;
+            hero.frame = (hero.frame + 1) % count;
+        }
+    } else {
+        hero.frame = 0;
+        hero.frame_t = 0.0;
+    }
 }
 
 } /* namespace engine */
diff --git a/src/hero.h b/src/hero.h
index cd658ff..c602349 100644
--- a/src/hero.h
+++ b/src/hero.h
@@ -28,6 +28,9 @@ constexpr double HERO_TIME = 0.12; /* seconds to close on the intent */
    ground at the straight-line speed, not sqrt(2) times it. */
 constexpr double HERO_DIAG = 0.70710678;
 
+/* Lesson 086: how long one frame of the walk cycle shows. */
+constexpr double ANIM_STEP = 0.12;
+
 /* The hero's movement, once per frame of play. The held direction is the
    intent, normalized so the diagonal is no faster than straight; the
    hero's velocity eases toward that intent (accel) and toward rest
diff --git a/src/main.cpp b/src/main.cpp
index 96436f6..e1993f9 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -15,6 +15,7 @@
 #include "audio.h"
 #include "blit.h"
 #include "entity.h"
+#include "feel.h"
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
@@ -248,6 +249,14 @@ int Run(void)
     Game game;
     GameInit(game, hero.health);
 
+    /* Lesson 086: the feedback hooks — a screenshake and a hitstop, both
+       at rest. The juice toolkit (lessons 092-093) will fire these from
+       the game's events; here a demonstration fires both once so their
+       fire-and-rest life is visible. */
+    Feedback feel;
+    FeelInit(feel);
+    bool feel_demo = false;
+
     /* Lesson 080: the vertical slice — the game's shape, and nothing
        else. The hero is the row the game asks for by name (it is the
        one the player controls); the world's other kinds come from the
@@ -326,6 +335,7 @@ int Run(void)
     GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
     bool was_blocked = false; /* lesson 077: the mover's state report */
     int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
+    int was_frame = 0;          /* lesson 086: the hero's walk-cycle frame */
 
     /* The slice's identity: what the run is, named at once — L0*, the
        gate this part closes on. Every service it uses was finished
@@ -409,7 +419,25 @@ int Run(void)
            screen. The hero's movement request is written here (play's
            arrows) and left at rest in every other state. */
         GameInput(game, opened.window, hero, wall_dt);
-        GameTimeSetScale(game_time, GameScale(game));
+
+        /* Lesson 086: the feedback hooks run on their own wall-time —
+           each fires, decays, and rests. The demonstration fires both
+           once, in play, so their fire-and-rest life is visible; the
+           juice toolkit (lessons 092-093) will fire them from the game's
+           events instead of this script. */
+        if (!feel_demo && game.state == GAME_PLAY &&
+            platform::Now() - started >= 3.0) {
+            feel_demo = true;
+            FeelShake(feel, 6.0, 0.5);
+            FeelHitstop(feel, 0.25, 0.4);
+            std::printf("engine: feel: shake fired (6 px, 0.5s), hitstop fired (0.25x, 0.4s)\n");
+        }
+        FeelUpdate(feel, wall_dt, game.camera);
+
+        /* The game-time scale is the state's (play runs, the rest hold)
+           times the hitstop's factor (a fraction during a hitstop, full
+           at rest) — one knob, two drivers, multiplied. */
+        GameTimeSetScale(game_time, GameScale(game) * FeelTimeScale(feel));
 
         /* Lesson 078: the update advances by game time — the wall
            clock's step, scaled. Everything the simulation does with dt
@@ -469,6 +497,14 @@ int Run(void)
             }
         }
 
+        /* Lesson 086: the walk cycle — the frame advances while the hero
+           steps, and this is the measurement of it advancing. */
+        if (hero.frame != was_frame) {
+            std::printf("engine: hero frame %d (t=%.3f)\n", hero.frame,
+                        platform::Now() - started);
+            was_frame = hero.frame;
+        }
+
         /* Lesson 083: the game's world-view — the camera's base follows
            the hero, clamped to the map's bounds, and its additive offset
            rests at exactly zero. The game owns the camera now (GameFollow,
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Feedback starts with the event *(extend-the-code)*

The hooks fire on a wall-time script here, but the game-feel rule is that
a feel effect begins **in the frame its triggering event happens**. Make
the feedback start with the event: when the hero takes a hit (the Space
stand-in for combat), fire a short hitstop and a small screenshake — the
hit *lands* with weight. Keep the hooks' own fire-and-rest (they should
still return to full speed and to `0,0` on their own); the change is
*what fires them*. Then run it and press Space: does the hit read as one
moment — the shake and the slowdown starting on the hit's own frame?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-086/ex1.md)

### Exercise 2 — Why wall-time *(explain-in-prose)*

Both hooks run down their own **wall-time**, not game time. Defend that
choice in your own words. Concretely: a hitstop sets the game-time scale
to a fraction — if the hitstop's *own* countdown ran on game time, what
would happen when the scale is a fraction (would the hitstop ever end)?
And what would happen to a screenshake if the game were paused mid-shake
and its decay ran on game time? Then answer the measurement question:
the run above shows `hitstop rested — full speed again` and `shake rested
at 0,0` — what do those two lines prove about which clock the hooks
used?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-086/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 085 — hero movement](lesson-085-hero-movement.md) ·
**Next:** [the course home](../../index.md) ·
**Code tag:** [`lesson-086`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-086)
