# Leçon 014 — boutisme et disposition des en-têtes d'image

{{#include ../../stability-horizon.md}}

## Prose

La leçon 013 a fait du tampon de pixels rien que des octets. Cette leçon fait
de même pour le *fichier* image : un BMP est des octets aussi, et le format —
pas la machine qui les a écrits — décide ce que veut dire chaque octet. Les
deux idées d'aujourd'hui sont le boutisme, concrètement, et la disposition
d'un en-tête champ par champ.

Commençons par la machine. Un entier 32 bits est quatre octets, mais les
octets n'ont pas d'ordre propre — c'est le CPU qui l'a. Sur une machine
**petit boutiste** l'octet le moins significatif vit à l'adresse la plus basse
; sur une machine **gros boutiste**, le plus significatif. La sonde de `main`
demande à la machine directement : elle stocke `0x01020304` et imprime les
quatre octets à travers une vue `unsigned char *` de la même mémoire. Voici la
vraie sortie sur cette machine :

```
0x01020304 in memory: 04 03 02 01
```

`04` d'abord — l'octet le moins significatif en premier — aussi ceci est-il
une machine petit boutiste. Deux détails méritent une pause. D'abord, le
transtypage d'`unsigned int *` vers `unsigned char *` est l'*unique*
transtypage de reinterpretation que C autorise toujours : un pointeur `char`
peut inspecter les octets de n'importe quel objet. Tout autre transtypage
« de réinterprétation » est un piège que la leçon 018 visitera. Ensuite, la
valeur `0x01020304` est portable ; la *séquence d'octets* `04 03 02 01` est un
fait sur ce CPU. Lancez la même sonde sur une machine gros boutiste et elle
imprime `01 02 03 04`.

Maintenant le problème que cela crée pour les formats de fichier. Si un BMP
stockait sa largeur comme « les quatre octets d'un `int`, dans l'ordre
mémoire », le fichier voudrait dire des choses différentes selon la machine
qui l'a écrit — le même fichier se décoderait comme largeur 8 sur un
ordinateur et largeur 134217728 sur un autre. Aussi tout format de fichier
sérieux **épingle l'ordre des octets**. BMP épingle le petit boutisme pour
chaque champ d'en-tête multi-octets. Nos encodeurs, `PutU16LE` et `PutU32LE`,
écrivent cet ordre à la main : masques et décalages, octet le moins
significatif d'abord, quel que soit ce que l'hôte en pense. `GetU16LE` et
`GetU32LE` le relisent de la même façon — un décodage est juste l'encodage
marché à reculons. Parce que les encodeurs ne manipulent que des octets et des
décalages, ils sont corrects sur *chaque* machine : un BMP écrit sur une
machine gros boutiste est identique octet par octet à un écrit ici.

Les encodeurs en place, `BuildBmpHeader` assemble l'en-tête de 54 octets que
le format prescrit : un en-tête de fichier de 14 octets (« BM », taille du
fichier, zéros réservés, décalage des données) suivi d'un en-tête
d'information de 40 octets (taille, largeur, hauteur, plans, bits par pixel,
compression, taille de l'image, résolution, couleurs). Chaque champ reçoit son
décalage exact — la largeur aux octets 18 à 21, la hauteur aux 22 à 25, et
ainsi de suite. `main` vide les octets résultants puis décode chaque champ en
retour ; sur cette machine le dump lit

```
42 4D C6 00 00 00 00 00 00 00 36 00 00 00 28 00
00 00 08 00 00 00 06 00 00 00 01 00 18 00 00 00
...
```

Lisez-le lentement : `42 4D` est « BM », `C6 00 00 00` est la taille du
fichier 198 en petit boutisme, `36 00 00 00` est le décalage des données 54,
`28 00 00 00` est la taille de l'en-tête d'information 40, puis
`08 00 00 00 06 00 00 00` — largeur 8, hauteur 6, les deux en petit boutisme,
exactement là où le format dit qu'ils doivent être.

Une question reste, et l'exercice 3 vous fait mesurer la réponse : pourquoi
construire cela avec des stockages d'octets au lieu de déclarer un
`struct BmpHeader` et de le `fwrite` ? Version courte : le compilateur C est
libre d'insérer du remplissage entre les champs de structure et de ne rien
réordonner tout en ne garantissant rien — la structure vit par les règles de
disposition du *compilateur*, le fichier vit par celles du *format*. Seul
l'un des deux peut être écrit sur disque en sécurité, et ce n'est pas la
structure.

La commande de construction est inchangée depuis la leçon 013 :

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Étape de code

Un seul changement pour cette leçon : `paint.c` gagne les encodeurs et
décodeurs petit boutiste, `BuildBmpHeader`, un videur d'octets, et la sonde de
boutisme dans `main`. Commité avec ce texte ; son état final est étiqueté
`lesson-014`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index cfbe8bd..2c95a68 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,9 +1,9 @@
-// paint.c — Lesson 013: raw bytes and pixel formats.
+// paint.c — Lesson 014: endianness and image-header layout.
 //
-// A pixel buffer is nothing but bytes.  Ours is width x height pixels at
-// 3 bytes each (RGB888), laid out row by row.  PutPixel and GetPixel map
-// (x, y, color) onto byte offsets; HexDump shows the buffer as it really
-// sits in memory.
+// A pixel buffer is bytes (lesson 013); an image FILE is bytes too, with a
+// header whose field order and byte order the format pins down.  PutU16LE
+// and PutU32LE write integers byte by byte in little-endian order, and
+// BuildBmpHeader lays out the 54-byte BMP header field by field.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -15,7 +15,7 @@ static unsigned char *pixels;
 static void PutPixel(unsigned char *px, int w, int h, int x, int y,
                      unsigned char r, unsigned char g, unsigned char b)
 {
-    (void)h; /* unused so far — bounds arrive in the exercises below */
+    (void)h; /* unused for now — no bounds checks yet */
     int off = y * (w * 3) + x * 3;
     px[off + 0] = r;
     px[off + 1] = g;
@@ -42,6 +42,71 @@ static void HexDump(const unsigned char *px, int w, int h)
     }
 }
 
+static void DumpBytes(const unsigned char *p, int n)
+{
+    for (int i = 0; i < n; i++) {
+        printf("%02X", p[i]);
+        if (i % 16 == 15 || i == n - 1) putchar('\n');
+        else putchar(' ');
+    }
+}
+
+static void PutU16LE(unsigned char *dst, unsigned int v)
+{
+    dst[0] = (unsigned char)(v & 0xFF);
+    dst[1] = (unsigned char)((v >> 8) & 0xFF);
+}
+
+static void PutU32LE(unsigned char *dst, unsigned int v)
+{
+    dst[0] = (unsigned char)(v & 0xFF);
+    dst[1] = (unsigned char)((v >> 8) & 0xFF);
+    dst[2] = (unsigned char)((v >> 16) & 0xFF);
+    dst[3] = (unsigned char)((v >> 24) & 0xFF);
+}
+
+static unsigned int GetU16LE(const unsigned char *src)
+{
+    return (unsigned int)src[0] | ((unsigned int)src[1] << 8);
+}
+
+static unsigned int GetU32LE(const unsigned char *src)
+{
+    return (unsigned int)src[0]
+         | ((unsigned int)src[1] << 8)
+         | ((unsigned int)src[2] << 16)
+         | ((unsigned int)src[3] << 24);
+}
+
+// BuildBmpHeader — lay out the 54-byte BMP header field by field.
+// 14-byte file header:  "BM", file size, reserved, data offset.
+// 40-byte info header: size, width, height, planes, bpp, compression,
+//                      image size, resolution, colors.
+static void BuildBmpHeader(unsigned char *header, int w, int h)
+{
+    unsigned int row_size = (unsigned int)w * 3;
+    unsigned int pad = (4 - row_size % 4) % 4;
+    unsigned int image_size = (row_size + pad) * (unsigned int)h;
+
+    header[0] = 'B';
+    header[1] = 'M';
+    PutU32LE(header + 2, 54 + image_size);      /* file size */
+    PutU32LE(header + 6, 0);                    /* reserved */
+    PutU32LE(header + 10, 54);                  /* data offset */
+
+    PutU32LE(header + 14, 40);                  /* info header size */
+    PutU32LE(header + 18, (unsigned int)w);     /* width */
+    PutU32LE(header + 22, (unsigned int)h);     /* height */
+    PutU16LE(header + 26, 1);                   /* planes */
+    PutU16LE(header + 28, 24);                  /* bits per pixel */
+    PutU32LE(header + 30, 0);                   /* compression: none */
+    PutU32LE(header + 34, image_size);          /* image size */
+    PutU32LE(header + 38, 2835);                /* x pixels per meter */
+    PutU32LE(header + 42, 2835);                /* y pixels per meter */
+    PutU32LE(header + 46, 0);                   /* colors used */
+    PutU32LE(header + 50, 0);                   /* important colors */
+}
+
 int main(void)
 {
     pixels = calloc((size_t)W * H * 3, 1);
@@ -59,6 +124,28 @@ int main(void)
     printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);
 
     HexDump(pixels, W, H);
+
+    /* What does a multi-byte integer look like in memory? */
+    unsigned int probe = 0x01020304;
+    printf("0x01020304 in memory:");
+    for (int i = 0; i < 4; i++)
+        printf(" %02X", ((unsigned char *)&probe)[i]);
+    putchar('\n');
+
+    unsigned char header[54];
+    BuildBmpHeader(header, W, H);
+    printf("BMP header:\n");
+    DumpBytes(header, 54);
+
+    printf("file size   %u\n", GetU32LE(header + 2));
+    printf("data offset %u\n", GetU32LE(header + 10));
+    printf("header size %u\n", GetU32LE(header + 14));
+    printf("width       %u\n", GetU32LE(header + 18));
+    printf("height      %u\n", GetU32LE(header + 22));
+    printf("planes      %u\n", GetU16LE(header + 26));
+    printf("bpp         %u\n", GetU16LE(header + 28));
+    printf("image size  %u\n", GetU32LE(header + 34));
+
     free(pixels);
     return 0;
 }
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Largeur 258 *(predict-the-output)*

Dans `main`, construisez l'en-tête d'une image différente : changez l'appel à
`BuildBmpHeader` en `BuildBmpHeader(header, 258, 4);`. Avant de compiler,
prédisez par écrit deux choses : (a) les huit octets exacts que le dump
montrera aux décalages d'en-tête 18 à 25 — les champs largeur et hauteur — et
(b) les deux lignes que l'impression des champs décodés montrera pour la
largeur et la hauteur. Puis exécutez et accordez. Pourquoi 258 est-il une
meilleure valeur de test ici que 8 ne l'aurait été ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-014/ex1.md)

### Exercice 2 — Dans le mauvais sens *(extend-the-code)*

Ajoutez un encodeur `PutU32BE` (gros boutiste : l'octet le plus significatif
d'abord) à côté de `PutU32LE`, et servez-vous-en pour le champ largeur dans
`BuildBmpHeader` — une ligne changée. Exécutez le programme et regardez ce que
le décodeur rapporte désormais comme largeur. D'où exactement vient ce nombre,
et quelle partie de l'argument de la leçon cette petite expérience
confirme-t-elle ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-014/ex2.md)

### Exercice 3 — Les structures ne font pas de fichiers *(explain-in-prose)*

Au lieu de stockages d'octets, on pourrait déclarer un `struct BmpHeader`
contenant chaque champ dans l'ordre et l'écrire avec un seul `fwrite`. Mesurez
ce que le compilateur fait réellement de cette idée : déclarez une telle
structure (un `unsigned short` pour les champs de deux octets,
`unsigned int`/`int` pour les champs de quatre octets, tout dans l'ordre des
champs BMP) et imprimez le `sizeof` de la structure et l'`offsetof` pour au
moins les champs `type`, `file_size`, `data_offset`, `info_size`, `planes`,
et `bpp`. Il vous faudra `#include <stddef.h>` pour `offsetof`. Comparez vos
nombres aux décalages d'octets qu'utilise `BuildBmpHeader`, puis expliquez
avec vos propres mots pourquoi écrire la structure sur disque produirait un
fichier qu'aucun décodeur BMP n'accepte — et pourquoi `#pragma pack` ou
`__attribute__((packed))` colmate le symptôme plutôt que la maladie. *(Bonus
si vous découvrez, et vérifiez, ce que les deux octets manquants feraient à un
décodeur.)*

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-014/ex3.md)

### Exercice 4 — Interrogez votre propre machine *(port-to-your-own-machine)*

Faites rapporter au programme l'ordre des octets de son hôte à l'exécution —
une ligne, sans macros du compilateur ni symboles prédéfinis : décidez en
inspectant les octets d'un `int` comme le fait la sonde. Lancez-le sur votre
machine et notez ce qu'il dit. Puis découvrez ce que votre machine *croit*
être (`lscpu` imprime une ligne `Byte order:` sur Linux ; `uname -m` est une
piste) et confirmez que les deux sont d'accord. Enfin : quelles lignes du
dump d'en-tête changeraient si vous exécutiez ce programme sur une machine
gros boutiste, et quelles lignes resteraient identiques ? Argumentez depuis
le code, pas depuis la mémoire.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-014/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 013 — octets bruts et formats de pixel](lesson-013-raw-bytes.md) ·
**Suivante :** [Leçon 015 — remplir un rectangle dans un tampon mémoire](lesson-015-fill-rect.md) ·
**Étiquette de code :** [`lesson-014`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-014)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-014-image-headers.md`, révision `993f045`.*

<!-- translation-source: book/lessons/part-0/lesson-014-image-headers.md @ 993f045 -->
