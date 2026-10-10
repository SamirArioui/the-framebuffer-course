# Leçon 013 — octets bruts et formats de pixel

{{#include ../../stability-horizon.md}}

## Prose

Cette leçon démarre `paint`, le troisième programme jetable de la partie 0 :
un peintre BMP qui, d'ici la fin de cette tranche, écrira de vrais fichiers
image à partir d'un tampon de pixels dont chaque octet est écrit par notre
propre code. Aujourd'hui est le fondement : ce qu'*est* un tampon de pixels.
Spoiler : des octets. Rien que des octets.

Python vous donne `bytes`, `bytearray`, et un écosystème de bibliothèques où
« image » est un type que quelqu'un d'autre a défini. C vous donne de la
mémoire. Une image en C est un bloc d'`unsigned char` — le même type dont est
fait un fichier texte — et toute notion de « pixel » est de l'arithmétique que
vous faites sur des décalages dans ce bloc. Il n'y a pas de type image à
importer parce qu'il n'y a pas d'import : si vous voulez du RGB888, vous
décidez que trois octets consécutifs signifient rouge, vert, et bleu, et vous
écrivez le code qui le croit.

Notre format est le **RGB888** : chaque pixel est exactement trois octets,
le rouge d'abord, puis le vert, puis le bleu, chaque canal une valeur de
8 bits de `0` à `255`. C'est le format de pixel utile le plus simple, et il
vaut la peine de le confronter aux deux autres familles que vous rencontrerez
dans du vrai code. Un format **compacté 32 bits** (souvent appelé ARGB8888 ou
XRGB8888) stocke chaque pixel dans un mot de 32 bits : quatre canaux, un mot
machine, un chargement — rapide, mais l'ordre des octets *à l'intérieur* de ce
mot dépend du boutisme de la machine, ce qui est exactement le problème que la
leçon 014 ouvre. Un format **indexé** stocke un octet par pixel plus une table
de palette mappant indice → couleur : compact et bon marché à recolorier, mais
chaque pixel a besoin d'une recherche. RGB888 se place au milieu : trois
octets par pixel, pas de palette, pas de questions d'ordre de mots — au prix
de ne pas être un mot machine.

Ces trois octets par pixel sont disposés **par lignes (row-major)** : toute la
première ligne de l'image se trouve en mémoire avant que la seconde ne
commence, et dans une ligne les pixels vont de gauche à droite. Ainsi le pixel
en `(x, y)` vit au décalage d'octet

```
y * (width * 3) + x * 3
```

Le terme `width * 3` est le **pas** : le nombre d'octets qu'occupe une ligne
entière. Mémorisez cette forme — c'est le morceau d'arithmétique le plus
répété de tout le cours. Deux pièges y vivent. D'abord, oublier le `* 3` et
indexer `y * width + x * 3` fonctionne pour la ligne 0 et se chevauche
silencieusement à partir de la ligne 1. Ensuite, le pas d'une ligne dans un
tampon *en mémoire* est `width * 3`, mais le pas d'une ligne dans un
*fichier* est arrondi au multiple de 4 octets — une bizarrerie du format BMP
que la leçon 017 paie directement.

`PutPixel` et `GetPixel` sont cette arithmétique plus trois stockages ou trois
chargements : calculer `off`, puis toucher `px[off]`, `px[off + 1]`,
`px[off + 2]`. Notez ce qui n'y est *pas* : aucune vérification de limites.
L'appelant est cru, comme une fonction C croit d'habitude son appelant, et
l'exercice 4 montre ce que coûte cette confiance. (Le paramètre `h` est
accepté mais inutilisé pour l'instant — la ligne `(void)h;` est l'idiome C
standard pour « oui, je sais que le paramètre est inutilisé ; ne m'avertissez
pas », ce que `-Wall -Wextra` ferait autrement.)

Le tampon lui-même est `calloc`'é : `width * height * 3` octets, tous à zéro.
En RGB888 le zéro est le noir, aussi un tampon intact est-il une image noire.
`calloc` renvoie de la mémoire à zéro ou `NULL` et — la discipline de la
leçon 001 — `NULL` est une valeur que vous vérifiez, pas une exception.
`HexDump` imprime ensuite le tampon une ligne par ligne pour que la
disposition par lignes soit visible : les 24 octets de la ligne 0 d'abord,
puis ceux de la ligne 1, et ainsi de suite. Exécutez le programme et la ligne
0 commence

```
row 0: FF 00 00 00 FF 00 00 00 00 00 00 00 ...
```

`FF 00 00` est le pixel rouge en `(0, 0)`, `00 FF 00` le vert en `(1, 0)` —
trois octets chacun, exactement là où l'arithmétique des décalages dit qu'ils
doivent être. La ligne 5 finit par `... 00 00 00 FF` : le pixel bleu en
`(7, 5)`, le dernier pixel de la dernière ligne.

Enfin la commande de construction. Depuis l'intérieur de `sandbox/paint/` :

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

Chaque option ici a déjà fait sa présentation — `-std=c11 -Wall -Wextra`
dans la leçon 001, `-O0 -g` depuis le travail sur gdb de la leçon 002 — aussi
une ligne suffira : c'est la construction canonique de la partie 0, sans
optimisations, informations de débogage, avertissements activés. La leçon 018
change exactement une de ces options et tout le programme change de
comportement.

## Étape de code

Un seul changement pour cette leçon : la totalité de `paint.c`, commitée avec
ce texte. Son état final est étiqueté `lesson-013`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
new file mode 100644
index 0000000..cfbe8bd
--- /dev/null
+++ b/sandbox/paint/paint.c
@@ -0,0 +1,64 @@
+// paint.c — Lesson 013: raw bytes and pixel formats.
+//
+// A pixel buffer is nothing but bytes.  Ours is width x height pixels at
+// 3 bytes each (RGB888), laid out row by row.  PutPixel and GetPixel map
+// (x, y, color) onto byte offsets; HexDump shows the buffer as it really
+// sits in memory.
+#include <stdio.h>
+#include <stdlib.h>
+
+#define W 8
+#define H 6
+
+static unsigned char *pixels;
+
+static void PutPixel(unsigned char *px, int w, int h, int x, int y,
+                     unsigned char r, unsigned char g, unsigned char b)
+{
+    (void)h; /* unused so far — bounds arrive in the exercises below */
+    int off = y * (w * 3) + x * 3;
+    px[off + 0] = r;
+    px[off + 1] = g;
+    px[off + 2] = b;
+}
+
+static void GetPixel(const unsigned char *px, int w, int h, int x, int y,
+                     unsigned char *r, unsigned char *g, unsigned char *b)
+{
+    (void)h;
+    int off = y * (w * 3) + x * 3;
+    *r = px[off + 0];
+    *g = px[off + 1];
+    *b = px[off + 2];
+}
+
+static void HexDump(const unsigned char *px, int w, int h)
+{
+    for (int y = 0; y < h; y++) {
+        printf("row %d:", y);
+        for (int i = 0; i < w * 3; i++)
+            printf(" %02X", px[y * (w * 3) + i]);
+        putchar('\n');
+    }
+}
+
+int main(void)
+{
+    pixels = calloc((size_t)W * H * 3, 1);
+    if (pixels == NULL) {
+        fprintf(stderr, "out of memory\n");
+        return 1;
+    }
+
+    PutPixel(pixels, W, H, 0, 0, 255, 0, 0); /* red, top-left */
+    PutPixel(pixels, W, H, 1, 0, 0, 255, 0); /* green, beside it */
+    PutPixel(pixels, W, H, 7, 5, 0, 0, 255); /* blue, bottom-right */
+
+    unsigned char r, g, b;
+    GetPixel(pixels, W, H, 1, 0, &r, &g, &b);
+    printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);
+
+    HexDump(pixels, W, H);
+    free(pixels);
+    return 0;
+}
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Trois pixels, à la main *(predict-the-output)*

Ajoutez un appel de plus à la scène dans `main` :

```c
PutPixel(pixels, W, H, 2, 0, 0x11, 0x22, 0x33);
```

Avant de compiler ou d'exécuter quoi que ce soit, écrivez les douze premiers
octets exacts de `row 0` tels que `HexDump` les imprimera — chaque octet, dans
l'ordre. Puis construisez, exécutez, et accordez la ligne imprimée avec votre
prédiction. Si elles ne sont pas d'accord, dites précisément quel octet vous a
surpris et d'où vient sa valeur.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-013/ex1.md)

### Exercice 2 — Le même pixel en 32 bits compacté *(extend-the-code)*

Ajoutez un second tampon à `main` — un tableau d'`unsigned int` de `W * H`
mots, qui est le format compacté 32 bits — et stockez-y le pixel vert
`(1, 0)` comme mot ARGB `0x0000FF00u`. Puis imprimez, côte à côte, les quatre
octets en mémoire de ce mot (via une vue `unsigned char *` du mot) et les
trois octets du tampon RGB888 pour le même pixel. Combien d'octets chaque
format dépense-t-il pour le même pixel vert, et laquelle des deux réponses
changerait sur une machine avec l'ordre des octets opposé ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-013/ex2.md)

### Exercice 3 — Les décalages, à la main *(explain-in-prose)*

Calculez les décalages d'octets qu'utilise `PutPixel` pour les pixels `(0, 0)`,
`(1, 0)`, `(7, 5)`, et `(3, 2)` dans le tampon de ce programme — faites-le
avec la formule, pas avec le programme. Ajoutez ensuite un
`fprintf(stderr, ...)` à `PutPixel` imprimant `x`, `y`, et le décalage qu'il a
calculé, exécutez, et vérifiez-vous. Enfin, expliquez avec vos propres mots ce
que le bug classique du pas oublié — indexer avec `y * w + x * 3` au lieu de
`y * (w * 3) + x * 3` — fait à l'image, et pourquoi la ligne 0 peut avoir
l'air correcte pendant que tout ce qui est en dessous est de la camelote.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-013/ex3.md)

### Exercice 4 — Le pixel qui n'est pas là *(fix-the-crash)*

`PutPixel` croit son appelant. Prouvez qu'il ne devrait pas : ajoutez
`PutPixel(pixels, W, H, 0, 6, 1, 2, 3);` à la fin de la scène et exécutez le
programme. Rien ne semble se produire — l'écriture atterrit juste après le
tampon et le dump a l'air normal. Recompilez maintenant avec une option de
plus :

```
gcc -std=c11 -O0 -g -Wall -Wextra -fsanitize=address paint.c -o paint
```

`-fsanitize=address` active AddressSanitizer, une exécution intégrée au
compilateur qui rembourre chaque allocation de tas de zones de garde
empoisonnées et fait avorter le programme avec une trace de pile dès qu'une
lecture ou une écriture en touche une. Lancez-le et regardez-le attraper
l'écriture égarée. Puis corrigez le vrai problème : faites refuser à
`PutPixel` les coordonnées hors de `w × h` avec un message sur `stderr` au
lieu d'écrire, et vérifiez que la même exécution est désormais propre sous
AddressSanitizer.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-013/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 012 — constructions multi-fichiers : unités de traduction et édition de liens](lesson-012-multi-file.md) ·
**Suivante :** [Leçon 014 — boutisme et disposition des en-têtes d'image](lesson-014-image-headers.md) ·
**Étiquette de code :** [`lesson-013`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-013)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-013-raw-bytes.md`,
révision `66eaafc`.*

<!-- translation-source: book/lessons/part-0/lesson-013-raw-bytes.md @ 66eaafc -->
