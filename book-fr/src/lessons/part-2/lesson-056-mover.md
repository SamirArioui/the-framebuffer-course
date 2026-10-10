# Leçon 056 — le mover qui s'arrête aux murs

{{#include ../../stability-horizon.md}}

## Prose

Il y a deux leçons, la carte a commencé à répondre aux questions ; aujourd'hui,
quelque chose les pose : **le mover** — un sprite doté d'une intention, les
flèches du clavier, et une règle : le mouvement n'a lieu que là où la carte
l'autorise. Le héros de la partie 4 commence ici : c'est le premier morceau de
code où *vouloir* se déplacer et *pouvoir* se déplacer sont deux choses
différentes. Les requêtes de la leçon 055 sont le mur ; le mover est la chose
qui s'arrête.

### L'intention, puis la permission

La phase d'update sépare les deux :

```c++
        double move_x = 0.0, move_y = 0.0;
        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
            move_x -= SPRITE_SPEED * dt;
        ... /* intent: where the player wants to go this frame */

        double next_x = sprite_x + move_x;
        if (!TileRectSolid(map, (int)next_x, (int)sprite_y, sprite.width,
                           sprite.height))
            sprite_x = next_x;
        double next_y = sprite_y + move_y;
        if (!TileRectSolid(map, (int)sprite_x, (int)next_y, sprite.width,
                           sprite.height))
            sprite_y = next_y;
```

Trois décisions dans ces lignes :

- **L'intention n'est pas le mouvement.** Les touches calculent où le sprite
  *veut* être ; la requête décide où il *parvient* à être. Un déplacement refusé
  n'a simplement pas lieu — la position ne change pas, pas de glissement, pas de
  rebond, pas d'erreur.
- **Un axe à la fois.** Le déplacement en x est testé puis appliqué, ensuite
  celui en y. C'est ce qui fait fonctionner le glissement le long des murs : en
  poussant en diagonale contre un mur, le mouvement *le long* du mur survit
  tandis que le mouvement *vers* le mur est refusé. Testez plutôt les deux axes
  ensemble, et un appui en diagonale contre un mur colle le joueur dessus.
- **Le rectangle, c'est le sprite.** La requête reçoit la taille du sprite — sa
  boîte 16×16 en coordonnées monde — donc « puis-je être ici ? » se pose avec
  l'emprise réelle du sprite, pas avec son point central.

Et une fois les requêtes en service, le bornage de la leçon 054 prend sa
retraite : la bordure de la carte est solide, les requêtes traitent le bord du
monde comme solide, donc le mover ne peut pas quitter la carte et aucun test de
bornes séparé n'est nécessaire. La frontière est désormais une donnée.

### L'exécution

Entrée scriptée — maintenez Gauche contre la bordure, puis Haut le long du mur
de gauche, puis Droite le long du bord supérieur :

```
engine: the sprite stops at walls (lesson 056's mover)
engine: sprite at 312,232
engine: camera base 0,0 (t=0.000)
engine: sprite at 27,232 (t=2.718)
engine: sprite at 18,232 (t=2.758)
engine: sprite blocked at 18,232 (t=2.799)
engine: sprite unblocked at 18,232 (t=3.552)
engine: sprite at 18,156 (t=3.867)
engine: sprite blocked at 18,156 (t=4.528)
engine: sprite unblocked at 18,146 (t=4.568)
engine: sprite at 18,146 (t=4.568)
engine: sprite at 18,136 (t=4.609)
...
engine: sprite at 18,21 (t=5.090)
engine: sprite blocked at 18,21 (t=5.130)
engine: sprite unblocked at 21,18 (t=5.383)
engine: sprite at 21,18 (t=5.383)
engine: sprite at 179,18 (t=6.044)
```

Relisez cette exécution en regardant la carte : le sprite démarre sur le sol en
`(312, 232)`, marche vers la gauche et **s'arrête à x = 18** — le mur de
bordure occupe les pixels 0..15 et la boîte de 16 pixels de large du sprite ne
peut jamais les chevaucher. La ligne de rapport `sprite blocked at 18,232` est
le mover qui constate qu'il voulait bouger et ne l'a pas fait. Puis Haut le
long du mur : le sprite grimpe à x = 18 — là même où le mur avait bloqué son
demi-pas vers la gauche — atteint le mur du haut (y s'arrête à 21), et quand
Droite est enfoncée il **glisse** le long du mur supérieur : `unblocked at
21,18` puis jusqu'à `179,18`. Bloquer un axe ne bloque jamais l'autre.

### Le pas qui bondit

Une paire de lignes de cette exécution ne parle pas de murs du tout :

```
engine: sprite at 18,156 (t=3.867)
engine: sprite blocked at 18,156 (t=4.528)
engine: sprite unblocked at 18,146 (t=4.568)
```

Le sprite montait, à 156 — loin de tout mur — et le mover a rapporté *blocked*.
Puis il a bougé de nouveau, dix pixels d'un coup. La cause est la vieille
connaissance de la leçon 034 : **les frames arrivent quand les nouvelles
arrivent.** Regardez les horodatages : la frame à t=3.867 est arrivée 0,315 s
après la précédente, et la frame à t=4.528 est arrivée 0,66 s plus tard. Le
mover avance de `speed × dt` par frame — donc cette frame unique a tenté de
déplacer le sprite de **158 pixels en un seul pas**, de y=156 jusqu'au-delà du
bord supérieur de la carte. La requête a fait son travail : une boîte 16×16 à
la destination est hors de la carte, le bord est solide, le pas est refusé —
*tout le pas*, y compris les 140 pixels parfaitement valides qu'il contenait.

C'est le comportement honnête du code tel qu'il est écrit, et il nomme la vraie
limite du mover : **le pas, c'est tout ou rien.** Une longue frame ne déplace
pas le joueur à mi-chemin du mur ; elle ne le déplace pas du tout, et la frame
ordinaire suivante reprend à partir de là. Le correctif — borner `dt` à la
valeur d'une frame, ou avancer par sous-pas de taille fixe jusqu'à ce que la
requête refuse — est celui de l'exercice 2, et le héros de la partie 4 voudra
l'avoir. Le contrôle à garder : le rapport disait *blocked* et le sprite n'a
vraiment pas bougé. Le mover ne ment jamais sur ce qu'il a fait ; il fait
simplement moins que ce qu'il pourrait.

### Ce dont le mover est le commencement

La règle du mover — *vouloir, demander, bouger ou pas* — est toute la forme de
la logique de jeu dans ce moteur. Le héros de la partie 4 ajoute l'accélération
et le knockback à l'intention, les ennemis ajoutent leurs propres intentions,
et le projet final ajoute la résolution quand deux movers veulent la même
case ; la demande passe toujours par ces requêtes, et les réponses viennent
toujours des données de la carte. Les transitions du rapport (`blocked` /
`unblocked`) sont la graine du genre de logique de jeu instrumentée dont le
rapport de budget de frames rendra compte un jour.

## Étape de code

Un seul changement pour cette leçon : `main.cpp` remplace le bornage du mover
par un mouvement conditionné par la collision — l'intention calculée à partir
des touches, les pas en x et en y testés séparément contre `TileRectSolid`, et
les transitions `blocked` / `unblocked` rapportées. La carte, les requêtes et
le chemin de dessin ne sont pas touchés. Son état final est étiqueté
`lesson-056`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 180c505..9bfc720 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -591,9 +591,11 @@ int Run(void)
     double started = platform::Now();
     double last = started;
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
+    bool was_blocked = false; /* lesson 056: the mover's state report */
 
     std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
+    std::printf("engine: the sprite stops at walls (lesson 056's mover)\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
     /* The frame step: read news, update from polled state, draw, present —
@@ -616,25 +618,42 @@ int Run(void)
         last = now;
 
         int old_x = (int)sprite_x, old_y = (int)sprite_y;
+        double was_x = sprite_x, was_y = sprite_y;
+        double move_x = 0.0, move_y = 0.0;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            sprite_x -= SPRITE_SPEED * dt;
+            move_x -= SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            sprite_x += SPRITE_SPEED * dt;
+            move_x += SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_UP))
-            sprite_y -= SPRITE_SPEED * dt;
+            move_y -= SPRITE_SPEED * dt;
         if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            sprite_y += SPRITE_SPEED * dt;
-
-        /* The sprite stays in the world — the map's bounds now, not the
-           screen's: the camera moves the view, the world is bigger. */
-        if (sprite_x < 0)
-            sprite_x = 0;
-        if (sprite_x > map.width * TILE_SIZE - sprite.width)
-            sprite_x = map.width * TILE_SIZE - sprite.width;
-        if (sprite_y < 0)
-            sprite_y = 0;
-        if (sprite_y > map.height * TILE_SIZE - sprite.height)
-            sprite_y = map.height * TILE_SIZE - sprite.height;
+            move_y += SPRITE_SPEED * dt;
+
+        /* Lesson 056: the mover — intent becomes motion only where the
+           map allows it. One axis at a time, so a wall blocks the
+           movement into it and the movement along it still works. The
+           clamp of lesson 054 retires: the world's edge is solid, and
+           the queries are the boundary now. */
+        double next_x = sprite_x + move_x;
+        if (!TileRectSolid(map, (int)next_x, (int)sprite_y, sprite.width,
+                           sprite.height))
+            sprite_x = next_x;
+        double next_y = sprite_y + move_y;
+        if (!TileRectSolid(map, (int)sprite_x, (int)next_y, sprite.width,
+                           sprite.height))
+            sprite_y = next_y;
+
+        /* The mover reports its state on transitions: moving, or pushed
+           against something that will not move. The comparison is on the
+           exact positions — a sub-pixel step is movement, not a wall. */
+        bool blocked = (move_x != 0.0 || move_y != 0.0) &&
+                       sprite_x == was_x && sprite_y == was_y;
+        if (blocked != was_blocked) {
+            std::printf("engine: sprite %s at %d,%d (t=%.3f)\n",
+                        blocked ? "blocked" : "unblocked", (int)sprite_x,
+                        (int)sprite_y, platform::Now() - started);
+            was_blocked = blocked;
+        }
 
         /* Lesson 054: the camera's base follows the sprite — the world
            scrolls under the movement — clamped to the map's bounds. */
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La hitbox *(extend-the-code)*

Le dessin du sprite et sa boîte de collision sont le même rectangle
aujourd'hui. Rétrécissez la boîte de collision de deux pixels de chaque côté —
la hitbox classique à l'intérieur du dessin — et poussez le mover contre le mur
qu'il ne pouvait pas atteindre auparavant. Relisez les pixels là où le dessin
touche le mur et dites ce que le joueur voit, puis répondez à la question de
design : pourquoi les jeux gardent-ils une hitbox plus petite que le dessin, et
quand est-ce le mauvais choix ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-056/ex1.md)

### Exercice 2 — Le pas et le mur *(predict-the-output)*

L'exécution de la leçon contient un rapport `blocked` à y=156 sans aucun mur
en vue. Avant de changer quoi que ce soit, notez par écrit ce qui est arrivé à
cette frame — les horodatages du journal sont la preuve — et prédisez ce que le
rapport de position fractionnaire (imprimé à deux décimales) montrera dans le
cas ordinaire : en allant vers la gauche contre le mur de bordure, à quelle
distance de x=16 le sprite s'arrête-t-il réellement, et pourquoi n'atterrit-il
pas exactement sur la frontière ? Étendez ensuite le rapport à deux décimales,
relancez les deux cas et réconciliez. La question à emporter : que devrait
faire le mover des pas trop grands pour être franchis ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-056/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 055 — les types de tuile et la solidité](lesson-055-collision.md) ·
**Suivante :** [Leçon 057 — la démo de clôture : le monde, dessiné](lesson-057-demo.md) ·
**Étiquette de code :** [`lesson-056`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-056)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-056-mover.md`,
révision `853ad02`.*

<!-- translation-source: book/lessons/part-2/lesson-056-mover.md @ 853ad02 -->
