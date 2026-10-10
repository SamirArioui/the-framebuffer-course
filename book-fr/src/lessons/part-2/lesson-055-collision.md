# Leçon 055 — les types de tuile et la solidité

{{#include ../../stability-horizon.md}}

## Prose

Le monde sait de quoi il est fait — la table des types de la leçon 052 porte
la solidité depuis que le format est défini. Cette leçon fait *répondre la
carte* à des questions grâce à elle : **ce point chevauche-t-il un mur ? ce
rectangle entre-t-il en collision avec quelque chose de solide ?** La
quatrième obligation du MVD s'achève ici : la collision par tuiles, sous forme
de requêtes pures contre des données chargées — pas de pixels, pas d'écran,
pas de boucle de frame. Le but énoncé par la spécification explique pourquoi
cela compte : un monde doit être testable sans fenêtre, et un système de
collision qu'on ne peut vérifier qu'en marchant un personnage dans un mur est
un système qu'on ne peut pas tester du tout.

### La politique d'abord

Toute requête finit par rencontrer des coordonnées hors de la carte, et « ce
qui se passe là » est une décision, pas un accident. La réponse de ce moteur,
documentée dans le code et dans la leçon : **l'extérieur de la carte compte
comme solide** — le bord du monde bloque comme un mur. Le héros ne quitte
jamais le monde, parce que toute requête qui pointe au-delà du bord répond
*collision*.

L'exigence de la spécification n'est pas *quelle* réponse, mais *qu'il y en
ait une* : les positions hors de la carte ont une réponse définie et le moteur
ne lit jamais en dehors des données de la carte. L'exercice 1 essaie la
politique opposée le temps d'une balade — les deux sont légitimes ; le péché,
c'est de ne pas choisir.

### Les requêtes

Trois fonctions, qui répondent toutes à partir des seules données de la
carte :

```c++
bool TileSolid(const TileMap &map, int x, int y);        /* one cell */
bool TilePointSolid(const TileMap &map, int world_x, int world_y);
bool TileRectSolid(const TileMap &map, int x, int y, int w, int h);
```

- **`TileSolid`** est la primitive : le type de la cellule, l'indicateur
  `solid` du type, un `!= 0`. Les cellules hors de la carte répondent solide
  (la politique).
- **`TilePointSolid`** est l'enveloppe pour les coordonnées monde : vérifiez
  les limites du point par rapport à la taille en pixels de la carte, puis
  divisez par `TILE_SIZE` — la correspondance monde → cellule qu'a écrite
  l'exercice de la leçon 053 — et interrogez la cellule. La vérification des
  limites vient d'abord, pour qu'un point à `(-1, 100)` reçoive la réponse de
  la politique et non celle d'une division qui l'appellerait volontiers
  cellule 0.
- **`TileRectSolid`** couvre les cellules que le rectangle traverse : de
  `(x, y)` à `(x + w − 1, y + h − 1)` en pixels, convertis en cellules
  `[x/TILE_SIZE .. (x+w−1)/TILE_SIZE]`. Le `− 1` vient de ce que le bord droit
  et le bord bas du rectangle sont *exclusifs* : un rectangle en
  `(16, 0, 16, 16)` couvre les pixels 16..31 et pas un de plus — la cellule 1,
  pas la cellule 0. Ratez cela et les rectangles rapportent des collisions
  avec des murs qu'ils n'ont jamais touchés.

Les requêtes sont *des fonctions pures de la carte et des coordonnées* : la
même question reçoit la même réponse à chaque fois, sur n'importe quelle
machine, sans fenêtre impliquée. C'est ce qui les rend testables — et le bloc
de vérification les teste comme des données.

### Les réponses, vérifiées

Douze cas, chacun avec la réponse que la documentation promet, exécutés au
démarrage et comparés :

```
engine: collision check: 12 of 12 answers as documented (out-of-bounds is solid)
```

La table derrière cette ligne (la réponse attendue de chaque ligne écrite
*avant* l'exécution) :

| Requête | Position | Réponse |
| ------- | -------- | ------- |
| point au-dessus du sol | (256, 224) | libre |
| point dans le mur de bordure | (8, 8) | solide |
| point dans un pilier | (128, 96) | solide |
| point dans l'eau | (528, 416) | libre — l'eau est dessinée mais praticable |
| point à gauche de la carte | (−1, 100) | solide — la politique |
| point au-delà du bord droit | (768, 100) | solide — la politique |
| point sous la carte | (100, 512) | solide — la politique |
| rectangle au-dessus du sol | (240, 216, 32, 32) | libre |
| rectangle atteignant un pilier | (120, 88, 32, 32) | solide |
| rectangle quittant la carte | (−8, 100, 16, 16) | solide — la politique |
| rectangle au-delà du bord droit | (760, 100, 16, 16) | solide — la politique |
| rectangle vide | (100, 100, 0, 0) | libre — rien ne chevauche rien |

Les lignes sol/eau/pilier incarnent le scénario de solidité de la
spécification : trois types, deux solidités, et les requêtes distinguent les
cellules **par la seule solidité des types** — le code de dessin n'est
consulté nulle part dans `tilemap.cpp`. Renommez l'art du mur, recolorez
l'eau, dessinez la carte à l'envers : les réponses de collision ne changent
pas, parce qu'elles n'ont jamais porté sur les images.

### À quoi servent les requêtes

La leçon 056 construit le mover : un sprite avec une intention (les flèches)
qui interroge `TileRectSolid` avant de bouger et s'arrête quand la réponse est
*solide*. Le mover est la première pièce du jeu proprement dit — le précurseur
du héros — et chaque règle à laquelle il obéit est l'une des douze réponses de
cette leçon. La partie 5 transformera les mêmes requêtes en knockback,
en ennemis qui contournent les piliers, en résolution de collision du projet
final ; les données sont prêtes depuis la leçon 052, et les questions sont
répondables depuis maintenant.

## Étape de code

Un seul changement pour cette leçon : `tilemap.h` / `tilemap.cpp` accueillent
les requêtes de collision (`TileSolid`, `TilePointSolid`, `TileRectSolid` —
des coordonnées monde en entrée, la solidité des types en sortie, la politique
hors limites définie et documentée), `TILE_SIZE` déménage dans l'en-tête de la
carte, là où vit la géométrie du monde, et `main.cpp` vérifie douze réponses
contre les données de la carte elle-même. Rien ne change dans le dessin. Son
état final est étiqueté `lesson-055`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 3c0c5bf..180c505 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -548,6 +548,44 @@ int Run(void)
     std::printf("engine: camera check: additive cleared restores the base view — %d pixels compared, %d mismatches\n",
                 cam_cmp, cam_bad);
 
+    /* Lesson 055: the collision queries — every documented answer
+       checked against the map's own data, out-of-bounds included. */
+    struct CollisionCase {
+        const char *what;
+        bool point; /* true: a point query; false: a rectangle */
+        int x, y, w, h;
+        bool expected;
+    };
+    const CollisionCase cases[] = {
+        { "point over floor", true, 256, 224, 0, 0, false },
+        { "point in the border wall", true, 8, 8, 0, 0, true },
+        { "point in a pillar", true, 128, 96, 0, 0, true },
+        { "point in the water", true, 528, 416, 0, 0, false },
+        { "point left of the map", true, -1, 100, 0, 0, true },
+        { "point past the right edge", true, 768, 100, 0, 0, true },
+        { "point below the map", true, 100, 512, 0, 0, true },
+        { "rect over floor", false, 240, 216, 32, 32, false },
+        { "rect reaching a pillar", false, 120, 88, 32, 32, true },
+        { "rect leaving the map", false, -8, 100, 16, 16, true },
+        { "rect past the right edge", false, 760, 100, 16, 16, true },
+        { "empty rect", false, 100, 100, 0, 0, false },
+    };
+    int passed = 0;
+    for (unsigned ci = 0; ci < sizeof cases / sizeof cases[0]; ++ci) {
+        const CollisionCase &c = cases[ci];
+        bool answer = c.point ? TilePointSolid(map, c.x, c.y)
+                              : TileRectSolid(map, c.x, c.y, c.w, c.h);
+        if (answer == c.expected) {
+            ++passed;
+        } else {
+            std::printf("engine: collision check: %s — expected %s, got %s\n",
+                        c.what, c.expected ? "solid" : "free",
+                        answer ? "solid" : "free");
+        }
+    }
+    std::printf("engine: collision check: %d of %d answers as documented (out-of-bounds is solid)\n",
+                passed, (int)(sizeof cases / sizeof cases[0]));
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
diff --git a/src/tilemap.cpp b/src/tilemap.cpp
index fadf9fe..b65ec21 100644
--- a/src/tilemap.cpp
+++ b/src/tilemap.cpp
@@ -161,4 +161,44 @@ int TileAt(const TileMap &map, int x, int y)
     return map.cells[y * map.width + x];
 }
 
+bool TileSolid(const TileMap &map, int x, int y)
+{
+    int kind = TileAt(map, x, y);
+    if (kind < 0)
+        return true; /* outside the map: the edge blocks like a wall */
+    return map.kinds[kind].solid != 0;
+}
+
+bool TilePointSolid(const TileMap &map, int world_x, int world_y)
+{
+    if (world_x < 0 || world_y < 0 ||
+        world_x >= map.width * TILE_SIZE ||
+        world_y >= map.height * TILE_SIZE)
+        return true; /* out of bounds: the policy's answer, no cell read */
+    return TileSolid(map, world_x / TILE_SIZE, world_y / TILE_SIZE);
+}
+
+bool TileRectSolid(const TileMap &map, int x, int y, int w, int h)
+{
+    if (w <= 0 || h <= 0)
+        return false; /* an empty rectangle overlaps nothing */
+
+    /* The map's edge is solid: a rectangle that leaves the map answers
+       without reading a single cell. */
+    if (x < 0 || y < 0 || x + w > map.width * TILE_SIZE ||
+        y + h > map.height * TILE_SIZE)
+        return true;
+
+    /* Otherwise only the cells the rectangle covers can say yes. */
+    int cx0 = x / TILE_SIZE;
+    int cy0 = y / TILE_SIZE;
+    int cx1 = (x + w - 1) / TILE_SIZE;
+    int cy1 = (y + h - 1) / TILE_SIZE;
+    for (int cy = cy0; cy <= cy1; ++cy)
+        for (int cx = cx0; cx <= cx1; ++cx)
+            if (map.kinds[map.cells[cy * map.width + cx]].solid)
+                return true;
+    return false;
+}
+
 } /* namespace engine */
diff --git a/src/tilemap.h b/src/tilemap.h
index 1c076cf..d34fa8d 100644
--- a/src/tilemap.h
+++ b/src/tilemap.h
@@ -22,6 +22,11 @@ namespace engine {
 constexpr int TILE_MAX_DIM = 256;
 constexpr int TILE_MAX_KINDS = 8;
 
+/* The cell's size in pixels: the geometry every world coordinate walks
+   on. Lesson 053 draws cells at this size; lesson 055's queries read
+   world positions through it. */
+constexpr int TILE_SIZE = 16;
+
 /* One tile kind: the character that names it in the file, and whether it
    is solid for collision (lesson 055 reads this; the format carries it
    from the first day). */
@@ -62,6 +67,22 @@ TileResult LoadTileMap(Arena &arena, const char *path);
 /* The kind of a cell, or -1 outside the map. */
 int TileAt(const TileMap &map, int x, int y);
 
+/* Lesson 055: the collision queries — answered from the map data alone
+   (the kinds' solidity), never from drawing code. The out-of-bounds
+   policy is defined, not accidental: a position or rectangle outside the
+   map counts as solid, so the world's edge blocks like a wall and no
+   query ever reads outside the map's cells. */
+
+/* Is the cell at (x, y) solid? Cells outside the map answer solid. */
+bool TileSolid(const TileMap &map, int x, int y);
+
+/* Point query: does this world position overlap a solid tile? */
+bool TilePointSolid(const TileMap &map, int world_x, int world_y);
+
+/* Rectangle query: does this world rectangle (w, h > 0) overlap any
+   solid tile — or the world's edge, which counts as solid? */
+bool TileRectSolid(const TileMap &map, int x, int y, int w, int h);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/tiles.h b/src/tiles.h
index 21ceb95..6474146 100644
--- a/src/tiles.h
+++ b/src/tiles.h
@@ -13,9 +13,6 @@
 
 namespace engine {
 
-/* The format's cell size: every tile is TILE_SIZE x TILE_SIZE pixels. */
-constexpr int TILE_SIZE = 16;
-
 /* A tile sheet: one sprite per kind, cut from the sheet at load. */
 struct TileSheet {
     Sprite kinds[TILE_MAX_KINDS];
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La politique est la vôtre *(extend-the-code)*

La réponse hors limites est un choix ; rendez-le visible. Écrivez les deux
mêmes requêtes avec la politique opposée — hors de la carte est *libre* — à
côté des versions solides, et lancez les quatre cas hors limites sous les
deux. Prédisez d'abord quels cas basculeront et lesquels ne basculeront pas ;
puis réconciliez, et répondez à la question que pose la carte : pourquoi
`rect leaving the map` rapporte-t-il *solide* sous **les deux** politiques ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-055/ex1.md)

### Exercice 2 — Le rectangle à la frontière *(predict-the-output)*

Les bords de rectangle sont l'endroit où vivent les bugs de collision. Avant
de lancer quoi que ce soit, notez la réponse pour six rectangles contre le
pilier aux cellules (8,6)-(9,7) — la cellule exacte du pilier, la cellule à
côté, un pixel *à l'intérieur* du pilier, en diagonale, un pixel touchant son
pixel de coin, et un pixel au-delà de ce coin. Lancez ensuite la table et
réconciliez chaque réponse avec l'arithmétique de couverture du rectangle.
Quel caractère unique dans `TileRectSolid` décide des cas à un pixel ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-055/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 054 — la caméra](lesson-054-camera.md) ·
**Suivante :** [Leçon 056 — le mover qui s'arrête aux murs](lesson-056-mover.md) ·
**Étiquette de code :** [`lesson-055`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-055)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-055-collision.md`,
révision `c39d430`.*

<!-- translation-source: book/lessons/part-2/lesson-055-collision.md @ c39d430 -->
