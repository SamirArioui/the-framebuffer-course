# Lesson 082 — the game skeleton

{{#include ../../stability-horizon.md}}

## Prose

Part 4 closed with a hero walking the tilemap on finished services — the
vertical slice — and one thing the slice deliberately did *not* have: a
game. It was one loop with a demo script standing in for the parts that
had not arrived. This lesson stands up the real thing. The idea is the
one the whole of Part 5 turns on: **a game is not one loop with flags —
it runs in exactly one of five states, and each state owns its screen
and its input.**

### Five states, not one loop

The states are named (design D7): **title, play, pause, death,
victory**. A `GameState` value says which one the game is in, and only
that state's input acts. There is no "if paused draw the menu else draw
the game" scattered through the loop — the loop is still the loop (read
news, update, feed the stream, draw, present), and the *game* underneath
it is a state machine in its own file pair, `game.h` / `game.cpp`, beside
the services rather than inside them (design D2).

```cpp
enum GameState {
    GAME_TITLE = 0,
    GAME_PLAY,
    GAME_PAUSE,
    GAME_DEATH,
    GAME_VICTORY,
};
```

Each state answers two questions and nothing else: *what does this state
draw?* and *what input does this state accept?* The title screen draws
its panel and accepts the start key; play draws the world and accepts
movement; pause draws its panel and accepts resume. The state also sets
the one number that moves the world — the game-time scale (lesson 078's
knob): **play is full speed, every other state is zero.**

### The transitions are named

State changes are not accidents — they are named conditions, and the run
prints each one the moment it happens. From a real run of this lesson's
end state, driven by scripted input under the headless display (arrows,
Enter, Escape, Space):

```
engine: game: 5 states, starting on title
engine: state title -> play (the player started)
engine: hero at 557,232 (t=4.048)
engine: state play -> pause (the player paused)
engine: state pause -> play (the player resumed)
engine: hero takes a hit — health 2 (t=2.595)
engine: hero takes a hit — health 1 (t=3.440)
engine: hero takes a hit — health 0 (t=3.985)
engine: state play -> death (the hero's health reached zero)
engine: state death -> title (the player returned to the title)
```

and, in a second run where the completion key is pressed in play, the
game's own completion:

```
engine: state title -> play (the player started)
engine: the waves are complete (t=0.644)
engine: state play -> victory (the game's waves are complete)
```

Read the parentheticals: those are the named conditions. Play begins
from the title ("the player started"); pause suspends play and resumes
it; **death follows the hero's defeat** — the hero's health reached zero;
**victory follows the game's completion** — the game's waves are
complete. The `GameInput` switch reads the current state's keys and the
named conditions once per frame, and every transition prints the reason
it happened. A pause key is edge-triggered through the lesson-033 latch,
so one press cannot pause and immediately resume.

### The simulation stands still outside play

This is the part that makes "one state at a time" more than a drawing
convenience. The state sets the game-time scale, and the scale is the
*only* thing that advances the world. Outside play the scale is zero, so
the update's step is zero game-seconds — the world freezes — while the
presentation keeps drawing that state's screen. The frame record shows
both halves at once. Three frames from the run above — the title panel,
a play frame, and the pause panel:

```
frame 1: step 0.000 ms, update 0.001 ms (entities 0.000), audio 0.000 ms, render 0.859 ms (sprites 0.000, text 0.008, tilemap 0.000), present 0.924 ms, total 1.784 ms
frame 3: step 20.211 ms, update 0.002 ms (entities 0.002), audio 0.000 ms, render 1.391 ms (sprites 0.002, text 0.008, tilemap 0.941), present 0.942 ms, total 2.336 ms
frame 4: step 0.000 ms, update 0.004 ms (entities 0.002), audio 0.000 ms, render 0.472 ms (sprites 0.000, text 0.006, tilemap 0.000), present 0.448 ms, total 0.924 ms
```

Row by row, this is the contract, measured:

- **`step 0.000` outside play** (frames 1 and 4) and a real step in play
  (frame 3). The step is game time (lesson 079) — the world advanced
  nothing on the title and pause frames.
- **The update still runs.** `update 0.001 / 0.004 ms` on those same
  paused frames — the input read, the reports, the frame's own work all
  happened. The *world* stood still; the *frame* did not. That is the
  difference between freezing with the scale and skipping the update.
- **The screen is the state's own.** In play, `tilemap 0.941` — the world
  is drawn. On the panels, `tilemap 0.000` and only `text` costs
  anything — the panel is drawn and the world is not. Each state draws
  its own screen, and the numbers say which screen ran.

### Two stand-ins, named

Two of the five named conditions read game state that does not exist
yet: combat reduces the hero's health (lesson 087) and the waves spend
the game's completion (lesson 091). Until those land, this lesson
demonstrates the two transitions with a **stand-in**, the same device
lesson 078 used to show the scale before the juice toolkit drove it:

- **Space is a hit on the hero** — it lowers the hero's health by one,
  and health reaching zero is the defeat condition. Lesson 087 replaces
  the key with real hits.
- **Enter in play is the game being complete** — it sets the waves to
  zero, and no waves left is the completion condition. Lesson 091
  replaces it with real waves.

Both are keyed — they fire when the player presses a key and never on
their own during a gameplay test — and both are marked as scaffolding in
`game.cpp`, removed when the real triggers arrive. The *transitions*
they fire — defeat on zero health, completion on no waves — are the
game's own and do not change.

### What this run verified, and what it did not

- **One state at a time, each with its own screen and input.** The
  transition log visits every state exactly once per path, and the frame
  rows show each state's screen running (the world in play, a panel
  elsewhere).
- **The transitions are the named ones,** each printed with the reason
  the spec gives: started, paused, resumed, the hero's health reached
  zero, the waves are complete, returned to the title.
- **The simulation stands still outside play** — `step 0.000` on every
  non-play frame while the update keeps running and the presentation
  keeps drawing.

What this lesson does **not** do is make the game fun, or the states
interesting. The play state is still Part 4's slice; the panels are two
lines of text; the two end conditions are stand-ins. Those are the next
fourteen lessons. The skeleton's job is smaller and more important: the
game has a shape now, and every later lesson grows *into* it — combat
into play's update, waves into the victory condition, the screens into
final form — rather than bolting flags onto a loop.

## Code step

One change for this lesson: the game-state machine stands up as the game
layer's first file pair, and the loop is wired to it. `src/game.h` and
`src/game.cpp` are new — the `GameState` enum, the `Game` state, and the
four things the loop asks of the machine (`GameInit`, `GameInput`,
`GameScale`, `GameDrawPanel`). `src/main.cpp` keeps the loop and the
Part 4 play behaviour, but now consults the machine: it calls `GameInput`
for the state's input and the named transitions, sets the game-time scale
from `GameScale` instead of a demo script, and draws the current state's
screen — the scene in play, the state's panel otherwise. The demo
scaffolding the slice carried (the wall-time scale script and the Space
shake) is gone: the scale is the state's now, and Space is the hit. The
camera's additive offset is left at exactly zero, the hook waiting for
lesson 092. Its end state is tagged `lesson-082`.

```diff
diff --git a/src/game.cpp b/src/game.cpp
new file mode 100644
index 0000000..a6e1c24
--- /dev/null
+++ b/src/game.cpp
@@ -0,0 +1,194 @@
+// game.cpp — the game-state machine: the states, their input, their
+// transitions, and the scale the state sets.
+//
+// Lesson 082: the whole of the machine lives here so the loop stays the
+// loop. `Run` reads news, calls GameInput (the state's input and the
+// named transitions), sets the scale from GameScale, advances the world
+// by game time, and draws the current state's screen. Nothing in the
+// machine reaches behind the platform seam, and nothing allocates.
+
+#include "game.h"
+
+#include <cstdio>
+
+#include "text.h"
+
+namespace engine {
+
+/* The demonstration stand-in, named so it cannot be mistaken for the
+   game. Lesson 082 has the state machine but not yet the gameplay that
+   drives two of its named conditions: combat reduces the hero's health
+   (lesson 087) and the waves spend the game's completion (lesson 091).
+   Until those land, keys stand in for them: SPACE is a hit on the hero,
+   and ENTER in play says the game is complete — the same kind of keyed
+   demonstration, and like lesson 078's script before the juice toolkit
+   drove it. Both are removed when the real triggers arrive; the
+   transitions they fire are the game's own (defeat on zero health,
+   completion on no waves). Keyed, they never fire on their own during a
+   gameplay test. */
+
+/* A transition, named once here and printed the moment it happens, so a
+   run shows the machine moving between states and why. */
+static void Transition(Game &game, GameState to, const char *why)
+{
+    std::printf("engine: state %s -> %s (%s)\n", GameStateName(game.state),
+                GameStateName(to), why);
+    game.state = to;
+}
+
+const char *GameStateName(GameState state)
+{
+    switch (state) {
+    case GAME_TITLE:
+        return "title";
+    case GAME_PLAY:
+        return "play";
+    case GAME_PAUSE:
+        return "pause";
+    case GAME_DEATH:
+        return "death";
+    default:
+        return "victory";
+    }
+}
+
+void GameInit(Game &game, int hero_health_full)
+{
+    game.state = GAME_TITLE;
+    game.waves_remaining = GAME_WAVES;
+    game.hero_health_full = hero_health_full;
+    game.play_clock = 0.0;
+    std::printf("engine: game: %d state%s, starting on %s\n", 5, "s",
+                GameStateName(game.state));
+}
+
+void GameInput(Game &game, platform::Window *window, Entity &hero,
+               double wall_dt)
+{
+    /* Play's movement is the hero's own request; every other state leaves
+       the hero at rest, so the walk moves nothing and the simulation
+       stands still. */
+    hero.move_x = 0.0;
+    hero.move_y = 0.0;
+
+    switch (game.state) {
+    case GAME_TITLE:
+        /* The title screen accepts one thing: the start key. */
+        if (platform::KeyPressed(window, platform::KEY_ENTER)) {
+            /* A fresh game restores the hero's health and the game's
+               waves — the row's facts, not remembered state. */
+            hero.health = game.hero_health_full;
+            game.waves_remaining = GAME_WAVES;
+            game.play_clock = 0.0;
+            Transition(game, GAME_PLAY, "the player started");
+        }
+        break;
+
+    case GAME_PLAY: {
+        /* Play's input: polled movement state, written to the hero's
+           request. The walk turns it into motion (lesson 076). */
+        if (platform::KeyDown(window, platform::KEY_LEFT))
+            hero.move_x -= 1.0;
+        if (platform::KeyDown(window, platform::KEY_RIGHT))
+            hero.move_x += 1.0;
+        if (platform::KeyDown(window, platform::KEY_UP))
+            hero.move_y -= 1.0;
+        if (platform::KeyDown(window, platform::KEY_DOWN))
+            hero.move_y += 1.0;
+
+        /* The stand-in for combat: SPACE is a hit on the hero (lesson 087
+           makes real hits land). The named condition below reads the
+           health this lowers. */
+        if (platform::KeyPressed(window, platform::KEY_SPACE) &&
+            hero.health > 0) {
+            hero.health -= 1;
+            std::printf("engine: hero takes a hit — health %d (t=%.3f)\n",
+                        hero.health, game.play_clock);
+        }
+
+        game.play_clock += wall_dt;
+
+        /* The named transitions, read from the game's own state. Defeat
+           first: a hero at zero health is dead, whatever else holds. */
+        if (hero.health <= 0) {
+            Transition(game, GAME_DEATH, "the hero's health reached zero");
+            break;
+        }
+
+        /* The stand-in for the waves: ENTER in play says the game is
+           complete (lesson 091 spends the waves for real). Completion is
+           the game's waves reaching zero. A keyed stand-in, like SPACE
+           is a hit — it never fires on its own during a gameplay test. */
+        if (platform::KeyPressed(window, platform::KEY_ENTER)) {
+            game.waves_remaining = 0;
+            std::printf("engine: the waves are complete (t=%.3f)\n",
+                        game.play_clock);
+        }
+        if (game.waves_remaining == 0) {
+            Transition(game, GAME_VICTORY, "the game's waves are complete");
+            break;
+        }
+
+        /* Pause suspends play. The latch keeps one press from acting
+           twice (lesson 033). */
+        if (platform::KeyPressed(window, platform::KEY_ESCAPE))
+            Transition(game, GAME_PAUSE, "the player paused");
+        break;
+    }
+
+    case GAME_PAUSE:
+        /* The pause screen accepts one thing: resume. Play continues from
+           where it stood — the world was frozen, not reset. */
+        if (platform::KeyPressed(window, platform::KEY_ESCAPE))
+            Transition(game, GAME_PLAY, "the player resumed");
+        break;
+
+    case GAME_DEATH:
+    case GAME_VICTORY:
+        /* The end screens accept one thing: back to the title. */
+        if (platform::KeyPressed(window, platform::KEY_ENTER))
+            Transition(game, GAME_TITLE, "the player returned to the title");
+        break;
+    }
+}
+
+double GameScale(const Game &game)
+{
+    /* The one number the state sets: play advances the world, every other
+       state holds it still. This is D7, and it is why the frame record's
+       step is 0 outside play without the world knowing anything special. */
+    return game.state == GAME_PLAY ? GAMETIME_FULL : 0.0;
+}
+
+/* One panel screen: a title line and a hint, centred. The screen is the
+   state's — the world is not drawn behind it. The backdrop is cleared by
+   the frame's render phase before this runs, so the panel's own time is
+   the text it draws and nothing else. */
+static void Panel(Framebuffer &fb, const Font &font, const char *title,
+                  const char *hint)
+{
+    int title_x = (FRAME_WIDTH - TextWidth(title)) / 2;
+    int hint_x = (FRAME_WIDTH - TextWidth(hint)) / 2;
+    DrawText(fb, font, title, title_x, FRAME_HEIGHT / 2 - FONT_CELL);
+    DrawText(fb, font, hint, hint_x, FRAME_HEIGHT / 2 + FONT_CELL);
+}
+
+void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font)
+{
+    switch (game.state) {
+    case GAME_TITLE:
+        Panel(fb, font, "THE FRAMEBUFFER GAME", "ENTER: PLAY");
+        break;
+    case GAME_PAUSE:
+        Panel(fb, font, "PAUSED", "ESCAPE: RESUME");
+        break;
+    case GAME_DEATH:
+        Panel(fb, font, "GAME OVER", "ENTER: TITLE");
+        break;
+    default:
+        Panel(fb, font, "VICTORY", "ENTER: TITLE");
+        break;
+    }
+}
+
+} /* namespace engine */
diff --git a/src/game.h b/src/game.h
new file mode 100644
index 0000000..eccc9de
--- /dev/null
+++ b/src/game.h
@@ -0,0 +1,84 @@
+// game.h — the game-state machine: the game standing on finished services.
+//
+// Lesson 082: the game is not one loop with flags — it runs in exactly
+// one of five states, and each state owns its screen and its input. The
+// states are named (D7): title, play, pause, death, victory. Transitions
+// happen on named conditions — play begins from the title, pause
+// suspends play and resumes it, death follows the hero's defeat, victory
+// follows the game's completion — and never by accident.
+//
+// The state also owns the game-time scale (D7): play runs at full speed,
+// every other state at zero. So the simulation stands still outside play
+// while the presentation keeps drawing that state's screen — the frame
+// record stays honest (lesson 079), and pause is the same hook lesson
+// 078 named, extended to the other screens.
+//
+// This is the game layer's first file pair, beside the services (D2):
+// the services (table, store, mover, game-time, camera) never grow game
+// behavior, and the game never grows inside them. The play state's
+// gameplay stands on those services and invents nothing.
+#ifndef GAME_H
+#define GAME_H
+
+#include "entity.h"
+#include "font.h"
+#include "framebuffer.h"
+#include "gametime.h"
+#include "platform.h"
+
+namespace engine {
+
+/* The five states. The game runs in exactly one at a time, and only that
+   state's input acts. */
+enum GameState {
+    GAME_TITLE = 0,
+    GAME_PLAY,
+    GAME_PAUSE,
+    GAME_DEATH,
+    GAME_VICTORY,
+};
+
+/* The game's wave count — the named condition for victory reads it: the
+   game is complete when its waves are done. Lesson 091 builds the waves
+   that spend it; until then it is the game's plan, named here. */
+constexpr int GAME_WAVES = 3;
+
+/* The game's own state: which state it is in, and the facts the named
+   transitions read. Nothing here is a service's — it is the game's. */
+struct Game {
+    GameState state;
+    int waves_remaining;  /* the named condition for victory */
+    int hero_health_full; /* the health a fresh game starts the hero at */
+    double play_clock;    /* wall seconds spent in play this game */
+};
+
+/* The game begins on the title screen. The hero's starting health is the
+   row's fact, handed over so a fresh game can restore it. */
+void GameInit(Game &game, int hero_health_full);
+
+/* A state's name, for the run's reports. */
+const char *GameStateName(GameState state);
+
+/* The current state's input and the named transitions, once per frame.
+   The window is polled for held movement (play) and edge-triggered menu
+   keys (the latch of lesson 033 keeps one press from acting twice). The
+   hero's movement request is written here — play's arrows — and left at
+   rest in every other state. The named conditions (defeat, completion)
+   are read from the game's own state each frame. */
+void GameInput(Game &game, platform::Window *window, Entity &hero,
+               double wall_dt);
+
+/* The game-time scale the current state sets: play at full speed, every
+   other state at zero. This one number is what makes the simulation
+   stand still outside play. */
+double GameScale(const Game &game);
+
+/* The current state's screen, for the four states whose screen is a
+   panel over a still world — title, pause, death, victory. Play's screen
+   is the world the frame draws; the loop draws it and calls this for the
+   rest. */
+void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 9bc7e46..7f72983 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -19,6 +19,7 @@
 #include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
+#include "game.h"
 #include "gametime.h"
 #include "platform.h"
 #include "sprite.h"
@@ -239,6 +240,14 @@ int Run(void)
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
+    /* Lesson 082: the game-state machine. The game is a state now, not a
+       loop with flags — it starts on the title screen, each state owns
+       its screen and its input, and the transitions are named conditions
+       (D7). The hero's starting health is the row's fact, handed to the
+       machine so a fresh game can restore it. */
+    Game game;
+    GameInit(game, hero.health);
+
     /* Lesson 080: the vertical slice — the game's shape, and nothing
        else. The hero is the row the game asks for by name (it is the
        one the player controls); the world's other kinds come from the
@@ -308,9 +317,7 @@ int Run(void)
     double started = platform::Now();
     double last = started;
     double distance = 0.0; /* the score: the world the hero has walked */
-    int shake_frames = 0; /* lesson 054: the additive hook's demo */
-    GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale, at play */
-    int scale_phase = 0;  /* lesson 078: the demo's script, by wall seconds */
+    GameTime game_time = { GAMETIME_FULL }; /* lesson 078: the scale — set by the state now */
     bool was_blocked = false; /* lesson 077: the mover's state report */
 
     /* The slice's identity: what the run is, named at once — L0*, the
@@ -379,7 +386,7 @@ int Run(void)
         if (platform::CloseRequested(opened.window))
             break;
 
-        FrameRecord frame;
+        FrameRecord frame = {};
         frame.number = ++frame_number;
         double t0 = platform::Now();
 
@@ -388,32 +395,15 @@ int Run(void)
         double wall_dt = now - last;
         last = now;
 
-        /* Lesson 078: the game-time scale — the one knob the game sets.
-           The demo's script is the game here: play, then hitstop (a
-           fraction of full speed), then pause (0), then play again —
-           the same three settings Part 5's juice toolkit and pause
-           screen will make. Each transition names the step that comes
-           out: the wall clock's step, scaled. */
-        double running = now - started;
-        if (scale_phase == 0 && running >= 3.0) {
-            GameTimeSetScale(game_time, 0.25);
-            scale_phase = 1;
-            std::printf("engine: game-time: scale %.2f (hitstop) — step %.3f ms of a %.3f ms wall step\n",
-                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
-                        wall_dt * 1e3);
-        } else if (scale_phase == 1 && running >= 5.0) {
-            GameTimeSetScale(game_time, 0.0);
-            scale_phase = 2;
-            std::printf("engine: game-time: scale %.2f (pause) — step %.3f ms of a %.3f ms wall step\n",
-                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
-                        wall_dt * 1e3);
-        } else if (scale_phase == 2 && running >= 7.0) {
-            GameTimeSetScale(game_time, GAMETIME_FULL);
-            scale_phase = 3;
-            std::printf("engine: game-time: scale %.2f (play) — step %.3f ms of a %.3f ms wall step\n",
-                        game_time.scale, GameTimeStep(game_time, wall_dt) * 1e3,
-                        wall_dt * 1e3);
-        }
+        /* Lesson 082: the state machine reads this frame's input and the
+           named transitions, and sets the game-time scale the current
+           state calls for. Play advances the world at full speed; every
+           other state holds it still — so the simulation stands still
+           outside play while the presentation keeps drawing the state's
+           screen. The hero's movement request is written here (play's
+           arrows) and left at rest in every other state. */
+        GameInput(game, opened.window, hero, wall_dt);
+        GameTimeSetScale(game_time, GameScale(game));
 
         /* Lesson 078: the update advances by game time — the wall
            clock's step, scaled. Everything the simulation does with dt
@@ -423,20 +413,6 @@ int Run(void)
         double dt = GameTimeStep(game_time, wall_dt);
         frame.step = dt;
 
-        /* Lesson 076: the hero's intent — polled input state, read once
-           per frame and written to the hero's own movement request. The
-           walk turns every entity's request into motion; the game never
-           moves an entity except through it. */
-        hero.move_x = 0.0;
-        hero.move_y = 0.0;
-        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            hero.move_x -= 1.0;
-        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            hero.move_x += 1.0;
-        if (platform::KeyDown(opened.window, platform::KEY_UP))
-            hero.move_y -= 1.0;
-        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            hero.move_y += 1.0;
         double was_x = hero.x, was_y = hero.y;
 
         /* Lesson 075: the walk — every live entity, once per frame, in
@@ -503,24 +479,12 @@ int Run(void)
                         base_y, platform::Now() - started);
         }
 
-        /* The additive offset: the hook the juice toolkit will drive.
-           Here SPACE demonstrates it — a shake that ends at zero, which
-           is where it lives at rest. */
-        if (platform::KeyPressed(opened.window, platform::KEY_SPACE) &&
-            shake_frames <= 0) {
-            shake_frames = 30;
-            std::printf("engine: camera additive 6,0 (shake starts)\n");
-        }
-        if (shake_frames > 0) {
-            --shake_frames;
-            camera.add_x = (shake_frames % 2) ? 6 : -6;
-            camera.add_y = 0;
-            if (shake_frames == 0) {
-                camera.add_x = 0;
-                camera.add_y = 0;
-                std::printf("engine: camera additive 0,0 (at rest)\n");
-            }
-        }
+        /* Lesson 082: the camera's additive offset is the juice hook
+           lesson 054 defined, and the game skeleton keeps it at exactly
+           zero — the screenshake that will drive it arrives in lesson
+           092. At rest the sum every scene draw uses is the base alone. */
+        camera.add_x = 0;
+        camera.add_y = 0;
 
         frame.update = platform::Now() - t0;
 
@@ -601,36 +565,49 @@ int Run(void)
 
         double t1 = platform::Now();
 
-        /* Render: every frame draws the whole scene — clear, the world
-           through the camera, and the HUD over it — each timed as its own
-           named phase: the subsystems the frame record can name. */
-        ClearBuffer(*fb, 32, 32, 64);
-        double t_tilemap = platform::Now();
-        DrawTileMap(*fb, map, sheet, -CameraX(camera), -CameraY(camera));
-        frame.tilemap = platform::Now() - t_tilemap;
-        double t_sprites = platform::Now();
-
-        /* Lesson 076: the draw walk — every live entity, its art at its
-           position, through the camera's summed offset. Per-entity work
-           expressed once, in one loop, like the update's walk. */
-        for (int i = 0; i < ENTITY_CAP; ++i) {
-            if (!store.slots[i].live)
-                continue;
-            const Entity &e = store.slots[i];
-            BlitSprite(*fb, *e.sprite, (int)e.x - CameraX(camera),
-                       (int)e.y - CameraY(camera));
+        /* Render: the current state's screen, and only that one (lesson
+           082). The backdrop is the state's own — the world's blue in
+           play, the panel's darker blue on the panel screens — cleared
+           once here, in the render phase, before the named sub-phases. */
+        if (game.state == GAME_PLAY)
+            ClearBuffer(*fb, 32, 32, 64);
+        else
+            ClearBuffer(*fb, 24, 24, 40);
+        if (game.state == GAME_PLAY) {
+            double t_tilemap = platform::Now();
+            DrawTileMap(*fb, map, sheet, -CameraX(camera), -CameraY(camera));
+            frame.tilemap = platform::Now() - t_tilemap;
+            double t_sprites = platform::Now();
+
+            /* Lesson 076: the draw walk — every live entity, its art at its
+               position, through the camera's summed offset. Per-entity work
+               expressed once, in one loop, like the update's walk. */
+            for (int i = 0; i < ENTITY_CAP; ++i) {
+                if (!store.slots[i].live)
+                    continue;
+                const Entity &e = store.slots[i];
+                BlitSprite(*fb, *e.sprite, (int)e.x - CameraX(camera),
+                           (int)e.y - CameraY(camera));
+            }
+            frame.sprites = platform::Now() - t_sprites;
+            double t_text = platform::Now();
+            char score_line[32];
+            std::snprintf(score_line, sizeof score_line, "SCORE %06d",
+                          (int)distance);
+            DrawText(*fb, font, score_line, 8, 8);
+            char pos_line[32];
+            std::snprintf(pos_line, sizeof pos_line, "X %3d Y %3d",
+                          (int)hero.x, (int)hero.y);
+            DrawText(*fb, font, pos_line, 8, 8 + FONT_CELL + 4);
+            frame.text = platform::Now() - t_text;
+        } else {
+            /* The state's own screen. The world is frozen outside play —
+               the simulation stands still — and the panel is what the
+               window shows. */
+            double t_text = platform::Now();
+            GameDrawPanel(game, *fb, font);
+            frame.text = platform::Now() - t_text;
         }
-        frame.sprites = platform::Now() - t_sprites;
-        double t_text = platform::Now();
-        char score_line[32];
-        std::snprintf(score_line, sizeof score_line, "SCORE %06d",
-                      (int)distance);
-        DrawText(*fb, font, score_line, 8, 8);
-        char pos_line[32];
-        std::snprintf(pos_line, sizeof pos_line, "X %3d Y %3d",
-                      (int)hero.x, (int)hero.y);
-        DrawText(*fb, font, pos_line, 8, 8 + FONT_CELL + 4);
-        frame.text = platform::Now() - t_text;
 
         frame.render = platform::Now() - t1;
         double t2 = platform::Now();
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The score on the end screens *(extend-the-code)*

The run keeps a score — the distance the hero has walked — and play's HUD
shows it. But when the hero dies or the game is won, the end screen
shows nothing about how the run went. Carry the score in the game's own
state (it is the game's fact, not the loop's) and draw it on the death
and the victory panels, so the player sees the run's result on the
screen that reports it. While you are in the end states, widen their
input: today only Enter returns to the title — let Escape do it too, the
way Escape already leaves play for pause. Keep the machine's rule that
only the current state's input acts, and keep the input latch doing its
job (one press, one action).

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-082/ex1.md)

### Exercise 2 — Why the scale *(explain-in-prose)*

The simulation stands still outside play because the state sets the
game-time scale to zero — not because the loop skips the update and not
because the draw stops. Defend that choice in your own words. Concretely:
what would break if, instead of scaling the step to zero, the loop
simply `continue`d past the update when the game was paused? Think about
the frame record (what does lesson 079's contract require of a paused
frame?), the input latch (would a pause key still behave?), and the day
a *fractional* scale arrives (lesson 092's hitstop — is a skipped update
the same thing as a slowed one?). Then answer the measurement question:
the frame rows above show `update` costing real time while `step` is
zero — what does that pair of numbers prove about where the freeze
lives?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-082/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 081 — the slice's cost in the frame budget](../part-4/lesson-081-entities-row.md) ·
**Next:** [Lesson 083 — the tilemap and camera](lesson-083-tilemap-camera.md) ·
**Code tag:** [`lesson-082`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-082)
