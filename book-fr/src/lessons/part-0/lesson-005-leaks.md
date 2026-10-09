# Leçon 005 — les fuites rendues visibles avec les sanitizers

{{#include ../../stability-horizon.md}}

## Prose

La leçon 004 s'est terminée sur une dette délibérée : `CountStream` alloue un
bloc de tas par fichier et ne le rend jamais, avec un commentaire promettant
que la leçon 005 rendrait la fuite visible. Aujourd'hui est ce jour. L'outil
est un *sanitizer* — un mode du compilateur qui réécrit le programme pour que
ses propres erreurs de mémoire ne puissent pas se cacher — et le gain est un
rapport de bug que vous n'avez pas eu à écrire.

**Ce qu'est un sanitizer.** Construire avec `-fsanitize=address` fait
instrumenter le programme par gcc : presque chaque accès mémoire gagne une
vérification, et une bibliothèque d'exécution remplace la machinerie de
mémoire en dessous. `malloc` et `free` ne sont plus ceux de la bibliothèque C
— ils sont ceux de l'exécution, et chaque allocation est enregistrée (avec la
pile d'appels qui l'a demandée), barrée de *redzones* de mémoire empoisonnée,
et suivie dans une carte de *shadow memory* qui dit, pour chaque octet de la
mémoire du programme, si y toucher est légal. Les écritures hors limites
atterrissent dans les redzones, les usages-après-libération touchent de la
mémoire que l'exécution a empoisonnée, et — la partie dont aujourd'hui a
besoin — *LeakSanitizer* voyage avec sur Linux et audite le tas à la sortie du
programme : chaque bloc encore alloué, sans aucun pointeur restant nulle part
vers lui, est une fuite, et le rapport est la pile enregistrée de celui qui
l'a demandé.

**La commande de construction** gagne deux options. Depuis l'intérieur de
`sandbox/wordcount/` :

```
gcc -std=c11 -O0 -g -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer wordcount.c -o wordcount
```

- `-fsanitize=address` est l'instrumentation ci-dessus. Elle coûte de la
  vitesse (environ 1,6× sur ce programme tel qu'écrit — l'exercice 2 la
  mesure) et de la mémoire (la carte fantôme, les redzones, une quarantaine
  de blocs libérés), et elle achète la certitude sur chaque accès mémoire que
  le programme fait.
- `-fno-omit-frame-pointer` garde un registre réservé au pointeur de trame
  dans chaque fonction. Les compilateurs aiment réutiliser ce registre pour
  des données ; quand ils le font, parcourir la pile d'appels exige des tables
  de déroulage par fonction et les tracés deviennent lacunaires. Pour les
  constructions de débogage, nous gardons la chaîne des trames intacte pour
  que la trace de pile de chaque rapport soit complète. Cela ne coûte presque
  rien et c'est pourquoi les tracés ci-dessous sont fiables.

**Le rapport.** Voici l'état de la leçon 004 — le programme qui fuit,
exactement comme `lesson-004` le garde — construit avec cette commande et
exécuté sur deux fichiers :

```
$ ./wordcount story.txt long.txt
2 4 21 11 story.txt
1 1 301 300 long.txt

=================================================================
==102059==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 576 byte(s) in 2 object(s) allocated from:
    #0 0x7658364fc778 in realloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:85
    #1 0x620bd3f3f4b8 in BufferGrow .../sandbox/wordcount/wordcount.c:36
    #2 0x620bd3f3f594 in BufferPush .../sandbox/wordcount/wordcount.c:44
    #3 0x620bd3f3f8d9 in CountStream .../sandbox/wordcount/wordcount.c:78
    #4 0x620bd3f3fdec in main .../sandbox/wordcount/wordcount.c:111
    ...

SUMMARY: AddressSanitizer: 576 byte(s) leaked in 2 allocation(s).
```

Lisez-le comme un reçu. `Direct leak of 576 byte(s) in 2 object(s)` — deux
blocs, un par fichier (le bloc de 64 octets de `story.txt` plus le bloc de
512 octets de `long.txt` ; les blocs intermédiaires sont morts à chaque
`realloc`, comme ils le doivent). *Direct* veut dire que rien ne pointe vers
ces blocs du tout ; *indirect* voudrait dire qu'ils ne sont atteignables
qu'à travers d'autres blocs fuyards. La trace de pile n'est pas quelque chose
que le programme a imprimé — c'est la pile d'appels enregistrée de
l'allocation, et elle nomme la scène de crime avec précision : `BufferGrow` a
fait l'allocation, depuis `BufferPush`, depuis `CountStream`, une fois par
fichier. Le processus sort avec le statut 1 : une fuite est une note
éliminatoire. (Les adresses, les chemins et l'identifiant de processus
diffèrent sur votre machine ; la forme, non. Sur un terminal, les compteurs
s'impriment avant le rapport ; à travers un tube, ils peuvent disparaître
entièrement — le sanitizer termine le processus avant que le tampon de stdio
ne se vide, l'histoire de la mise en tampon de la leçon 001.)

**La correction** est une fonction et un appel. `BufferFree` rend le bloc
avec `free` et remet la structure à zéro pour que le tampon ne puisse pas
servir à toucher de la mémoire libérée ensuite. `free(NULL)` est
explicitement légal — libérer un tampon vide est un no-op — et appeler `free`
deux fois sur le même bloc est un comportement indéfini de sa propre espèce,
si inoffensif que le second appel ait l'air. La discipline qu'encode la
correction est celle à garder pour le reste du cours : chaque allocation a
exactement un propriétaire, chaque propriétaire libère sur chaque chemin de
sortie, et une fonction qui prend un tampon en prend la responsabilité
jusqu'à la fin. Recompilez, et la même exécution est silencieuse —
compteurs seulement, statut de sortie 0. Le motif pour le reste de la partie 0 :
quand une leçon touche des bugs de mémoire, la commande de construction
apporte l'outil qui les rend visibles, et l'état n'est pas terminé tant que
l'outil ne s'est pas tu.

Une option de plus à connaître pour le jour où vous voudrez les deux
vérificateurs à la fois : les sanitizers se composent, et
`-fsanitize=address,undefined` est ce avec quoi la leçon 006 construit.

## Étape de code

Un seul changement pour cette leçon : `BufferFree` est ajouté et chaque chemin
de sortie de `CountStream` libère le tampon exactement une fois, si bien que
l'exécution sous sanitizer est propre — commité avec ce texte. Son état final
est étiqueté `lesson-005`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index cf8a0fb..e63016f 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,7 +1,7 @@
 // wordcount.c — count lines, words, bytes, and the longest line in every
 // file named on the command line.
 //
-// Lesson 004: malloc and free — growing buffers on the heap.
+// Lesson 005: leaks made visible with sanitizers.
 #include <ctype.h>
 #include <stdio.h>
 #include <stdlib.h>
@@ -17,10 +17,8 @@ struct Buffer {
     size_t len, cap;
 };
 
-// BufferInit prepares an empty buffer. The first push allocates.
-//
-// NOTE: nothing in this lesson ever frees the buffer's memory. We are
-// leaking this on purpose; lesson 005 makes it visible.
+// BufferInit prepares an empty buffer. The first push allocates; the
+// buffer's memory is owned here and released by BufferFree.
 static void BufferInit(struct Buffer *buf)
 {
     buf->data = NULL;
@@ -28,6 +26,17 @@ static void BufferInit(struct Buffer *buf)
     buf->cap = 0;
 }
 
+// BufferFree gives the buffer's memory back to the heap. Every path out of
+// CountStream must call it exactly once. free(NULL) is legal, so freeing an
+// empty buffer is safe.
+static void BufferFree(struct Buffer *buf)
+{
+    free(buf->data);
+    buf->data = NULL;
+    buf->len = 0;
+    buf->cap = 0;
+}
+
 // BufferGrow makes room for more bytes, doubling the capacity each time so
 // that pushing N bytes costs O(log N) reallocations instead of N.
 static void BufferGrow(struct Buffer *buf)
@@ -91,6 +100,8 @@ static void CountStream(FILE *f, struct Counts *out)
         if (line_len > out->longest)
             out->longest = line_len;
     }
+
+    BufferFree(&line);
 }
 
 int main(int argc, char **argv)
```

## Exercices

Trois courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Trois verdicts *(predict-the-output)*

Avec l'état de cette leçon construit sous le sanitizer, prédisez ce que
LeakSanitizer dira — s'il dit quoi que ce soit — pour trois exécutions :
`./wordcount nope.txt` (un fichier qui n'existe pas), `./wordcount empty.txt`,
et `./wordcount story.txt`. Pour chacune : un rapport apparaît-il, que
prétend-il, et quel est le statut de sortie ? Commentez ensuite le seul
`free` du programme — la solution montre la plus petite façon — et vérifiez
chaque prédiction contre les vrais verdicts.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-005/ex1.md)

### Exercice 2 — Ce que coûte la certitude *(measure-the-performance)*

Mesurez le prix du sanitizer. Fabriquez un fichier texte de 20 Mo
(`yes 'the quick brown fox' | head -c 20000000 > med.txt`), construisez le
programme de cette leçon deux fois — sans fioritures
(`gcc -std=c11 -O0 -g -Wall -Wextra …`) et avec la commande sanitizer de la
prose — et chronométrez les deux sur le fichier avec
`time ./wordcount med.txt`. Si les deux nombres sont plus proches que la
promesse d'« un facteur deux » de la prose, appliquez le changement de
lecture par blocs de la solution et remesurez pour démasquer le signal.
Rapportez les ratios, et une phrase sur où va le temps supplémentaire.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-005/ex2.md)

### Exercice 3 — Comment le sanitizer sait *(explain-in-prose)*

Expliquez le mécanisme en deux paragraphes : d'où vient la trace de pile du
rapport (le programme ne l'a jamais imprimée), ce que l'exécution doit faire
à `malloc` et `free` pour attraper une fuite à la sortie, et pourquoi le même
programme sous la construction ordinaire ne montre aucun signe du bug. La
construction instrumentée de la solution imprime chaque `free` au moment où il
se produit ; servez-vous-en pour vérifier l'histoire que vous écrivez.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-005/ex3.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 004 — malloc et free : faire grandir les tampons sur le tas](lesson-004-heap-buffers.md) ·
**Suivante :** [Leçon 006 — comportement indéfini et débordements de tampon](lesson-006-undefined-behavior.md) ·
**Étiquette de code :** [`lesson-005`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-005)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-005-leaks.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/lessons/part-0/lesson-005-leaks.md @ bb8d4ab -->
