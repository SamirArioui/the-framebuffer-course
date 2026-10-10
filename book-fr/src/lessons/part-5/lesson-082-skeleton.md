# Leçon 082 — le squelette du jeu

{{#include ../../stability-horizon.md}}

## Prose

La partie 4 s'est refermée sur un héros parcourant la tilemap sur des services
terminés — la tranche verticale — et sur une chose que la tranche n'avait
délibérément *pas* : un jeu. C'était une seule boucle avec un script de démo
tenant lieu des parties qui n'étaient pas encore arrivées. Cette leçon met en
place la vraie chose. L'idée est celle sur laquelle toute la partie 5 repose :
**un jeu n'est pas une boucle avec des drapeaux — il tourne dans exactement un
de cinq états, et chaque état possède son écran et son entrée.**

### Cinq états, pas une boucle

Les états sont nommés (conception D7) : **titre, jeu, pause, mort, victoire**.
Une valeur `GameState` dit dans lequel le jeu se trouve, et seule l'entrée de
cet état agit. Rien de « si en pause, dessine le menu, sinon dessine le jeu »
éparpillé dans la boucle — la boucle reste la boucle (lire les nouvelles,
mettre à jour, alimenter le flux, dessiner, présenter), et le *jeu* en dessous
est une machine à états dans sa propre paire de fichiers, `game.h` /
`game.cpp`, à côté des services plutôt qu'en leur sein (conception D2).

```cpp
enum GameState {
    GAME_TITLE = 0,
    GAME_PLAY,
    GAME_PAUSE,
    GAME_DEATH,
    GAME_VICTORY,
};
```

Chaque état répond à deux questions et à rien d'autre : *que dessine cet
état ?* et *quelle entrée cet état accepte-t-il ?* L'écran titre dessine son
panneau et accepte la touche de démarrage ; le jeu dessine le monde et accepte
le déplacement ; la pause dessine son panneau et accepte la reprise. L'état
règle aussi le seul nombre qui fait bouger le monde — l'échelle du temps de jeu
(le bouton de la leçon 078) : **le jeu est à pleine vitesse, tout autre état est
à zéro.**

### Les transitions sont nommées

Les changements d'état ne sont pas des accidents — ce sont des conditions
nommées, et l'exécution imprime chacune au moment où elle survient. D'après une
vraie exécution de l'état final de cette leçon, pilotée par une entrée scriptée
sous l'affichage sans écran (flèches, Enter, Escape, Space) :

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

et, dans une seconde exécution où la touche de complétion est pressée en jeu,
la complétion du jeu elle-même :

```
engine: state title -> play (the player started)
engine: the waves are complete (t=0.644)
engine: state play -> victory (the game's waves are complete)
```

Lisez les parenthèses : ce sont les conditions nommées. Le jeu commence depuis
le titre (« the player started ») ; la pause suspend le jeu et le reprend ; **la
mort suit la défaite du héros** — les points de vie du héros ont atteint zéro ;
**la victoire suit la complétion du jeu** — les vagues du jeu sont terminées.
Le `switch` de `GameInput` lit les touches de l'état courant et les conditions
nommées une fois par frame, et chaque transition imprime la raison pour laquelle
elle a eu lieu. Une touche de pause est déclenchée sur front à travers la
mémorisation (latch) de la leçon 033, si bien qu'un appui ne peut pas mettre en
pause et reprendre aussitôt.

### La simulation reste immobile en dehors de l'état jeu

C'est la partie qui fait de « un état à la fois » plus qu'une commodité de
dessin. L'état règle l'échelle du temps de jeu, et l'échelle est la *seule*
chose qui fait avancer le monde. En dehors de l'état jeu, l'échelle est zéro,
donc le pas de l'update vaut zéro seconde de jeu — le monde se fige — pendant
que la présentation continue de dessiner l'écran de cet état. L'enregistrement
de frame montre les deux moitiés à la fois. Trois frames de l'exécution
ci-dessus — le panneau titre, une frame de jeu, et le panneau de pause :

```
frame 1: step 0.000 ms, update 0.001 ms (entities 0.000), audio 0.000 ms, render 0.859 ms (sprites 0.000, text 0.008, tilemap 0.000), present 0.924 ms, total 1.784 ms
frame 3: step 20.211 ms, update 0.002 ms (entities 0.002), audio 0.000 ms, render 1.391 ms (sprites 0.002, text 0.008, tilemap 0.941), present 0.942 ms, total 2.336 ms
frame 4: step 0.000 ms, update 0.004 ms (entities 0.002), audio 0.000 ms, render 0.472 ms (sprites 0.000, text 0.006, tilemap 0.000), present 0.448 ms, total 0.924 ms
```

Ligne par ligne, voici le contrat, mesuré :

- **`step 0.000` en dehors de l'état jeu** (frames 1 et 4) et un vrai pas en jeu
  (frame 3). Le pas est le temps de jeu (leçon 079) — le monde n'a rien avancé
  sur les frames de titre et de pause.
- **L'update tourne quand même.** `update 0.001 / 0.004 ms` sur ces mêmes
  frames en pause — la lecture de l'entrée, les rapports, le travail propre de
  la frame ont tous eu lieu. Le *monde* est resté immobile ; la *frame* non.
  C'est la différence entre figer avec l'échelle et sauter l'update.
- **L'écran est celui de l'état.** En jeu, `tilemap 0.941` — le monde est
  dessiné. Sur les panneaux, `tilemap 0.000` et seul `text` coûte quoi que ce
  soit — le panneau est dessiné et le monde ne l'est pas. Chaque état dessine
  son propre écran, et les nombres disent quel écran a tourné.

### Deux substituts, nommés

Deux des cinq conditions nommées lisent un état du jeu qui n'existe pas encore :
le combat réduit les points de vie du héros (leçon 087) et les vagues dépensent
la complétion du jeu (leçon 091). En attendant qu'ils arrivent, cette leçon
démontre les deux transitions avec un **substitut**, le même procédé que la
leçon 078 utilisait pour montrer l'échelle avant que la boîte à outils du juice
ne la pilote :

- **Space est un coup porté au héros** — elle baisse les points de vie du héros
  de un, et des points de vie à zéro sont la condition de défaite. La leçon 087
  remplace la touche par de vrais coups.
- **Enter en jeu, c'est le jeu terminé** — elle met les vagues à zéro, et plus
  de vague restante est la condition de complétion. La leçon 091 la remplace
  par de vraies vagues.

Les deux sont déclenchés par une touche — ils se déclenchent quand le joueur
appuie sur une touche et jamais d'eux-mêmes pendant un test de jeu — et les
deux sont marqués comme échafaudage dans `game.cpp`, retirés quand les vrais
déclencheurs arrivent. Les *transitions* qu'ils déclenchent — défaite à zéro
point de vie, complétion sans vague — sont celles du jeu et ne changent pas.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Un état à la fois, chacun avec son propre écran et sa propre entrée.** Le
  journal des transitions visite chaque état exactement une fois par chemin, et
  les lignes de frame montrent l'écran de chaque état à l'œuvre (le monde en
  jeu, un panneau ailleurs).
- **Les transitions sont les conditions nommées,** chacune imprimée avec la
  raison que la spécification donne : started, paused, resumed, the hero's
  health reached zero, the waves are complete, returned to the title.
- **La simulation reste immobile en dehors de l'état jeu** — `step 0.000` sur
  chaque frame hors jeu pendant que l'update continue de tourner et la
  présentation de dessiner.

Ce que cette leçon ne fait **pas**, c'est rendre le jeu amusant, ou les états
intéressants. L'état jeu est encore la tranche de la partie 4 ; les panneaux
font deux lignes de texte ; les deux conditions de fin sont des substituts.
C'est l'affaire des quatorze prochaines leçons. Le travail du squelette est plus
petit et plus important : le jeu a une forme désormais, et chaque leçon
ultérieure grandit *en* elle — le combat dans l'update du jeu, les vagues dans
la condition de victoire, les écrans jusqu'à leur forme finale — plutôt que de
visser des drapeaux sur une boucle.

## Étape de code

Un changement pour cette leçon : la machine à états se dresse comme la première
paire de fichiers de la couche jeu, et la boucle y est câblée. `src/game.h` et
`src/game.cpp` sont nouveaux — l'énumération `GameState`, l'état `Game`, et les
quatre choses que la boucle demande à la machine (`GameInit`, `GameInput`,
`GameScale`, `GameDrawPanel`). `src/main.cpp` garde la boucle et le comportement
de jeu de la partie 4, mais consulte désormais la machine : il appelle
`GameInput` pour l'entrée de l'état et les transitions nommées, règle l'échelle
du temps de jeu depuis `GameScale` au lieu d'un script de démo, et dessine
l'écran de l'état courant — la scène en jeu, le panneau de l'état sinon.
L'échafaudage de démo que la tranche portait (le script d'échelle à l'horloge
murale et la secousse de Space) a disparu : l'échelle est à l'état désormais, et
Space est le coup. Le décalage additif de la caméra est laissé à exactement
zéro, le hook qui attend la leçon 092. Son état final est étiqueté `lesson-082`.

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

## Exercices

Deux défis plus conséquents. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le score sur les écrans de fin *(extend-the-code)*

L'exécution tient un score — la distance que le héros a parcourue — et le HUD du
jeu l'affiche. Mais quand le héros meurt ou que le jeu est gagné, l'écran de fin
ne dit rien de la façon dont l'exécution s'est passée. Portez le score dans
l'état du jeu lui-même (c'est un fait du jeu, pas de la boucle) et dessinez-le
sur les panneaux de mort et de victoire, pour que le joueur voie le résultat de
l'exécution sur l'écran qui le rapporte. Pendant que vous êtes dans les états de
fin, élargissez leur entrée : aujourd'hui, seul Enter ramène au titre — laissez
Escape le faire aussi, comme Escape quitte déjà le jeu pour la pause. Gardez la
règle de la machine : seule l'entrée de l'état courant agit, et gardez la
mémorisation d'entrée à son poste (un appui, une action).

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-082/ex1.md)

### Exercice 2 — Pourquoi l'échelle *(explain-in-prose)*

La simulation reste immobile en dehors de l'état jeu parce que l'état règle
l'échelle du temps de jeu à zéro — pas parce que la boucle saute l'update, et
pas parce que le dessin s'arrête. Défendez ce choix dans vos propres mots.
Concrètement : que casserait le fait que la boucle fasse simplement `continue`
au-delà de l'update quand le jeu est en pause, au lieu de mettre le pas à zéro ?
Pensez à l'enregistrement de frame (que requiert le contrat de la leçon 079
d'une frame en pause ?), à la mémorisation d'entrée (une touche de pause se
comporterait-elle encore bien ?), et au jour où une échelle *fractionnaire*
arrive (le hitstop de la leçon 092 — un update sauté est-il la même chose qu'un
update ralenti ?). Répondez ensuite à la question de mesure : les lignes de
frame ci-dessus montrent `update` coûtant du vrai temps pendant que `step` est à
zéro — que prouve cette paire de nombres sur l'endroit où vit le gel ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-082/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 081 — le coût de la tranche dans le budget de frames](../part-4/lesson-081-entities-row.md) ·
**Suivante :** [Leçon 083 — la tilemap et la caméra](lesson-083-tilemap-camera.md) ·
**Étiquette de code :** [`lesson-082`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-082)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-082-skeleton.md`,
révision `3e7f026`.*

<!-- translation-source: book/lessons/part-5/lesson-082-skeleton.md @ 3e7f026 -->
