# Leçon 016 — tracer des lignes dans le tampon

{{#include ../../stability-horizon.md}}

## Prose

Un rectangle est facile : deux boucles imbriquées. Une ligne est la première
forme qui exige un *algorithme*, et l'algorithme est assez vieux pour être
célèbre : celui de Bresenham, de 1962, quand « ordinateur » voulait dire une
machine où le virgule flottante était assez coûteuse pour que l'éviter soit un
objectif de conception.

Commençons par ce que les gens écrivent d'abord. La ligne naïve calcule une
pente une fois — `slope = (y1 - y0) / (x1 - x0)` — puis, pour chaque `x` de
`x0` à `x1`, calcule `y = y0 + slope * (x - x0)` en virgule flottante, tronque
en pixel entier, et trace. Cela se lit comme les maths dont ça vient. C'est
aussi instable de trois façons distinctes, que toutes vous pouvez voir sans
croire sur parole. D'abord, la troncation choisit des *pixels différents* de
la vraie ligne : pour la diagonale `(0, 0)` vers `(7, 5)` la boucle flottante
atterrit sur `(1, 0)`, `(4, 2)`, `(5, 3)` là où la ligne exacte passe par
`(1, 1)`, `(4, 3)`, `(5, 4)` — six des pixels que les deux rastériseurs
touchent ne sont pas d'accord. Ensuite, elle est *asymétrique* : faites courir
la même ligne à reculons et la troncation arrondit dans l'autre sens — la
ligne flottante allume un ensemble de pixels différent en descendant que en
montant. Troisièmement, une ligne verticale a `x0 == x1` et la pente divise
par zéro, aussi la boucle naïve a-t-elle besoin d'un cas spécial juste pour
survivre. Et le folklore selon lequel elle est plus lente : elle l'était,
décisivement — une multiplication flottante et une conversion int par pixel
sur des machines qui n'avaient ni l'un ni l'autre à bon marché. Mesurez-le
vous-même dans l'exercice 3 ; sur un CPU moderne l'écart s'est largement
refermé, et les pixels sont le vrai coût.

La réponse de Bresenham est de ne pas calculer `y` du tout. L'algorithme suit
un **terme d'erreur** `err` — la distance mise à l'échelle de la ligne idéale
au centre du pixel courant — et chaque pas pose une question : le pixel
suivant vers la droite est-il plus proche de la ligne, ou celui au-dessus ?
Si l'erreur dit « en haut », `y` avance ; dans les deux cas `x` avance et
l'erreur est ajustée par simple addition entière. Deux additions, deux
comparaisons, zéro multiplication, zéro division, et les choix de pixels sont
*exactement* l'approximation au pixel le plus proche de la ligne — les mêmes
pixels dans les deux sens. `err` est exact parce que tout ce qu'il suit est
une différence entière d'entiers : il n'y a pas d'arrondi qui dérive, à
n'importe quelle taille de coordonnée. C'est ce que calcule Bresenham : pas la
ligne, mais les *pixels les plus proches par lesquels la ligne passe*, décidés
un pas entier à la fois.

Le découpage est la même discipline plier-avant-d'écrire que la leçon 015,
appliquée à un segment. `ClipLine` est le classique Cohen-Sutherland : chaque
extrémité reçoit un code de sortie (out-code) disant de quel(s) côté(s) du
tampon elle est extérieure (`OutCode` en construit un à partir de quatre
tests). Si les deux extrémités partagent un bord extérieur, le segment manque
le tampon entièrement ; si les deux codes sont zéro, il est entièrement à
l'intérieur ; sinon l'extrémité extérieure est déplacée à son intersection
avec la frontière du tampon et les codes sont recalculés. La boucle converge
après au plus quelques tours, et seulement alors `DrawLine` commence à tracer
— chaque pixel est connu comme étant dans les limites à l'avance, aussi la
boucle interne, comme celle de `FillRect`, écrit sans vérifier. Les divisions
du découpage tournent une ou deux fois par ligne ; la boucle par pixel est
entièrement entière. Un détail honnête : les intersections sont calculées en
`double` et tronquées vers zéro quand converties en `int`, ce qui explique que
la ligne cyan découpée de la scène atterrisse là où le dump la montre et non
un pixel plus haut.

La scène trace trois lignes : une diagonale jaune à travers tout le tampon, une
cyan depuis hors écran en haut à gauche vers hors écran à droite, et une
verticale magenta par le milieu qui pend des deux bords et se découpe en
`(4, 0)`–`(4, 5)`. La colonne de la ligne magenta tranche à travers tout ce que
les rectangles ont dessiné — la dernière écriture gagne, comme toujours.

Commande de construction, inchangée :

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Étape de code

Un seul changement pour cette leçon : `paint.c` gagne `OutCode`, `ClipLine`
(Cohen-Sutherland), et `DrawLine` (Bresenham), et la scène trace trois lignes.
Commité avec ce texte ; son état final est étiqueté `lesson-016`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index e42ce65..4bc0fdc 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,9 +1,9 @@
-// paint.c — Lesson 015: fill-rect onto a memory buffer.
+// paint.c — Lesson 016: drawing lines onto the buffer.
 //
-// A pixel buffer is bytes (lesson 013) and a file header is pinned bytes
-// (lesson 014).  Now we draw: FillRect fills a rectangle of the buffer,
-// clipping it to the buffer first — fold the rectangle to the visible
-// region BEFORE writing, never pixel by pixel.
+// A pixel buffer is bytes (lesson 013), a file header is pinned bytes
+// (lesson 014), and rectangles fold-clip before they write (lesson 015).
+// Now lines: DrawLine rasterizes with Bresenham's integer error term and
+// clips the segment to the buffer before stepping a single pixel.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -96,6 +96,61 @@ static void FillRect(unsigned char *px, int w, int h,
             PutPixel(px, w, h, x + i, y + j, r, g, b);
 }
 
+// OutCode — which side(s) of the buffer a point is outside of.
+static int OutCode(int x, int y, int w, int h)
+{
+    int code = 0;
+    if (x < 0) code |= 1;
+    else if (x >= w) code |= 2;
+    if (y < 0) code |= 4;
+    else if (y >= h) code |= 8;
+    return code;
+}
+
+// ClipLine — Cohen-Sutherland clipping: shrink the segment to the part
+// inside the buffer, in place.  Returns 0 if it misses the buffer.
+// The one division per intersection happens per segment end, never per
+// pixel; the rasterizer afterwards is pure integers.
+static int ClipLine(int *x0, int *y0, int *x1, int *y1, int w, int h)
+{
+    int c0 = OutCode(*x0, *y0, w, h), c1 = OutCode(*x1, *y1, w, h);
+    for (;;) {
+        if (!(c0 | c1)) return 1;   /* both ends inside */
+        if (c0 & c1) return 0;      /* both outside the same edge */
+        int c = c0 ? c0 : c1;
+        int x = 0, y = 0;
+        double dx = (double)(*x1 - *x0), dy = (double)(*y1 - *y0);
+        if (c & 8)      { x = *x0 + (int)(dx * (h - 1 - *y0) / dy); y = h - 1; }
+        else if (c & 4) { x = *x0 + (int)(dx * (0 - *y0) / dy);     y = 0; }
+        else if (c & 2) { y = *y0 + (int)(dy * (w - 1 - *x0) / dx); x = w - 1; }
+        else            { y = *y0 + (int)(dy * (0 - *x0) / dx);     x = 0; }
+        if (c == c0) { *x0 = x; *y0 = y; c0 = OutCode(x, y, w, h); }
+        else         { *x1 = x; *y1 = y; c1 = OutCode(x, y, w, h); }
+    }
+}
+
+// DrawLine — Bresenham's line.  After clipping, step from (x0, y0) to
+// (x1, y1) one pixel at a time; `err` tracks the doubled distance from
+// the ideal line, so the pixel choice is exact and entirely integer.
+static void DrawLine(unsigned char *px, int w, int h,
+                     int x0, int y0, int x1, int y1,
+                     unsigned char r, unsigned char g, unsigned char b)
+{
+    if (!ClipLine(&x0, &y0, &x1, &y1, w, h))
+        return;
+
+    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
+    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
+    int err = dx + dy;
+    for (;;) {
+        PutPixel(px, w, h, x0, y0, r, g, b);
+        if (x0 == x1 && y0 == y1) break;
+        int e2 = 2 * err;
+        if (e2 >= dy) { err += dy; x0 += sx; }
+        if (e2 <= dx) { err += dx; y0 += sy; }
+    }
+}
+
 // BuildBmpHeader — lay out the 54-byte BMP header field by field.
 // 14-byte file header:  "BM", file size, reserved, data offset.
 // 40-byte info header: size, width, height, planes, bpp, compression,
@@ -143,6 +198,11 @@ int main(void)
     FillRect(pixels, W, H, 5, 4, 10, 10, 128, 0, 255); /* off the bottom-right */
     FillRect(pixels, W, H, 2, 2, 3, 2, 255, 255, 255); /* fully inside */
 
+    /* Lines: a diagonal, one drawn from off-screen, one straight through. */
+    DrawLine(pixels, W, H, 0, 0, 7, 5, 255, 255, 0);   /* yellow diagonal */
+    DrawLine(pixels, W, H, -5, -3, 12, 2, 0, 255, 255); /* cyan, clipped */
+    DrawLine(pixels, W, H, 4, -2, 4, 9, 255, 0, 255);   /* magenta vertical */
+
     unsigned char r, g, b;
     GetPixel(pixels, W, H, 1, 0, &r, &g, &b);
     printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La ligne verticale à travers tout *(predict-the-output)*

Regardez seulement la ligne magenta, `(4, -2)` à `(4, 9)`. Prédisez, avant
d'exécuter quoi que ce soit : (a) les extrémités exactes que `ClipLine`
calcule pour elle, et (b) les trois octets exacts au décalage 12 de chacune
des six lignes du dump après, y compris les lignes où la ligne écrase quelque
chose que les rectangles ont dessiné — nommez ces pixels volés. Ajoutez ensuite
un `fprintf` à `DrawLine` imprimant les extrémités découpées, exécutez, et
accordez.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-016/ex1.md)

### Exercice 2 — Des rectangles en contour *(extend-the-code)*

Ajoutez `DrawRect`, l'équivalent en contour de `FillRect`, construit à partir
d'exactly quatre appels à `DrawLine` (bords haut, bas, gauche, droit), et
dessinez un contour qui pend du bord gauche du tampon — `(−2, 1, 6, 4)` en
forme `(x, y, w, h)` est un bon test. Les lignes doivent se découper ; rien ne
doit être écrit hors du tampon. Vérifiez le dump contre les parties du contour
réellement visibles.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-016/ex2.md)

### Exercice 3 — Flottant contre entier *(measure-the-performance)*

Implémentez `DrawLineFloat` — la ligne naïve de la prose : découper, calculer
`slope` en `double`, puis un `slope * (x - x0)` par pixel — et mettez en
concurrence les deux rastériseurs sur la même longue diagonale (un tampon
brouillon de 512×512 et `clock()` autour de chaque boucle ; la petite scène est
trop bruitée pour être chronométrée). Rapportez les microsecondes par ligne
pour les deux. Puis tracez la petite diagonale `(0, 0)`–`(7, 5)` avec les deux
dans des tampons séparés et comptez les pixels sur lesquels ils ne sont pas
d'accord. Lequel des deux résultats — le chronométrage ou les pixels —
tranche réellement la question de quel rastériseur utiliser, et pourquoi ?

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-016/ex3.md)

### Exercice 4 — Le terme d'erreur, observé *(explain-in-prose)*

Ajoutez un `fprintf(stderr, "step (%d,%d) err=%d\n", ...)` à l'intérieur de la
boucle de `DrawLine`, position de trace et terme d'erreur, et exécutez le
programme. Puis expliquez avec vos propres mots, en suivant la trace imprimée
de la diagonale jaune : (a) ce qu'un tour `e2 >= dy` / `e2 <= dx` décide du
pixel suivant ; (b) pourquoi `err` est toujours un entier et ne dérive jamais
— où, dans la trace, une version en virgule flottante aurait-elle arrondi
autrement ? (c) pourquoi le premier `err` de la trace est `dx + dy` et pas
zéro.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-016/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 015 — remplir un rectangle dans un tampon mémoire](lesson-015-fill-rect.md) ·
**Suivante :** [Leçon 017 — écrire un vrai fichier image à la main](lesson-017-image-file.md) ·
**Étiquette de code :** [`lesson-016`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-016)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-016-lines.md`,
révision `df71716`.*

<!-- translation-source: book/lessons/part-0/lesson-016-lines.md @ df71716 -->
