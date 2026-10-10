# Leçon 076 — le héros comme entité

{{#include ../../stability-horizon.md}}

## Prose

La leçon 075 s'est terminée sur un magasin (store) que la frame parcourt et rien
dedans qui vaille la peine d'être parcouru : le sprite de la démo se déplaçait
encore tout seul sur ses propres `double`s à côté du magasin, et les entités
restaient dans leurs emplacements à se faire compter. Deux choses clochent là-dedans, et
cette leçon corrige les deux d'un coup : le héros n'est pas un sprite plus des
variables en pagaille, et le travail par entité d'une frame n'est pas une
vérification de mise à mort. L'idée ici est donc : **le héros est la première
entité sur laquelle le jeu agit** — une ligne portant position, sprite et
vitesse, déplacée par l'état d'entrée par scrutation, dessinée à travers la
caméra. Le sprite de la démo est retiré ; l'entité est ce que le jeu voit
désormais.

### La ligne porte le mouvement

L'entité du héros vient de sa ligne depuis trois leçons ; ce que cette leçon
change, c'est qui lui écrit. Les champs de l'entité appartiennent au jeu :

```cpp
struct Entity {
    char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
    double x, y;               /* where it is, in world pixels */
    int facing;                /* 0 right, 1 down, 2 left, 3 up */
    int speed;                 /* world pixels per second */
    int health;                /* points */
    const Sprite *sprite;      /* the art it draws, from its row */
    double move_x, move_y;     /* this frame's movement request */
    bool live;
};
```

Deux champs méritent leur mot. **La position est un `double`** — la ligne
énonce des pixels entiers (`312`), parce qu'un fichier le doit, mais le
mouvement n'est pas entier : un pas de `240 px/s × 16 ms` fait 3,84 pixels, et
une position qui tronque à chaque frame perd la fraction et déplace le héros
plus lentement que sa ligne ne le dit. Les pixels sont entiers ; les positions
ne le sont pas (le sprite de la leçon 046 se déplaçait de la même façon, et le
dessin arrondit une fois). Et **la requête de déplacement appartient à
l'entité** — `move_x`, `move_y`, la direction où le travail de cette frame veut
aller. L'entrée du joueur l'écrit pour le héros ; l'IA de la partie 5 l'écrit
pour les ennemis. Rien n'écrit `x` et `y` directement, sauf la marche.

### L'intention est au jeu ; le pas est à la marche

L'update lit l'entrée une fois par frame — l'état par scrutation, jamais les
événements (leçon 032) — et écrit la requête du héros :

```cpp
hero.move_x = 0.0;
hero.move_y = 0.0;
if (platform::KeyDown(opened.window, platform::KEY_LEFT))
    hero.move_x -= 1.0;
...
```

Puis la marche transforme la requête de chaque entité en mouvement, une fois,
dans un seul corps :

```cpp
e.x += e.move_x * e.speed * dt;
e.y += e.move_y * e.speed * dt;
```

C'est le modèle de mouvement, et il est volontairement simple : la requête est
une direction, le `speed` de la ligne dit à quelle vitesse, et le `dt` de la
frame (leçon 035) la transforme en pixels. La ligne du héros dit 240 — donc
maintenir une direction le déplace de 240 pixels par seconde de temps de jeu, et
rien d'autre dans le moteur ne connaît ce nombre. Changez la ligne et le héros
change de vitesse sans reconstruction. Le `facing` de l'entité suit là où elle
va (0 droite, 1 bas, 2 gauche, 3 haut — les quatre du format), donc ce que le
jeu lit pour l'animation ou la visée n'est jamais périmé.

Ce que la marche ne fait *pas* encore, c'est demander la permission au monde. Le
mouvement est de l'arithmétique ; la carte n'est pas consultée. C'est délibéré
et c'est la prochaine leçon — mais c'est visible dans l'exécution d'aujourd'hui :
le héros scripté a marché droit au-delà du bord droit de la carte, jusqu'à x 872
dans un monde large de 768 pixels, pendant que la caméra — bornée aux limites de
la carte comme la leçon 054 les a définies — restait à 128,32 et montrait le
vide au-delà des tuiles. Le mouvement appartient à l'entité ; la réponse du
monde à ce mouvement appartient à la leçon 077.

### Dessiné à travers la caméra

La phase sprites du render est désormais une marche, le miroir de celle de
l'update :

```cpp
for (int i = 0; i < ENTITY_CAP; ++i) {
    if (!store.slots[i].live)
        continue;
    const Entity &e = store.slots[i];
    BlitSprite(*fb, *e.sprite, (int)e.x - CameraX(camera),
               (int)e.y - CameraY(camera));
}
```

L'art de chaque entité vivante à sa position, à travers le décalage sommé de la
caméra — le même blit, la même caméra, une boucle au lieu d'un appel. Une
nouvelle ligne dans la table est une nouvelle entité, et elle se dessine sans
changement de code. La base de la caméra suit le héros (le centre de son art,
borné aux limites de la carte) exactement comme elle suivait le sprite de la
démo — la caméra n'a jamais su qu'elle suivait un sprite ; elle suit une
position.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

D'une vraie exécution de l'état final de cette leçon sur cette machine, pilotée
par une entrée scriptée (`xdotool key --repeat`, douze Droites et six Bas) :

- **Le héros bouge sous l'entrée scriptée, à la vitesse de sa ligne.**
  L'exécution rapporte `engine: hero at 312,232` au repos puis une ligne par
  frame en mouvement : `engine: hero at 792,232 (t=2.004)` — la première frame
  appuyée a porté 481 pixels de mouvement (les 240 px/s de la ligne × les 2,004
  s de `dt` d'inactivité de la frame) et la position entière lit 792 — puis
  `800`, `807`, `814`, `821` … à l'espacement des frames du script, et
  `872,276` après les appuis Bas. Le pas est la vitesse de la ligne fois le `dt`
  de la frame, à chaque frame.
- **La caméra suit et borne.** `engine: camera base 128,8` … `128,32` pendant
  que le héros allait à droite et en bas : 128, c'est `map.width * TILE_SIZE -
  FRAME_WIDTH` et 32 celui de la hauteur, les limites propres de la carte — la
  caméra réconciliée contre le monde, pas contre le héros.
- **Chaque entité est parcourue et dessinée une fois par frame.** Le bilan de la
  marche se referme encore : `engine: walk: 301 visits over 38 frames` — 8 + 5
  + 36 × 8, le compte des vivantes de chaque frame, sommé.

Ce que cette leçon ne vérifie **pas**, c'est le monde qui répond : le héros
traverse les murs et sort de la carte, comme documenté ci-dessus. La leçon 077
résout le mouvement de chaque entité contre les requêtes de collision du
tilemap, et le héros s'arrête aux murs et glisse le long d'eux — l'habitude du
mover de la leçon 056, appliquée aux entités.

Une note honnête sur les chiffres : sur cette machine, l'exécution n'a pas de
sortie audio, donc la boucle ne se réveille que sur les nouvelles de l'entrée et
chaque appui scripté réveille deux frames (l'appui et le relâchement). Seule la
frame qui trouve la touche enfoncée déplace le héros, c'est pourquoi la cadence
scriptée avance de 7-8 pixels par frame plutôt que 14. Sur un vrai bureau — ou
avec une sortie audio fonctionnelle qui donne le tempo à la boucle — une touche
maintenue déplace le héros à chaque frame, et l'exercice ci-dessous vous demande
de mesurer exactement cela.

## Étape de code

Un seul changement pour cette leçon, du sprite à l'entité : `src/entity.h`
accueille la position dont le mouvement a besoin (`x`, `y` deviennent des
doubles) et la requête de déplacement de la frame (`move_x`, `move_y`) — les
champs que le jeu écrit et que la marche lit. `src/main.cpp` retire le sprite de
la démo et ses variables en pagaille — `SPRITE_SPEED`, `sprite_x`, `sprite_y`,
le mover en ligne — et accueille l'exécution autour de l'entité du héros :
l'intention du joueur depuis l'état d'entrée par scrutation, le pas par entité
de la marche (requête × vitesse × dt, le facing qui suit), la marche de dessin à
travers la caméra, et la caméra qui suit le héros. Le magasin, la table, le son
et les mesures de la boucle sont intacts. Son état final est étiqueté
`lesson-076`.

```diff
diff --git a/src/entity.h b/src/entity.h
index 785f868..bcda6b1 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -19,11 +19,18 @@ namespace engine {
    and writes entity.x like any other values. */
 struct Entity {
     char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
-    int x, y;                  /* where it is, in world pixels */
+    double x, y;               /* where it is, in world pixels — the
+                                  row's whole pixels, the motion's
+                                  doubles (lesson 076) */
     int facing;                /* 0 right, 1 down, 2 left, 3 up */
     int speed;                 /* world pixels per second */
     int health;                /* points */
     const Sprite *sprite;      /* the art it draws, from its row */
+    double move_x, move_y;     /* lesson 076: this frame's movement
+                                  request — the game sets it (the
+                                  player's input for the hero, Part 5's
+                                  AI for enemies) and the walk turns it
+                                  into motion */
     bool live;                 /* lesson 074: this entity exists — the
                                   slot's state, set by the store */
 };
diff --git a/src/main.cpp b/src/main.cpp
index 51ae3de..2bdcaa7 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -28,11 +28,6 @@
 
 namespace engine {
 
-/* The scene's one object: the sprite the arrow keys move. Its speed is
-   the engine's — pixels per second — and the clock's dt turns it into a
-   per-frame step. */
-constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
-
 /* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
    the horizon the loop keeps queued. Lesson 062: a feed is always
    exactly this much stream — the sample's frames where the sample has
@@ -124,18 +119,9 @@ int Run(void)
     Framebuffer *fb = GetFramebuffer(arena);
 
     /* The world's assets, loaded whole at startup (lessons 044-053):
-       a sprite, a font, a map, and the map's tile art. Every load is a
-       typed failure or a complete asset — and a failure ends the run by
-       name. */
-    SpriteResult loaded = LoadSprite(arena, "assets/sprite.ppm");
-    if (loaded.error != SPRITE_OK) {
-        std::fprintf(stderr, "engine: assets/sprite.ppm: could not load\n");
-        platform::CloseWindow(opened.window);
-        ArenaRelease(arena);
-        return 1;
-    }
-    Sprite &sprite = loaded.sprite;
-
+       a font, a map, and the map's tile art — and, since lesson 073,
+       the art each definition names. Every load is a typed failure or a
+       complete asset — and a failure ends the run by name. */
     FontResult font_loaded = LoadFont(arena, "assets/font.ppm");
     if (font_loaded.error != FONT_OK) {
         std::fprintf(stderr, "engine: assets/font.ppm: could not load\n");
@@ -248,7 +234,7 @@ int Run(void)
         return 1;
     }
     Entity &hero = *hero_made.entity;
-    std::printf("engine: entity %s: x %d y %d facing %d speed %d health %d sprite %dx%d\n",
+    std::printf("engine: entity %s: x %.0f y %.0f facing %d speed %d health %d sprite %dx%d\n",
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
@@ -315,25 +301,23 @@ int Run(void)
     int effect_count = 0;
     int music_wraps = 0;
 
-    double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
     double last = started;
-    double distance = 0.0; /* the score: the world the sprite has walked */
+    double distance = 0.0; /* the score: the world the hero has walked */
     int shake_frames = 0; /* lesson 054: the additive hook's demo */
-    bool was_blocked = false; /* lesson 056: the mover's state report */
 
-    /* The demo's identity: what the run is, named at once — the world
-       and its sound, one measured frame loop. */
-    std::printf("engine: part 3 done — the world draws and the sound plays\n");
-    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; sprite %dx%d\n",
+    /* The demo's identity: what the run is, named at once — the hero,
+       an entity the game moves, over the world the map draws. */
+    std::printf("engine: the hero, as an entity — a row the game moves, a camera that follows\n");
+    std::printf("engine: world %dx%d cells (%dx%d px), %d kinds; %d glyphs; hero %dx%d\n",
                 map.width, map.height, map.width * TILE_SIZE,
                 map.height * TILE_SIZE, map.kind_count, FONT_COUNT,
-                sprite.width, sprite.height);
+                hero.sprite->width, hero.sprite->height);
     std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
                 music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
                 AUDIO_MIXER_CHANNELS);
-    std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
-    std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
+    std::printf("engine: arrow keys move the hero, space shakes the camera; close the window to stop\n");
+    std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);
 
     /* Lesson 059: the run's sound is a run of amplitude at the engine's
        rate, and the seam's audio output is what puts those frames in
@@ -396,11 +380,29 @@ int Run(void)
         double dt = now - last;
         last = now;
 
+        /* Lesson 076: the hero's intent — polled input state, read once
+           per frame and written to the hero's own movement request. The
+           walk turns every entity's request into motion; the game never
+           moves an entity except through it. */
+        hero.move_x = 0.0;
+        hero.move_y = 0.0;
+        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
+            hero.move_x -= 1.0;
+        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
+            hero.move_x += 1.0;
+        if (platform::KeyDown(opened.window, platform::KEY_UP))
+            hero.move_y -= 1.0;
+        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
+            hero.move_y += 1.0;
+        double was_x = hero.x, was_y = hero.y;
+
         /* Lesson 075: the walk — every live entity, once per frame, in
            slot order. The per-entity work is expressed here, once, and
-           not per type; today it is the demo's kill check. An entity
-           retired in passing is not visited again and no other is
-           skipped — the slots do not move under the walk. */
+           not per type; lesson 076 makes it the entity's step — its
+           movement request becomes motion, and its facing follows where
+           it is going. An entity retired in passing is not visited again
+           and no other is skipped — the slots do not move under the
+           walk. */
         int visited = 0;
         for (int i = 0; i < ENTITY_CAP; ++i) {
             if (!store.slots[i].live)
@@ -410,7 +412,19 @@ int Run(void)
                 std::printf("engine: walk (frame %ld): retiring slot %d in passing\n",
                             frame.number, i);
                 EntityRetire(store, store.slots[i]);
+                continue;
             }
+            Entity &e = store.slots[i];
+            e.x += e.move_x * e.speed * dt;
+            e.y += e.move_y * e.speed * dt;
+            if (e.move_x > 0.0)
+                e.facing = 0;
+            else if (e.move_y > 0.0)
+                e.facing = 1;
+            else if (e.move_x < 0.0)
+                e.facing = 2;
+            else if (e.move_y < 0.0)
+                e.facing = 3;
         }
         walk_visits += visited;
         if (frame_number == 1)
@@ -432,49 +446,18 @@ int Run(void)
             std::printf("engine: store: live %d of %d\n", store.live, ENTITY_CAP);
         }
 
-        double was_x = sprite_x, was_y = sprite_y;
-        double move_x = 0.0, move_y = 0.0;
-        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
-            move_x -= SPRITE_SPEED * dt;
-        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
-            move_x += SPRITE_SPEED * dt;
-        if (platform::KeyDown(opened.window, platform::KEY_UP))
-            move_y -= SPRITE_SPEED * dt;
-        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
-            move_y += SPRITE_SPEED * dt;
-
-        /* Lesson 056: the mover — intent becomes motion only where the
-           map allows it. One axis at a time, so a wall blocks the
-           movement into it and the movement along it still works. */
-        double next_x = sprite_x + move_x;
-        if (!TileRectSolid(map, (int)next_x, (int)sprite_y, sprite.width,
-                           sprite.height))
-            sprite_x = next_x;
-        double next_y = sprite_y + move_y;
-        if (!TileRectSolid(map, (int)sprite_x, (int)next_y, sprite.width,
-                           sprite.height))
-            sprite_y = next_y;
-        distance += (sprite_x > was_x ? sprite_x - was_x : was_x - sprite_x) +
-                    (sprite_y > was_y ? sprite_y - was_y : was_y - sprite_y);
-
-        /* The mover reports its state on transitions: moving, or pushed
-           against something that will not move. */
-        bool blocked = (move_x != 0.0 || move_y != 0.0) &&
-                       sprite_x == was_x && sprite_y == was_y;
-        if (blocked != was_blocked) {
-            std::printf("engine: sprite %s at %d,%d (t=%.3f)\n",
-                        blocked ? "blocked" : "unblocked", (int)sprite_x,
-                        (int)sprite_y, platform::Now() - started);
-            was_blocked = blocked;
-        }
-        if ((int)sprite_x != (int)was_x || (int)sprite_y != (int)was_y)
-            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
-                        (int)sprite_y, platform::Now() - started);
+        /* The score, and the hero's own report: where the entity the
+           game moves has got to. */
+        distance += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
+                    (hero.y > was_y ? hero.y - was_y : was_y - hero.y);
+        if ((int)hero.x != (int)was_x || (int)hero.y != (int)was_y)
+            std::printf("engine: hero at %d,%d (t=%.3f)\n", (int)hero.x,
+                        (int)hero.y, platform::Now() - started);
 
-        /* Lesson 054: the camera's base follows the sprite — the world
+        /* Lesson 054: the camera's base follows the hero — the world
            scrolls under the movement — clamped to the map's bounds. */
-        int base_x = (int)sprite_x + sprite.width / 2 - FRAME_WIDTH / 2;
-        int base_y = (int)sprite_y + sprite.height / 2 - FRAME_HEIGHT / 2;
+        int base_x = (int)hero.x + hero.sprite->width / 2 - FRAME_WIDTH / 2;
+        int base_y = (int)hero.y + hero.sprite->height / 2 - FRAME_HEIGHT / 2;
         if (base_x < 0)
             base_x = 0;
         if (base_y < 0)
@@ -596,8 +579,17 @@ int Run(void)
         DrawTileMap(*fb, map, sheet, -CameraX(camera), -CameraY(camera));
         frame.tilemap = platform::Now() - t_tilemap;
         double t_sprites = platform::Now();
-        BlitSprite(*fb, sprite, (int)sprite_x - CameraX(camera),
-                   (int)sprite_y - CameraY(camera));
+
+        /* Lesson 076: the draw walk — every live entity, its art at its
+           position, through the camera's summed offset. Per-entity work
+           expressed once, in one loop, like the update's walk. */
+        for (int i = 0; i < ENTITY_CAP; ++i) {
+            if (!store.slots[i].live)
+                continue;
+            const Entity &e = store.slots[i];
+            BlitSprite(*fb, *e.sprite, (int)e.x - CameraX(camera),
+                       (int)e.y - CameraY(camera));
+        }
         frame.sprites = platform::Now() - t_sprites;
         double t_text = platform::Now();
         char score_line[32];
@@ -606,7 +598,7 @@ int Run(void)
         DrawText(*fb, font, score_line, 8, 8);
         char pos_line[32];
         std::snprintf(pos_line, sizeof pos_line, "X %3d Y %3d",
-                      (int)sprite_x, (int)sprite_y);
+                      (int)hero.x, (int)hero.y);
         DrawText(*fb, font, pos_line, 8, 8 + FONT_CELL + 4);
         frame.text = platform::Now() - t_text;
 
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le pas, prédit *(predict-the-output)*

Le mouvement est `requête × vitesse × dt`, et la ligne du héros dit 240. Avant
de lancer quoi que ce soit, prédisez la position du héros après chacune de ces
deux frames, en partant des `312,232` de sa ligne :

1. une frame de `dt = 0.5 s` avec la requête de la touche Droite ;
2. puis une frame de `dt = 0.5 s` avec **à la fois** Droite et Bas.

Écrivez les deux positions, et une phrase sur ce que la seconde réponse dit du
déplacement en diagonale. Écrivez ensuite une sonde jetable qui force exactement
ces deux frames (posez `move_x`/`move_y` du héros à la main et avancez la même
arithmétique que la marche) et lancez-la. Réconciliez — et répondez : si un jeu
veut un déplacement diagonal pas plus rapide qu'un déplacement droit, qu'est-ce
qui change, et où ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-076/ex1.md)

### Exercice 2 — Le héros sur votre machine *(port-to-your-own-machine)*

Les chiffres de cette machine viennent d'un affichage sans écran, d'appuis
scriptés et d'aucun périphérique sonore — le héros avance de 7-8 pixels par
frame parce qu'une frame sur deux seulement trouve la touche enfoncée. Lancez le
héros sur votre propre machine : une fenêtre sur votre bureau, une touche
maintenue, une sortie audio qui fonctionne si vous en avez une. Rapportez les
lignes de position du héros et le journal de frames à côté de ceux du livre, et
répondez avec vos chiffres : quelle est la vitesse réelle *mesurée* du héros
(distance sur temps), comment se compare-t-elle aux 240 de la ligne, et
qu'est-ce qui, dans le modèle de mouvement, explique un écart ? Maintenez
ensuite deux directions et mesurez la diagonale.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-076/ex2.md)

---

**Partie :** [Partie 4 — les services](../../index.md) ·
**Précédente :** [Leçon 075 — la marche et l'emplacement libre](lesson-075-lifetime.md) ·
**Suivante :** [Leçon 077 — le mover sur une entité](lesson-077-mover.md) ·
**Étiquette de code :** [`lesson-076`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-076)

*Page traduite de la version anglaise `book/lessons/part-4/lesson-076-hero.md`,
révision `08d2195`.*

<!-- translation-source: book/lessons/part-4/lesson-076-hero.md @ 08d2195 -->
