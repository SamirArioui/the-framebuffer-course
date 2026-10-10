# Leçon 051 — du texte à l'écran

{{#include ../../stability-horizon.md}}

## Prose

À la leçon précédente, la police est devenue des sprites ; aujourd'hui, les
sprites deviennent du **texte** : des chaînes disposées, des glyphes dessinés,
et la règle pour les caractères que la police n'a pas. Cette leçon livre la
troisième obligation du MVD — *du texte bitmap pour les écrans* — sous la forme
que tout le reste du cours utilise : une ligne de HUD dessinée à travers
l'unique blit. Des chaînes, de l'espacement et une règle de glyphe manquant ont
l'air de petites choses ; elles font la différence entre un moteur de rendu et
un moteur de rendu qui sait parler.

### La règle de disposition

`DrawText` est une boucle et une règle : **un caractère, un emplacement**.

```c++
void DrawText(Framebuffer &fb, const Font &font, const char *text, int x,
              int y)
{
    for (int i = 0; text[i]; ++i) {
        const Sprite *glyph = FontGlyph(font, text[i]);
        if (glyph) /* the missing-glyph rule: draw nothing, keep the slot */
            BlitSprite(fb, *glyph, x + i * FONT_CELL, y);
    }
}
```

Le caractère *i* se dessine à `x + i × FONT_CELL` — la position du caractère se
calcule à partir de son seul index. Toute la disposition est là : pas d'état de
curseur, pas de crénage, pas de mesure. `TextWidth` est la même arithmétique
lue à voix haute (`length × FONT_CELL`), ce dont le centrage ou l'alignement à
droite d'une ligne aura besoin plus tard.

L'avance fixe de 8 pixels est une simplification délibérée — une disposition
*monospace*. Les polices proportionnelles (où `i` est plus étroit que `W`)
demandent des largeurs d'avance par glyphe dans le format de la planche et un
curseur qui les accumule ; c'est une extension de format pour une édition
ultérieure, pas une fonctionnalité cachée ici. Le format du cours reste celui
que la leçon 050 a fixé.

### La règle du glyphe manquant

Certains octets ne sont pas dans la planche — le `µ` de `µs`, un accent UTF-8,
un caractère de contrôle. La règle (les mots de la spéc) : *un caractère que la
police ne fournit pas est sauté sans corrompre la disposition des caractères
suivants.* Dans la boucle ci-dessus, c'est exactement un `if` : pas de glyphe,
pas de dessin — **mais l'emplacement avance quand même**. Le caractère suivant
se trouve là où la disposition dit qu'il se trouve, pas là où le placerait un
redessin sans le caractère manquant.

La vérification dessine le cas et le relit :

```
engine: font assets/font.ppm: 96 glyphs of 8x8 from a 128x48 sheet
...
engine: text check: "AB" at 200,120 — 2 of 2 slots have ink, width 16
engine: text check: "A?B" with a missing character — ink pixels per slot: 18, 0, 0, 23
```

La seconde ligne est `A`, `µ`, `B` — et le µ fait **deux octets** (U+00B5 en
UTF-8 vaut `0xC2 0xB5`), donc la chaîne compte quatre caractères pour le moteur
et la vérification lit quatre emplacements : `18` pixels d'encre pour `A`, `0`
et `0` pour les emplacements des deux octets manquants, et `23` pour `B` — à
`x + 3 × 8`, exactement où la disposition l'a placé. Le trou est *vide* et le
texte est *intact*. Ce sont deux affirmations différentes et la carte des
emplacements vérifie les deux.

Une distinction de plus, dont la règle dépend et qui mérite d'être vue
explicitement :

|  | `space` (octet 32) | octet 200 |
| --- | --- | --- |
| `FontGlyph` | **a un glyphe** — cellule 0 | **retourne 0** — hors de la planche |
| Les pixels du glyphe | 64 pixels de couleur clé | — |
| Ce qui se dessine | rien (la clé n'écrit rien) | rien (il n'y a pas de sprite) |
| L'emplacement | avance | avance |

Deux mécanismes différents, les mêmes pixels à l'écran. Un espace est un glyphe
qui se trouve être entièrement transparent ; un octet manquant n'a aucun
glyphe. La disposition ne se soucie pas de savoir lequel — c'est pourquoi la
règle peut tenir en un `if`.

### L'enregistrement nomme le texte

Le texte est désormais son propre sous-système, et l'enregistrement de frame le
dit — la deuxième phase nommée à l'intérieur de `render`, après `sprites` de la
leçon 046 :

```
frame 1: update 0.000 ms, render 0.388 ms (sprites 0.001, text 0.002), present 0.808 ms, total 1.197 ms
...
engine: 1 frames — avg 1.197 ms (update 0.000, render 0.388 incl. sprites 0.001, text 0.002, present 0.808)
```

`text 0.002` correspond à cinq glyphes à travers le blit — un cinquième de
microseconde par glyphe, le même coût par pixel que n'importe quel dessin de
sprite, parce que c'*est* un dessin de sprite avec une boucle autour. La phase
nommée existe pour que le budget de frames puisse le dire, et pour que la table
finale (leçon 058) ait une ligne text qui grandit avec le HUD au lieu de se
cacher dans `render`.

### O3, livrée

La ligne de la checklist du MVD — *du texte bitmap pour les écrans* — est
maintenant implémentée et démontrable : des chaînes de n'importe quelle
longueur, avec n'importe lequel des 96 caractères, disposées et dessinées avec
le même découpage et la même transparence que tout le reste à l'écran. Le HUD
de la démo (l'étiquette `SCORE`) passe par `DrawText` à partir de cette leçon ;
la table de budget de frames de la leçon 058 utilise le même appel pour
imprimer ses propres nombres. Le texte était la dernière capacité de dessin
dont la démo de clôture a besoin — le monde derrière lui est la suite.

## Étape de code

Un seul changement pour cette leçon : `src/text.h` / `src/text.cpp` apportent la
boucle de disposition et la règle du glyphe manquant, `frame.h` / `frame.cpp`
font grandir la phase `text` nommée de l'enregistrement, et `main.cpp` vérifie
les affirmations sur le texte (y compris le cas du caractère manquant), dessine
le HUD à travers `DrawText`, et fait passer le nouveau champ par la ligne de
journal et par le cumul. La police, le sprite et le blitter restent intacts.
Son état final est étiqueté `lesson-051`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index e104e47..36a2827 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -14,6 +14,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
     stats.present_sum += frame.present;
     stats.total_sum += frame.total;
     stats.sprites_sum += frame.sprites;
+    stats.text_sum += frame.text;
     if (frame.total > stats.worst) {
         stats.worst = frame.total;
         stats.worst_number = frame.number;
diff --git a/src/frame.h b/src/frame.h
index bea2fbf..3a68029 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -23,6 +23,7 @@ struct FrameRecord {
        (lesson 058) grows from. The named times are inside render, never
        instead of it: render stays the phase, these say where it went. */
     double sprites; /* sprite draws through the blit */
+    double text;    /* lesson 051: text drawing — glyphs through the blit */
 };
 
 /* The running account: every frame measured so far. */
@@ -33,6 +34,7 @@ struct FrameStats {
     double present_sum;
     double total_sum;
     double sprites_sum; /* lesson 046's named sub-phase, summed like the rest */
+    double text_sum;
     double worst;      /* the longest frame so far */
     long worst_number; /* and which one it was */
 };
diff --git a/src/main.cpp b/src/main.cpp
index 9aba782..c60358b 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -14,6 +14,7 @@
 #include "frame.h"
 #include "platform.h"
 #include "sprite.h"
+#include "text.h"
 
 namespace engine {
 
@@ -289,6 +290,48 @@ int Run(void)
                     ink, key);
     }
 
+    /* Lesson 051: the text claims — glyphs laid out in order, and the
+       missing-glyph rule: the character the font lacks draws nothing and
+       the characters after it keep their slots. */
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawText(*fb, font, "AB", 200, 120);
+    {
+        int slots_with_ink = 0;
+        for (int s = 0; s < 2; ++s) {
+            bool ink = false;
+            for (int j = 0; j < FONT_CELL && !ink; ++j)
+                for (int i = 0; i < FONT_CELL && !ink; ++i) {
+                    unsigned char r, g, b;
+                    GetPixel(*fb, 200 + s * FONT_CELL + i, 120 + j, r, g, b);
+                    if (r != 32 || g != 32 || b != 64)
+                        ink = true;
+                }
+            if (ink)
+                ++slots_with_ink;
+        }
+        std::printf("engine: text check: \"AB\" at 200,120 — %d of 2 slots have ink, width %d\n",
+                    slots_with_ink, TextWidth("AB"));
+    }
+    ClearBuffer(*fb, 32, 32, 64);
+    DrawText(*fb, font, "A\xC2\xB5" "B", 200, 140); /* "AµB": µ is not in the sheet */
+    {
+        /* Four bytes, four slots: the µ is two bytes, and each keeps its
+           slot — the layout follows the bytes, and B lands where the
+           layout says. */
+        int slot_state[4] = { 0, 0, 0, 0 };
+        for (int s = 0; s < 4; ++s) {
+            for (int j = 0; j < FONT_CELL; ++j)
+                for (int i = 0; i < FONT_CELL; ++i) {
+                    unsigned char r, g, b;
+                    GetPixel(*fb, 200 + s * FONT_CELL + i, 140 + j, r, g, b);
+                    if (r != 32 || g != 32 || b != 64)
+                        ++slot_state[s];
+                }
+        }
+        std::printf("engine: text check: \"A?B\" with a missing character — ink pixels per slot: %d, %d, %d, %d\n",
+                    slot_state[0], slot_state[1], slot_state[2], slot_state[3]);
+    }
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
@@ -345,19 +388,15 @@ int Run(void)
                         (int)sprite_y, platform::Now() - started);
 
         /* Render: every frame draws the whole scene — clear, then the
-           sprite through the one blit. The sprite draw is timed as its own
-           named phase: the first subsystem the frame record can name.
-           Glyphs are sprites too (lesson 050) — they count here until
-           lesson 051 names the text phase. */
+           sprite and the text, each timed as its own named phase: the
+           subsystems the frame record can name. */
         ClearBuffer(*fb, 32, 32, 64);
         double t_sprites = platform::Now();
         BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
-        for (int li = 0; HUD_LABEL[li]; ++li) {
-            const Sprite *glyph = FontGlyph(font, HUD_LABEL[li]);
-            if (glyph)
-                BlitSprite(*fb, *glyph, 8 + li * FONT_CELL, 8);
-        }
         frame.sprites = platform::Now() - t_sprites;
+        double t_text = platform::Now();
+        DrawText(*fb, font, HUD_LABEL, 8, 8);
+        frame.text = platform::Now() - t_text;
 
         frame.render = platform::Now() - t1;
         double t2 = platform::Now();
@@ -380,20 +419,21 @@ int Run(void)
 
         /* The frame log: one line per record — the format grows its named
            fields, one per subsystem, as the parts name them. */
-        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f), present %.3f ms, total %.3f ms\n",
+        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f), present %.3f ms, total %.3f ms\n",
                     frame.number, frame.update * 1e3, frame.render * 1e3,
-                    frame.sprites * 1e3, frame.present * 1e3,
-                    frame.total * 1e3);
+                    frame.sprites * 1e3, frame.text * 1e3,
+                    frame.present * 1e3, frame.total * 1e3);
     }
 
     /* The account: what the frames actually cost, including the honest
        price of the presentation copy. */
     if (stats.frames) {
         double n = (double)stats.frames;
-        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, present %.3f)\n",
+        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, present %.3f)\n",
                     stats.frames, stats.total_sum / n * 1e3,
                     stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
-                    stats.sprites_sum / n * 1e3, stats.present_sum / n * 1e3);
+                    stats.sprites_sum / n * 1e3, stats.text_sum / n * 1e3,
+                    stats.present_sum / n * 1e3);
         std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
                     stats.worst * 1e3, stats.worst_number,
                     100.0 * stats.present_sum / stats.total_sum);
diff --git a/src/text.cpp b/src/text.cpp
new file mode 100644
index 0000000..3d9edff
--- /dev/null
+++ b/src/text.cpp
@@ -0,0 +1,31 @@
+// text.cpp — the layout loop.
+//
+// Lesson 051: every character advances the layout; only characters with
+// a glyph draw. Two rules, and the second one is the whole of the
+// missing-glyph behavior.
+
+#include "text.h"
+
+#include "blit.h"
+
+namespace engine {
+
+void DrawText(Framebuffer &fb, const Font &font, const char *text, int x,
+              int y)
+{
+    for (int i = 0; text[i]; ++i) {
+        const Sprite *glyph = FontGlyph(font, text[i]);
+        if (glyph) /* the missing-glyph rule: draw nothing, keep the slot */
+            BlitSprite(fb, *glyph, x + i * FONT_CELL, y);
+    }
+}
+
+int TextWidth(const char *text)
+{
+    int n = 0;
+    while (text[n])
+        ++n;
+    return n * FONT_CELL;
+}
+
+} /* namespace engine */
diff --git a/src/text.h b/src/text.h
new file mode 100644
index 0000000..93c0bb4
--- /dev/null
+++ b/src/text.h
@@ -0,0 +1,27 @@
+// text.h — strings drawn through the blit, glyph by glyph.
+//
+// Lesson 051: text is sprites with bookkeeping — a layout rule and the
+// one blit. The missing-glyph rule lives here: a character the font does
+// not have draws nothing, and the layout does not notice.
+#ifndef TEXT_H
+#define TEXT_H
+
+#include "font.h"
+#include "framebuffer.h"
+
+namespace engine {
+
+/* Draws a string with its top-left at (x, y): one glyph per character,
+   each FONT_CELL pixels to the right of the last — including characters
+   with no glyph, which draw nothing but keep their slot. The layout is
+   per character, so the string's shape never depends on which glyphs the
+   font happens to have. Clipping and transparency are the blitter's. */
+void DrawText(Framebuffer &fb, const Font &font, const char *text, int x,
+              int y);
+
+/* How many pixels a string's layout covers: its length x FONT_CELL. */
+int TextWidth(const char *text);
+
+} /* namespace engine */
+
+#endif
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Le HUD qui compte *(extend-the-code)*

Une étiquette est statique ; un HUD est de la donnée. Construisez la chaîne à
l'exécution — la position du sprite, formatée dans un tampon avec `snprintf` —
et dessinez-la comme seconde ligne de texte à chaque frame, avec une
vérification au démarrage qui rapporte le nombre d'emplacements de la ligne et
combien d'emplacements portent de l'encre. Prédisez les deux nombres avant de
lancer, puis réconciliez — et répondez à la question que pose le format :
qu'arrive-t-il au nombre d'emplacements quand la position passe de `312` à
`99` ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-051/ex1.md)

### Exercice 2 — Les deux sortes de rien *(predict-the-output)*

Un espace et un octet manquant ne dessinent tous deux rien — mais pas de la
même façon. Avant d'exécuter quoi que ce soit, notez ce que `FontGlyph`
retourne pour `' '` et pour l'octet `200`, et ce que rapportera la carte des
emplacements de `"A B"`. Étendez ensuite l'exécution pour imprimer les deux
réponses et la carte des emplacements, et expliquez pourquoi les deux « riens »
de la table de cette leçon sont les mêmes pixels par des *mécanismes
différents* — et ce que cela signifie pour une police dont la planche n'a
vraiment pas un glyphe dont vous avez besoin.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-051/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 050 — la police bitmap comme asset](lesson-050-font.md) ·
**Suivante :** [Leçon 052 — le format d'asset du tilemap](lesson-052-tilemap.md) ·
**Étiquette de code :** [`lesson-051`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-051)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-051-text.md`,
révision `35b111c`.*

<!-- translation-source: book/lessons/part-2/lesson-051-text.md @ 35b111c -->
