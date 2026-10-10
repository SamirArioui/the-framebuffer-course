# Leçon 096 — le peaufinage des écrans

{{#include ../../stability-horizon.md}}

## Prose

Quatre écrans se tiennent derrière ce jeu depuis la leçon 082 — title, pause,
death, victory — chacun une ligne de titre et une indication, faisant leur
travail et rien de plus. Cette leçon est la dernière de l'assemblage : les
écrans sous leur **forme finale**. Ce que « finale » veut dire ici est petit
et concret : un écran-titre qui enseigne les commandes, des écrans de fin qui
racontent l'histoire de l'exécution, chaque écran nommant l'entrée précise sur
laquelle il agit — et le quatrième effet de la boîte à outils trouvant enfin
son foyer naturel, dans les **fondus des écrans**.

### Les écrans disent ce qu'ils sont

L'écran de jeu possède le HUD (leçon 094) ; les autres états possèdent leurs
écrans (design D9), et le contenu de chaque écran est désormais toute
l'histoire de l'état :

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

L'écran-titre enseigne : le nom du jeu, les quatre commandes (déplacement,
tir, les deux armes, pause) et l'invite qui le démarre. La pause et les écrans
de fin portent les nombres finaux de l'exécution —
`SCORE 000424   TIME 0:06   WAVE 1/3` — les *mêmes valeurs que lit le HUD* :
le score du jeu, l'horloge de jeu, la vague sur laquelle le jeu s'est terminé.
L'état est l'histoire du jeu, et les écrans de fin ne font que la raconter.

Et chaque écran nomme son entrée — `ENTER: PLAY`, `ESCAPE: RESUME`,
`ENTER: TITLE` — qui est l'entrée que la machine écoute, ni plus ni moins. La
transcription ci-dessus est la paire que la tâche demande : chaque ligne
`screen:` est l'écran en train de se dessiner (son rapport imprime le contenu
dessiné à la première frame où il l'a dessiné), et la ligne `state` à côté est
l'entrée documentée *en action*. La paire de l'écran de victoire, tirée du
combat jetable (la liste des sacs fragiles, ses propres nombres) :

```
engine: the waves are complete (t=1.023)
engine: state play -> victory (the game's waves are complete)
engine: screen: victory: "VICTORY" / "SCORE 000000   TIME 0:01   WAVE 3/3" / "ENTER: TITLE"
engine: screen: victory fade arrived at 16,48,16 (its own color)
```

### Les fondus : l'easing, sur l'horloge de la présentation

Le quatrième effet de la boîte à outils — l'easing — attendait les valeurs qui
le méritent. Un écran qui apparaît à partir de rien en est exactement une : la
couleur du fond s'anime du noir vers la couleur propre à l'écran sur
`GAME_FADE_S`, façonnée par `EaseInOutQuad`, et l'exécution rapporte où elle
atterrit :

```
engine: screen: death fade arrived at 56,16,16 (its own color)
engine: screen: victory fade arrived at 16,48,16 (its own color)
```

`56,16,16` — l'écran de mort rougit en arrivant ; l'écran de victoire verdit
(`16,48,16`) ; le titre et la pause se posent sur le bleu du panneau
(`24,24,40`). « Arrivé » veut dire exactement ce que dit le contrat de la
leçon 093 : la valeur passée par l'easing est la cible à la fin — `56,16,16`,
pas à une largeur d'arrondi d'elle.

L'horloge du fondu mérite sa propre phrase, car c'est la même séparation que la
leçon 078 a tracée, et cette fois elle se trouve de l'autre côté : **le fondu
tourne à l'horloge murale**. Le temps de jeu est *nul* partout où un écran
s'affiche — le monde reste immobile derrière chaque panneau — si bien qu'un
fondu au temps de jeu ne quitterait jamais sa première frame. La présentation
a sa propre horloge, les hooks du ressenti ont la leur, et la simulation a le
temps de jeu. Trois horloges, chacune faisant ce qu'elle doit.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Chaque écran se dessine** — les quatre écrans rapportent leur contenu au
  moment où ils le dessinent (`screen: title …`, `screen: pause …`,
  `screen: death …`, `screen: victory …`), chacun dans la frame où son état
  est arrivé.
- **Son entrée agit comme documenté** — chaque invite nomme une touche et le
  journal des transitions montre cette touche en action : `ENTER: PLAY` →
  `state title -> play (the player started)`, `ESCAPE: RESUME` → `state pause
  -> play (the player resumed)`, `ENTER: TITLE` → `state death -> title (the
  player returned to the title)`.
- **Les fondus arrivent exactement** — `fade arrived at 56,16,16 (its own
  color)`, la valeur passée par l'easing atterrissant sur la couleur propre de
  l'écran.

Ce que cette exécution n'a **pas** vérifié, c'est si tout cela est *beau* —
cette machine n'a pas d'écran où regarder, et une mise en page se juge à
l'œil. Le contenu, les entrées et les nombres du fondu sont mesurés ; la
composition relève du jugement du designer, et un exercice ci-dessous la route
vers une machine pourvue d'yeux. C'est la fin honnête de l'assemblage : le jeu
est complet selon la liste du contrat, et savoir si c'est *un bon jeu* est
désormais la seule question qui reste et que la mesure ne peut pas trancher.

## Étape de code

Un changement : les écrans. `src/game.h/.cpp` font grandir les quatre écrans
jusqu'à leur forme finale — `GameDrawPanel` dispose le nom et les commandes du
titre, les nombres finaux des écrans de fin (le score, l'horloge et la vague
du jeu lui-même) et l'invite de chaque écran ; `Report` imprime le contenu de
l'écran à la première frame où il le dessine ; `GameScreenColor` fait
apparaître le fond en fondu depuis le noir par l'easing, en arrivant
exactement sur la couleur propre de l'écran (l'écran de mort rougit, l'écran
de victoire verdit). `Game` porte l'horloge du fondu — l'horloge murale, celle
de la présentation — remise à zéro à chaque transition. Le render de
`src/main.cpp` efface avec la couleur de l'écran. Son état final est étiqueté
`lesson-096`.

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

## Exercices

Deux défis plus ambitieux. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La pause par-dessus le monde figé *(extend-the-code)*

L'écran de pause remplace aujourd'hui le monde — le panneau *est* l'écran. La
pause classique est plus douce : la scène figée reste visible, le panneau se
dessinant **par-dessus**. Faites en sorte qu'il en soit ainsi — le monde se
dessine derrière le panneau de pause, et le panneau repose sur un rectangle de
couleur de panneau pour que ses lignes restent lisibles (le framebuffer
grandit ce qu'il faut pour cela : un effacement découpé à un rectangle, avec
la règle du pli). Lancez ensuite une pause et citez ce que l'enregistrement de
frame dit du monde dessiné derrière — et ce qu'il dit sur les autres écrans,
qui ne dessinent aucun monde.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-096/ex1.md)

### Exercice 2 — L'horloge du fondu *(predict-the-output)*

Le fondu de l'écran tourne à l'horloge murale — la leçon le dit sans détour.
Rendez l'affirmation mesurable : avant de rien lancer, prédisez ce qu'une
sonde imprimant la valeur du fondu à côté du pas de la frame montre pendant la
première seconde de l'écran de mort — les valeurs du pas, celles du fondu, et
les frames qu'il faut au fondu pour arriver. Lancez-la ensuite et comparez —
puis répondez au contrefactuel dans vos propres mots : à quoi ressemblerait
l'écran de mort si le fondu tournait au *pas du jeu* à la place, et laquelle
des horloges de la leçon 078 est chacune ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-096/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 095 — l'intégration audio](lesson-095-audio.md) ·
**Suivante :** [Leçon 097 — payer la dette](lesson-097-debt.md) ·
**Étiquette de code :** [`lesson-096`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-096)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-096-screens.md`,
révision `3e7f026`.*

<!-- translation-source: book/lessons/part-5/lesson-096-screens.md @ 3e7f026 -->
