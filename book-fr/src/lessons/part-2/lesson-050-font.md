# Leçon 050 — la police bitmap comme asset

{{#include ../../stability-horizon.md}}

## Prose

Le texte à l'écran est un mensonge raconté en pixels, et cette leçon commence à
le raconter : une **police bitmap** (font) — une planche de minuscules images de
glyphes, une par caractère — chargée comme asset et dessinée à travers l'unique
blitter. Aucun nouveau code de dessin n'apparaît aujourd'hui. Un glyphe est un
sprite ; la police est une planche de sprites ; tout le travail du chargeur
consiste à découper la planche en sprites pour que le chemin de blit construit
par la leçon 045 dessine les lettres avec les mêmes règles que le héros. C'est
la promesse de la leçon 045 — « le texte et les tuiles sont des sprites avec de
la comptabilité » — payée sur son premier versement.

### Le format de la planche, défini à la main

L'asset est `assets/font.ppm` — une image PPM (P6) comme le sprite, avec une
convention par-dessus : **la planche est une grille de cellules fixes de 8×8, 16
colonnes sur 6 lignes, portant les ASCII 32 (`space`) à 127 dans l'ordre de
lecture.** La cellule 0 est `space`, la cellule 16 est `P`, la cellule 65 est
`a`. Le format entier tient dans ces phrases ; le chargeur les encode :

```c++
constexpr int FONT_CELL = 8;
constexpr int FONT_COLS = 16;
constexpr int FONT_ROWS = 6;
constexpr int FONT_FIRST = 32;
constexpr int FONT_COUNT = FONT_COLS * FONT_ROWS; /* ASCII 32..127 */
```

La planche fait `128 × 48` pixels — 16 × 8 par 6 × 8 — et chaque pixel est soit
de l'encre (blanche dans cet asset), soit la couleur clé du sprite (magenta),
que le chargeur hérite de la convention de sprite de la leçon 045.
L'arrière-plan d'un glyphe est transparent ; les lettres flottent au-dessus de
ce qui se trouve derrière elles.

La règle caractère→cellule est de l'arithmétique, pas une table :

```
index = (unsigned char)c - 32
cell  = (index % 16, index / 16)        /* column, row of the cell */
```

et `FontGlyph` est cette arithmétique avec une vérification de bornes :

```c++
const Sprite *FontGlyph(const Font &font, char c)
{
    int index = (int)(unsigned char)c - FONT_FIRST;
    if (index < 0 || index >= FONT_COUNT)
        return 0; /* outside the sheet: the character has no glyph */
    return &font.glyphs[index];
}
```

Le cast `(unsigned char)` est là parce qu'un `char` sur cette plateforme est
*signé* : un octet comme `0xC3` (le premier octet d'un `Ö` en UTF-8) vaut −61
en tant que `char`, et l'arithmétique d'index doit voir la valeur de l'octet,
pas son signe. Le retour `0` est le « ce caractère n'existe pas » honnête de la
planche — la graine de la règle du glyphe manquant de la leçon 051.

### La découpe

`LoadFont` se fait en trois mouvements, tous familiers :

1. **Allouer la mémoire des glyphes** — un seul bloc d'arena de
   `96 × 8 × 8 × 3` = 18 432 octets, exactement le nombre de pixels de la
   planche. Chaque glyphe sera un sprite contigu de 192 octets dans ce bloc.
2. **Charger la planche à travers le chargeur de sprites** — c'est une image
   P6 ; `LoadSprite` sait déjà en analyser une, couleur clé comprise. La marque
   d'arena est prise *après* le bloc des glyphes, donc le retour arrière de
   l'étape suivante ne libère que les octets de la planche (la marque de la
   leçon 041 fait exactement ce travail).
3. **Découper, cellule par cellule** — pour chacune des 96 cellules, copier ses
   8 lignes × 8 pixels depuis les lignes de la planche vers le sprite contigu
   propre au glyphe.

```c++
    for (int k = 0; k < FONT_COUNT; ++k) {
        int cell_x = (k % FONT_COLS) * FONT_CELL;
        int cell_y = (k / FONT_COLS) * FONT_CELL;
        ...
        for (int r = 0; r < FONT_CELL; ++r)
            for (int c = 0; c < FONT_CELL; ++c) {
                const unsigned char *src = &sheet.sprite.pixels[...];
                unsigned char *dst = &glyph.pixels[...];
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
            }
    }
    ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
```

Pourquoi découper, au fond — pourquoi ne pas faire pointer les sprites de
glyphes dans la planche ? Parce que les lignes d'une cellule ne sont *pas*
contiguës dans la planche (la ligne 2 de la cellule `A` se trouve à 128 pixels
de la ligne 1, pas à 8), et `Sprite` n'a qu'une seule disposition simple :
ligne après ligne. La copie fait de chaque glyphe un sprite d'école — qui n'a
besoin d'aucun cas particulier dans le blitter, les vérifications, ni quoi que
ce soit de dessiné plus tard. Les 18 Ko de planche que la découpe remplace
reviennent dans l'arena avant que le jeu démarre.

### L'affirmation, vérifiée

L'exécution dessine deux glyphes à travers `BlitSprite` et relit chaque pixel
face aux propres octets du glyphe — une capitale et une lettre à jambage, les
deux formes qui attrapent les bugs de disposition :

```
engine: font assets/font.ppm: 96 glyphs of 8x8 from a 128x48 sheet
engine: font check: glyph 'A' — 64 pixels read back, 0 mismatches (18 ink, 46 key)
engine: font check: glyph 'g' — 64 pixels read back, 0 mismatches (24 ink, 40 key)
```

64 pixels par glyphe — 8 × 8 — et zéro écart : chaque pixel d'encre est là où
la planche le dit, chaque pixel de couleur clé n'a rien écrit. Les comptes sont
ceux de la forme du dessin lui-même (le `A` de cette police compte 18 pixels
d'encre ; le `g` en compte 24) et ils bougent quand le dessin bouge — c'est
ainsi que l'on sait que la vérification regarde vraiment les pixels au lieu de
hocher la tête.

La fenêtre confirme : l'étiquette que la phase de rendu dessine désormais — les
cinq glyphes posés à la main de `SCORE`, chacun son propre blit à `8 + i × 8` —
apparaît en blanc sur l'arrière-plan à travers tout le chemin de présentation.

### Un blit, cinq glyphes

La phase de rendu dessine l'étiquette de la manière brute, exprès :

```c++
        for (int li = 0; HUD_LABEL[li]; ++li) {
            const Sprite *glyph = FontGlyph(font, HUD_LABEL[li]);
            if (glyph)
                BlitSprite(*fb, *glyph, 8 + li * FONT_CELL, 8);
        }
```

Un glyphe par blit, une position par glyphe, calculée à la main dans l'index de
la boucle. Cette main est exactement ce que la leçon 051 remplace par une
disposition — des chaînes, de l'espacement, et la règle pour les caractères que
la planche n'a pas. Et remarquez où va le temps des dessins de glyphes : la
phase `sprites` du cumul de frames les absorbe (0,001 ms → 0,003 ms avec cinq
blits de plus), parce qu'un glyphe *est* un sprite. Quand la leçon 051 nommera
la phase text, ce temps passera sur sa propre ligne — mesuré, pas supposé.

## Étape de code

Un seul changement pour cette leçon : `assets/font.ppm` est écrit à la main (une
planche P6 de 128×48, 96 glyphes de 8×8 dessinés à la main), `src/font.h` /
`src/font.cpp` apportent le chargeur (la planche découpée en sprites de glyphes
via le chargeur de sprites et la marque d'arena), et `main.cpp` charge la
police, vérifie deux glyphes pixel par pixel, et dessine l'étiquette `SCORE`
posée à la main à travers le blit. Le sprite, le blitter et la sonde restent
intacts. Son état final est étiqueté `lesson-050`.

```diff
diff --git a/assets/font.ppm b/assets/font.ppm
new file mode 100644
index 0000000..9e78c68
Binary files /dev/null and b/assets/font.ppm differ
diff --git a/src/font.cpp b/src/font.cpp
new file mode 100644
index 0000000..75ff026
--- /dev/null
+++ b/src/font.cpp
@@ -0,0 +1,85 @@
+// font.cpp — cutting the glyph sheet into sprites.
+//
+// Lesson 050: the sheet is one sprite; a glyph is a cell of it. The cut
+// is the same byte-by-byte habit as every asset here — each cell's
+// pixels are copied, in order, into their own sprite.
+
+#include "font.h"
+
+namespace engine {
+
+FontResult LoadFont(Arena &arena, const char *path)
+{
+    FontResult result = { {}, FONT_OK };
+
+    /* One allocation for every glyph's pixels — 96 cells of 8x8x3. */
+    size_t glyph_bytes =
+        (size_t)FONT_COUNT * FONT_CELL * FONT_CELL * 3;
+    unsigned char *pixels =
+        (unsigned char *)ArenaAlloc(arena, glyph_bytes, 4);
+    if (!pixels) {
+        result.error = FONT_NO_ROOM;
+        return result;
+    }
+
+    /* The sheet loads through the sprite loader (it is a P6 image like
+       any other), and its bytes go back to the arena when the cut is
+       done — the mark sits after the glyphs, so only the sheet rolls
+       back. */
+    size_t mark = ArenaMark(arena);
+    SpriteResult sheet = LoadSprite(arena, path);
+    if (sheet.error == SPRITE_MISSING) {
+        result.error = FONT_MISSING;
+        return result;
+    }
+    if (sheet.error != SPRITE_OK) {
+        result.error = FONT_MALFORMED;
+        return result;
+    }
+
+    /* The format is fixed: the sheet is exactly FONT_COLS x FONT_ROWS
+       cells. A sheet of any other size is a different format's file. */
+    if (sheet.sprite.width != FONT_COLS * FONT_CELL ||
+        sheet.sprite.height != FONT_ROWS * FONT_CELL) {
+        ArenaRollback(arena, mark);
+        result.error = FONT_MALFORMED;
+        return result;
+    }
+
+    for (int k = 0; k < FONT_COUNT; ++k) {
+        int cell_x = (k % FONT_COLS) * FONT_CELL;
+        int cell_y = (k / FONT_COLS) * FONT_CELL;
+        Sprite &glyph = result.font.glyphs[k];
+        glyph.pixels =
+            pixels + (size_t)k * FONT_CELL * FONT_CELL * 3;
+        glyph.width = FONT_CELL;
+        glyph.height = FONT_CELL;
+        glyph.key_r = sheet.sprite.key_r;
+        glyph.key_g = sheet.sprite.key_g;
+        glyph.key_b = sheet.sprite.key_b;
+        for (int r = 0; r < FONT_CELL; ++r)
+            for (int c = 0; c < FONT_CELL; ++c) {
+                const unsigned char *src =
+                    &sheet.sprite.pixels[(((cell_y + r) * sheet.sprite.width) +
+                                          (cell_x + c)) * 3];
+                unsigned char *dst =
+                    &glyph.pixels[((r * FONT_CELL) + c) * 3];
+                dst[0] = src[0];
+                dst[1] = src[1];
+                dst[2] = src[2];
+            }
+    }
+    ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
+    result.error = FONT_OK;
+    return result;
+}
+
+const Sprite *FontGlyph(const Font &font, char c)
+{
+    int index = (int)(unsigned char)c - FONT_FIRST;
+    if (index < 0 || index >= FONT_COUNT)
+        return 0; /* outside the sheet: the character has no glyph */
+    return &font.glyphs[index];
+}
+
+} /* namespace engine */
diff --git a/src/font.h b/src/font.h
new file mode 100644
index 0000000..fb95e47
--- /dev/null
+++ b/src/font.h
@@ -0,0 +1,52 @@
+// font.h — the bitmap font: a glyph sheet cut into sprites.
+//
+// Lesson 050: the font is an asset like the sprite — a PPM (P6) sheet of
+// 8x8 glyph cells, 16 columns by 6 rows, covering ASCII 32..127 in
+// reading order. The loader cuts the sheet into one Sprite per glyph, so
+// every glyph draws through the one blitter (lesson 045) with no special
+// case, no second drawing path, and no new rules.
+#ifndef FONT_H
+#define FONT_H
+
+#include "sprite.h"
+
+namespace engine {
+
+/* The sheet's format, defined by hand (lesson 050): fixed 8x8 cells in a
+   16x6 grid, one character per cell starting at ASCII 32. */
+constexpr int FONT_CELL = 8;
+constexpr int FONT_COLS = 16;
+constexpr int FONT_ROWS = 6;
+constexpr int FONT_FIRST = 32;
+constexpr int FONT_COUNT = FONT_COLS * FONT_ROWS; /* ASCII 32..127 */
+
+/* A font: one sprite per glyph, cut from the sheet at load. */
+struct Font {
+    Sprite glyphs[FONT_COUNT];
+};
+
+/* A load either hands over a font or names what went wrong. */
+enum FontError {
+    FONT_OK = 0,
+    FONT_MISSING,   /* the sheet is not there or cannot be read */
+    FONT_MALFORMED, /* the bytes are not the 16x6 sheet the format fixes */
+    FONT_NO_ROOM,   /* the arena had no room for the glyphs */
+};
+
+struct FontResult {
+    Font font;
+    FontError error;
+};
+
+/* Loads a glyph sheet and cuts it into FONT_COUNT glyph sprites — one
+   arena allocation for all the glyph pixels, the sheet's own bytes given
+   back when the copy is done. */
+FontResult LoadFont(Arena &arena, const char *path);
+
+/* The glyph sprite for a character, or 0 for a character the sheet does
+   not cover. Lesson 051's layout rule builds on this answer. */
+const Sprite *FontGlyph(const Font &font, char c);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 887d551..9aba782 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -9,6 +9,7 @@
 
 #include "arena.h"
 #include "blit.h"
+#include "font.h"
 #include "framebuffer.h"
 #include "frame.h"
 #include "platform.h"
@@ -21,6 +22,11 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
+/* Lesson 050: the label the demo lays out by hand — one glyph per
+   blit, one position per glyph. Lesson 051 replaces the hand with a
+   layout loop. */
+constexpr char HUD_LABEL[] = "SCORE";
+
 /* Lesson 047: the caches deep dive's evidence — a copy walk over arena
    memory at two strides, timed at working-set sizes that cross this
    machine's caches. The walk is the blit's inner copy with the
@@ -224,6 +230,65 @@ int Run(void)
     /* Lesson 047's evidence, measured before the loop starts. */
     CacheProbe(arena);
 
+    /* Lesson 050: the font is an asset too — a glyph sheet the loader
+       cuts into sprites. */
+    const char *font_path = "assets/font.ppm";
+    FontResult font_loaded = LoadFont(arena, font_path);
+    if (font_loaded.error != FONT_OK) {
+        switch (font_loaded.error) {
+        case FONT_MISSING:
+            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
+                         font_path);
+            break;
+        case FONT_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: %s: not a 16x6 sheet of 8x8 glyphs\n",
+                         font_path);
+            break;
+        default:
+            std::fprintf(stderr, "engine: %s: no room in the arena\n",
+                         font_path);
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Font &font = font_loaded.font;
+    std::printf("engine: font %s: %d glyphs of %dx%d from a %dx%d sheet\n",
+                font_path, FONT_COUNT, FONT_CELL, FONT_CELL,
+                FONT_COLS * FONT_CELL, FONT_ROWS * FONT_CELL);
+
+    /* The glyph claim, checked against the framebuffer's bytes: a glyph
+       drawn through the blit is the sheet's cell, pixel for pixel. */
+    const char checked[2] = { 'A', 'g' };
+    for (int gi = 0; gi < 2; ++gi) {
+        const Sprite *glyph = FontGlyph(font, checked[gi]);
+        ClearBuffer(*fb, 32, 32, 64);
+        BlitSprite(*fb, *glyph, 200 + gi * 16, 64);
+        int ink = 0, key = 0, mismatches = 0;
+        for (int j = 0; j < glyph->height; ++j)
+            for (int i = 0; i < glyph->width; ++i) {
+                const unsigned char *p = &glyph->pixels[(j * glyph->width + i) * 3];
+                unsigned char r, g, b;
+                GetPixel(*fb, 200 + gi * 16 + i, 64 + j, r, g, b);
+                bool is_key = p[0] == glyph->key_r && p[1] == glyph->key_g &&
+                              p[2] == glyph->key_b;
+                if (is_key) {
+                    ++key;
+                    if (r != 32 || g != 32 || b != 64)
+                        ++mismatches;
+                } else {
+                    ++ink;
+                    if (r != p[0] || g != p[1] || b != p[2])
+                        ++mismatches;
+                }
+            }
+        std::printf("engine: font check: glyph '%c' — %d pixels read back, %d mismatches (%d ink, %d key)\n",
+                    checked[gi], glyph->width * glyph->height, mismatches,
+                    ink, key);
+    }
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
@@ -281,10 +346,17 @@ int Run(void)
 
         /* Render: every frame draws the whole scene — clear, then the
            sprite through the one blit. The sprite draw is timed as its own
-           named phase: the first subsystem the frame record can name. */
+           named phase: the first subsystem the frame record can name.
+           Glyphs are sprites too (lesson 050) — they count here until
+           lesson 051 names the text phase. */
         ClearBuffer(*fb, 32, 32, 64);
         double t_sprites = platform::Now();
         BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
+        for (int li = 0; HUD_LABEL[li]; ++li) {
+            const Sprite *glyph = FontGlyph(font, HUD_LABEL[li]);
+            if (glyph)
+                BlitSprite(*fb, *glyph, 8 + li * FONT_CELL, 8);
+        }
         frame.sprites = platform::Now() - t_sprites;
 
         frame.render = platform::Now() - t1;
```

Les pixels de la police sont binaires, et son diff l'est aussi — git imprime
`Binary files … differ` pour lui. Le format de la planche tient dans les quatre
constantes de `font.h` ; son art, c'est le fichier.

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Le dump de la police *(extend-the-code)*

La planche est une grille de 96 glyphes ; prouvez que la découpe est sans perte
en redessinant la grille : dessinez chaque glyphe par blit à sa position dans
la planche (une grille de 16×6 cellules de 8×8, dans un coin fixe de la frame),
puis relisez chaque pixel du dump et comparez-le avec les propres octets des
glyphes. Rapportez un seul nombre : le nombre d'écarts. Que vous dirait un
compte non nul sur la découpe — et quel diagnostic correspond à quel bug : des
écarts dans *chaque* glyphe, ou des écarts dans *exactement un* ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-050/ex1.md)

### Exercice 2 — Les octets dont la planche n'a jamais entendu parler *(predict-the-output)*

`FontGlyph` répond `0` pour les caractères hors d'ASCII 32..127 — mais la partie
intéressante est l'arithmétique qui l'y amène. Avant d'exécuter quoi que ce
soit, notez ce que la fonction retourne pour cinq sondes : `A`, le saut de
ligne, l'octet `0`, l'octet `200`, et l'octet `195` (un octet de tête UTF-8).
Étendez ensuite l'exécution pour imprimer les cinq réponses et pour faire un
essai à blanc d'une étiquette contenant un `Ö` — dont l'encodage UTF-8 fait
*deux* octets — et rapportez combien d'octets de l'étiquette ont trouvé un
glyphe. Réconciliez chaque réponse avec l'arithmétique d'index, y compris le
cas où un `char` signé aurait calculé un index différent de celui que l'octet
mérite.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-050/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 049 — la lentille SIMD](lesson-049-simd.md) ·
**Suivante :** [Leçon 051 — du texte à l'écran](lesson-051-text.md) ·
**Étiquette de code :** [`lesson-050`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-050)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-050-font.md`,
révision `2230eb0`.*

<!-- translation-source: book/lessons/part-2/lesson-050-font.md @ 2230eb0 -->
