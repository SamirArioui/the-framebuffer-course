# Leçon 045 — le blit découpé et transparent

{{#include ../../stability-horizon.md}}

## Prose

La leçon précédente a fait arriver les octets du sprite ; aujourd'hui, ils
deviennent des pixels. Voici le moteur de rendu : **une boucle de copie** — le
*blitter* — qui fait entrer les pixels source dans le framebuffer à une origine
donnée, en obéissant à exactement deux règles. Un pixel de la couleur
transparente du sprite n'écrit **rien** (transparence), et un pixel dont la
destination tombe hors du framebuffer est **abandonné** (découpage). Tout ce que
la partie 2 dessine à partir d'ici — glyphes, tuiles, le monde de la démo de
clôture — passera par cette unique boucle. C'est délibéré : les plongées des
leçons 047-049 mesurent et lisent *une honnête unité de travail*, et
l'optimiseur de la partie 5 corrige la même unité.

### Deux règles, une boucle

**La transparence**, c'est le sprite qui désigne une couleur comme *rien*. La
nôtre est le magenta — `255, 0, 255` — et c'est la première règle qu'applique la
boucle de copie : si le pixel source est de la couleur clé, sautez-le, en
laissant exactement tel quel ce qui était dans le framebuffer. Pas du noir, pas
un mélange : *rien*. La couleur clé est une donnée du sprite (`key_r, key_g,
key_b` dans `Sprite`), parce que le même sprite pourra être dessiné plus tard
par n'importe quoi et que « quelle couleur veut dire rien » appartient à
l'image, pas à l'appel de dessin. Le PPM n'a pas de champ pour cela, alors le
cours fixe la convention au chargement : le magenta est la clé.

**Le découpage**, c'est le pli de la leçon 015 à l'échelle du rectangle. Un
sprite dessiné en `(-4, -4)` a ses quatre premières lignes et colonnes qui
pendent hors de la frame ; ces pixels sont abandonnés — jamais retournés dans
d'autres lignes, jamais écrits dans de la mémoire qui n'est pas le framebuffer.
Le pli était par pixel dans `PutPixel` ; le blitter prend la même décision *une
fois par bord* : calculer le rectangle où sprite et framebuffer se recouvrent,
et ne laisser la boucle de copie tourner que là.

### La boucle

```c++
void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
{
    /* The fold, at rectangle scale: the in-bounds region computed once,
       so the copy loop below never checks bounds again. */
    int left = x < 0 ? 0 : x;
    int top = y < 0 ? 0 : y;
    int right = x + s.width < fb.width ? x + s.width : fb.width;
    int bottom = y + s.height < fb.height ? y + s.height : fb.height;

    for (int j = top; j < bottom; ++j) {
        for (int i = left; i < right; ++i) {
            const unsigned char *src =
                &s.pixels[(((size_t)(j - y) * s.width) + (i - x)) * 3];
            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
                continue; /* the transparent color writes nothing */
            unsigned char *dst =
                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
            dst[0] = src[2]; /* blue */
            dst[1] = src[1]; /* green */
            dst[2] = src[0]; /* red */
            dst[3] = 0;
        }
    }
}
```

Lisez-la en quatre mouvements :

- **Le rectangle de découpage.** Quatre bornages transforment l'origine demandée
  en rectangle `left, top .. right, bottom` — l'intersection de la boîte du
  sprite avec la frame. Dessiné entièrement à l'intérieur ? Les bornages ne
  changent rien. Pendu à un bord ? Le rectangle rétrécit à ce qui survit. La
  boucle interne n'a alors aucune vérification de limites : chaque pixel qu'elle
  touche est connu comme étant à l'intérieur. Une décision par bord au lieu
  d'une par pixel — et la boucle en est moins chère, ce que la leçon 047 rendra
  concret.
- **L'indice source.** `(j - y, i - x)` ramène un pixel de destination au pixel
  du sprite qui lui revient. Le `(0, 0)` du sprite atterrit en `(x, y)` ; le
  découpage n'a jamais bougé cette origine, il a seulement refusé de dessiner en
  dehors.
- **Le test de la clé.** Trois comparaisons d'octets et un `continue`. Voilà
  toute la transparence : le pixel n'écrit rien, et la copie passe au suivant.
- **La copie elle-même.** Trois octets sortis du sprite, quatre octets entrés
  dans le framebuffer — et *échangés* : le pixel du framebuffer est bleu, vert,
  rouge, un octet inutilisé (le format de la leçon 030, ce que transporte
  `Present`), tandis que le pixel du PPM est rouge, vert, bleu. La copie écrit
  `dst[0] = src[2]` et ses amis ; les couleurs du sprite arrivent à l'écran
  inchangées. Les octets sont écrits directement — pas via `PutPixel` — parce
  que cette boucle interne est celle que les quatre leçons suivantes mesurent,
  lisent et vectorisent.

### Les affirmations, vérifiées

Le bloc de vérification de l'étape de code dessine le sprite via le blit et
relit le framebuffer avec `GetPixel`, un pixel à la fois — la même habitude de
relecture que la leçon 044 appliquait au fichier :

```
$ DISPLAY=:99 ./build/game
engine: sprite assets/sprite.ppm: 16x16, 768 pixel bytes
engine: pixel 0,0 = 255,0,255
engine: pixel 8,8 = 220,40,40
engine: pixel bytes sum to 125580
engine: blit check: 130 opaque pixels drawn unchanged, 0 mismatches
engine: blit check: 126 key pixels wrote nothing over the background
engine: blit check: clip at -4,-4 landed 144 pixels, 0 wrong, 0 touched outside
engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames
...
frame 1: update 0.000 ms, render 0.381 ms, present 0.602 ms, total 0.984 ms
```

Trois affirmations, trois verdicts :

- **Dessinés inchangés** — les 130 pixels opaques se relisent exactement dans la
  couleur que contient le fichier, et zéro écart. La copie est fidèle : des
  octets sources aux pixels du framebuffer, ordre échangé, valeurs préservées.
- **La transparence n'écrit rien** — les 126 pixels de couleur clé du sprite ont
  été dessinés *par-dessus* un fond effacé et l'ont laissé intact à chacun
  d'eux. Le compte compte : 130 + 126 = 256 = 16 × 16. Chaque pixel est
  comptabilisé — copié, ou délibérément pas.
- **Le découpage abandonne** — dessiné en `(-4, -4)`, exactement
  `12 × 12 = 144` pixels ont atterri (les lignes et colonnes qui survivent), 0 a
  mal atterri, et 0 pixel *hors* du rectangle atterri a été touché. Cette
  dernière colonne est le détecteur de retournement (wrap) : une boucle qui
  écrirait les pixels abandonnés à des adresses retournées l'allumerait.

La fenêtre raconte la même histoire côté OS (la relecture de la leçon 031,
échantillonnée à la position du sprite à l'écran, `(32, 32)`) :

```
32,32 -> r=32 g=32 b=64      <- sprite pixel (0,0): the key — background shows through
36,36 -> r=32 g=32 b=64      <- sprite pixel (4,4): also key
44,44 -> r=220 g=40 b=40     <- sprite pixel (12,12): red, copied unchanged
100,100 -> r=32 g=32 b=64    <- outside the sprite: untouched
```

Les deux premières lignes sont la règle de transparence, visible à travers tout
le chemin de présentation : les coins magenta du sprite ne sont pas à l'écran.
Les pixels derrière eux, si.

### Une boucle pour régner sur le dessin

Le sprite est posé à un endroit fixe aujourd'hui et le marqueur bouge toujours.
Le propos de la leçon n'est pas l'image — c'est l'entonnoir. Les glyphes de la
leçon 050 seront des sprites découpés dans une planche de police ; les tuiles de
la leçon 053 seront des sprites découpés dans une planche de tuiles ; tous deux
se dessineront via `BlitSprite`, avec rien d'autre autour de l'appel que de la
comptabilité. Un découpage, un test de clé, une copie — tout le comportement du
moteur de rendu est auditable en une trentaine de lignes, et les plongées qui
suivent n'ont qu'une seule boucle à passer au crible.

## Étape de code

Un seul changement pour cette leçon : `src/blit.h` / `src/blit.cpp` apportent le
blitter (la boucle de copie découpée et transparente), `Sprite` gagne la couleur
transparente que le chargeur remplit d'après la convention du cours, et
`main.cpp` vérifie les trois affirmations du blit contre le framebuffer au
démarrage et dessine le sprite via lui à chaque frame. Le marqueur est intact.
Son état final est étiqueté `lesson-045`.

```diff
diff --git a/src/blit.cpp b/src/blit.cpp
new file mode 100644
index 0000000..1c52f0d
--- /dev/null
+++ b/src/blit.cpp
@@ -0,0 +1,38 @@
+// blit.cpp — the copy loop.
+//
+// Lesson 045: the engine's one drawing loop. It clips first — the
+// intersection of the sprite's rectangle with the framebuffer is the only
+// region that can be drawn — then copies bytes: three of the sprite's
+// bytes into four of the framebuffer's, in the framebuffer's order,
+// skipping pixels of the transparent color.
+
+#include "blit.h"
+
+namespace engine {
+
+void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
+{
+    /* The fold, at rectangle scale: the in-bounds region computed once,
+       so the copy loop below never checks bounds again. */
+    int left = x < 0 ? 0 : x;
+    int top = y < 0 ? 0 : y;
+    int right = x + s.width < fb.width ? x + s.width : fb.width;
+    int bottom = y + s.height < fb.height ? y + s.height : fb.height;
+
+    for (int j = top; j < bottom; ++j) {
+        for (int i = left; i < right; ++i) {
+            const unsigned char *src =
+                &s.pixels[(((size_t)(j - y) * s.width) + (i - x)) * 3];
+            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
+                continue; /* the transparent color writes nothing */
+            unsigned char *dst =
+                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
+            dst[0] = src[2]; /* blue */
+            dst[1] = src[1]; /* green */
+            dst[2] = src[0]; /* red */
+            dst[3] = 0;
+        }
+    }
+}
+
+} /* namespace engine */
diff --git a/src/blit.h b/src/blit.h
new file mode 100644
index 0000000..259ddf3
--- /dev/null
+++ b/src/blit.h
@@ -0,0 +1,25 @@
+// blit.h — the blitter: one clipped, transparent copy from a sprite's
+// bytes into the framebuffer.
+//
+// Lesson 045: every drawn pixel in the game comes through this loop —
+// sprites now, glyphs and tiles later. One copy, one place where clipping
+// and transparency live, and one honest unit of work for the deep dives of
+// lessons 047-049 to measure and read.
+#ifndef BLIT_H
+#define BLIT_H
+
+#include "framebuffer.h"
+#include "sprite.h"
+
+namespace engine {
+
+/* Draws a sprite with its top-left corner at (x, y): each source pixel
+   becomes one framebuffer pixel carrying the exact color the sprite has,
+   except the sprite's transparent color, which writes nothing at all.
+   Pixels whose destination falls outside the framebuffer are dropped —
+   lesson 015's fold at rectangle scale, never a wrap into other pixels. */
+void BlitSprite(Framebuffer &fb, const Sprite &sprite, int x, int y);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 8febd4a..7b750ac 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -8,6 +8,7 @@
 #include <cstdio>
 
 #include "arena.h"
+#include "blit.h"
 #include "framebuffer.h"
 #include "frame.h"
 #include "platform.h"
@@ -100,6 +101,64 @@ int Run(void)
                                sprite.width / 2) * 3 + 2]);
     std::printf("engine: pixel bytes sum to %ld\n", byte_sum);
 
+    /* Lesson 045: the blitter's three claims, checked against the
+       framebuffer's own bytes before anything depends on them. */
+    ClearBuffer(*fb, 32, 32, 64);
+    BlitSprite(*fb, sprite, 100, 100);
+    int opaque = 0, key_pixels = 0, mismatches = 0;
+    for (int j = 0; j < sprite.height; ++j)
+        for (int i = 0; i < sprite.width; ++i) {
+            const unsigned char *p =
+                &sprite.pixels[(j * sprite.width + i) * 3];
+            unsigned char r, g, b;
+            GetPixel(*fb, 100 + i, 100 + j, r, g, b);
+            bool is_key = p[0] == sprite.key_r && p[1] == sprite.key_g &&
+                          p[2] == sprite.key_b;
+            if (is_key) {
+                ++key_pixels;
+                if (r != 32 || g != 32 || b != 64)
+                    ++mismatches; /* the key must have written nothing */
+            } else {
+                ++opaque;
+                if (r != p[0] || g != p[1] || b != p[2])
+                    ++mismatches;
+            }
+        }
+    std::printf("engine: blit check: %d opaque pixels drawn unchanged, %d mismatches\n",
+                opaque, mismatches);
+    std::printf("engine: blit check: %d key pixels wrote nothing over the background\n",
+                key_pixels);
+
+    ClearBuffer(*fb, 32, 32, 64);
+    BlitSprite(*fb, sprite, -4, -4);
+    int landed = 0, wrong = 0, wrapped = 0;
+    for (int j = 0; j < sprite.height; ++j)
+        for (int i = 0; i < sprite.width; ++i) {
+            if (i < 4 || j < 4)
+                continue; /* these pixels landed outside and were dropped */
+            const unsigned char *p =
+                &sprite.pixels[(j * sprite.width + i) * 3];
+            unsigned char r, g, b;
+            GetPixel(*fb, i - 4, j - 4, r, g, b);
+            ++landed;
+            bool is_key = p[0] == sprite.key_r && p[1] == sprite.key_g &&
+                          p[2] == sprite.key_b;
+            if (is_key ? (r != 32 || g != 32 || b != 64)
+                       : (r != p[0] || g != p[1] || b != p[2]))
+                ++wrong;
+        }
+    for (int y = 0; y < FRAME_HEIGHT; ++y)
+        for (int x = 0; x < FRAME_WIDTH; ++x) {
+            if (x < sprite.width - 4 && y < sprite.height - 4)
+                continue; /* the landed region, checked above */
+            unsigned char r, g, b;
+            GetPixel(*fb, x, y, r, g, b);
+            if (r != 32 || g != 32 || b != 64)
+                ++wrapped;
+        }
+    std::printf("engine: blit check: clip at -4,-4 landed %d pixels, %d wrong, %d touched outside\n",
+                landed, wrong, wrapped);
+
     double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
     double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
     double started = platform::Now();
@@ -155,8 +214,10 @@ int Run(void)
             std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
                         (int)marker_y, platform::Now() - started);
 
-        /* Render: every frame draws the whole scene — clear, then marker. */
+        /* Render: every frame draws the whole scene — clear, then the
+           sprite through the one blit. */
         ClearBuffer(*fb, 32, 32, 64);
+        BlitSprite(*fb, sprite, 32, 32);
         DrawMarker(*fb, (int)marker_x, (int)marker_y);
 
         frame.render = platform::Now() - t1;
diff --git a/src/sprite.cpp b/src/sprite.cpp
index 449b4dc..4d04e96 100644
--- a/src/sprite.cpp
+++ b/src/sprite.cpp
@@ -64,7 +64,7 @@ bool ReadNumber(const unsigned char *data, size_t size, size_t &at, long &out)
 
 SpriteResult LoadSprite(Arena &arena, const char *path)
 {
-    SpriteResult result = { { 0, 0, 0 }, SPRITE_OK };
+    SpriteResult result = { { 0, 0, 0, 0, 0, 0 }, SPRITE_OK };
 
     platform::FileData file = platform::ReadFile(path);
     if (file.error != platform::FILE_OK) {
@@ -116,6 +116,9 @@ SpriteResult LoadSprite(Arena &arena, const char *path)
     result.sprite.pixels = pixels;
     result.sprite.width = (int)width;
     result.sprite.height = (int)height;
+    result.sprite.key_r = SPRITE_KEY_R;
+    result.sprite.key_g = SPRITE_KEY_G;
+    result.sprite.key_b = SPRITE_KEY_B;
     result.error = SPRITE_OK;
     return result;
 }
diff --git a/src/sprite.h b/src/sprite.h
index 5be7140..b9ef087 100644
--- a/src/sprite.h
+++ b/src/sprite.h
@@ -12,12 +12,21 @@
 
 namespace engine {
 
+/* The transparent color of the course's sprites: magenta. PPM carries no
+   key field, so the format's convention is the loader's job — every sprite
+   loaded here names 255,0,255 as "draw nothing". */
+constexpr unsigned char SPRITE_KEY_R = 255;
+constexpr unsigned char SPRITE_KEY_G = 0;
+constexpr unsigned char SPRITE_KEY_B = 255;
+
 /* A sprite: one image's pixels in the engine's memory — row after row,
-   three bytes each (red, green, blue), exactly the file's pixel section. */
+   three bytes each (red, green, blue), exactly the file's pixel section —
+   and the color that means "nothing" when it is drawn. */
 struct Sprite {
     unsigned char *pixels; /* width * height * 3 bytes */
     int width;
     int height;
+    unsigned char key_r, key_g, key_b; /* the transparent color */
 };
 
 /* A load either hands over a complete sprite or names what went wrong —
```

## Exercices

Deux extensions « faites-les vôtres ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — Votre propre clé *(extend-the-code)*

Le magenta est une convention que le chargeur a inventée — le PPM n'a aucun
champ qui pourrait nommer la clé. Faites plutôt nommer la clé par les octets du
sprite lui-même : le pixel en haut à gauche de l'image *est* la couleur
transparente, quelle qu'elle soit, et les lignes d'inspection impriment la clé
que le chargeur a choisie. Recoloriez ensuite une copie du sprite pour que son
coin soit une couleur de votre choix — lime, cyan, n'importe quoi sauf le
magenta — avec des pixels magenta comme art véritable, et lancez les trois
vérifications du blit contre elle. Que coûte cette convention à une image qui a
réellement besoin de la couleur de son coin comme art ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-045/ex1.md)

### Exercice 2 — Les quatre coins *(predict-the-output)*

La vérification de découpage dessine à une seule position. Étendez-la à quatre :
`(-4, -4)`, `(632, 472)`, `(-100, 0)` et `(640, 480)`. Pour chacune, notez le
nombre de pixels du sprite que vous attendez voir atterrir *avant de lancer quoi
que ce soit* — le sprite fait 16×16, la frame fait 640×480, et l'arithmétique
est l'exercice. Lancez ensuite, réconciliez chaque compte, et dites ce qu'aurait
rapporté la colonne `0 touched outside` si la boucle de copie avait retourné au
lieu d'abandonner — pourquoi est-ce cette colonne qui attrape le bug ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-045/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 044 — un sprite comme des octets chargés](lesson-044-sprite-bytes.md) ·
**Suivante :** [Leçon 046 — le sprite se déplace](lesson-046-movable-sprite.md) ·
**Étiquette de code :** [`lesson-045`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-045)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-045-blit.md`,
révision `d843c56`.*

<!-- translation-source: book/lessons/part-2/lesson-045-blit.md @ d843c56 -->
