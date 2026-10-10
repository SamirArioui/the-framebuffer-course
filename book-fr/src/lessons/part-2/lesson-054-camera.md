# Leçon 054 — la caméra

{{#include ../../stability-horizon.md}}

## Prose

Le monde est plus grand que la fenêtre — la leçon 053 s'en est assurée. La vue
doit donc se déplacer au-dessus de lui : cette leçon livre la deuxième
obligation du MVD, les **décalages de caméra**, sous la forme d'une structure
à deux décalages et d'une règle. Le décalage de base fait défiler le monde
(l'intention du jeu : là où regarde le joueur). Le décalage additif s'empile
par-dessus et c'est le hook que la boîte à outils du juice pilotera pour le
screenshake — nul au repos, toujours. O2 est un contrat plus qu'une
fonctionnalité : le moteur doit pouvoir déplacer tout ce qui est à l'écran
*sans que la vue du jeu bouge du tout*, et cette distinction est exactement ce
que cette leçon construit et vérifie.

### Une structure, deux décalages, une règle

```c++
struct Camera {
    int base_x, base_y; /* where the view sits over the world */
    int add_x, add_y;   /* the juice hook: added on top, zero at rest */
};
```

et la règle vit dans une seule fonction, pour qu'aucun dessin ne somme jamais
les deux à sa façon :

```c++
int CameraX(const Camera &camera) { return camera.base_x + camera.add_x; }
int CameraY(const Camera &camera) { return camera.base_y + camera.add_y; }
```

Chaque dessin de scène utilise la somme **une fois, à son origine** :

```c++
    int cam_x = CameraX(camera);
    int cam_y = CameraY(camera);
    DrawTileMap(*fb, map, sheet, -cam_x, -cam_y);
    BlitSprite(*fb, sprite, (int)sprite_x - cam_x, (int)sprite_y - cam_y);
```

Position monde moins le décalage — la règle que la marche de la leçon 053 suit
déjà, désormais alimentée par la caméra. Notez qui n'est *pas* dans la liste :
le HUD. `DrawText` dessine dans l'espace écran, à dessein. Le monde défile ; la
ligne de score du joueur appartient au joueur et ne part pas vagabonder avec le
décor. « Dessin de scène » désigne le monde — la carte et ce qui vit dedans.

Une conséquence traverse tout le code : **le sprite vit désormais dans l'espace
monde.** Son bornage est passé des limites de l'écran à celles de la carte
(`0` à `map_width × TILE_SIZE − sprite_width`) — le héros marche dans le monde,
pas dans la fenêtre, et la caméra décide ce que la fenêtre montre.

### Les trois scénarios, vérifiés

La spécification derrière cette leçon nomme trois comportements ; le bloc de
vérification transforme chacun en comparaison de pixels — dessinez la scène
avec un état de caméra, dessinez-la avec un autre, et confrontez les deux
pixel par pixel :

```
engine: camera check: base (100,50) scrolls the scene — 232200 pixels compared, 0 mismatches
engine: camera check: additive (7,-3) stacks over base — 307200 pixels compared, 0 mismatches
engine: camera check: additive cleared restores the base view — 307200 pixels compared, 0 mismatches
```

- **La caméra de base fait défiler la scène** — la vue avec une base (100, 50)
  est la vue en (0, 0) avec chaque pixel déplacé d'exactement (−100, −50) :
  232 200 pixels de chevauchement comparés, tous identiques.
- **Le décalage additif s'empile** — la base (100, 50) avec un additif (7, −3)
  dessine *exactement* ce que dessine la base (107, 47) sans additif : la somme
  est la seule chose que les dessins voient, vérifiée sur toute la frame.
- **Effacer l'additif restaure la vue de base** — la scène après le retour de
  l'additif à zéro est identique à l'octet près à la scène avec la base seule.
  Pas « à peu près » : les mêmes pixels, 307 200 d'entre eux.

### Le hook, piloté

La démo fait bouger les deux décalages. La base de la caméra suit le sprite —
bornée à la carte, si bien que la vue s'arrête aux bords du monde — et le
sprite marche dans le monde pendant que la fenêtre suit :

```
engine: arrow keys move the sprite, space shakes the camera; close the window to stop
engine: sprite at 312,232
engine: camera base 0,0 (t=0.000)
engine: camera base 128,0 (t=2.473)
engine: sprite at 471,232 (t=2.473)
...
```

`camera base 128,0` est le bornage qui fait son travail : le monde fait 768
pixels de large et la frame 640, donc `base_x` plafonne à 128 — la vue montre
le bord droit du monde et pas au-delà.

Le hook additif est sur la touche espace :

```
engine: camera additive 6,0 (shake starts)
...
engine: camera additive 0,0 (at rest)
```

Ceci est une *démonstration* du hook, pas la boîte à outils : une secousse fixe
qui se termine à zéro. La boîte à outils du juice de la partie 4 possédera la
vraie chose — hitstop, screenshake, salves de particules, easing — et pilotera
exactement ce champ. Le contrat dont elle a besoin est ce que cette leçon
livre : l'additif peut déplacer chaque pixel de l'écran sans perturber la base,
et l'effacer restaure la vue exactement.

### Pourquoi deux décalages, au juste

L'alternative — un seul décalage, la secousse ajoutée dans la base puis
retranchée — fonctionne jusqu'à ce que les deux choses se contredisent : le jeu
veut regarder le joueur pendant que la secousse veut ne regarder rien en
particulier. Des champs séparés signifient que l'intention du jeu (la base)
n'est jamais corrompue par la rétroaction (l'additif), et qu'une secousse de
n'importe quelle taille — ou une poussée de hitstop à l'écran, ou un rebond
d'easing — se compose par-dessus n'importe quelle vue sans que la logique du
monde sache qu'elle existe. L'additif est un hook *pour le moteur de rendu* ;
la base est une déclaration *sur le jeu*.

## Étape de code

Un seul changement pour cette leçon : `src/camera.h` / `src/camera.cpp`
apportent la caméra (la structure et la somme), et `main.cpp` dessine la scène
à travers elle (`DrawScene` — la carte et le sprite à position monde moins le
décalage sommé), vérifie les trois scénarios de la spécification pixel par
pixel, suit le sprite avec la caméra de base et pilote le hook additif depuis
la touche espace. Le bornage du sprite passe des limites de l'écran à celles
de la carte. Le HUD, la police (font) et le blitter sont intacts. Son état
final est étiqueté `lesson-054`.

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

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La secousse qui décroît *(extend-the-code)*

La secousse de la démo est une onde carrée : ±6 pendant trente frames, puis
zéro. Faites-la décroître — l'amplitude qui descend par paliers (6, 4, 2, 0) à
mesure que la secousse s'épuise — et rapportez chaque changement de l'additif
quand il se produit. Vérifiez le chemin dans le journal et que l'exécution se
termine exactement à `0,0`. Qu'est-ce que le fait de « décroître » apporte au
ressenti de l'effet, et qu'est-ce que le *rapport* apporte à la prochaine
personne qui lira votre exécution ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-054/ex1.md)

### Exercice 2 — L'additif qui annule *(predict-the-output)*

La somme est la seule chose que les dessins voient — l'additif peut donc faire
subir à la base des choses qui ressemblent à du travail de caméra sans en
être. Avant de lancer quoi que ce soit, prédisez à quoi ressemble la scène
avec la base `(100, 50)` et l'additif `(−100, −50)`, et dites à quelle
vérification de la leçon 045 cela ressemble. Ajoutez ensuite le cas aux
vérifications de caméra — dessinez-le, comparez-le à la vue d'origine — et
réconciliez. La question à garder : si la boîte à outils du juice peut annuler
la base, que doit-elle promettre de ne jamais faire ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-054/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 053 — le dessin de tilemap](lesson-053-tiles.md) ·
**Suivante :** [Leçon 055 — les types de tuile et la solidité](lesson-055-collision.md) ·
**Étiquette de code :** [`lesson-054`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-054)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-054-camera.md`,
révision `b7f3628`.*

<!-- translation-source: book/lessons/part-2/lesson-054-camera.md @ b7f3628 -->
