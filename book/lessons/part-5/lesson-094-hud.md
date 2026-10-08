# Lesson 094 — the HUD

{{#include ../../stability-horizon.md}}

## Prose

The game can be played now, but not *read*: the score, the hero's
health, and the clock live inside the run and only reach the player as
printed lines in a log. This lesson draws them where a player looks —
the **HUD**, the play screen's readouts — and the rule that governs
every number on it:

> the readouts are **the game's own state**, read at the moment of the
> draw — the same values the states act on — and drawn over the world
> **without the camera**.

Two halves, and both are measurable. The first is about *time*: a
readout that shows a stale value is worse than none. The second is
about *space*: the HUD is the player's instrument panel, and instrument
panels do not scroll away.

### The readouts change in the state's own frame

The HUD is a new file pair beside the services (`hud.h`/`hud.cpp`,
design D2) with one function: `HudDraw` reads the game and draws four
lines — `SCORE` and `HEALTH` at the left edge, `WAVE` and `TIME` at the
right. Nothing here keeps its own copy of anything: the score is the
game's score, the health is `hero.health` against the hero's full, the
wave is `game.wave` of the game's waves, the time is the play clock —
the values the state machine's transitions read and print.

The spec's first scenario is that a change of state reaches the readout
**in the same frame**. The run measures it: the bats land two hits on
the hero, and each hit's `hud:` line sits in the same frame's account as
the hit itself — before that frame's `frame` line, never the frame
after:

```
engine: hit: bolt hits hero — damage 1, health 3 -> 2
engine: hud: score 000000, health 2/3, time 0:04, wave 1/3 — at 8,8 over camera 8,0
frame 102: step 43.246 ms, …
engine: hit: bolt hits hero — damage 1, health 2 -> 1
engine: hud: score 000000, health 1/3, time 0:04, wave 1/3 — at 8,8 over camera 8,0
frame 103: step 10.972 ms, …
```

(The `10.972 ms` step in frame 103 is lesson 092's hitstop, answering
the same frame's hit — the toolkit and the HUD reading one event.)

The wave readout moves with the wave fight, in the wave's own frame:

```
engine: state title -> play (the player started)
engine: wave 1 begins — 2 enemies
engine: hud: score 000000, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 8,0
frame 2: step 3.671 ms, …
```

and the second scenario — the health falling *and being restored* — is
the whole death-and-restart cycle, readouts tracking every step:

```
engine: hit: bolt hits hero — damage 1, health 1 -> 0
engine: hit: bolt hits hero — damage 1, health 0 -> 0
engine: hud: score 000000, health 0/3, time 0:07, wave 1/3 — at 8,8 over camera 8,0
engine: state play -> death (the hero's health reached zero)
engine: state death -> title (the player returned to the title)
engine: state title -> play (the player started)
engine: wave 1 begins — 2 enemies
engine: hud: score 000000, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 8,0
```

`health 0/3` in the frame the hero fell; `health 3/3` in the frame the
fresh game restored him. (Two bolts land in the last frame of the hero's
life — the volley's second bolt hits a hero already at zero, the same
one-frame edge lesson 092 recorded; the readout shows `0/3` once, in
the frame the state changed.)

### The HUD does not scroll

Lesson 054 split the screen into two coordinate spaces and gave them
one rule each: the **scene** is drawn at minus the camera — it scrolls;
**text on screen** belongs to the player and does not move with the
world. `HudDraw` obeys it literally: the readouts are laid out at
screen coordinates (`8,8` and the frame's right edge), and no camera
value is applied to them anywhere. The world draws behind them — the
frame's render phase draws the map and the sprites first and the HUD's
text after, over them.

Walk the map and watch the run report both coordinates together — the
anchor the HUD drew at, and where the camera was:

```
engine: hud: score 000000, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 8,0
engine: hud: score 000005, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 13,0
engine: hud: score 000012, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 20,0
engine: hud: score 000020, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 28,0
engine: hud: score 000029, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 37,0
engine: hud: score 000039, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 47,0
…
engine: hud: score 000235, health 3/3, time 0:01, wave 1/3 — at 8,8 over camera 128,0
```

The camera travels its whole range — `8` to `128`, the map's full
scroll — and every line says `at 8,8`. The score ticks beside it (the
hero is walking: the game's score is the ground the hero has covered,
the value the slice's score line has counted since lesson 080 and now
kept in the game's own state). The readouts stay; the world moves.

### What this run verified, and what it did not

- **The readouts match the game in the same frame** — every `hit:` and
  every `wave N begins` is followed by the `hud:` line showing it
  before that frame's `frame` line: the health falls `3/3 → 2/3` in the
  hit's frame and returns to `3/3` in the fresh game's; the wave
  readout moves with the wave fight.
- **The HUD does not scroll** — through the camera's full `8 → 128`
  travel the anchor reads `8,8` on every line, drawn over the world
  (the render phase's `text` sub-phase runs after `tilemap` and
  `sprites`).

What this run did **not** verify is the HUD's *look* — no screen here
to judge whether four lines of 8-pixel text read well over a busy
world. The coordinates and the values are measured; the legibility is
the player's judgment again (and one exercise routes it to a machine
with eyes). And what the score *means* is the game's choice — this game
counts the ground the hero has covered; a score **for the kills** wants
the rows to say what a kill is worth, which is data the format can grow
— the first exercise below makes that extension, done the lesson 087
way.

## Code step

One change: the HUD. `src/hud.h/.cpp` are the readouts' own file pair
(design D2): `HudDraw` reads the game's state and draws the four lines
at screen coordinates, and the run reports what it read and where it
drew it whenever either changes. `src/game.h/.cpp` grow the score — the
ground the hero has walked moves from the loop's local into `Game`,
where the HUD and the states can read it, and a fresh game resets it.
`src/main.cpp`'s play render draws through `HudDraw` instead of its
inline text. Its end state is tagged `lesson-094`.

```diff
diff --git a/src/game.cpp b/src/game.cpp
index d52e01d..e9ce2f0 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -57,6 +57,7 @@ void GameInit(Game &game, int hero_health_full)
     game.waves_remaining = GAME_WAVES;
     game.hero_health_full = hero_health_full;
     game.play_clock = 0.0;
+    game.score = 0.0;
     game.wave = 0;
     game.camera = { 0, 0, 0, 0 };
     std::printf("engine: game: %d state%s, starting on %s\n", 5, "s",
@@ -77,6 +78,7 @@ void GameInput(Game &game, platform::Window *window, Entity &hero,
             game.waves_remaining = GAME_WAVES;
             game.wave = 0; /* lesson 091: the fight starts over */
             game.play_clock = 0.0;
+            game.score = 0.0; /* lesson 094: a fresh game's score */
             hero.move_x = 0.0;
             hero.move_y = 0.0;
             Transition(game, GAME_PLAY, "the player started");
diff --git a/src/game.h b/src/game.h
index 92f49dc..4a6642e 100644
--- a/src/game.h
+++ b/src/game.h
@@ -54,6 +54,9 @@ struct Game {
     int waves_remaining;  /* the named condition for victory */
     int hero_health_full; /* the health a fresh game starts the hero at */
     double play_clock;    /* wall seconds spent in play this game */
+    double score;         /* lesson 094: the game's score — the ground
+                            the hero has walked, the value the HUD
+                            reads and the states' reports carry */
     int wave;             /* lesson 091: the wave being fought (0 = the
                             fight has not started) */
     Camera camera;        /* lesson 083: the game's world-view — one
diff --git a/src/hud.cpp b/src/hud.cpp
new file mode 100644
index 0000000..9c8a304
--- /dev/null
+++ b/src/hud.cpp
@@ -0,0 +1,73 @@
+// hud.cpp — the readouts: the game's state, in text, over the world.
+//
+// Lesson 094: every number drawn here is read from the game's own state
+// at the moment of the draw — the same values the states act on — and
+// drawn at screen coordinates the camera never touches. The run reports
+// what it read and where it drew it, whenever either changes.
+
+#include "hud.h"
+
+#include <cstdio>
+
+#include "text.h"
+
+namespace engine {
+
+/* The HUD's anchor: screen pixels from the frame's top-left. The world
+   is drawn at minus the camera; the readouts at this and nothing else. */
+constexpr int HUD_X = 8;
+constexpr int HUD_Y = 8;
+
+void HudDraw(const Game &game, const Entity &hero, Framebuffer &fb,
+             const Font &font)
+{
+    /* The readouts, each the game's own value as text: the score the
+       game counts, the hero's health against its full, the wave being
+       fought of the game's waves, and the play clock as minutes and
+       seconds. */
+    char score_line[32], health_line[32], wave_line[32], time_line[32];
+    std::snprintf(score_line, sizeof score_line, "SCORE %06d",
+                  (int)game.score);
+    std::snprintf(health_line, sizeof health_line, "HEALTH %d/%d",
+                  hero.health, game.hero_health_full);
+    std::snprintf(wave_line, sizeof wave_line, "WAVE %d/%d", game.wave,
+                  GAME_WAVES);
+    int secs = (int)game.play_clock;
+    std::snprintf(time_line, sizeof time_line, "TIME %d:%02d", secs / 60,
+                  secs % 60);
+
+    /* The left column at the anchor; the right column against the
+       frame's edge. Neither knows where the world is — the HUD is
+       screen, the world is world (lesson 054's rule). */
+    DrawText(fb, font, score_line, HUD_X, HUD_Y);
+    DrawText(fb, font, health_line, HUD_X, HUD_Y + FONT_CELL + 4);
+    DrawText(fb, font, wave_line, FRAME_WIDTH - HUD_X - TextWidth(wave_line),
+             HUD_Y);
+    DrawText(fb, font, time_line, FRAME_WIDTH - HUD_X - TextWidth(time_line),
+             HUD_Y + FONT_CELL + 4);
+
+    /* The run's report — what the HUD read and where it drew it,
+       whenever anything changed: the values (so a change of the game's
+       state is in the same frame's account as the readout showing it)
+       or the camera (so the anchor is visible staying put while the
+       world scrolls under it). */
+    static int was_score = -1, was_health = -1, was_wave = -1, was_secs = -1;
+    static int was_cam_x = -1, was_cam_y = -1;
+    int score = (int)game.score;
+    if (score != was_score || hero.health != was_health ||
+        game.wave != was_wave || secs != was_secs ||
+        game.camera.base_x != was_cam_x || game.camera.base_y != was_cam_y) {
+        was_score = score;
+        was_health = hero.health;
+        was_wave = game.wave;
+        was_secs = secs;
+        was_cam_x = game.camera.base_x;
+        was_cam_y = game.camera.base_y;
+        std::printf("engine: hud: score %06d, health %d/%d, time %d:%02d, wave %d/%d — at %d,%d over camera %d,%d\n",
+                    score, hero.health, game.hero_health_full, secs / 60,
+                    secs % 60, game.wave, GAME_WAVES, HUD_X, HUD_Y,
+                    game.camera.base_x, game.camera.base_y);
+    }
+}
+
+} /* namespace engine */
diff --git a/src/hud.h b/src/hud.h
new file mode 100644
index 0000000..eb06cad
--- /dev/null
+++ b/src/hud.h
@@ -0,0 +1,27 @@
+// hud.h — the HUD: the play screen's readouts, drawn over the world.
+//
+// Lesson 094: score, health, and the game's timers — read from the
+// game's own state (the same values the states act on), drawn over the
+// scene, and never with the camera (lesson 054's HUD rule): the world
+// scrolls, the readouts stay. The play screen owns this; the other
+// states draw their own screens (game.cpp's panels).
+#ifndef HUD_H
+#define HUD_H
+
+#include "entity.h"
+#include "font.h"
+#include "framebuffer.h"
+#include "game.h"
+
+namespace engine {
+
+/* The play screen's readouts, at fixed screen positions: the score, the
+   hero's health, the wave, and the play clock — each the game's own
+   value at the moment of the draw, so the readout and the state change
+   in the same frame. */
+void HudDraw(const Game &game, const Entity &hero, Framebuffer &fb,
+             const Font &font);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 472c017..72f0aec 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -24,6 +24,7 @@
 #include "game.h"
 #include "gametime.h"
 #include "hero.h"
+#include "hud.h"
 #include "platform.h"
 #include "sprite.h"
 #include "table.h"
@@ -418,7 +419,6 @@ int Run(void)
 
     double started = platform::Now();
     double last = started;
-    double distance = 0.0; /* the score: the world the hero has walked */
     GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
     bool was_blocked = false; /* lesson 077: the mover's state report */
     int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
@@ -575,9 +575,10 @@ int Run(void)
         }
 
         /* The score, and the hero's own report: where the entity the
-           game moves has got to. */
-        distance += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
-                    (hero.y > was_y ? hero.y - was_y : was_y - hero.y);
+           game moves has got to. Lesson 094: the score is the game's
+           own state now — the HUD reads it where the states can. */
+        game.score += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
+                      (hero.y > was_y ? hero.y - was_y : was_y - hero.y);
 
         /* Lesson 077: the mover's state report, on transitions — the
            hero moving, or pushed against something that will not move. */
@@ -728,15 +729,11 @@ int Run(void)
             double t_sprites = platform::Now();
             GameDrawSprites(game, *fb, store);
             frame.sprites = platform::Now() - t_sprites;
+            /* Lesson 094: the play screen's readouts are the HUD's
+               (HudDraw, in hud.cpp) — the game's own state in text,
+               drawn over the world and never with the camera. */
             double t_text = platform::Now();
-            char score_line[32];
-            std::snprintf(score_line, sizeof score_line, "SCORE %06d",
-                          (int)distance);
-            DrawText(*fb, font, score_line, 8, 8);
-            char pos_line[32];
-            std::snprintf(pos_line, sizeof pos_line, "X %3d Y %3d",
-                          (int)hero.x, (int)hero.y);
-            DrawText(*fb, font, pos_line, 8, 8 + FONT_CELL + 4);
+            HudDraw(game, hero, *fb, font);
             frame.text = platform::Now() - t_text;
         } else {
             /* The state's own screen. The world is frozen outside play —
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the score for the kills *(extend-the-code)*

The game counts the ground the hero walks, but a wave fight wants a
score **for the kills** — and what a kill is worth is a per-kind fact:
the data for it is a column this format can grow, the way lesson 087
grew it (named, additive, every shipped file still loading
byte-for-byte). Make the enemies' rows price their kinds and make a
kill add its row's points to the game's score. Then run it and quote
the score moving in the kill's own frame — and the compatibility rule
holding on `assets/entities.txt`, which must not change a byte.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-094/ex1.md)

### Exercise 2 — why the HUD never scrolls *(explain-in-prose)*

Lesson 054's rule — text on screen belongs to the player and does not
move with the world — is what keeps this lesson's readouts at `8,8`.
Defend the rule in your own words. Concretely: if `HudDraw` applied the
camera's offsets the way `GameDrawMap` does, where would the `SCORE`
line sit once the camera reaches the map's full scroll (the frame is
640 wide and the camera travels 128), and why is that not merely ugly
but wrong about what a HUD *is*? Then answer the measurement question
from a run of your own that scrolls the map end to end with a probe
printing the readouts' draw coordinates beside the camera's — and say
what the numbers prove.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-094/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 093 — particle bursts and easing](lesson-093-bursts-easing.md) ·
**Next:** [Lesson 095 — audio integration](lesson-095-audio.md) ·
**Code tag:** [`lesson-094`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-094)
