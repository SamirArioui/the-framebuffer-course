# Lesson 096 — screen polish

{{#include ../../stability-horizon.md}}

## Prose

Four screens have stood behind this game since lesson 082 — title,
pause, death, victory — each a title line and a hint, doing their job
and nothing more. This lesson is the assembly's last: the screens in
**final form**. What "final" means here is small and concrete: a title
screen that teaches the controls, end screens that tell the run's
story, every screen naming the very input it acts on — and the
toolkit's fourth effect finding its natural home at last, in the
**screens' fades**.

### The screens say what they are

The play screen owns the HUD (lesson 094); the other states own their
screens (design D9), and each screen's content is now the state's
whole story:

```
engine: screen: title: "THE FRAMEBUFFER GAME" / "ARROWS MOVE ... ESCAPE PAUSE" / "ENTER: PLAY"
engine: screen: title fade arrived at 24,24,40 (its own color)
engine: state title -> play (the player started)
engine: state play -> pause (the player paused)
engine: screen: pause: "PAUSED" / "SCORE 000424   TIME 0:02   WAVE 1/3" / "ESCAPE: RESUME"
engine: screen: pause fade arrived at 24,24,40 (its own color)
engine: state pause -> play (the player resumed)
engine: state play -> death (the hero's health reached zero)
engine: screen: death: "GAME OVER" / "SCORE 000424   TIME 0:06   WAVE 1/3" / "ENTER: TITLE"
engine: screen: death fade arrived at 56,16,16 (its own color)
engine: state death -> title (the player returned to the title)
engine: screen: title: "THE FRAMEBUFFER GAME" / "ARROWS MOVE ... ESCAPE PAUSE" / "ENTER: PLAY"
```

The **title** teaches: the game's name, the four controls (movement,
fire, the two weapons, pause), and the prompt that starts it. The
**pause** and the end screens carry the run's final numbers —
`SCORE 000424   TIME 0:06   WAVE 1/3` — the *same values the HUD
reads*: the game's score, the play clock, the wave the game ended on.
The state is the game's story, and the end screens just tell it.

And every screen names its input — `ENTER: PLAY`, `ESCAPE: RESUME`,
`ENTER: TITLE` — which is the input the machine listens for, no more
and no less. The transcript above is the pair the task asks for: each
`screen:` line is the screen rendering (its report prints the content
it drew the first frame it drew it), and the `state` line beside it is
the documented input *acting*. The victory screen's pair, from the
scratch fight (the fragile-bag roster, its own numbers):

```
engine: the waves are complete (t=1.023)
engine: state play -> victory (the game's waves are complete)
engine: screen: victory: "VICTORY" / "SCORE 000000   TIME 0:01   WAVE 3/3" / "ENTER: TITLE"
engine: screen: victory fade arrived at 16,48,16 (its own color)
```

### The fades: easing, on the presentation's clock

The toolkit's fourth effect — easing — has been waiting for the values
that deserve it. A screen appearing from nothing is exactly one: the
backdrop's color animates from black to the screen's own color over
`GAME_FADE_S`, shaped by `EaseInOutQuad`, and the run reports where it
lands:

```
engine: screen: death fade arrived at 56,16,16 (its own color)
engine: screen: victory fade arrived at 16,48,16 (its own color)
```

`56,16,16` — the death screen reddens as it arrives; the victory
screen greens (`16,48,16`); the title and pause settle on the panel
blue (`24,24,40`). "Arrived" means exactly what lesson 093's contract
says: the eased value is the target at the end — `56,16,16`, not a
rounding's width from it.

The fade's clock deserves a sentence of its own, because it is the
same split lesson 078 drew and this time it is on the other side of
it: **the fade runs on wall time**. Game time is *zero* wherever a
screen shows — the world stands still behind every panel — so a fade
on game time would never leave its first frame. The presentation has
its own clock, the feel hooks have theirs, and the simulation has
game time. Three clocks, each doing what it must.

### What this run verified, and what it did not

- **Each screen renders** — all four screens report their content as
  they draw it (`screen: title …`, `screen: pause …`, `screen: death
  …`, `screen: victory …`), each in the frame its state arrived.
- **Its input acts as documented** — every prompt names a key and the
  transition log shows that key acting: `ENTER: PLAY` → `state title ->
  play (the player started)`, `ESCAPE: RESUME` → `state pause -> play
  (the player resumed)`, `ENTER: TITLE` → `state death -> title (the
  player returned to the title)`.
- **The fades arrive exactly** — `fade arrived at 56,16,16 (its own
  color)`, the eased value landing on the screen's own color.

What this run did **not** verify is whether any of it looks *good* —
this machine has no screen to look at, and a layout is judged by eyes.
The content, the inputs, and the fade's numbers are measured; the
composition is the designer's judgment, and one exercise below routes
it to a machine with eyes. That is the honest end of the assembly: the
game is complete by the contract's checklist, and whether it is *a
good game* is now the only question left that measurement cannot
answer.

## Code step

One change: the screens. `src/game.h/.cpp` grow the four screens into
final form — `GameDrawPanel` lays out the title's name and controls,
the end screens' final numbers (the game's own score, clock, and wave),
and each screen's prompt; `Report` prints the screen's content the
first frame it draws; `GameScreenColor` fades the backdrop in from
black by the easing, arriving exactly at the screen's own color (the
death screen reddens, the victory screen greens). `Game` carries the
fade's clock — wall time, the presentation's — reset with every
transition. `src/main.cpp`'s render clears with the screen's color.
Its end state is tagged `lesson-096`.

```diff
diff --git a/src/game.cpp b/src/game.cpp
index add2def..f88c950 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -33,6 +33,9 @@ static void Transition(Game &game, GameState to, const char *why)
     std::printf("engine: state %s -> %s (%s)\n", GameStateName(game.state),
                 GameStateName(to), why);
     game.state = to;
+    /* Lesson 096: the new screen fades in from black — the fade's
+       clock starts with the screen. */
+    game.fade = 0.0;
 }
 
 const char *GameStateName(GameState state)
@@ -58,6 +61,7 @@ void GameInit(Game &game, int hero_health_full)
     game.hero_health_full = hero_health_full;
     game.play_clock = 0.0;
     game.score = 0.0;
+    game.fade = 0.0;
     game.wave = 0;
     game.camera = { 0, 0, 0, 0 };
     std::printf("engine: game: %d state%s, starting on %s\n", 5, "s",
@@ -67,6 +71,25 @@ void GameInit(Game &game, int hero_health_full)
 void GameInput(Game &game, platform::Window *window, Entity &hero,
                double wall_dt)
 {
+    /* Lesson 096: the screen's fade runs on wall time — the
+       presentation's clock. Game time is zero wherever a screen shows
+       (the world stands still behind the panel), so a fade on game
+       time would never arrive; this is lesson 078's split again, on
+       the other side of it. */
+    if (game.fade < GAME_FADE_S) {
+        game.fade += wall_dt;
+        if (game.fade >= GAME_FADE_S && game.state != GAME_PLAY) {
+            /* The fade arrives exactly at the screen's color — the
+               eased value's contract (lesson 093), measured on the
+               screen's own backdrop. Play's screen is the world and
+               fades nothing. */
+            int r, g, b;
+            GameScreenColor(game, r, g, b);
+            std::printf("engine: screen: %s fade arrived at %d,%d,%d (its own color)\n",
+                        GameStateName(game.state), r, g, b);
+        }
+    }
+
     switch (game.state) {
     case GAME_TITLE:
         /* The title screen accepts one thing: the start key. */
@@ -139,37 +162,104 @@ double GameScale(const Game &game)
     return game.state == GAME_PLAY ? GAMETIME_FULL : 0.0;
 }
 
-/* One panel screen: a title line and a hint, centred. The screen is the
-   state's — the world is not drawn behind it. The backdrop is cleared by
-   the frame's render phase before this runs, so the panel's own time is
-   the text it draws and nothing else. */
-static void Panel(Framebuffer &fb, const Font &font, const char *title,
-                  const char *hint)
+/* Lesson 096: one centered line of a screen. */
+static void Line(Framebuffer &fb, const Font &font, const char *text, int y)
 {
-    int title_x = (FRAME_WIDTH - TextWidth(title)) / 2;
-    int hint_x = (FRAME_WIDTH - TextWidth(hint)) / 2;
-    DrawText(fb, font, title, title_x, FRAME_HEIGHT / 2 - FONT_CELL);
-    DrawText(fb, font, hint, hint_x, FRAME_HEIGHT / 2 + FONT_CELL);
+    DrawText(fb, font, text, (FRAME_WIDTH - TextWidth(text)) / 2, y);
+}
+
+/* The run's final numbers, the same values the HUD reads — the score,
+   the play clock as minutes and seconds, and the wave the game ended
+   on. The end screens show them; the game's state is the game's story. */
+static void Numbers(const Game &game, char *line, int size)
+{
+    int secs = (int)game.play_clock;
+    std::snprintf(line, size, "SCORE %06d   TIME %d:%02d   WAVE %d/%d",
+                  (int)game.score, secs / 60, secs % 60, game.wave,
+                  GAME_WAVES);
+}
+
+/* Lesson 096: the screens in final form. Each state's screen is its
+   own — the world is not drawn behind it — and each names the input it
+   acts on, so the screen documents the very input the machine listens
+   for. The report below prints the screen's content the first frame it
+   draws: a run shows every screen it staged. */
+static void Report(const Game &game, const char *title, const char *body,
+                   const char *prompt)
+{
+    static GameState was = GAME_TITLE;
+    static bool first = true;
+    if (!first && was == game.state)
+        return;
+    first = false;
+    was = game.state;
+    std::printf("engine: screen: %s: \"%s\" / \"%s\" / \"%s\"\n",
+                GameStateName(game.state), title, body, prompt);
 }
 
 void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font)
 {
+    char body[64];
     switch (game.state) {
     case GAME_TITLE:
-        Panel(fb, font, "THE FRAMEBUFFER GAME", "ENTER: PLAY");
+        Line(fb, font, "THE FRAMEBUFFER GAME", 140);
+        Line(fb, font, "ARROWS  MOVE      SPACE  FIRE", 196);
+        Line(fb, font, "1 / 2   WEAPONS   ESCAPE  PAUSE", 212);
+        Line(fb, font, "ENTER: PLAY", 268);
+        Report(game, "THE FRAMEBUFFER GAME",
+               "ARROWS MOVE ... ESCAPE PAUSE", "ENTER: PLAY");
         break;
+
     case GAME_PAUSE:
-        Panel(fb, font, "PAUSED", "ESCAPE: RESUME");
+        Numbers(game, body, sizeof body);
+        Line(fb, font, "PAUSED", 180);
+        Line(fb, font, body, 228);
+        Line(fb, font, "ESCAPE: RESUME", 268);
+        Report(game, "PAUSED", body, "ESCAPE: RESUME");
         break;
+
     case GAME_DEATH:
-        Panel(fb, font, "GAME OVER", "ENTER: TITLE");
+        Numbers(game, body, sizeof body);
+        Line(fb, font, "GAME OVER", 180);
+        Line(fb, font, body, 228);
+        Line(fb, font, "ENTER: TITLE", 268);
+        Report(game, "GAME OVER", body, "ENTER: TITLE");
         break;
+
     default:
-        Panel(fb, font, "VICTORY", "ENTER: TITLE");
+        Numbers(game, body, sizeof body);
+        Line(fb, font, "VICTORY", 180);
+        Line(fb, font, body, 228);
+        Line(fb, font, "ENTER: TITLE", 268);
+        Report(game, "VICTORY", body, "ENTER: TITLE");
         break;
     }
 }
 
+void GameScreenColor(const Game &game, int &r, int &g, int &b)
+{
+    /* The screen's own backdrop, faded in from black by the ease — the
+       toolkit's easing (lesson 093) applied to a screen's fade, the
+       value arriving exactly at the screen's color. The end screens
+       carry their own tint: the death screen reddens, the victory
+       screen greens. */
+    int tr = 24, tg = 24, tb = 40;
+    if (game.state == GAME_DEATH) {
+        tr = 56;
+        tg = 16;
+        tb = 16;
+    } else if (game.state == GAME_VICTORY) {
+        tr = 16;
+        tg = 48;
+        tb = 16;
+    }
+    double t = game.fade < GAME_FADE_S ? game.fade / GAME_FADE_S : 1.0;
+    double k = EaseInOutQuad(t);
+    r = (int)(tr * k);
+    g = (int)(tg * k);
+    b = (int)(tb * k);
+}
+
 void GameFollow(Game &game, const Entity &hero, const TileMap &map)
 {
     /* Lesson 083: the camera's base follows the hero — the world scrolls
diff --git a/src/game.h b/src/game.h
index a95b466..b3bcccc 100644
--- a/src/game.h
+++ b/src/game.h
@@ -48,6 +48,10 @@ enum GameState {
    the next begins when the last enemy of the current one is retired. */
 constexpr int GAME_WAVES = 3;
 
+/* Lesson 096: the screen's fade — how long a screen takes to fade in
+   from black, on the presentation's (wall) clock. */
+constexpr double GAME_FADE_S = 0.30;
+
 /* The game's own state: which state it is in, and the facts the named
    transitions read. Nothing here is a service's — it is the game's. */
 struct Game {
@@ -58,6 +62,9 @@ struct Game {
     double score;         /* lesson 094: the game's score — the ground
                             the hero has walked, the value the HUD
                             reads and the states' reports carry */
+    double fade;          /* lesson 096: how long the screen on show
+                            has been fading in — the presentation's
+                            own clock, like the feel hooks' */
     int wave;             /* lesson 091: the wave being fought (0 = the
                             fight has not started) */
     Camera camera;        /* lesson 083: the game's world-view — one
@@ -88,11 +95,18 @@ void GameInput(Game &game, platform::Window *window, Entity &hero,
 double GameScale(const Game &game);
 
 /* The current state's screen, for the four states whose screen is a
-   panel over a still world — title, pause, death, victory. Play's screen
-   is the world the game draws below; the loop draws it and calls this
-   for the rest. */
+   panel over a still world — title, pause, death, victory — in final
+   form (lesson 096): the title's name and controls, the end screens'
+   final numbers, the prompt each input acts on. Play's screen is the
+   world the game draws below; the loop draws it and calls this for the
+   rest. */
 void GameDrawPanel(const Game &game, Framebuffer &fb, const Font &font);
 
+/* Lesson 096: the screen's backdrop — its color, faded in from black
+   over the screen's fade. The render phase clears with it; the value
+   animates by easing and arrives exactly at the screen's own color. */
+void GameScreenColor(const Game &game, int &r, int &g, int &b);
+
 /* Lesson 083: the game's world-view. The camera's base follows the hero
    — the world scrolls under the movement — clamped to the map's bounds,
    and its additive offset rests at exactly zero. The game owns the
diff --git a/src/main.cpp b/src/main.cpp
index faedbc3..f85e2f2 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -726,10 +726,16 @@ int Run(void)
            082). The backdrop is the state's own — the world's blue in
            play, the panel's darker blue on the panel screens — cleared
            once here, in the render phase, before the named sub-phases. */
-        if (game.state == GAME_PLAY)
+        if (game.state == GAME_PLAY) {
             ClearBuffer(*fb, 32, 32, 64);
-        else
-            ClearBuffer(*fb, 24, 24, 40);
+        } else {
+            /* Lesson 096: the screen's own backdrop, faded in from
+               black by its ease — the clear stays the render phase's
+               work, its color the screen's (GameScreenColor). */
+            int screen_r, screen_g, screen_b;
+            GameScreenColor(game, screen_r, screen_g, screen_b);
+            ClearBuffer(*fb, screen_r, screen_g, screen_b);
+        }
         if (game.state == GAME_PLAY) {
             /* Lesson 083: the game draws its own world — the scrolling
                map and the live entities, through the game's camera. The
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the pause over the frozen world *(extend-the-code)*

The pause screen replaces the world today — the panel *is* the screen.
The classic pause is kinder: the frozen scene stays visible, with the
panel drawn **over** it. Make it so — the world draws behind the pause
panel, and the panel sits on a rectangle of panel color so its lines
stay legible (the framebuffer grows what that needs: a clear, clipped
to a rectangle, with the fold's rule). Then run a pause and quote what
the frame record says about the world being drawn behind — and what it
says on the other screens, which draw no world at all.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-096/ex1.md)

### Exercise 2 — the fade's clock *(predict-the-output)*

The screen's fade runs on wall time — the lesson says so plainly. Make
the claim measurable: before running anything, predict what a probe
printing the fade's value beside the frame's step shows over the
death screen's first second — the step's values, the fade's values, and
the frames it takes the fade to arrive. Then run it and compare — and
answer the counterfactual in your own words: what would the death
screen look like if the fade ran on the *game's* step instead, and
which of lesson 078's clocks is each one?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-096/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 095 — audio integration](lesson-095-audio.md) ·
**Next:** [the course home](../../index.md) ·
**Code tag:** [`lesson-096`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-096)
