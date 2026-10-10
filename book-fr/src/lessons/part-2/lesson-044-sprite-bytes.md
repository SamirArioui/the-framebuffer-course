# Leçon 044 — un sprite comme des octets chargés

{{#include ../../stability-horizon.md}}

## Prose

La partie 1 s'est achevée avec chaque pixel de l'écran écrit par du code qui est
le nôtre — et chaque pixel d'une couleur unie. La partie 2 commence là d'où
viendront les pixels : **un sprite, ce sont les octets d'un fichier dans la
mémoire du moteur**. Avant que le moteur de rendu ne copie quoi que ce soit où
que ce soit, les octets doivent arriver — depuis le disque, par la lecture de
fichier entier de la couture (leçon 037), dans l'arena de la leçon 041, sous un
format assez petit pour être défini à la main et lu à l'œil. Aujourd'hui définit
le format et fait entrer les octets. C'est toute la leçon : pas encore de
dessin, et c'est précisément le propos — un moteur de rendu qui fait confiance à
ses assets est un moteur de rendu qui dessine honnêtement des ordures.

### Le format, défini à la main

Le format est **le PPM, variante P6** — celui-là même que la leçon 017 de la
partie 0 écrivait de zéro. Sa définition complète :

```
P6\n
<width> <height>\n
<maxval>\n
<width × height × 3 bytes of pixels>
```

Trois lignes d'ASCII, puis des octets bruts. `P6` dit « des pixels binaires
suivent » ; `<width> <height>` sont des nombres décimaux ; `<maxval>` vaut `255`
ici, ce qui signifie un octet par canal de couleur. Viennent ensuite les trois
octets de chaque pixel — rouge, vert, bleu — dans l'ordre des lignes, ligne du
haut en premier. Pas de compression, pas de palette, pas de longueurs sur
lesquelles se tromper. Voici notre fichier de sprite, `assets/sprite.ppm`, une
image 16×16 avec une couleur clé magenta :

```
$ xxd assets/sprite.ppm | head -2
00000000: 5036 0a31 3620 3136 0a32 3535 0aff 00ff  P6.16 16.255....
00000010: ff00 ffff 00ff ff00 ffff 00ff ff00 ffff  ................
```

Lisez-le en octets, à la manière de la partie 0 : `50 36` c'est `P6` ; `0a` est
un saut de ligne ; `31 36 20 31 36` est le texte `16 16` ; `0a` ; `32 35 35`
est `255` ; `0a` — et voilà tout l'en-tête, **treize octets**. Puis les pixels
commencent : `ff 00 ff` est le pixel `(0,0)`, magenta ; le `ff 00 ff` suivant
est le pixel `(1,0)`, magenta lui aussi (les premières lignes du sprite sont
surtout de la couleur clé). Un pixel fait trois octets, le fichier fait
`13 + 16 × 16 × 3 = 781` octets, et chaque octet après l'en-tête est un pixel
sans autre signification. Cette propriété — chaque octet est ce à quoi il
ressemble — est ce qui rend le format digne d'être défini à la main : vous
pouvez poser un hexdump à côté de l'image et vérifier vous-même.

Deux règles du format font ici un vrai travail :

- **La partie texte est du texte.** Des espaces et des commentaires `#` peuvent
  apparaître entre les champs de l'en-tête — c'est la règle du format lui-même,
  et le lecteur y obéit.
- **Exactement un octet d'espacement sépare l'en-tête des pixels.** Pas « sauter
  les espaces jusqu'aux données » — les octets des pixels sont arbitraires, et
  le premier d'entre eux peut lui-même ressembler à un espace. Un lecteur qui
  saute aveuglément mange des pixels ; un lecteur qui compte exactement un octet
  ne le fait jamais. Quand un format vous tend une règle qui sonne pédante, elle
  se tient en général entre vous et un bug comme celui-ci.

### Chargé par la couture, gardé dans l'arena

Le chargement tient en trois mouvements, tous des leçons déjà payées :

1. **Lire le fichier en entier** — `platform::ReadFile` (leçon 037). Un fichier
   manquant ou illisible est un échec typé, pas un crash et pas un sprite vide.
2. **Analyser l'en-tête à la main** — treize octets de texte vérifiés avec des
   tests de caractères et un lecteur de nombre. Si les octets magiques ne sont
   pas `P6`, si les nombres ne s'analysent pas, si la valeur maximale n'est pas
   `255`, ou si le fichier ne contient pas *exactement* les pixels que l'en-tête
   annonce, le chargement échoue. Pas à moitié : `LoadSprite` rend soit un
   sprite complet, soit le nom de ce qui n'a pas marché.
3. **Copier les pixels dans l'arena** — la mémoire de la leçon 041, allouée avec
   `ArenaAlloc`, puis les octets du fichier repartent vers l'OS avec
   `ReleaseFile`. Ce que le moteur garde, c'est sa copie.

La copie de l'étape 3 est une décision, pas un réflexe. Les octets du fichier
appartiennent au côté OS de la couture ; les octets du moteur appartiennent à
l'arena. Une allocation au démarrage, une libération de l'arena à la fin — le
sprite vit exactement aussi longtemps que le moteur le dit, et la question « qui
possède les octets ? » de la leçon 004 a exactement une réponse pour tout ce qui
vit dans le moteur.

Les échecs typés sont au nombre de trois, et chacun nomme un mensonge différent
qu'un fichier peut raconter :

```c++
enum SpriteError {
    SPRITE_OK = 0,
    SPRITE_MISSING,   /* the file is not there or cannot be read */
    SPRITE_MALFORMED, /* the bytes are not a complete P6 image */
    SPRITE_NO_ROOM,   /* the arena had no room for the pixels */
};
```

### Les octets, inspectés

L'exécution charge le sprite au démarrage, puis le regarde comme la partie 0
regardait tout — octet par octet :

```
$ DISPLAY=:99 ./build/game
engine: sprite assets/sprite.ppm: 16x16, 768 pixel bytes
engine: pixel 0,0 = 255,0,255
engine: pixel 8,8 = 220,40,40
engine: pixel bytes sum to 125580
engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
frame 1: update 0.000 ms, render 0.791 ms, present 0.480 ms, total 1.271 ms
...
engine: 24 frames — avg 1.247 ms (update 0.000, render 0.546, present 0.701)
engine: worst frame 1.947 ms (frame 23); present is 56% of the frame
engine: arena: 1229568 of 4194304 bytes used
engine: close reported
engine: closed
```

Chaque ligne d'inspection prouve quelque chose :

- **`16x16, 768 pixel bytes`** — l'en-tête s'est analysé et le compte tombe
  juste : `16 × 16 × 3 = 768`, et le fichier contenait exactement ce nombre
  d'octets après l'en-tête. Un fichier qui aurait annoncé 32×32 aurait exigé
  3072 octets et aurait été refusé.
- **`pixel 0,0 = 255,0,255`** — les trois premiers octets de pixels, magenta, la
  couleur clé que montre l'hexdump ci-dessus. Les octets que contient le fichier
  sont ceux que le moteur a reçus.
- **`pixel 8,8 = 220,40,40`** — le centre du sprite, un pixel rouge. Un point
  échantillonné comparé à l'image que vous avez dessinée dit que la copie est
  *ordonnée*, pas seulement complète.
- **`pixel bytes sum to 125580`** — chaque octet compté une fois. Changez un
  seul octet n'importe où et ce nombre change : une somme de contrôle du genre
  le moins cher, et l'habitude d'où elle vient (les octets sont des données
  qu'on peut additionner) est celle sur laquelle les plongées s'appuieront.
- **`arena: 1229568 of 4194304 bytes used`** — `1228800` pour le framebuffer
  (leçon 043) plus `768` pour le sprite. L'asset est dans la mémoire du moteur,
  compté comme tout le reste.

La fenêtre montre encore le marqueur de la leçon précédente ; le sprite n'est pas
encore dessiné. Ses octets sont chargés, vérifiés et en attente — ce qui est
l'ordre honnête. La boucle de copie qui les dessinera est la leçon suivante, et
ce sera une boucle que les plongées sur les caches et sur l'assembleur pourront
passer au crible.

### Ce qui est figé désormais

Le contrat de format se referme ici, comme tout autre contrat de ce cours : les
leçons suivantes liront *plus* de ce format (la planche de police (font sheet)
de la leçon 050 passera par le même chargeur P6), jamais ne le réinterpréteront.
Si vous dessinez vos propres sprites, ce sont des P6 avec `255` comme valeur
maximale — le lecteur que vous avez écrit aujourd'hui les chargera, ou vous dira
exactement laquelle de ses règles vous avez enfreinte.

## Étape de code

Un seul changement pour cette leçon : `assets/sprite.ppm` voit le jour (une
image P6 de 16×16, 13 octets d'en-tête + 768 octets de pixels), `src/sprite.h` /
`src/sprite.cpp` apportent le chargeur — en-tête analysé à la main, pixels
copiés dans l'arena, échec typé pour tout le reste — et `main.cpp` charge le
sprite au démarrage et inspecte ses octets. La boucle de frame du marqueur est
intacte. Son état final est étiqueté `lesson-044`.

```diff
diff --git a/assets/sprite.ppm b/assets/sprite.ppm
new file mode 100644
index 0000000..b6f9e58
Binary files /dev/null and b/assets/sprite.ppm differ
diff --git a/src/main.cpp b/src/main.cpp
index 75dfaa6..8febd4a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -11,6 +11,7 @@
 #include "framebuffer.h"
 #include "frame.h"
 #include "platform.h"
+#include "sprite.h"
 
 namespace engine {
 
@@ -54,6 +55,51 @@ int Run(void)
     ArenaInit(arena, 4 * 1024 * 1024);
     Framebuffer *fb = GetFramebuffer(arena);
 
+    /* Lesson 044: the sprite is a file's bytes. It is loaded once, at
+       startup, through the seam's whole-file read into the arena — and
+       then inspected like Part 0 inspected everything: by byte. */
+    const char *sprite_path = "assets/sprite.ppm";
+    SpriteResult loaded = LoadSprite(arena, sprite_path);
+    if (loaded.error != SPRITE_OK) {
+        switch (loaded.error) {
+        case SPRITE_MISSING:
+            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
+                         sprite_path);
+            break;
+        case SPRITE_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: %s: not a complete P6 image\n",
+                         sprite_path);
+            break;
+        default:
+            std::fprintf(stderr, "engine: %s: no room in the arena\n",
+                         sprite_path);
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Sprite &sprite = loaded.sprite;
+    long pixel_bytes = (long)sprite.width * sprite.height * 3;
+    long byte_sum = 0;
+    for (long i = 0; i < pixel_bytes; ++i)
+        byte_sum += sprite.pixels[i];
+
+    std::printf("engine: sprite %s: %dx%d, %ld pixel bytes\n", sprite_path,
+                sprite.width, sprite.height, pixel_bytes);
+    std::printf("engine: pixel 0,0 = %d,%d,%d\n", sprite.pixels[0],
+                sprite.pixels[1], sprite.pixels[2]);
+    std::printf("engine: pixel %d,%d = %d,%d,%d\n", sprite.width / 2,
+                sprite.height / 2,
+                sprite.pixels[(sprite.height / 2 * sprite.width +
+                               sprite.width / 2) * 3 + 0],
+                sprite.pixels[(sprite.height / 2 * sprite.width +
+                               sprite.width / 2) * 3 + 1],
+                sprite.pixels[(sprite.height / 2 * sprite.width +
+                               sprite.width / 2) * 3 + 2]);
+    std::printf("engine: pixel bytes sum to %ld\n", byte_sum);
+
     double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
     double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
     double started = platform::Now();
diff --git a/src/sprite.cpp b/src/sprite.cpp
new file mode 100644
index 0000000..449b4dc
--- /dev/null
+++ b/src/sprite.cpp
@@ -0,0 +1,123 @@
+// sprite.cpp — the PPM (P6) reader: a header parsed by hand, pixels copied.
+//
+// Lesson 044: the format is small enough to read byte by byte, and this
+// file does exactly that. Part 0's habit holds: nothing in a file is
+// assumed to be there until the bytes say so.
+
+#include "sprite.h"
+
+#include "platform.h"
+
+namespace engine {
+namespace {
+
+/* The header is ASCII; the pixels are anything. These three helpers are
+   the whole "parser" — character tests, comment skipping, one number. */
+
+bool IsSpace(unsigned char c)
+{
+    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' ||
+           c == '\f';
+}
+
+bool IsDigit(unsigned char c)
+{
+    return c >= '0' && c <= '9';
+}
+
+/* Skips whitespace and #-to-end-of-line comments between header fields —
+   the PPM format's own rules for its text part. */
+void SkipBlanks(const unsigned char *data, size_t size, size_t &at)
+{
+    for (;;) {
+        while (at < size && IsSpace(data[at]))
+            ++at;
+        if (at < size && data[at] == '#') {
+            while (at < size && data[at] != '\n')
+                ++at;
+        } else {
+            return;
+        }
+    }
+}
+
+/* One decimal field, or false when the bytes do not form one. Sizes above
+   the sanity bound are refused early — a lying header is malformed, not an
+   allocation request. */
+bool ReadNumber(const unsigned char *data, size_t size, size_t &at, long &out)
+{
+    SkipBlanks(data, size, at);
+    if (at >= size || !IsDigit(data[at]))
+        return false;
+    long value = 0;
+    while (at < size && IsDigit(data[at])) {
+        value = value * 10 + (data[at] - '0');
+        if (value > 1000000L)
+            return false;
+        ++at;
+    }
+    out = value;
+    return true;
+}
+
+} /* namespace */
+
+SpriteResult LoadSprite(Arena &arena, const char *path)
+{
+    SpriteResult result = { { 0, 0, 0 }, SPRITE_OK };
+
+    platform::FileData file = platform::ReadFile(path);
+    if (file.error != platform::FILE_OK) {
+        result.error = SPRITE_MISSING;
+        return result;
+    }
+
+    const unsigned char *data = file.data;
+    size_t size = file.size;
+    size_t at = 0;
+    long width = 0, height = 0, maxval = 0;
+
+    bool ok = size >= 2 && data[0] == 'P' && data[1] == '6';
+    at = 2;
+    ok = ok && ReadNumber(data, size, at, width);
+    ok = ok && ReadNumber(data, size, at, height);
+    ok = ok && ReadNumber(data, size, at, maxval);
+    ok = ok && maxval == 255; /* one byte per channel, as lesson 017 wrote */
+    ok = ok && width > 0 && width <= 4096 && height > 0 && height <= 4096;
+
+    /* Exactly one whitespace byte separates the header from the pixels.
+       Not "skip whitespace here" — the pixel bytes are arbitrary, and the
+       first one may itself look like whitespace. This is the line that
+       keeps the parse honest. */
+    ok = ok && at < size && IsSpace(data[at]);
+    ++at;
+
+    /* Complete or nothing: the file must hold exactly the pixels the
+       header claims — no short read presented as a sprite. */
+    size_t pixel_bytes = (size_t)width * (size_t)height * 3;
+    ok = ok && size - at == pixel_bytes;
+
+    if (!ok) {
+        result.error = SPRITE_MALFORMED;
+        platform::ReleaseFile(file);
+        return result;
+    }
+
+    unsigned char *pixels = (unsigned char *)ArenaAlloc(arena, pixel_bytes, 4);
+    if (!pixels) {
+        result.error = SPRITE_NO_ROOM;
+        platform::ReleaseFile(file);
+        return result;
+    }
+    for (size_t i = 0; i < pixel_bytes; ++i)
+        pixels[i] = data[at + i];
+    platform::ReleaseFile(file);
+
+    result.sprite.pixels = pixels;
+    result.sprite.width = (int)width;
+    result.sprite.height = (int)height;
+    result.error = SPRITE_OK;
+    return result;
+}
+
+} /* namespace engine */
diff --git a/src/sprite.h b/src/sprite.h
new file mode 100644
index 0000000..5be7140
--- /dev/null
+++ b/src/sprite.h
@@ -0,0 +1,44 @@
+// sprite.h — a sprite as loaded bytes: a PPM image's pixels in our arena.
+//
+// Lesson 044: an asset is a file, read whole through the seam (lesson 037)
+// and kept in the engine's own memory (lesson 041). The format is PPM (P6)
+// — the one Part 0's lesson 017 wrote by hand: a tiny text header, then
+// every pixel's three bytes in order. No library parses it; we do, and the
+// bytes stay inspectable.
+#ifndef SPRITE_H
+#define SPRITE_H
+
+#include "arena.h"
+
+namespace engine {
+
+/* A sprite: one image's pixels in the engine's memory — row after row,
+   three bytes each (red, green, blue), exactly the file's pixel section. */
+struct Sprite {
+    unsigned char *pixels; /* width * height * 3 bytes */
+    int width;
+    int height;
+};
+
+/* A load either hands over a complete sprite or names what went wrong —
+   never a half-loaded sprite presented as success. */
+enum SpriteError {
+    SPRITE_OK = 0,
+    SPRITE_MISSING,   /* the file is not there or cannot be read */
+    SPRITE_MALFORMED, /* the bytes are not a complete P6 image */
+    SPRITE_NO_ROOM,   /* the arena had no room for the pixels */
+};
+
+struct SpriteResult {
+    Sprite sprite;
+    SpriteError error; /* SPRITE_OK exactly when sprite.pixels is non-0 */
+};
+
+/* Loads a PPM (P6) image from a file. The header is parsed byte by byte,
+   the pixel bytes are copied into the arena, and the file's own bytes go
+   back to the OS — what the engine keeps is its copy. */
+SpriteResult LoadSprite(Arena &arena, const char *path);
+
+} /* namespace engine */
+
+#endif
```

Les octets du sprite sont binaires, et son diff aussi — git affiche
`Binary files … differ` pour lui. L'en-tête et les premiers octets de pixels
sont l'hexdump de la prose ci-dessus ; `git diff --binary` affiche le patch
complet.

## Exercices

Deux extensions « faites-les vôtres ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon plus une visite guidée — après l'énoncé.

### Exercice 1 — L'en-tête qui ment *(predict-the-output)*

Copiez `assets/sprite.ppm` vers un fichier à vous et modifiez *uniquement* son
en-tête pour qu'il annonce `32 32`. Avant de lancer quoi que ce soit, notez ce
que l'exécution rapportera et quelle vérification refusera — les champs de
taille ou le compte de pixels. Faites ensuite nommer par le rapport d'échec
laquelle des vérifications a refusé : un message pour les champs de l'en-tête,
un pour les octets de pixels, et un pour un fichier impossible à lire — et
donnez à l'exécution un moyen de prendre le chemin depuis son premier argument,
pour que vous puissiez braquer le chargeur où vous voulez. Lancez contre votre
copie corrompue et comparez avec votre prédiction. Terminez par la question que
posent les octets : si le chargeur avait fait confiance à l'en-tête, ces 768
pixels auraient été dessinés comme 32 lignes de quoi — et d'où seraient venus
les 2304 octets manquants ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-044/ex1.md)

### Exercice 2 — Sprite, voici la fenêtre *(extend-the-code)*

Les octets du sprite sont dans l'arena ; la fenêtre montre encore le marqueur de
la leçon précédente. Rassemblez les deux avec la boucle que la leçon 045
transforme en fonction : dessinez le sprite dans le framebuffer avec `PutPixel`
à un endroit fixe, à chaque frame, avant le marqueur. Relisez ensuite trois
pixels avec `GetPixel` — le coin supérieur gauche, le centre, et un pixel de la
couleur clé — et imprimez les octets du fichier face à ceux du framebuffer à
chacun de ces points. Que vous dit le pixel de couleur clé sur ce que la leçon
045 vous doit ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-044/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 043 — la démo de clôture : la couche plateforme terminée](../part-1/lesson-043-demo.md) ·
**Suivante :** [Leçon 045 — le blit découpé et transparent](lesson-045-blit.md) ·
**Étiquette de code :** [`lesson-044`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-044)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-044-sprite-bytes.md`,
révision `d843c56`.*

<!-- translation-source: book/lessons/part-2/lesson-044-sprite-bytes.md @ d843c56 -->
