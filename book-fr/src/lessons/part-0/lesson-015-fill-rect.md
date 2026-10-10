# Leçon 015 — remplir un rectangle dans un tampon mémoire

{{#include ../../stability-horizon.md}}

## Prose

La leçon 013 nous a donné des pixels ; cette leçon nous donne le *dessin*. Un
rectangle rempli est la forme la plus simple qui vaille la peine d'exister, et
la partie intéressante n'est pas le remplissage — c'est ce qui arrive quand le
rectangle dépasse le bord de l'image. Ce qui, dans un jeu, est toujours le cas :
les sprites traversent les bords, les rectangles de caméra glissent hors de la
zone d'affichage, les nombres de dégâts apparaissent hors écran. Un moteur de
rendu qui ne gère que les rectangles dans les limites n'est pas un moteur de
rendu.

`FillRect` prend un rectangle en `(x, y, rw, rh)` — le coin supérieur gauche
et la taille — et une couleur. Notez que `x` et `y` sont signés : un rectangle
commençant en `(-3, 1)` est légal et veut dire « une forme dont les trois
colonnes de gauche sont au-delà du bord gauche ». La façon naïve de gérer cela
est de vérifier les limites de chaque pixel dans la boucle d'écriture : pour
chaque `(i, j)`, demander si `(x + i, y + j)` est dans le tampon avant d'appeler
`PutPixel`. Cela fonctionne. C'est aussi la mauvaise forme pour un moteur de
rendu : la vérification tourne une fois par pixel au lieu d'une fois par
rectangle, et — pire — elle recouvre la géométrie au lieu de la calculer.

La façon dont ce cours dessine est **plier avant d'écrire** : découper le
rectangle sur sa partie visible *une seule fois*, d'avance, puis écrire des
pixels déjà connus comme étant à l'intérieur. Regardez ce que font les quatre
plis de `FillRect`. Si `x < 0`, les colonnes à gauche du tampon sont perdues :
il y en a `-x`, aussi `rw += x` rétrécit la largeur d'exactement la partie
perdue et `x = 0` glisse l'origine vers le bord. Si `x + rw > w`, le rectangle
déborde du bord droit et la largeur survivante est `w - x`. Les deux plis
verticaux sont la même arithmétique sur `y` et `rh`. Ce qui reste, `(x, y, rw, rh)`
dans des variables locales portant les mêmes noms, est la partie visible — et
la boucle imbriquée écrit alors sans aucune vérification.

Deux détails portent un vrai poids. D'abord, **les deux plis par axe sont
porteurs**. Supprimez le pli `x < 0` et le rectangle qui pend à gauche calcule
des décalages comme `1 * (8 * 3) + (-3) * 3 = 15` — une écriture qui file dans
les pixels de la *ligne précédente* : dans le dump de cette scène, la ligne 0
gagne un pixel orange à la colonne 5 que personne n'a demandé, et chaque ligne
dessinée saigne vers le haut dans sa voisine. Poussez le même rectangle à
`y = 0` et les décalages deviennent négatifs — des octets *avant* le tampon,
qu'AddressSanitizer rapporte comme un heap-buffer-overflow. Le pli n'est pas
une des deux options ; c'est ce qui rend l'arithmétique de la boucle d'écriture
honnête. Ensuite, le **cas vide** : après pliage, un rectangle entièrement
extérieur a `rw <= 0` ou `rh <= 0` — ce n'est pas une erreur, c'est le
résultat normal du découpage, et le retour anticipé est ce qui fait de « le
sprite a quitté l'écran » un non-événement.

La scène de la leçon fait pendre quatre rectangles hors des bards exprès : un
hors du bord gauche, un hors du coin supérieur droit, un hors du coin inférieur
droit, et un entièrement à l'intérieur. Le dump montre le résultat — la ligne 1
commence

```
row 1: FF 80 00 FF 80 00 FF 80 00 00 00 00 ...
```

trois pixels orange au bord gauche : du rectangle qui commençait à `x = -3`,
les trois premières colonnes ont été pliées et le reste a atterri à `x = 0`.
Les trois derniers octets de la ligne 5 lisent `80 00 FF` — le rectangle
inférieur droit a écrasé le pixel bleu que la leçon 013 avait planté en
`(7, 5)`. La dernière écriture gagne ; personne n'a prévenu le pixel bleu.

Une note de bas de page honnête sur l'arithmétique : les tests de fin
`x + rw > w` peuvent *déborder* si un appelant passe un `x` absurde proche de
`INT_MAX` — l'addition retombe, le test ment, et `PutPixel` écrit
dieu-sait-où. L'exercice 3 fait planter cela à la demande et corrige le test
pour qu'il ne puisse pas additionner son chemin vers un comportement indéfini.
Retenez celle-là ; la leçon 018 est une leçon entière sur exactement cette
classe d'arithmétique.

Ce motif — plier la demande sur la région valide avant de toucher la mémoire,
puis faire tourner une boucle sans vérifications — est le cœur du moteur de
rendu de la partie 2. Il reviendra pour les lignes, pour les sprites, et pour
la copie du framebuffer. Apprenez-le ici pendant que la forme est petite.

Commande de construction, inchangée :

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Étape de code

Un seul changement pour cette leçon : `paint.c` gagne `FillRect` avec son pli
de découpage, et `main` fait pendre quatre rectangles hors des bords. Commité
avec ce texte ; son état final est étiqueté `lesson-015`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index 2c95a68..e42ce65 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,9 +1,9 @@
-// paint.c — Lesson 014: endianness and image-header layout.
+// paint.c — Lesson 015: fill-rect onto a memory buffer.
 //
-// A pixel buffer is bytes (lesson 013); an image FILE is bytes too, with a
-// header whose field order and byte order the format pins down.  PutU16LE
-// and PutU32LE write integers byte by byte in little-endian order, and
-// BuildBmpHeader lays out the 54-byte BMP header field by field.
+// A pixel buffer is bytes (lesson 013) and a file header is pinned bytes
+// (lesson 014).  Now we draw: FillRect fills a rectangle of the buffer,
+// clipping it to the buffer first — fold the rectangle to the visible
+// region BEFORE writing, never pixel by pixel.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -78,6 +78,24 @@ static unsigned int GetU32LE(const unsigned char *src)
          | ((unsigned int)src[3] << 24);
 }
 
+// FillRect — fill a rectangle, clipping it to the buffer first.
+// The clip is a fold: shrink (x, y, rw, rh) to the visible part once,
+// up front, and then write only pixels that are known to be inside.
+static void FillRect(unsigned char *px, int w, int h,
+                     int x, int y, int rw, int rh,
+                     unsigned char r, unsigned char g, unsigned char b)
+{
+    if (x < 0) { rw += x; x = 0; }
+    if (y < 0) { rh += y; y = 0; }
+    if (x + rw > w) rw = w - x;
+    if (y + rh > h) rh = h - y;
+    if (rw <= 0 || rh <= 0) return;
+
+    for (int j = 0; j < rh; j++)
+        for (int i = 0; i < rw; i++)
+            PutPixel(px, w, h, x + i, y + j, r, g, b);
+}
+
 // BuildBmpHeader — lay out the 54-byte BMP header field by field.
 // 14-byte file header:  "BM", file size, reserved, data offset.
 // 40-byte info header: size, width, height, planes, bpp, compression,
@@ -119,6 +137,12 @@ int main(void)
     PutPixel(pixels, W, H, 1, 0, 0, 255, 0); /* green, beside it */
     PutPixel(pixels, W, H, 7, 5, 0, 0, 255); /* blue, bottom-right */
 
+    /* Rectangles that hang off the edges — only the visible part lands. */
+    FillRect(pixels, W, H, -3, 1, 6, 3, 255, 128, 0);  /* off the left */
+    FillRect(pixels, W, H, 6, -2, 4, 4, 0, 128, 255);  /* off the top-right */
+    FillRect(pixels, W, H, 5, 4, 10, 10, 128, 0, 255); /* off the bottom-right */
+    FillRect(pixels, W, H, 2, 2, 3, 2, 255, 255, 255); /* fully inside */
+
     unsigned char r, g, b;
     GetPixel(pixels, W, H, 1, 0, &r, &g, &b);
     printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);
```

## Exercices

Quatre courts exercices, tous sur des cas de découpage. Chacun se termine par
sa solution — un diff contre l'état final de cette leçon, plus une visite
guidée — après l'énoncé.

### Exercice 1 — Le rectangle hors du bord gauche *(predict-the-output)*

Regardez seulement le premier rectangle de la scène, `(-3, 1, 6, 3)` en
orange, et prédisez le découpage avant de faire confiance au dump : (a) que
calcule le pli comme partie visible — exactement quels `x`, `y`, `w`, `h` ? —
et (b) quels sont les neuf premiers octets exacts de `row 1` dans le dump ?
Ajoutez ensuite un `fprintf` à `FillRect` imprimant la partie visible après les
plis, exécutez, et accordez les deux prédictions. Un des rectangles de la scène
*perd* contre un plus tardif dans le dump — dites lequel et pourquoi.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-015/ex1.md)

### Exercice 2 — Compter ce qui a survécu *(extend-the-code)*

Faites renvoyer à `FillRect` le nombre de pixels qu'il a réellement écrits
(`int`, zéro pour les rectangles qui se découpaient à rien), accumulez le
compteur dans `main` sur les quatre appels, et imprimez-le. Avant d'exécuter,
calculez les quatre compteurs à la main depuis les rectangles découpés et
vérifiez-les contre le programme — et faites un appel de plus avec un
rectangle entièrement hors du tampon (disons `(20, 20, 4, 4)`) pour voir le cas
zéro fonctionner.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-015/ex2.md)

### Exercice 3 — L'addition qui a mangé le découpage *(fix-the-crash)*

Ajoutez cet appel à la scène et exécutez le programme :

```c
FillRect(pixels, W, H, 2147483640, 0, 100, 1, 255, 0, 0);
```

Le programme meurt d'une erreur de segmentation. Les pixels sont innocents :
le test de droite du découpage calcule `x + rw`, et `2147483640 + 100` fait
déborder l'`int` — le résultat retombé passe le test, le pli ne bride jamais,
et `PutPixel` écrit loin hors du tampon. Réécrivez les deux tests de fin pour
qu'ils ne puissent pas déborder pour des entrées `int` quelconques, et vérifiez
que le même appel se découpe désormais à rien au lieu de planter.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-015/ex3.md)

### Exercice 4 — Pourquoi plier d'abord *(explain-in-prose)*

L'alternative au pliage est une vérification de limites par pixel dans la
boucle d'écriture — un `PutPixelChecked` qui refuse les coordonnées hors du
tampon. Expliquez avec vos propres mots, pour cette forme de rectangle : (a)
quel travail le pli fait exactement une fois qu'une vérification par pixel
ferait `rw * rh` fois ; (b) pourquoi les plis de départ sont porteurs — avec le
pli `x < 0` supprimé, calculez à la main où atterrissent les trois octets du
premier pixel du rectangle qui pend à gauche (`off = y * (w * 3) + x * 3`),
puis supprimez réellement le pli, recompilez, et comparez le dump à celui
d'avant avant de le restaurer ; et (c) une situation où la vérification par
pixel est réellement le meilleur choix d'ingénierie. Pour ancrer (a) et (b),
ajoutez un `fprintf` à chaque pli de `FillRect` imprimant les valeurs au moment
où le pli se déclenche, exécutez, et citez ce que les rectangles de la scène
font à l'état du découpage.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-015/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 014 — boutisme et disposition des en-têtes d'image](lesson-014-image-headers.md) ·
**Suivante :** [Leçon 016 — tracer des lignes dans le tampon](lesson-016-lines.md) ·
**Étiquette de code :** [`lesson-015`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-015)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-015-fill-rect.md`,
révision `b7d43c4`.*

<!-- translation-source: book/lessons/part-0/lesson-015-fill-rect.md @ b7d43c4 -->
