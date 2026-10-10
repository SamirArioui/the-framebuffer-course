# Leçon 083 — la tilemap et la caméra

{{#include ../../stability-horizon.md}}

## Prose

Le squelette a une forme désormais — cinq états, un à la fois. Mais l'écran du
jeu est encore le monde que la *boucle* dessine. Cette leçon le fait devenir
celui du jeu : **le monde du jeu est une seule carte défilante, et le jeu la
regarde à travers sa propre caméra.** La carte et la caméra ne sont pas de
nouveaux services — la partie 2 a construit la tilemap et la leçon 054 a défini
la caméra — mais jusqu'ici, la boucle les possédait et les déplaçait. À partir
d'ici, le jeu possède son monde et sa vue sur lui (conception D2) : la caméra
vit dans `Game`, le jeu dessine sa carte et ses entités à travers elle, et la
boucle ne garde que le chronométrage et l'enregistrement de frame.

### La caméra a deux décalages, et le jeu les possède

La caméra, c'est une structure et deux décalages (leçon 054) :

```cpp
struct Camera {
    int base_x, base_y; /* where the view sits over the world */
    int add_x, add_y;   /* the juice hook: added on top, zero at rest */
};
```

La **base** est l'origine de la vue sur la carte — c'est elle qui fait défiler
le monde. Le décalage **additif** est le hook que la boîte à outils du juice
pilotera pour le screenshake (leçon 092) ; il repose à exactement zéro. Chaque
dessin de scène utilise la somme, une fois. La caméra est `Game::camera`
désormais — la vue du monde du jeu — et deux choses lui arrivent à chaque frame
de jeu : la base suit le héros, et l'additif reste au repos.

### La base suit le héros, bornée à la carte

La base vise à garder le héros centré : la position du héros, plus la moitié de
son sprite, moins la moitié de la frame. C'est l'origine de la vue — *si* elle
est à l'intérieur de la carte. Elle est bornée pour que la fenêtre ne montre
jamais au-delà du bord du monde :

```cpp
int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2;
if (base_x < 0) base_x = 0;
if (base_x > map.width * TILE_SIZE - FRAME_WIDTH)
    base_x = map.width * TILE_SIZE - FRAME_WIDTH;
```

La carte fait `48×32` cellules de `TILE_SIZE` 16 — `768×512` pixels — et la
frame fait `640×480`. L'origine de la vue peut donc se trouver n'importe où dans
`x ∈ [0, 128]`, `y ∈ [0, 32]` : `768 − 640 = 128`, `512 − 480 = 32`. À
l'intérieur de ce rectangle, la caméra suit le héros ; aux bords, elle s'arrête,
et le bord du monde s'aligne sur celui de la fenêtre.

D'après une vraie exécution de l'état final de cette leçon, pilotée par une
entrée scriptée sous l'affichage sans écran — le héros a marché à droite, puis
est revenu à gauche :

```
engine: hero at 312,232
engine: hero at 389,232 (t=3.792)
engine: camera base 77,0
engine: hero at 290,232 (t=9.224)
engine: camera base 0,0
```

et, dans une exécution où le héros a atteint l'extrême droite :

```
engine: hero at 437,232 (t=3.548)
engine: camera base 125,0
engine: hero at 441,235 (t=3.562)
engine: camera base 128,3
```

Lisez la base contre l'arithmétique. Au héros `389`, la base vaut
`389 + 8 − 320 = 77` — la caméra suit, centrée sur le héros. Au héros `441`,
`441 + 8 − 320 = 129`, mais la base affiche `128` : le bornage l'a retenue au
bord droit de la carte (`768 − 640`). Et au héros `290`, `290 + 8 − 320 = −22`,
mais la base affiche `0` : bornée au bord gauche. La caméra suit le héros où
qu'il aille et s'arrête aux limites du monde — elle ne montre jamais d'espace
vide au-delà de la carte.

### La carte est dessinée à travers la caméra du jeu

Le dessin du monde est au jeu désormais : `GameDrawMap` dessine l'unique carte
défilante et `GameDrawSprites` dessine les entités vivantes, les deux à travers
la caméra du jeu (la somme, une fois). La boucle ne tend plus la main vers la
carte ni vers une caméra à elle — elle chronomètre les deux dessins comme les
sous-phases nommées de l'enregistrement de frame et passe à la suite. Le HUD
reste au-dessus du monde et ne défile *pas* (la règle de la leçon 054) ; il est
dessiné en dehors de la caméra et c'est l'affaire des prochaines leçons.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **La carte se dessine** — la sous-phase `tilemap` coûte du vrai temps en jeu
  (le monde est rendu) et rien sur les panneaux.
- **La caméra suit le héros** — la base traque `hero + half sprite − half frame`,
  observée bouger avec le héros tout au long de l'exécution.
- **La caméra est bornée aux limites de la carte** — la base affiche `0` et
  `128` là où l'arithmétique passerait en négatif ou au-delà du bord : la vue
  ne montre jamais au-delà du monde.

Ce que cette leçon ne fait **pas**, c'est résoudre le héros contre la carte — le
héros marche encore dans les murs et à travers eux, et les exécutions ci-dessus
le montrent s'arrêtant à des endroits où il ne devrait pas. C'est la collision
avec les tuiles, et c'est la leçon suivante. Cette leçon n'est que la vue : le
jeu possède son monde et la caméra à travers laquelle il le regarde.

## Étape de code

Un changement : la caméra devient celle du jeu, et le jeu dessine son monde à
travers elle. `src/game.h` accueille `Game::camera` et déclare trois choses que
la boucle demande au jeu — `GameFollow` (la base suit le héros, bornée), et
`GameDrawMap` / `GameDrawSprites` (le monde à travers la caméra du jeu, scindé
pour que l'enregistrement de frame continue de chronométrer les deux
sous-phases). `src/game.cpp` les implémente — le bornage du suivi déménage ici
depuis la boucle, et les deux boucles de dessin aussi. `src/main.cpp` abandonne
sa propre `Camera`, le suivi en ligne et le dessin du monde en ligne ; il
appelle les trois fonctions du jeu et ne garde que le chronométrage. Le décalage
additif de la caméra repose à zéro dans `GameFollow`, le hook qui attend la
leçon 092. Son état final est étiqueté `lesson-083`.

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

## Exercices

Deux défis plus conséquents. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La caméra qui anticipe *(extend-the-code)*

Aujourd'hui la caméra centre le héros, si bien que le héros est toujours au
milieu de l'écran et que le joueur voit autant derrière que devant. Faites
*anticiper* la caméra : visez la base un peu dans la direction où le héros va,
pour que le joueur voie où il se dirige. La requête de déplacement du héros
(`move_x`, `move_y`) ou son facing vous dit la direction ; choisissez une
distance d'anticipation en pixels et décalez la base vers elle. Gardez le
bornage aux limites de la carte (la caméra ne doit toujours jamais montrer
au-delà du monde) et gardez le décalage additif au repos. Jugez ensuite en
exécutant : le héros est-il décentré dans la direction du déplacement, et le
bornage tient-il toujours aux bords ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-083/ex1.md)

### Exercice 2 — La base, aux coins *(predict-the-output)*

Le bornage est de l'arithmétique, donc il est prévisible. Avant de lancer quoi
que ce soit, calculez la base de la caméra quand le héros se trouve à chacun
des quatre coins de la carte (en pixels du monde `(0,0)`, `(752,0)`, `(0,496)`,
`(752,496)` — le coin supérieur gauche du sprite à chaque coin) et quand le
héros est à sa position de départ `(312,232)`. Rappelez-vous que le sprite fait
`16×16`, la frame `640×480`, et la carte `768×512`. Écrivez ensuite une petite
sonde (un print dans `GameFollow` quand la base change) et conduisez le héros
vers chaque coin pour confronter vos cinq réponses à l'exécution. Quels coins
bornent sur les deux axes, et lesquels sur un seul ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-083/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 082 — le squelette du jeu](lesson-082-skeleton.md) ·
**Suivante :** [Leçon 084 — la collision avec les tuiles](lesson-084-collision.md) ·
**Étiquette de code :** [`lesson-083`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-083)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-083-tilemap-camera.md`,
révision `3e7f026`.*

<!-- translation-source: book/lessons/part-5/lesson-083-tilemap-camera.md @ 3e7f026 -->
