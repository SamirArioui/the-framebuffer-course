# Leçon 094 — le HUD

{{#include ../../stability-horizon.md}}

## Prose

Le jeu se joue désormais, mais il ne se *lit* pas : le score, la vie du héros
et l'horloge vivent à l'intérieur de l'exécution et n'atteignent le joueur que
sous forme de lignes imprimées dans un journal. Cette leçon les dessine là où
le joueur regarde — le **HUD**, les indicateurs de l'écran de jeu — et la
règle qui gouverne chaque nombre qu'il porte :

> les indicateurs sont **l'état du jeu lui-même**, lus au moment du dessin —
> les mêmes valeurs sur lesquelles les états agissent — et dessinés par-dessus
> le monde **sans la caméra**.

Deux moitiés, et toutes deux mesurables. La première porte sur le *temps* : un
indicateur qui montre une valeur périmée est pire que pas d'indicateur du
tout. La seconde porte sur l'*espace* : le HUD est le tableau de bord du
joueur, et un tableau de bord ne défile pas hors de vue.

### Les indicateurs changent dans la frame de l'état lui-même

Le HUD est une nouvelle paire de fichiers à côté des services
(`hud.h`/`hud.cpp`, design D2) avec une seule fonction : `HudDraw` lit le jeu
et dessine quatre lignes — `SCORE` et `HEALTH` au bord gauche, `WAVE` et
`TIME` à droite. Rien ici ne garde sa propre copie de quoi que ce soit : le
score est celui du jeu, la vie est `hero.health` comparée au maximum du héros,
la vague est `game.wave` parmi les vagues du jeu, le temps est l'horloge de
jeu — les valeurs que les transitions de la machine à états lisent et
impriment.

Le premier scénario de la spécification est qu'un changement d'état atteint
l'indicateur **dans la même frame**. L'exécution le mesure : les
chauves-souris portent deux coups au héros, et la ligne `hud:` de chaque coup
figure au compte de la même frame que le coup lui-même — avant la ligne
`frame` de cette frame, jamais celle d'après :

```
engine: hit: bolt hits hero — damage 1, health 3 -> 2
engine: hud: score 000000, health 2/3, time 0:04, wave 1/3 — at 8,8 over camera 8,0
frame 102: step 43.246 ms, …
engine: hit: bolt hits hero — damage 1, health 2 -> 1
engine: hud: score 000000, health 1/3, time 0:04, wave 1/3 — at 8,8 over camera 8,0
frame 103: step 10.972 ms, …
```

(Le pas de `10.972 ms` de la frame 103 est le hitstop de la leçon 092, qui
répond au coup de la même frame — la boîte à outils et le HUD lisant un même
événement.)

L'indicateur de vague bouge avec le combat de vague, dans la frame de la
vague elle-même :

```
engine: state title -> play (the player started)
engine: wave 1 begins — 2 enemies
engine: hud: score 000000, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 8,0
frame 2: step 3.671 ms, …
```

et le second scénario — la vie qui chute *et qui est restaurée* — est tout le
cycle mort-et-redémarrage, les indicateurs suivant chaque pas :

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

`health 0/3` dans la frame où le héros tombe ; `health 3/3` dans celle où la
partie neuve le restaure. (Deux bolts arrivent dans la dernière frame de la
vie du héros — le second bolt de la volée touche un héros déjà à zéro, le
même cas limite d'une frame que la leçon 092 a consigné ; l'indicateur montre
`0/3` une fois, dans la frame où l'état a changé.)

### Le HUD ne défile pas

La leçon 054 a séparé l'écran en deux espaces de coordonnées et a donné à
chacun sa règle : la **scène** se dessine à moins la caméra — elle défile ;
le **texte à l'écran** appartient au joueur et ne bouge pas avec le monde.
`HudDraw` l'applique à la lettre : les indicateurs sont disposés en
coordonnées écran (`8,8` et le bord droit de la frame), et aucune valeur de
caméra ne leur est appliquée nulle part. Le monde se dessine derrière eux —
la phase de render de la frame dessine d'abord la carte et les sprites, puis
le texte du HUD par-dessus.

Parcourez la carte et regardez l'exécution rapporter les deux coordonnées
ensemble — l'ancre à laquelle le HUD a dessiné, et où se trouvait la caméra :

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

La caméra parcourt toute son étendue — de `8` à `128`, le défilement complet
de la carte — et chaque ligne dit `at 8,8`. Le score s'égrène à côté (le héros
marche : le score du jeu est le terrain que le héros a couvert, la valeur que
la ligne de score de la tranche compte depuis la leçon 080 et qui vit
désormais dans l'état du jeu lui-même). Les indicateurs restent ; le monde
bouge.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Les indicateurs correspondent au jeu dans la même frame** — chaque `hit:`
  et chaque `wave N begins` est suivi de la ligne `hud:` qui le montre avant
  la ligne `frame` de cette frame : la vie chute de `3/3 → 2/3` dans la frame
  du coup et revient à `3/3` dans celle de la partie neuve ; l'indicateur de
  vague bouge avec le combat de vague.
- **Le HUD ne défile pas** — sur tout le trajet `8 → 128` de la caméra,
  l'ancre lit `8,8` sur chaque ligne, dessinée par-dessus le monde (la
  sous-phase `text` de la phase de render s'exécute après `tilemap` et
  `sprites`).

Ce que cette exécution n'a **pas** vérifié, c'est l'*allure* du HUD — aucun
écran ici pour juger si quatre lignes de texte de 8 pixels se lisent bien
par-dessus un monde chargé. Les coordonnées et les valeurs sont mesurées ; la
lisibilité relève encore du jugement du joueur (et un exercice la route vers
une machine pourvue d'yeux). Et ce que le score *signifie* est le choix du jeu
— celui-ci compte le terrain que le héros a couvert ; un score **pour les
mises à mort** veut que les lignes disent ce que vaut une mise à mort, une
donnée que le format peut faire grandir — le premier exercice ci-dessous fait
cette extension, à la manière de la leçon 087.

## Étape de code

Un changement : le HUD. `src/hud.h/.cpp` sont la paire de fichiers propre aux
indicateurs (design D2) : `HudDraw` lit l'état du jeu et dessine les quatre
lignes en coordonnées écran, et l'exécution rapporte ce qu'elle a lu et où
elle l'a dessiné dès que l'un ou l'autre change. `src/game.h/.cpp` font
grandir le score — le terrain que le héros a parcouru passe de la variable
locale de la boucle dans `Game`, où le HUD et les états peuvent le lire, et
une partie neuve le remet à zéro. Le render du jeu dans `src/main.cpp` dessine
à travers `HudDraw` au lieu de son texte en ligne. Son état final est étiqueté
`lesson-094`.

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

## Exercices

Deux défis plus ambitieux. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le score pour les mises à mort *(extend-the-code)*

Le jeu compte le terrain que le héros parcourt, mais un combat de vague veut
un score **pour les mises à mort** — et ce que vaut une mise à mort est un
fait par type : sa donnée est une colonne que ce format peut faire grandir,
comme la leçon 087 l'a fait grandir (nommée, additive, chaque fichier livré se
chargeant encore octet pour octet). Faites que les lignes des ennemis donnent
un prix à leur type et qu'une mise à mort ajoute les points de sa ligne au
score du jeu. Puis lancez, et citez le score qui bouge dans la frame de la
mise à mort — et la règle de compatibilité qui tient sur `assets/entities.txt`,
qui ne doit pas changer d'un octet.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-094/ex1.md)

### Exercice 2 — Pourquoi le HUD ne défile jamais *(explain-in-prose)*

La règle de la leçon 054 — le texte à l'écran appartient au joueur et ne bouge
pas avec le monde — est ce qui garde les indicateurs de cette leçon à `8,8`.
Défendez cette règle dans vos propres mots. Concrètement : si `HudDraw`
appliquait les décalages de la caméra comme le fait `GameDrawMap`, où se
trouverait la ligne `SCORE` une fois la caméra au défilement complet de la
carte (la frame fait 640 de large et la caméra parcourt 128), et pourquoi
est-ce non seulement laid, mais faux sur ce qu'un HUD *est* ? Répondez ensuite
à la question de mesure à partir d'une exécution à vous qui parcourt la carte
de bout en bout avec une sonde imprimant les coordonnées de dessin des
indicateurs à côté de celles de la caméra — et dites ce que les nombres
prouvent.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-094/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 093 — rafales de particules et easing](lesson-093-bursts-easing.md) ·
**Suivante :** [Leçon 095 — l'intégration audio](lesson-095-audio.md) ·
**Étiquette de code :** [`lesson-094`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-094)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-094-hud.md`,
révision `bf9b9df`.*

<!-- translation-source: book/lessons/part-5/lesson-094-hud.md @ bf9b9df -->
