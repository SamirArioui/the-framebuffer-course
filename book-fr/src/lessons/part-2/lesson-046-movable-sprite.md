# Leçon 046 — le sprite se déplace

{{#include ../../stability-horizon.md}}

## Prose

Le marqueur tire sa révérence aujourd'hui. C'était un carré dessiné à coups
d'appels `PutPixel` — un placeholder avec un travail : tenir la part pilotée par
l'entrée de la boucle jusqu'à ce que quelque chose de réel puisse se tenir là.
Ce quelque chose est le sprite, et cette leçon le déplace avec la même entrée
scrutée que le marqueur utilisait. Rien de la boucle ne change : pompe, update,
render, present — la forme que la leçon 043 a fixée, tenue honnête par
l'enregistrement de la leçon 036. Ce qui change, c'est ce que la boucle *dessine*
et ce que l'enregistrement peut en *dire* : pour la première fois, la phase de
rendu d'une frame nomme où son temps est passé.

### Le marqueur tire sa révérence

La phase d'update est inchangée dans sa structure : l'état scruté des flèches
ajoute `speed × dt` à la position du sprite, et le sprite reste à l'écran grâce
au pli de la leçon 015 à l'échelle de la frame. Ce qui a changé, c'est l'objet :

- `DrawMarker` et `MARKER_SIZE` ont **disparu**. Le carré 24×24 redessiné chaque
  frame par quatre boucles imbriquées de `PutPixel` est remplacé par un seul
  appel : `BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y)`.
- La taille du sprite vient de l'asset (`sprite.width`, `sprite.height`), pas
  d'une constante. Le bornage est `FRAME_WIDTH - sprite.width` — la même
  arithmétique, qui lit désormais la chose qu'elle garde à l'écran.
- Les rapports disent `sprite` là où ils disaient `marker`, et la position
  qu'ils rapportent est la position où le blit dessine.

L'exécution sous entrée scriptée (flèche droite maintenue, puis bas) :

```
$ DISPLAY=:99 ./build/game
engine: sprite assets/sprite.ppm: 16x16, 768 pixel bytes
...
engine: arrow keys move the sprite; close the window to stop
engine: sprite at 312,232
frame 1: update 0.000 ms, render 0.372 ms (sprites 0.001), present 0.565 ms, total 0.938 ms
frame 2: update 0.000 ms, render 0.400 ms (sprites 0.001), present 0.928 ms, total 1.329 ms
engine: sprite at 470,232 (t=2.661)
engine: sprite at 480,232 (t=2.701)
engine: sprite at 490,232 (t=2.742)
...
engine: sprite at 547,235 (t=3.031)
...
engine: 15 frames — avg 0.913 ms (update 0.000, render 0.435 incl. sprites 0.001, present 0.478)
engine: worst frame 1.329 ms (frame 2); present is 52% of the frame
```

La vérification par relecture confirme que les pixels et le rapport sont
d'accord : à la position rapportée `547,235`, la fenêtre contient le fond au
coin de couleur clé du sprite `(547,235)` et la couleur du centre du sprite
`(220, 40, 40)` en `(555, 243)` — le pixel `(8, 8)` du sprite, exactement là où
le rapport a posé le sprite. Le moteur ne déplace pas une variable ; il déplace
une chose que la fenêtre montre.

### L'enregistrement nomme ses phases

L'enregistrement de `frame.h` gagne un champ, et c'est le début du rapport de
budget de frames :

```c++
    /* Lesson 046: the render phase starts naming what is inside it — one
       field per subsystem, the attribution the frame-budget table
       (lesson 058) grows from. The named times are inside render, never
       instead of it: render stays the phase, these say where it went. */
    double sprites; /* sprite draws through the blit */
```

La mesure suit la même discipline des deux lectures d'horloge que chaque phase :
lire `platform::Now()` avant le blit, le lire après, soustraire. La ligne du
journal gagne le champ nommé là où il appartient — à l'intérieur de la phase de
rendu à laquelle il appartient :

```
frame 1: update 0.000 ms, render 0.372 ms (sprites 0.001), present 0.565 ms, total 0.938 ms
```

et le cumul l'additionne comme il additionne tout le reste, pour que le résumé
de l'exécution puisse attribuer la frame moyenne :

```
engine: 15 frames — avg 0.913 ms (update 0.000, render 0.435 incl. sprites 0.001, present 0.478)
```

Deux règles gardent l'attribution honnête à mesure qu'elle grandit (c'est ce qui
rendra la table finale digne de confiance plutôt que décorative) :

1. **Les temps nommés vivent à l'intérieur de leur phase, jamais à sa place.**
   `render` reste dans l'enregistrement exactement tel que la leçon 036 l'a
   publié. Les sous-champs disent où le rendu est passé ; ils ne remplacent pas
   ce qu'il a mesuré.
2. **Le format grandit, la forme non.** Une ligne par enregistrement, des champs
   nommés — le contrat de la leçon 036 était un format *texte* stable, pas un
   format figé. Les phases texte et tilemap rejoindront `sprites` de la même
   façon, dans les leçons qui les introduisent.

### Et le premier nombre est une surprise

Regardez le cumul encore une fois : `render 0.435 incl. sprites 0.001`. La frame
passe 435 microsecondes à rendre et **une seule** à dessiner le sprite. Où sont
passées les 434 autres ? Dans `ClearBuffer` — peindre les 640 × 480 = 307 200
pixels de fond avant que quoi que ce soit y soit dessiné. Le sprite, c'est 256
pixels de vrai travail de dessin ; le clear, douze cents fois plus de pixels de
« il n'y a rien à voir ici ».

C'est le premier coût honnête d'un moteur de rendu logiciel, et c'est exactement
le sujet de la leçon suivante. Pas « comment le rendre plus rapide » — c'est le
travail de la partie 5 — mais *pourquoi copier des octets coûte-t-il si cher*,
ce qui s'avère une question sur la machine, pas sur le code. Les phases nommées
de l'enregistrement de frame ont gagné leur place dès leur première sortie :
sans `sprites` dans le journal, « render » aurait ressemblé à un unique nombre
opaque et l'histoire à l'intérieur serait restée invisible.

## Étape de code

Un seul changement pour cette leçon : `main.cpp` met `DrawMarker` à la retraite
et déplace le sprite avec la même entrée scrutée, `frame.h` / `frame.cpp` font
grandir la première sous-phase nommée du rendu dans l'enregistrement (`sprites`)
ainsi que sa somme dans le cumul, et la ligne de journal et le résumé portent le
nom. Le blit, le sprite et l'asset sont intacts. Son état final est étiqueté
`lesson-046`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index d5e2920..e104e47 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -13,6 +13,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.render_sum += frame.render;
     stats.present_sum += frame.present;
     stats.total_sum += frame.total;
+    stats.sprites_sum += frame.sprites;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
diff --git a/src/frame.h b/src/frame.h
index 3ca1cdb..bea2fbf 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -17,6 +17,12 @@ struct FrameRecord {
     double render; /* drawing the scene into the framebuffer */
     double present;/* the copy to the window, sync included */
     double total;  /* the whole frame step */
+
+    /* Lesson 046: the render phase starts naming what is inside it — one
+       field per subsystem, the attribution the frame-budget table
+       (lesson 058) grows from. The named times are inside render, never
+       instead of it: render stays the phase, these say where it went. */
+    double sprites; /* sprite draws through the blit */
 };
 
 /* The running account: every frame measured so far. */
@@ -26,6 +32,7 @@ struct FrameStats {
     double render_sum;
     double present_sum;
     double total_sum;
+    double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
 };
diff --git a/src/main.cpp b/src/main.cpp
index 7b750ac..1f8ad5c 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -16,17 +16,10 @@
 
 namespace engine {
 
-/* The marker: one square the arrow keys move. Its speed is the engine's —
-   pixels per second — and the clock's dt turns it into a per-frame step. */
-constexpr int MARKER_SIZE = 24;
-constexpr double MARKER_SPEED = 240.0; /* pixels per second */
-
-static void DrawMarker(Framebuffer &fb, int x, int y)
-{
-    for (int j = 0; j < MARKER_SIZE; ++j)
-        for (int i = 0; i < MARKER_SIZE; ++i)
-            PutPixel(fb, x + i, y + j, 240, 220, 80);
-}
+/* The scene's one object: the sprite the arrow keys move. Its speed is
+   the engine's — pixels per second — and the clock's dt turns it into a
+   per-frame step. */
+constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
 int Run(void)
 {
@@ -159,14 +152,14 @@ int Run(void)
     std::printf("engine: blit check: clip at -4,-4 landed %d pixels, %d wrong, %d touched outside\n",
                 landed, wrong, wrapped);
 
-    double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
-    double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
+    double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
+    double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
     double last = started;
 
     std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
-    std::printf("engine: arrow keys move the marker; close the window to stop\n");
-    std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
+    std::printf("engine: arrow keys move the sprite; close the window to stop\n");
+    std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
     /* The frame step: read news, update from polled state, draw, present —
        every phase measured, one record per frame. */
@@ -187,38 +180,40 @@ int Run(void)
         double dt = now - last;
         last = now;
 
-        int old_x = (int)marker_x, old_y = (int)marker_y;
+        int old_x = (int)sprite_x, old_y = (int)sprite_y;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            marker_x -= MARKER_SPEED * dt;
+            sprite_x -= SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            marker_x += MARKER_SPEED * dt;
+            sprite_x += SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_UP))
-            marker_y -= MARKER_SPEED * dt;
+            sprite_y -= SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            marker_y += MARKER_SPEED * dt;
+            sprite_y += SPRITE_SPEED * dt;
 
-        /* The marker stays on screen — lesson 015's fold at frame scale. */
-        if (marker_x < 0)
-            marker_x = 0;
-        if (marker_x > FRAME_WIDTH - MARKER_SIZE)
-            marker_x = FRAME_WIDTH - MARKER_SIZE;
-        if (marker_y < 0)
-            marker_y = 0;
-        if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
-            marker_y = FRAME_HEIGHT - MARKER_SIZE;
+        /* The sprite stays on screen — lesson 015's fold at frame scale. */
+        if (sprite_x < 0)
+            sprite_x = 0;
+        if (sprite_x > FRAME_WIDTH - sprite.width)
+            sprite_x = FRAME_WIDTH - sprite.width;
+        if (sprite_y < 0)
+            sprite_y = 0;
+        if (sprite_y > FRAME_HEIGHT - sprite.height)
+            sprite_y = FRAME_HEIGHT - sprite.height;
 
         frame.update = platform::Now() - t0;
         double t1 = platform::Now();
 
-        if ((int)marker_x != old_x || (int)marker_y != old_y)
-            std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
-                        (int)marker_y, platform::Now() - started);
+        if ((int)sprite_x != old_x || (int)sprite_y != old_y)
+            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
+                        (int)sprite_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then the
-           sprite through the one blit. */
+           sprite through the one blit. The sprite draw is timed as its own
+           named phase: the first subsystem the frame record can name. */
         ClearBuffer(*fb, 32, 32, 64);
-        BlitSprite(*fb, sprite, 32, 32);
-        DrawMarker(*fb, (int)marker_x, (int)marker_y);
+        double t_sprites = platform::Now();
+        BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
+        frame.sprites = platform::Now() - t_sprites;
 
         frame.render = platform::Now() - t1;
         double t2 = platform::Now();
@@ -239,20 +234,22 @@ int Run(void)
         frame.total = platform::Now() - t0;
         AccountFrame(stats, frame);
 
-        /* The frame log: one line per record — the format Part 2 grows. */
-        std::printf("frame %ld: update %.3f ms, render %.3f ms, present %.3f ms, total %.3f ms\n",
+        /* The frame log: one line per record — the format grows its named
+           fields, one per subsystem, as the parts name them. */
+        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.update * 1e3, frame.render * 1e3,
-                    frame.present * 1e3, frame.total * 1e3);
+                    frame.sprites * 1e3, frame.present * 1e3,
+                    frame.total * 1e3);
     }
 
     /* The account: what the frames actually cost, including the honest
        price of the presentation copy. */
     if (stats.frames) {
         double n = (double)stats.frames;
-        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f, present %.3f)\n",
+        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, present %.3f)\n",
                     stats.frames, stats.total_sum / n * 1e3,
                     stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
-                    stats.present_sum / n * 1e3);
+                    stats.sprites_sum / n * 1e3, stats.present_sum / n * 1e3);
         std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
                     stats.worst * 1e3, stats.worst_number,
                     100.0 * stats.present_sum / stats.total_sum);
```

## Exercices

Deux extensions « faites-les vôtres ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — Où passe le render ? *(measure-the-performance)*

Le cumul dit `render 0.435 incl. sprites 0.001` — alors nommez la phase
manquante et mesurez-la : donnez à `ClearBuffer` son propre champ nommé dans
l'enregistrement (`clear`), chronométrez-la de la même façon, et faites porter
les deux noms à la ligne de journal et au résumé du cumul. Avant de lancer :
prédisez comment les deux phases nommées se partageront le temps de rendu, et
dites ce que ce partage signifie pour un moteur de rendu qui dessine un seul
sprite 16×16 par frame. Lancez ensuite et réconciliez — et notez quelle phase un
second sprite changerait.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-046/ex1.md)

### Exercice 2 — Le sprite qui se retourne *(extend-the-code)*

Le bornage est une politique, pas une loi. Faites en sorte que le sprite se
retourne (wrap) : qu'il quitte un bord de la frame et réapparaisse au bord
opposé, en allant dans la même direction — et laissez-le voyager entièrement
hors de la frame en chemin, le découpage du blit gardant les pixels honnêtes
pendant les frames de traversée. Vérifiez avec une entrée scriptée et une
relecture à la position rapportée des deux côtés d'un bord. Qu'est-ce que le
bornage vous achetait, que le retournement coûte désormais ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-046/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 045 — le blit découpé et transparent](lesson-045-blit.md) ·
**Suivante :** [Leçon 047 — plongée dans les caches](lesson-047-caches.md) ·
**Étiquette de code :** [`lesson-046`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-046)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-046-movable-sprite.md`,
révision `cfc6de7`.*

<!-- translation-source: book/lessons/part-2/lesson-046-movable-sprite.md @ cfc6de7 -->
