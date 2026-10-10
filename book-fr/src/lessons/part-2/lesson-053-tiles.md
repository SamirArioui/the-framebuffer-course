# Leçon 053 — le dessin de tilemap

{{#include ../../stability-horizon.md}}

## Prose

La carte est une donnée (leçon 052), et les tuiles sont des sprites (la
promesse de la leçon 050, deuxième volet). Aujourd'hui, les deux se
rencontrent : **la marche de tilemap** — un blit par cellule, à des origines
calculées — qui transforme les caractères du fichier en un monde où l'on peut
se tenir. C'est la moitié « dessin » de l'obligation « monde » du MVD, et elle
est délibérément sans éclat : une boucle d'appels à `BlitSprite` et rien
d'autre. Toutes les règles dont elle a besoin — découpe, transparence, copies
à l'octet près — le blitter les possède déjà.

### Les tuiles sont des sprites

L'art est `assets/tiles.ppm` : une feuille de cellules `TILE_SIZE` (16) × 16
sur une seule rangée, **une cellule par type de tuile, dans l'ordre de la
table des types de la carte**. Les trois types de la carte de la leçon 052
deviennent trois cellules — sol (un damier vert discret), mur (brique grise),
eau (deux bleus avec une vague). Le chargeur est celui de la feuille de police
(font), adapté : `LoadTileSheet` vérifie que la feuille fait exactement
`kind_count × TILE_SIZE` pixels de large et `TILE_SIZE` de haut, puis découpe
chaque cellule dans son propre `Sprite`, en copiant les pixels et en rendant à
l'arena les octets de la feuille par un retour arrière (la marque de la leçon
041, le motif de la leçon 050).

L'ordre de la table des types est le contrat entre deux fichiers :
`assets/map.txt` dit que le type 1 est `#` et solide ; `assets/tiles.ppm` dit
que le type 1 ressemble à de la brique. La carte possède *ce qu'*est une
cellule ; la feuille possède *à quoi elle ressemble*. Une carte à quatre types
exige une feuille à quatre cellules — le chargeur refuse tout le reste plutôt
que de deviner.

### La marche

```c++
void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
                 int x, int y)
{
    for (int cy = 0; cy < map.height; ++cy)
        for (int cx = 0; cx < map.width; ++cx) {
            int kind = map.cells[cy * map.width + cx];
            BlitSprite(fb, sheet.kinds[kind],
                       x + cx * TILE_SIZE, y + cy * TILE_SIZE);
        }
}
```

La cellule `(cx, cy)` est dessinée à `(x + cx × 16, y + cy × 16)` — la
position monde, directement issue des indices. Tout ce qui concerne une
cellule qui atterrit hors du framebuffer est la découpe du blitter : la marche
calcule les positions, le blit abandonne ce qui tombe à l'extérieur. Cette
répartition explique pourquoi cette fonction peut tenir en quatre lignes — les
règles difficiles vivent à un seul endroit.

Cela signifie aussi qu'**une carte plus grande que le framebuffer est le cas
ordinaire**, pas un cas limite. Le monde de la démo fait 48 × 32 cellules =
768 × 512 pixels face à une frame de 640 × 480 ; dessiné à n'importe quelle
origine, la frame montre le rectangle du monde qui tombe à l'intérieur.

### L'affirmation, vérifiée

La vérification dessine toute la carte à deux origines différentes et compare
les pixels qui se chevauchent — les mêmes pixels du monde doivent apparaître à
position monde moins décalage, partout où les deux dessins se recouvrent :

```
engine: tiles assets/tiles.ppm: 3 tiles of 16x16
engine: tilemap check: map 768x512 px over frame 640x480 — 274365 pixels compared at offset 37,25, 0 mismatches
```

274 365 pixels comparés — chaque pixel d'écran où les deux dessins montrent le
même contenu de monde — et zéro écart. La comparaison est la vérification de
caméra de la leçon 045 à l'échelle du monde : dessine, déplace, dessine, et
confronte les deux. Les parties de la carte qui tombent hors de la frame dans
le dessin déplacé ont simplement été abandonnées par la découpe du blitter ;
pas de retournement, pas de plantage, pas de pixel erroné dans le
chevauchement.

### La troisième phase nommée

La marche de la carte, ce sont 1 536 blits, et l'enregistrement de frame dit
ce qu'ils coûtent — le troisième sous-système nommé dans `render`, après
`sprites` et `text` :

```
frame 1: update 0.000 ms, render 1.288 ms (sprites 0.001, text 0.002, tilemap 0.923), present 1.584 ms, total 2.872 ms
```

`tilemap 0.923 ms` est le premier nombre honnête du coût du monde : à 16×16
pixels par tuile, la marche copie jusqu'à 768 × 512 = 393 216 pixels par frame
— un tiers du rendu de la frame, et la plus grosse phase nommée jusqu'ici. La
table de budget de frames (leçon 058) portera cette ligne, et le profileur de
la partie 5 la regardera en premier : c'est un redessin complet d'un monde à
chaque frame, et l'exercice ci-dessous mesure exactement ce que la découpe du
blitter vous fait gagner — et ce qu'elle ne vous fait pas gagner.

## Étape de code

Un seul changement pour cette leçon : `assets/map.txt` passe de la pièce de la
leçon 052 au monde de la démo (48×32 cellules — plus grand que la frame),
`assets/tiles.ppm` est créé (trois cellules de tuile 16×16), `src/tiles.h` /
`src/tiles.cpp` apportent le chargeur de feuille et la marche, `frame.h` /
`frame.cpp` accueillent la phase nommée `tilemap`, et `main.cpp` charge la
feuille, vérifie la carte à deux décalages et dessine le monde à chaque frame.
Son état final est étiqueté `lesson-053`.

```diff
diff --git a/.gitattributes b/.gitattributes
new file mode 100644
index 0000000..f7e6e0e
--- /dev/null
+++ b/.gitattributes
@@ -0,0 +1,4 @@
+# Asset files are binary data, whatever their bytes happen to look like.
+# (Lesson 053: the tile art's pixel bytes contained no NULs and git
+# cheerfully diffed it as text — a one-line fact worth writing down.)
+*.ppm binary
diff --git a/assets/map.txt b/assets/map.txt
index f142cb1..fcd9ce5 100644
--- a/assets/map.txt
+++ b/assets/map.txt
@@ -1,16 +1,36 @@
-20 12 3
+48 32 3
 . 0
 # 1
 w 0
-####################
-#..................#
-#..................#
-#....##......##....#
-#....##......##....#
-#..................#
-#..................#
-#...w....##....w...#
-#...w....##....w...#
-#..................#
-#..................#
-####################
+################################################
+#..............................................#
+#..............................................#
+#..............................................#
+#..............................................#
+#..............................................#
+#.......##..........##..........##.............#
+#.......##..........##..........##.............#
+#..............................................#
+#..............................................#
+#..............................................#
+#..............................................#
+#.......##..........##..........##.............#
+#.......##..........##..........##.............#
+#..............................................#
+#..............................................#
+#.............##........##..............##.....#
+#.............##........##..............##.....#
+#.......................##.....................#
+#.......................##.....................#
+#..............................................#
+#..............................................#
+#.......##..........##..##......##.............#
+#.......##..........##..##......##.............#
+#.......................##....wwwwwwwww........#
+#.......................##....wwwwwwwww........#
+#.......................##....wwwwwwwww........#
+#.......................##....wwwwwwwww........#
+#.............................wwwwwwwww........#
+#..............................................#
+#..............................................#
+################################################
diff --git a/assets/tiles.ppm b/assets/tiles.ppm
new file mode 100644
index 0000000..287453f
Binary files /dev/null and b/assets/tiles.ppm differ
diff --git a/src/frame.cpp b/src/frame.cpp
index 36a2827..22cb968 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -15,6 +15,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.total_sum += frame.total;
     stats.sprites_sum += frame.sprites;
     stats.text_sum += frame.text;
+    stats.tilemap_sum += frame.tilemap;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
diff --git a/src/frame.h b/src/frame.h
index 3a68029..8f42a7d 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -24,6 +24,7 @@ struct FrameRecord {
        instead of it: render stays the phase, these say where it went. */
     double sprites; /* sprite draws through the blit */
     double text;    /* lesson 051: text drawing — glyphs through the blit */
+    double tilemap; /* lesson 053: the map's walk — tiles through the blit */
 };
 
 /* The running account: every frame measured so far. */
@@ -35,6 +36,7 @@ struct FrameStats {
     double total_sum;
     double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
     double text_sum;
+    double tilemap_sum;
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
 };
diff --git a/src/main.cpp b/src/main.cpp
index b63c353..0e77458 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -16,6 +16,7 @@
 #include "sprite.h"
 #include "text.h"
 #include "tilemap.h"
+#include "tiles.h"
 
 namespace engine {
 
@@ -382,6 +383,72 @@ int Run(void)
     std::printf("engine: map check: %d cells, %d unknown, %d of 2 corners solid\n",
                 map.width * map.height, unknown, solid_corners);
 
+    /* Lesson 053: tiles are sprites — the sheet, cut per kind. */
+    const char *tiles_path = "assets/tiles.ppm";
+    TileSheetResult tiles_loaded = LoadTileSheet(arena, tiles_path,
+                                                 map.kind_count);
+    if (tiles_loaded.error != TILES_OK) {
+        switch (tiles_loaded.error) {
+        case TILES_MISSING:
+            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
+                         tiles_path);
+            break;
+        case TILES_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: %s: not one %dx%d cell per map kind\n",
+                         tiles_path, TILE_SIZE, TILE_SIZE);
+            break;
+        default:
+            std::fprintf(stderr, "engine: %s: no room in the arena\n",
+                         tiles_path);
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    TileSheet &sheet = tiles_loaded.sheet;
+    std::printf("engine: tiles %s: %d tiles of %dx%d\n", tiles_path,
+                map.kind_count, TILE_SIZE, TILE_SIZE);
+
+    /* The map bigger than the frame, drawn at two offsets: the same
+       world pixels at world-position minus offset, everywhere the two
+       draws overlap. */
+    unsigned char *snap = (unsigned char *)ArenaAlloc(
+        arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 3, 4);
+    if (!snap) {
+        std::fprintf(stderr, "engine: no room for the tilemap check\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawTileMap(*fb, map, sheet, 0, 0);
+    for (int y = 0; y < FRAME_HEIGHT; ++y)
+        for (int x = 0; x < FRAME_WIDTH; ++x) {
+            unsigned char r, g, b;
+            GetPixel(*fb, x, y, r, g, b);
+            unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
+            p[0] = r;
+            p[1] = g;
+            p[2] = b;
+        }
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawTileMap(*fb, map, sheet, -37, -25);
+    int compared = 0, moved_mismatches = 0;
+    for (int y = 25; y < FRAME_HEIGHT; ++y)
+        for (int x = 37; x < FRAME_WIDTH; ++x) {
+            unsigned char r, g, b;
+            GetPixel(*fb, x - 37, y - 25, r, g, b);
+            const unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
+            ++compared;
+            if (r != p[0] || g != p[1] || b != p[2])
+                ++moved_mismatches;
+        }
+    std::printf("engine: tilemap check: map %dx%d px over frame %dx%d — %d pixels compared at offset 37,25, %d mismatches\n",
+                map.width * TILE_SIZE, map.height * TILE_SIZE, FRAME_WIDTH,
+                FRAME_HEIGHT, compared, moved_mismatches);
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
@@ -438,9 +505,12 @@ int Run(void)
                         (int)sprite_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then the
-           sprite and the text, each timed as its own named phase: the
-           subsystems the frame record can name. */
+           map, the sprite, and the text, each timed as its own named
+           phase: the subsystems the frame record can name. */
         ClearBuffer(*fb, 32, 32, 64);
+        double t_tilemap = platform::Now();
+        DrawTileMap(*fb, map, sheet, 0, 0);
+        frame.tilemap = platform::Now() - t_tilemap;
         double t_sprites = platform::Now();
         BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
         frame.sprites = platform::Now() - t_sprites;
@@ -469,21 +539,22 @@ int Run(void)
 
         /* The frame log: one line per record — the format grows its named
            fields, one per subsystem, as the parts name them. */
-        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f), present %.3f ms, total %.3f ms\n",
+        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.update * 1e3, frame.render * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
-                    frame.present * 1e3, frame.total * 1e3);
+                    frame.tilemap * 1e3, frame.present * 1e3,
+                    frame.total * 1e3);
     }
 
     /* The account: what the frames actually cost, including the honest
        price of the presentation copy. */
     if (stats.frames) {
         double n = (double)stats.frames;
-        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, present %.3f)\n",
+        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, tilemap %.3f, present %.3f)\n",
                     stats.frames, stats.total_sum / n * 1e3,
                     stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
                     stats.sprites_sum / n * 1e3, stats.text_sum / n * 1e3,
-                    stats.present_sum / n * 1e3);
+                    stats.tilemap_sum / n * 1e3, stats.present_sum / n * 1e3);
         std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
                     stats.worst * 1e3, stats.worst_number,
                     100.0 * stats.present_sum / stats.total_sum);
diff --git a/src/tiles.cpp b/src/tiles.cpp
new file mode 100644
index 0000000..66698af
--- /dev/null
+++ b/src/tiles.cpp
@@ -0,0 +1,85 @@
+// tiles.cpp — cutting the tile sheet, and the map's walk.
+//
+// Lesson 053: the cut is lesson 050's, the walk is lesson 045's blit in
+// a loop at computed origins. Nothing here draws a pixel by any other
+// path.
+
+#include "tiles.h"
+
+#include "blit.h"
+
+namespace engine {
+
+TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
+{
+    TileSheetResult result = { { { { 0, 0, 0, 0, 0, 0 } } }, TILES_OK };
+
+    if (kind_count <= 0 || kind_count > TILE_MAX_KINDS) {
+        result.error = TILES_MALFORMED;
+        return result;
+    }
+
+    /* One allocation for every kind's tile pixels. */
+    size_t tile_bytes = (size_t)kind_count * TILE_SIZE * TILE_SIZE * 3;
+    unsigned char *pixels = (unsigned char *)ArenaAlloc(arena, tile_bytes, 4);
+    if (!pixels) {
+        result.error = TILES_NO_ROOM;
+        return result;
+    }
+
+    size_t mark = ArenaMark(arena);
+    SpriteResult sheet = LoadSprite(arena, path);
+    if (sheet.error == SPRITE_MISSING) {
+        result.error = TILES_MISSING;
+        return result;
+    }
+    if (sheet.error != SPRITE_OK) {
+        result.error = TILES_MALFORMED;
+        return result;
+    }
+
+    /* The sheet is exactly kind_count cells in one row. */
+    if (sheet.sprite.width != kind_count * TILE_SIZE ||
+        sheet.sprite.height != TILE_SIZE) {
+        ArenaRollback(arena, mark);
+        result.error = TILES_MALFORMED;
+        return result;
+    }
+
+    for (int k = 0; k < kind_count; ++k) {
+        Sprite &tile = result.sheet.kinds[k];
+        tile.pixels = pixels + (size_t)k * TILE_SIZE * TILE_SIZE * 3;
+        tile.width = TILE_SIZE;
+        tile.height = TILE_SIZE;
+        tile.key_r = sheet.sprite.key_r;
+        tile.key_g = sheet.sprite.key_g;
+        tile.key_b = sheet.sprite.key_b;
+        for (int r = 0; r < TILE_SIZE; ++r)
+            for (int c = 0; c < TILE_SIZE; ++c) {
+                const unsigned char *src =
+                    &sheet.sprite.pixels[(((size_t)r * sheet.sprite.width) +
+                                          (k * TILE_SIZE + c)) * 3];
+                unsigned char *dst =
+                    &tile.pixels[(((size_t)r * TILE_SIZE) + c) * 3];
+                dst[0] = src[0];
+                dst[1] = src[1];
+                dst[2] = src[2];
+            }
+    }
+    ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
+    result.error = TILES_OK;
+    return result;
+}
+
+void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
+                 int x, int y)
+{
+    for (int cy = 0; cy < map.height; ++cy)
+        for (int cx = 0; cx < map.width; ++cx) {
+            int kind = map.cells[cy * map.width + cx];
+            BlitSprite(fb, sheet.kinds[kind],
+                       x + cx * TILE_SIZE, y + cy * TILE_SIZE);
+        }
+}
+
+} /* namespace engine */
diff --git a/src/tiles.h b/src/tiles.h
new file mode 100644
index 0000000..21ceb95
--- /dev/null
+++ b/src/tiles.h
@@ -0,0 +1,51 @@
+// tiles.h — the tile sheet, and the walk that draws a map with it.
+//
+// Lesson 053: tiles are sprites (lesson 045's promise, second
+// installment). The sheet is a PPM image of TILE_SIZE cells in one row —
+// one cell per tile kind, in the map's kind-table order — and the map's
+// walk is a loop of blits at computed origins.
+#ifndef TILES_H
+#define TILES_H
+
+#include "framebuffer.h"
+#include "sprite.h"
+#include "tilemap.h"
+
+namespace engine {
+
+/* The format's cell size: every tile is TILE_SIZE x TILE_SIZE pixels. */
+constexpr int TILE_SIZE = 16;
+
+/* A tile sheet: one sprite per kind, cut from the sheet at load. */
+struct TileSheet {
+    Sprite kinds[TILE_MAX_KINDS];
+};
+
+/* A load either hands over the sheet or names what went wrong. */
+enum TileSheetError {
+    TILES_OK = 0,
+    TILES_MISSING,   /* the sheet is not there or cannot be read */
+    TILES_MALFORMED, /* the sheet is not exactly kind_count cells wide */
+    TILES_NO_ROOM,   /* the arena had no room for the tiles */
+};
+
+struct TileSheetResult {
+    TileSheet sheet;
+    TileSheetError error;
+};
+
+/* Loads a sheet of exactly kind_count cells of TILE_SIZE, cutting each
+   cell into its own sprite — the map's kinds, as art. */
+TileSheetResult LoadTileSheet(Arena &arena, const char *path,
+                              int kind_count);
+
+/* Draws a map with its top-left cell at world origin (x, y): one blit
+   per cell, at (x + cell_x * TILE_SIZE, y + cell_y * TILE_SIZE). The
+   blitter's clipping drops the cells that fall outside the framebuffer —
+   a map bigger than the screen is an ordinary case. */
+void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
+                 int x, int y);
+
+} /* namespace engine */
+
+#endif
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La tuile sous le sprite *(extend-the-code)*

La marche projette la cellule vers le monde ; écrivez l'inverse — le monde
vers la cellule. Au démarrage, rapportez sur quelle cellule se tient le sprite
(sa position divisée par la taille de tuile) et quel est le type de cette
cellule ; puis dessinez à chaque frame un contour d'un pixel autour de cette
cellule, en suivant le sprite dans ses déplacements. Vérifiez avec une entrée
scriptée et une relecture de pixel au coin du contour. Qu'est-ce que cette
correspondance offre gratuitement à la leçon 055 ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-053/ex1.md)

### Exercice 2 — Le dessin à vide *(predict-the-output)*

La découpe abandonne les tuiles hors écran, mais la marche visite quand même
chaque cellule. Mesurez-le : chronométrez la marche de la carte à trois
origines — entièrement à l'écran `(0, 0)`, en grande partie hors écran
`(-400, -300)` et entièrement hors écran `(-4096, -4096)` — et prédisez les
trois nombres avant de lancer. Quelle part du coût à l'écran est de la
*copie*, et quelle part est de la *marche*, et que faudrait-il pour sauter des
cellules plutôt que de les découper ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-053/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 052 — le format d'asset du tilemap](lesson-052-tilemap.md) ·
**Suivante :** [Leçon 054 — la caméra](lesson-054-camera.md) ·
**Étiquette de code :** [`lesson-053`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-053)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-053-tiles.md`,
révision `d843c56`.*

<!-- translation-source: book/lessons/part-2/lesson-053-tiles.md @ d843c56 -->
