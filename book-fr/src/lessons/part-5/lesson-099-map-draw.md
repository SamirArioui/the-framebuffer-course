# Leçon 099 — passe 2a : corriger le dessin de la carte

{{#include ../../stability-horizon.md}}

## Prose

La passe 1 a nommé les deux pièces de travail les plus chaudes du jeu avec des
nombres de vraies frames, et le menu figé (design D11) les corrige dans l'ordre
— exactement deux, une leçon chacune, et rien d'autre. **Le point chaud n° 1
est le dessin de la carte** : la marche de `DrawTileMap`, un `BlitSprite` par
cellule — `tilemap 0.981 ms` sur les `1.944 ms` d'une frame de jeu, `BlitSprite`
à `2.11 s` des `3.53 s` échantillonnées par le profil (60,9 %). Cette leçon
corrige cela, avec les leviers que les plongées ont nommés et chiffrés :
**copier moins, copier plus rapproché, copier plus large** (la clôture de la
leçon 049 : « les trois leviers sont déjà nommés »). Et elle ne corrige rien
d'autre.

### Ce qu'est réellement le point chaud

1 536 tuiles par frame, chacune à travers la boucle interne du blit de sprites :
par pixel, recalculer l'index source (une multiplication et deux additions),
tester trois octets pour la couleur transparente, recalculer l'index de
destination, stocker quatre octets. Sur une frame de jeu cette boucle tourne
sur ~393 000 pixels — et, mesurée, elle fait la moitié de la frame.

Deux faits sur ce travail rendent les leviers évidents. L'adressage par pixel
est du *recalcul* — les pointeurs de la ligne sont les mêmes pour chaque pixel
qu'elle contient. Et le test de transparence par pixel est *gaspillé* sur les
tuiles de la carte presque partout où il tourne : c'est une décision sans
décision. Le culling est le troisième fait : la marche visite les 1 536
cellules de la carte, que la frame puisse les montrer ou non.

### La correction, trois mouvements

**Copier plus rapproché** — les lignes de `BlitSprite` calculent maintenant
leurs pointeurs source et destination une fois et les avancent (`src += 3`,
`dst += 4`), au lieu de reconstruire les deux index à chaque pixel. Accès
séquentiel, la forme avec laquelle la leçon 047 parcourt les lignes de cache.

**Copier plus large** — un sprite **sans** pixel transparent passe par une
expansion directe (straight expand) : aucun test par pixel. Qui sait si un
sprite en a un ? Le chargement : `Sprite` accueille `key_count`, compté une
fois là où un sprite naît — `LoadSprite`, la découpe de la planche de tuiles
dans `LoadTileSheet`, la découpe de la police dans `LoadFont` — pour que la
frame ne paie aucune décision que le chargement a déjà prise
(`CountKeyPixels`, à côté du chargeur). Les tuiles de la carte portent zéro
pixel clé et prennent le chemin direct ; le héros, les projectiles et les
glyphes gardent le test par pixel, car pour eux la décision est réelle — la
question de la leçon 049 tient : que voudrait dire une « transparence
vectorisée » ?

**Copier moins** — `DrawTileMap` ne parcourt que les cellules que la frame peut
montrer : la fenêtre visible calculée une fois depuis l'offset de la carte — la
première cellule dont le bord droit dépasse la gauche de la frame, jusqu'à la
dernière dont le bord gauche est dedans. Sur cette carte (48×32 cellules, une
frame de 640×480), cela fait environ 1 240 cellules sur les 1 536 — les autres
n'écrivent rien à travers le découpage du blit de toute façon, donc les pixels
dessinés sont les mêmes pixels.

Chaque pixel atterrit où il a toujours atterri ; cette affirmation est mesurée
à la fin de l'exécution de cette leçon, pas argumentée.

### Le coût mesuré baisse

La même exécution de mesure que la passe 1 — le même scénario (segments de jeu
avec redémarrages), le même découpage selon le champ `step` de
l'enregistrement (1 551 frames, dont 1 415 de jeu, dans les deux exécutions) :

```
play frames:                     before (lesson-098)   after (lesson-099)
  tilemap                          0.981 ms              0.559 ms
  clear                            0.456                 0.449
  sprites                          0.006                 0.006
  text                             0.012                 0.010
  present                          0.444                 0.434
  total                            1.944 ms              1.505 ms
```

Le point chaud nommé **baisse de 43 %** et emporte un cinquième de la frame
avec lui. Le profil du même scénario (2 583 frames par exécution) s'accorde au
niveau des fonctions : le temps propre de `BlitSprite` passe de `2.11 s → 1.14
s` (**−46 %** — `0.957 ms` par dessin de carte à `0.466 ms`), ses appels de
`3,501,190 → 3,045,953` (le culling, visible dans le compte), et le CPU profilé
de l'exécution entière de `3.53 s → 2.28 s` (−35 %).

### Le piège des pourcentages, lu dans le même profil

Regardez ce qui est arrivé au point chaud n° 2 dans ce profil : `ClearBuffer`
lit `37.68% → 46.05%`. Le clear est-il devenu *plus lent* ? Son temps propre a
baissé de `1.33 s → 1.05 s` (le bruit d'échantillonnage et l'état du cache de
la frame), et l'instrument précis — la ligne `clear` du bilan — lit `0.456 →
0.449 ms` : **inchangé**, exactement comme le menu l'exige. La part a monté
parce que le *total* a baissé : les pourcentages sont des fractions de ce qui
reste, et une correction déplace chaque ligne qui n'est pas la sienne. C'est
pourquoi la leçon cite des millisecondes à côté de chaque pourcentage, et
pourquoi « le coût baisse » se vérifie contre le bilan, jamais contre la
colonne des parts du profil.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

- **Le changement s'attaque au point chaud nommé et le coût mesuré baisse** —
  `tilemap 0.981 → 0.559 ms` sur 1 415 vraies frames de jeu ; temps propre de
  `BlitSprite` `2.11 s → 1.14 s` sur 2 583 vraies frames du build profilé.
- **Le comportement du jeu est inchangé** — de deux façons. Au niveau des
  octets : un harnais jetable a dessiné 2 000 sprites aléatoires (opaques et
  transparents, découpés à chaque bord) à travers l'ancienne et la nouvelle
  boucle dans des tampons séparés : **zéro pixel diffère**. Au niveau des
  rapports : la transcription de l'exécution scriptée se réduit à la même forme
  que celle de l'état précédent — 106 gabarits de rapport, ensembles identiques
  — et les lignes de démonstration (les états, les écrans, les vagues, le
  combat) se rejouent comme documenté.
- **Le menu a tenu** — la boucle jumelle des sprites (`BlitSpriteFrame`, la
  ligne `sprites`, `0.006 ms`) n'est **pas** dans ce diff. Elle n'a pas été
  nommée, donc elle n'a pas été corrigée ; son remède est celui de l'exercice 1,
  si vous voulez le dépenser là.

Ce que cette exécution n'a **pas** vérifié : que ces leviers sont tous les
leviers qui existent. Le recensement (la lentille de la leçon 049, `-O3`) lit
toujours **zéro instruction vectorielle pour `BlitSprite`** — l'expansion
directe est sans branchement et séquentielle, mais elle déplace 3 octets en
entrée et 4 octets en sortie, et GCC refuse d'élargir *cela* à ces réglages. La
liste montre la forme des leviers — la garde `key_count`, l'adressage replié,
les boucles scalaires — et ce qu'exigerait l'élargissement est un changement de
*disposition*, pas un changement de boucle : les pixels des tuiles vivant dans
l'ordre du framebuffer (une allocation d'arena de plus au chargement) pour que
la copie de la carte déplace des mots, pas des octets. Cela brise l'honnêteté
d'une seule boucle de copie du moteur (« chaque pixel dessiné passe par cette
boucle », `blit.h`) et c'est consigné comme travail futur — le genre de
changement que la mesure doit demander, pas qu'une leçon glisse en contrebande.
Et tous les nombres restent ceux de cette machine et de ce build (WSL2, Xvfb
`:99`, `-O0`) : la baisse est réelle ici ; les tailles sont celles de ce banc.

## Étape de code

Un seul changement : le point chaud n° 1. `BlitSprite` de `src/blit.cpp`
accueille les deux chemins — l'expansion directe pour les sprites sans pixel
transparent, le chemin de la couleur clé avec des pointeurs de ligne hissés
pour les autres — écrivant exactement les pixels que l'ancienne boucle
écrivait. `src/sprite.h/.cpp` accueillent `key_count` et `CountKeyPixels`,
comptés à chaque site où un sprite naît (le chargeur de `sprite.cpp`, la
découpe de planche de `tiles.cpp`, la découpe de glyphes de `font.cpp`).
`DrawTileMap` de `src/tiles.cpp` ne parcourt que la fenêtre visible de
cellules. `BlitSpriteFrame` est intact — il n'a pas été nommé. Son état final
est étiqueté `lesson-099`.

```diff
diff --git a/src/blit.cpp b/src/blit.cpp
index 0d417f4..4b70b5d 100644
--- a/src/blit.cpp
+++ b/src/blit.cpp
@@ -19,18 +19,54 @@ void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
     int right = x + s.width < fb.width ? x + s.width : fb.width;
     int bottom = y + s.height < fb.height ? y + s.height : fb.height;
 
-    for (int j = top; j < bottom; ++j) {
-        for (int i = left; i < right; ++i) {
+    /* Lesson 099: this loop is the map's draw, and the measure pass
+       named it the game's hottest work — so it gets the deep dives'
+       levers, and every pixel lands exactly where it always landed.
+
+       Copy closer together: each row's source and destination pointers
+       are computed once and stepped, never recomputed per pixel.
+
+       Copy wider: a sprite with no transparent pixel — `key_count`
+       zero, counted at load — takes the straight expand below, with no
+       per-pixel decision: the shape lesson 049's lens reads (a guard, a
+       wide loop, the tails folded into the row).
+
+       And the key path keeps its per-pixel decision because it must:
+       what would "vectorized transparency" mean? (lesson 049). */
+    if (s.key_count == 0) {
+        for (int j = top; j < bottom; ++j) {
             const unsigned char *src =
-                &s.pixels[(((size_t)(j - y) * s.width) + (i - x)) * 3];
-            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
-                continue; /* the transparent color writes nothing */
+                s.pixels + (((size_t)(j - y) * s.width) + (left - x)) * 3;
             unsigned char *dst =
-                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
-            dst[0] = src[2]; /* blue */
-            dst[1] = src[1]; /* green */
-            dst[2] = src[0]; /* red */
-            dst[3] = 0;
+                fb.pixels + (((size_t)j * fb.width) + left) * 4;
+            for (int i = left; i < right; ++i) {
+                dst[0] = src[2]; /* blue */
+                dst[1] = src[1]; /* green */
+                dst[2] = src[0]; /* red */
+                dst[3] = 0;
+                src += 3;
+                dst += 4;
+            }
+        }
+        return;
+    }
+
+    for (int j = top; j < bottom; ++j) {
+        const unsigned char *src =
+            s.pixels + (((size_t)(j - y) * s.width) + (left - x)) * 3;
+        unsigned char *dst =
+            fb.pixels + (((size_t)j * fb.width) + left) * 4;
+        for (int i = left; i < right; ++i) {
+            if (!(src[0] == s.key_r && src[1] == s.key_g &&
+                  src[2] == s.key_b)) { /* the transparent color writes
+                                           nothing */
+                dst[0] = src[2]; /* blue */
+                dst[1] = src[1]; /* green */
+                dst[2] = src[0]; /* red */
+                dst[3] = 0;
+            }
+            src += 3;
+            dst += 4;
         }
     }
 }
diff --git a/src/blit.h b/src/blit.h
index 3832e8b..8faf327 100644
--- a/src/blit.h
+++ b/src/blit.h
@@ -17,7 +17,11 @@ namespace engine {
    becomes one framebuffer pixel carrying the exact color the sprite has,
    except the sprite's transparent color, which writes nothing at all.
    Pixels whose destination falls outside the framebuffer are dropped —
-   lesson 015's fold at rectangle scale, never a wrap into other pixels. */
+   lesson 015's fold at rectangle scale, never a wrap into other pixels.
+   Lesson 099: a sprite with no transparent pixel (`key_count` zero —
+   counted where sprites are born) draws through a straight expand with
+   no per-pixel decision; a sprite with one keeps the decision per
+   pixel. Both paths write the same pixels. */
 void BlitSprite(Framebuffer &fb, const Sprite &sprite, int x, int y);
 
 /* Lesson 086: one frame of a sprite sheet — the `frame_w`-wide column of
diff --git a/src/font.cpp b/src/font.cpp
index 75ff026..a3586ab 100644
--- a/src/font.cpp
+++ b/src/font.cpp
@@ -68,6 +68,7 @@ FontResult LoadFont(Arena &arena, const char *path)
                 dst[1] = src[1];
                 dst[2] = src[2];
             }
+        glyph.key_count = CountKeyPixels(glyph); /* lesson 099 */
     }
     ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
     result.error = FONT_OK;
diff --git a/src/sprite.cpp b/src/sprite.cpp
index 4d04e96..fd92f3e 100644
--- a/src/sprite.cpp
+++ b/src/sprite.cpp
@@ -64,7 +64,7 @@ bool ReadNumber(const unsigned char *data, size_t size, size_t &at, long &out)
 
 SpriteResult LoadSprite(Arena &arena, const char *path)
 {
-    SpriteResult result = { { 0, 0, 0, 0, 0, 0 }, SPRITE_OK };
+    SpriteResult result = { { 0, 0, 0, 0, 0, 0, 0 }, SPRITE_OK };
 
     platform::FileData file = platform::ReadFile(path);
     if (file.error != platform::FILE_OK) {
@@ -119,8 +119,21 @@ SpriteResult LoadSprite(Arena &arena, const char *path)
     result.sprite.key_r = SPRITE_KEY_R;
     result.sprite.key_g = SPRITE_KEY_G;
     result.sprite.key_b = SPRITE_KEY_B;
+    result.sprite.key_count = CountKeyPixels(result.sprite); /* 099 */
     result.error = SPRITE_OK;
     return result;
 }
 
+int CountKeyPixels(const Sprite &sprite)
+{
+    int count = 0;
+    for (int i = 0; i < sprite.width * sprite.height; ++i) {
+        const unsigned char *p = &sprite.pixels[(size_t)i * 3];
+        if (p[0] == sprite.key_r && p[1] == sprite.key_g &&
+            p[2] == sprite.key_b)
+            count += 1;
+    }
+    return count;
+}
+
 } /* namespace engine */
diff --git a/src/sprite.h b/src/sprite.h
index b9ef087..5529c9f 100644
--- a/src/sprite.h
+++ b/src/sprite.h
@@ -27,6 +27,9 @@ struct Sprite {
     int width;
     int height;
     unsigned char key_r, key_g, key_b; /* the transparent color */
+    int key_count; /* lesson 099: how many pixels are that color, counted
+                      once at load. Zero says the sprite is opaque — the
+                      draw can skip its per-pixel decision entirely. */
 };
 
 /* A load either hands over a complete sprite or names what went wrong —
@@ -48,6 +51,12 @@ struct SpriteResult {
    back to the OS — what the engine keeps is its copy. */
 SpriteResult LoadSprite(Arena &arena, const char *path);
 
+/* Lesson 099: the sprite's transparent pixels, counted — the fact the
+   draw's fast path is keyed on. Counted once where a sprite is born
+   (the loader, the tile sheet's cut, the font's cut), never at draw
+   time: the frame pays no decision the load already made. */
+int CountKeyPixels(const Sprite &sprite);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/tiles.cpp b/src/tiles.cpp
index 66698af..0a25834 100644
--- a/src/tiles.cpp
+++ b/src/tiles.cpp
@@ -12,7 +12,7 @@ namespace engine {
 
 TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
 {
-    TileSheetResult result = { { { { 0, 0, 0, 0, 0, 0 } } }, TILES_OK };
+    TileSheetResult result = { { { { 0, 0, 0, 0, 0, 0, 0 } } }, TILES_OK };
 
     if (kind_count <= 0 || kind_count > TILE_MAX_KINDS) {
         result.error = TILES_MALFORMED;
@@ -65,6 +65,7 @@ TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
                 dst[1] = src[1];
                 dst[2] = src[2];
             }
+        tile.key_count = CountKeyPixels(tile); /* lesson 099 */
     }
     ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
     result.error = TILES_OK;
@@ -74,8 +75,25 @@ TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
 void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
                  int x, int y)
 {
-    for (int cy = 0; cy < map.height; ++cy)
-        for (int cx = 0; cx < map.width; ++cx) {
+    /* Lesson 099: copy less — the walk visits only the cells the frame
+       can show. The window is the visible span of cells computed once
+       from the map's offset: the first cell whose right edge passes the
+       frame's left (at offset x, cell -x/TILE_SIZE is the first one
+       with a pixel on screen), through the last whose left edge is
+       inside the frame (the fold's exclusive bound). A cell outside
+       writes nothing through the blit's clipping either way — the
+       pixels drawn are the same pixels; the walk is shorter. */
+    int cx0 = x < 0 ? -x / TILE_SIZE : 0;
+    int cy0 = y < 0 ? -y / TILE_SIZE : 0;
+    int cx1 = (fb.width - x + TILE_SIZE - 1) / TILE_SIZE;
+    int cy1 = (fb.height - y + TILE_SIZE - 1) / TILE_SIZE;
+    if (cx1 > map.width)
+        cx1 = map.width;
+    if (cy1 > map.height)
+        cy1 = map.height;
+
+    for (int cy = cy0; cy < cy1; ++cy)
+        for (int cx = cx0; cx < cx1; ++cx) {
             int kind = map.cells[cy * map.width + cx];
             BlitSprite(fb, sheet.kinds[kind],
                        x + cx * TILE_SIZE, y + cy * TILE_SIZE);
```

## Exercices

Deux exercices qui poussent le remède de la leçon plus loin — un là où le menu
l'a interdit ici, un dans la mesure. Chacun se termine par sa solution — un diff
contre l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le dessin des sprites reçoit le même remède *(extend-the-code)*

`BlitSpriteFrame` est la jumelle de `BlitSprite` — la même boucle par pixel avec
la colonne de frame de la planche repliée dedans — et cette leçon l'a
délibérément laissée tranquille parce que le menu ne corrige que ce que la
passe 1 a nommé. Dépensez le levier suivant où vous voulez : donnez-lui le même
traitement (lignes hissées, expansion directe indexée sur `key_count`), puis
mesurez la ligne `sprites` avant et après sur une frame chargée et rapportez
honnêtement ce que cela a jamais valu. La réponse intéressante est la réponse
honnête.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-099/ex1.md)

### Exercice 2 — Le coût par tuile *(measure-the-performance)*

Le dessin de la carte est un compte de tuiles et un coût par tuile, et la
correction a touché les deux nombres — le culling a coupé le compte (d'environ
1 536 cellules à environ 1 200-1 271 sur cette carte, bougeant à mesure que la
caméra se borne le long de la carte), la réécriture de la boucle a coupé le
coût que chacune paie. Rendez le compte visible : une sonde qui rapporte
combien de cellules la marche a réellement dessinées, et à quel offset de
carte. Mesurez ensuite la paire (le compte de la sonde à côté des millisecondes
`tilemap` du journal de frames) sur une traversée complète de la carte, et
donnez le coût par tuile — avant et après l'étape de cette leçon — en
nanosecondes. Dites à quel levier appartient chaque nombre.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-099/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 098 — passe 1 : mesurer](lesson-098-measure.md) ·
**Suivante :** [Leçon 100 — passe 2b : corriger le clear](lesson-100-clear.md) ·
**Étiquette de code :** [`lesson-099`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-099)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-099-map-draw.md`,
révision `20f49d8`.*

<!-- translation-source: book/lessons/part-5/lesson-099-map-draw.md @ 20f49d8 -->
