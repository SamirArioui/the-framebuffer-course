# Leçon 017 — écrire un vrai fichier image à la main

{{#include ../../stability-horizon.md}}

## Prose

Tout est en place : des octets pour les pixels (013), des octets pour les
en-têtes (014), le découpage (015), les lignes (016). Cette leçon franchit la
dernière brèche — le tampon devient un fichier que d'autres programmes peuvent
ouvrir. `WriteBmp` écrit l'en-tête de 54 octets que la leçon 014 a construit,
puis les pixels, et le fichier qui en sort est un vrai BMP : la preuve est à
une commande de distance.

`file paint.bmp` lit des octets magiques et des champs d'en-tête, et après
l'exécution de cette leçon il dit

```
paint.bmp: PC bitmap, Windows 3.x format, 8 x 6 x 24, image size 144,
resolution 2835 x 2835 px/m, cbSize 198, bits offset 54
```

Pas « data » — un format nommé avec des dimensions et une taille qui
correspondent au fichier sur disque à l'octet près. Si vous voulez une preuve
plus forte, décodez-le : les dix lignes de Python suivantes (bibliothèque
standard uniquement — ce sont exactement les décodeurs de la leçon 014 dans un
autre langage) déballent l'en-tête et échantillonnent deux pixels :

```python
import struct
d = open('paint.bmp', 'rb').read()
off, = struct.unpack_from('<I', d, 10)
w, h = struct.unpack_from('<ii', d, 18)
row = w * 3 + (-(w * 3)) % 4
def px(x, y):
    b, g, r = d[off + (h - 1 - y) * row + x * 3:][:3]
    return r, g, b
print("dims", w, h, "size", len(d))
print("pixel (1,0):", px(1, 0))
print("pixel (7,5):", px(7, 5))
```

Il imprime `dims 8 6 size 198`, puis le pixel vert en `(1, 0)` comme
`(0, 255, 0)` et l'extrémité jaune de la diagonale en `(7, 5)` comme
`(255, 255, 0)` — notre scène, décodée hors du fichier par du code qui n'a
jamais vu notre tampon. (L'`identify` d'ImageMagick, `feh`, GIMP, ou
n'importe quel visionneur l'ouvrira aussi.)

Maintenant les trois bizarreries que paie `WriteBmp`, une par une — les trois
sont des exigences du format BMP, pas des choix que nous avons faits.

**Les lignes sont bas en haut.** BMP stocke la *dernière* ligne d'image en
premier : la boucle de lignes commence à `y = h - 1` et descend jusqu'à 0.
Personne n'a conçu cela ; c'est tombé du système de coordonnées bas en haut du
premier Windows, et chaque lecteur BMP depuis s'y attend. Écrivez les lignes
haut en bas sans aussi négativer le champ hauteur et chaque visionneur montre
l'image retournée — l'exercice 3 vous fait regarder cela se produire.

**Les lignes sont remplies jusqu'à 4 octets.** Chaque ligne est complétée avec
des octets zéro jusqu'à ce que sa longueur soit un multiple de 4 — pour notre
image de largeur 8, `8 * 3 = 24`, ce qui l'est déjà, donc `pad = 0` ; à la
largeur 5 la ligne fait 15 octets et grandit à 18. Le remplissage est pourquoi
`BuildBmpHeader` calcule `image_size` depuis la taille de ligne complétée et
pourquoi le total du fichier est `54 + (row_size + pad) * h`. Sautez les
écritures de remplissage mais gardez l'arithmétique des tailles et les lignes
glissent hors alignement après la première.

**Les canaux sont bleu-vert-rouge.** Les triplets de pixels BMP sont BGR, le
miroir de notre tampon RGB888. L'émission par pixel dans `WriteBmp` échange
`p[2]` et `p[0]` à la sortie. Oublier l'échange produit un fichier parfaitement
valide et subtilement faux — chaque canal rouge et bleu échangent leurs places.

Ensuite il y a `ClearBuffer`, qui existe pour donner un fond à la scène et pour
être *le code délibérément mauvais de cette tranche*. Il remplit le tampon octet
par octet avec une boucle dont la condition d'arrêt est le décalage qui devient
négatif — un arrêt qu'un compteur qui retombe peut seul fournir. **Ceci est
écrit naïvement exprès ; la leçon 018 découvre ce que l'optimiseur en fait.**
À `-O0` la machine fait retomber le compteur, la boucle se termine, et la
première ligne de sortie du programme est `clear ended at offset -2147483648`
— un nombre qui devrait vous rendre méfiant à vue. Le fichier sort toujours
correct à `-O0` ; la correction sous optimisation est une question distincte,
et c'est tout le sujet de la leçon 018.

L'écrivain de sortie lui-même est naïf de la façon ordinaire : un `putc` par
octet. C'est le code clair d'abord et le code rapide plus tard — l'exercice 4
mesure quelle est réellement la différence sur cette machine.

Commande de construction, inchangée :

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Étape de code

Un seul changement pour cette leçon : `paint.c` gagne `ClearBuffer` (le
remplissage délibérément naïf) et `WriteBmp`, et `main` vide le tampon, dessine
la scène, et écrit `paint.bmp`. Commité avec ce texte ; son état final est
étiqueté `lesson-017`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index 4bc0fdc..3aeb7bd 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,9 +1,10 @@
-// paint.c — Lesson 016: drawing lines onto the buffer.
+// paint.c — Lesson 017: writing a real image file by hand.
 //
 // A pixel buffer is bytes (lesson 013), a file header is pinned bytes
-// (lesson 014), and rectangles fold-clip before they write (lesson 015).
-// Now lines: DrawLine rasterizes with Bresenham's integer error term and
-// clips the segment to the buffer before stepping a single pixel.
+// (lesson 014), rectangles fold-clip (lesson 015), lines rasterize
+// (lesson 016).  Now the buffer becomes a real file: WriteBmp emits the
+// 54-byte header plus bottom-up, padded rows — and a small scene lands
+// in paint.bmp.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -151,6 +152,21 @@ static void DrawLine(unsigned char *px, int w, int h,
     }
 }
 
+// ClearBuffer — fill the buffer with one byte value.  This fill is
+// written naively on purpose: its stop condition is the offset turning
+// negative, a stop that only a wrapping counter can deliver.
+// Lesson 018 finds out what the optimizer does with it.
+static void ClearBuffer(unsigned char *px, int nbytes, unsigned char v)
+{
+    int i = 0;
+    while (i >= 0) {          /* keep going while the offset is positive */
+        if (i < nbytes)       /* clip: never write past the buffer */
+            px[i] = v;
+        i++;
+    }
+    printf("clear ended at offset %d\n", i);
+}
+
 // BuildBmpHeader — lay out the 54-byte BMP header field by field.
 // 14-byte file header:  "BM", file size, reserved, data offset.
 // 40-byte info header: size, width, height, planes, bpp, compression,
@@ -180,6 +196,42 @@ static void BuildBmpHeader(unsigned char *header, int w, int h)
     PutU32LE(header + 50, 0);                   /* important colors */
 }
 
+// WriteBmp — write the pixel buffer to `path` as a 24-bit BMP: the
+// header from lesson 014, then the rows bottom-up (BMP stores the last
+// image row first), each row padded to a 4-byte boundary and with the
+// channels in BMP's blue-green-red order.
+static int WriteBmp(const char *path, const unsigned char *px, int w, int h)
+{
+    unsigned char header[54];
+    BuildBmpHeader(header, w, h);
+
+    unsigned int row_size = (unsigned int)w * 3;
+    unsigned int pad = (4 - row_size % 4) % 4;
+
+    FILE *f = fopen(path, "wb");
+    if (f == NULL) {
+        fprintf(stderr, "cannot write %s\n", path);
+        return -1;
+    }
+
+    for (int i = 0; i < 54; i++)
+        putc(header[i], f);
+
+    for (int y = h - 1; y >= 0; y--) {
+        const unsigned char *row = px + (size_t)y * row_size;
+        for (int x = 0; x < w; x++) {
+            const unsigned char *p = row + x * 3;
+            putc(p[2], f); /* blue first */
+            putc(p[1], f); /* then green */
+            putc(p[0], f); /* then red */
+        }
+        for (unsigned int k = 0; k < pad; k++)
+            putc(0, f);
+    }
+    fclose(f);
+    return 0;
+}
+
 int main(void)
 {
     pixels = calloc((size_t)W * H * 3, 1);
@@ -187,6 +239,7 @@ int main(void)
         fprintf(stderr, "out of memory\n");
         return 1;
     }
+    ClearBuffer(pixels, W * H * 3, 0x20); /* dark gray background */
 
     PutPixel(pixels, W, H, 0, 0, 255, 0, 0); /* red, top-left */
     PutPixel(pixels, W, H, 1, 0, 0, 255, 0); /* green, beside it */
@@ -230,6 +283,9 @@ int main(void)
     printf("bpp         %u\n", GetU16LE(header + 28));
     printf("image size  %u\n", GetU32LE(header + 34));
 
+    if (WriteBmp("paint.bmp", pixels, W, H) == 0)
+        printf("wrote paint.bmp (%dx%d, 24 bpp)\n", W, H);
+
     free(pixels);
     return 0;
 }
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Les lignes sur disque *(predict-the-output)*

Le fichier fait 198 octets ; le dump vous l'avez déjà. Prédisez la disposition
avant de toucher au code : (a) le décalage dans le fichier où commence chacune
des six lignes d'image, dans l'ordre où elles sont écrites, et (b) quel pixel
d'image atterrit au décalage 54 du fichier, et lequel au décalage 75 — avec les
trois octets exacts à chaque fois. Ajoutez ensuite un `fprintf` à la boucle de
lignes de `WriteBmp` imprimant le décalage de départ de chaque ligne
(`ftell`), exécutez, et accordez l'ordre des lignes et les décalages avec
votre prédiction.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-017/ex1.md)

### Exercice 2 — Aller-retour *(extend-the-code)*

Ajoutez un `CheckBmp` qui rouvre `paint.bmp` après l'écriture et le vérifie
contre le tampon qui l'a produit : décodez les champs taille, largeur, hauteur
et bpp avec `GetU32LE`/`GetU16LE`, puis positionnez-vous au décalage dans le
fichier du pixel d'image `(w - 1, h - 1)` — le dernier pixel de la première
ligne sur disque — lisez ses trois octets, défaites leur échange, et comparez
avec `GetPixel` pour la même coordonnée. Imprimez les deux côtés. Tout doit
correspondre ; faites un désaccord délibéré (comparez avec `(w - 1, 0)` à la
place) et regardez quelle vérification s'en aperçoit.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-017/ex2.md)

### Exercice 3 — L'image retournée *(explain-in-prose)*

L'ordre bas en haut des lignes BMP est une règle de format, pas une
préférence. Prouvez-le : changez la boucle de lignes pour marcher haut en bas
(`y = 0` jusqu'à `h - 1`) sans toucher à rien d'autre, recompilez, et décodez
le fichier avec l'échantillonneur Python de la prose (il suppose la règle
standard bas en haut). Dites ce que vous voyez et pourquoi l'*en-tête* ment
désormais. Puis expliquez avec vos propres mots : que faudrait-il qu'un
écrivain fasse pour produire légitimement un BMP haut en bas (renseignez-vous
sur le signe du champ hauteur), et pourquoi pensez-vous que le format porte
cette bizarrerie ? Restaurez la boucle ensuite.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-017/ex3.md)

### Exercice 4 — Par octet contre par ligne *(measure-the-performance)*

L'écrivain émet un `putc` par octet. Construisez l'alternative :
`WriteBmpFast` assemble chaque ligne de sortie (échange BGR et remplissage
inclus) dans un tampon de ligne et l'écrit avec un seul `fwrite`. Puis mettez
en concurrence les deux écrivains sur un tampon de dégradé de 1024×1024
(quelques répétitions chacun, `clock()` autour des boucles) et rapportez les
millisecondes par écriture — et vérifiez avec `cmp` que les deux fichiers sont
identiques octet par octet. Quel est l'écart sur votre machine, et sur quoi
chaque boucle passe-t-elle réellement son temps ?

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-017/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 016 — tracer des lignes dans le tampon](lesson-016-lines.md) ·
**Suivante :** [Leçon 018 — l'optimiseur et le comportement indéfini](lesson-018-optimizer-ub.md) ·
**Étiquette de code :** [`lesson-017`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-017)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-017-image-file.md`, révision `c6657c1`.*

<!-- translation-source: book/lessons/part-0/lesson-017-image-file.md @ c6657c1 -->
