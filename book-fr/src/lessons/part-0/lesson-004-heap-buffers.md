# Leçon 004 — malloc et free : faire grandir les tampons sur le tas

{{#include ../../stability-horizon.md}}

## Prose

La leçon 003 s'est terminée par un aveu dans la sortie : une ligne de 300
caractères a été mesurée à 255, parce que `char line[256]` est une taille
fixée à la compilation et que le programme a jeté en silence ce qui ne tenait
pas. Aujourd'hui, le tampon de ligne déménage vers le seul endroit en C où la
mémoire peut grandir pendant que le programme tourne : le tas. Le 255 devient
300, devient 5000, devient ce que le fichier contient.

**Pile et tas.** Tout ce qui a été local jusqu'ici — `line`, `counts`, `f`,
`c` — a vécu sur la pile : du stockage taillé quand une fonction est appelée
et détruit quand elle renvoie, avec une taille que le compilateur a fixée à
l'avance. Le tas est l'autre sorte : un bassin de mémoire que le *processus*
possède et distribue dans les tailles que vous demandez, au moment où vous le
demandez. Python et Ruby y mettent chaque objet et envoient un ramasse-miettes
ranger ; C vous donne la même mémoire et vous confie le travail du
ramasse-miettes. Trois verbes le dirigent : `malloc(n)` vous donne un bloc
neuf de `n` octets, `realloc(p, n)` déplace ou redimensionne un bloc existant
à `n` octets, et `free(p)` rend un bloc. Tout le reste de la programmation sur
le tas est de la comptabilité autour de ces trois-là.

Deux types apparaissent avec eux. Les tailles en C sont des `size_t` — le type
que renvoie `sizeof`, assez large pour n'importe quel objet en mémoire ;
`printf` l'épelle `%zu`. Et `malloc` renvoie `void *`, un pointeur sans type
pointé, que C convertit discrètement vers n'importe quel pointeur auquel vous
l'affectez — `char *p = malloc(n)` compile tel quel. (C++ est plus strict et
veut un transtypage ; le moteur de la partie 1 verra cette règle.)

**Realloc, en détail.** `realloc` est le cheval de bât des tampons qui
grandissent, et son contrat a trois clauses à mémoriser. Il copie vos octets
dans le nouveau bloc s'il doit déplacer — le contenu est préservé. S'il
déplace, l'*ancien* bloc est libéré, aussi l'ancien pointeur est-il mort
après, quoi qu'il arrive. Et `realloc(NULL, n)` est défini pour se comporter
exactement comme `malloc(n)` — ce qui fait que `BufferGrow` n'a jamais besoin
d'un premier appel spécial. L'échec est la quatrième clause : `realloc` renvoie
`NULL` et laisse l'ancien bloc intact, et le bug C classique est d'affecter
`p = realloc(p, n)` et de perdre ainsi le seul pointeur vers l'ancien bloc
quand cela arrive. Le `BufferGrow` de cette leçon prend le chemin heureux et
ne vérifie pas ; cette négligence est exactement ce que la leçon 006 arrête,
avec les outils pour voir pourquoi.

**Le Buffer.** `struct Buffer` est la forme que prend presque chaque
conteneur dynamique en C : `char *data` — où vivent les octets, `size_t len` —
combien sont utiles, `size_t cap` — combien tiennent. Trois petites fonctions
le possèdent : `BufferInit` le vide (et n'alloue encore rien — `data` est
`NULL`, et le premier envoi paie le premier bloc), `BufferGrow` double la
capacité, et `BufferPush` ajoute un octet, en grandissant d'abord quand le
tampon est plein. Doubler est une politique avec une analyse de coût derrière :
après N envois, il y a eu environ log₂(N) réallocations, et le nombre total
d'octets copiés à travers toutes est proportionnel à N, pas à N². L'exercice 2
vous demande de mesurer l'alternative.

Un appel à signaler chez `BufferGrow` : `realloc` rend un `void *` comme
`malloc`, et chaque bloc agrandi *remplace* le précédent — les pointeurs que
l'instrument de l'exercice 3 affiche à chaque croissance racontent la même
histoire que la première clause de `realloc`.

**Propriété, et la fuite que nous gardons.** Voici la phrase que cette leçon
vous doit : **nous fuyons le tampon exprès ; la leçon 005 le rend visible.**
`CountStream` alloue le tampon de ligne sur le tas et ne le libère jamais.
Quand `CountStream` renvoie, la structure `line` — une locale de pile —
disparaît, mais le bloc de tas qu'elle pointait reste alloué, inatteignable,
jusqu'à la sortie du processus. Le système d'exploitation récupère tout le tas
à la sortie, aussi une commande ponctuelle comme la nôtre ne paie-t-elle cela
qu'en principe. Une boucle de jeu qui fuit à chaque trame paie dans la seule
monnaie qu'un jeu possède. La raison d'écrire cela ainsi maintenant : la
correction est longue d'un appel de fonction, et l'*outil qui la prouve* est
la moitié intéressante — la leçon 005 exécute ce programme exact sous un
sanitizer de fuites et vous montre le rapport avant de vous montrer le remède.

Des règles de propriété à énoncer pendant que le code est petit : le bloc
appartient au `Buffer`, un `Buffer` par appel de `CountStream`, `realloc`
retire l'ancien bloc à chaque croissance, `fclose` rend l'objet `FILE` et ses
tampons internes mais ne sait rien des nôtres, et à la fin du programme
exactement un bloc par fichier est encore vivant.

**Le plafond, levé.** Même programme, mêmes lignes, nombres honnêtes :

```
$ ./wordcount story.txt
2 4 21 11 story.txt
$ ./wordcount long.txt
1 1 301 300 long.txt
$ ./wordcount huge.txt
1 1 5001 5000 huge.txt
```

`long.txt` est la ligne de 300 caractères de la leçon 003 ; `huge.txt` est une
ligne de 5000 caractères. Les deux sont désormais mesurés pour de bon. La
commande de construction est inchangée :

```
gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount
```

## Étape de code

Un seul changement pour cette leçon : le `char line[256]` fixe devient un
`struct Buffer` grandissable sur le tas, avec `BufferInit`, `BufferGrow` et
`BufferPush`, agrandi par `malloc`/`realloc` avec capacité doublée — et
délibérément jamais libéré. Commité avec ce texte ; son état final est
étiqueté `lesson-004`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index a7e8a96..cf8a0fb 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,14 +1,50 @@
 // wordcount.c — count lines, words, bytes, and the longest line in every
 // file named on the command line.
 //
-// Lesson 003: char buffers — strings by hand.
+// Lesson 004: malloc and free — growing buffers on the heap.
 #include <ctype.h>
 #include <stdio.h>
+#include <stdlib.h>
 
 struct Counts {
     unsigned long lines, words, bytes, longest;
 };
 
+// Buffer is a growable byte buffer. data points at len bytes of useful
+// content with room for cap bytes in total.
+struct Buffer {
+    char *data;
+    size_t len, cap;
+};
+
+// BufferInit prepares an empty buffer. The first push allocates.
+//
+// NOTE: nothing in this lesson ever frees the buffer's memory. We are
+// leaking this on purpose; lesson 005 makes it visible.
+static void BufferInit(struct Buffer *buf)
+{
+    buf->data = NULL;
+    buf->len = 0;
+    buf->cap = 0;
+}
+
+// BufferGrow makes room for more bytes, doubling the capacity each time so
+// that pushing N bytes costs O(log N) reallocations instead of N.
+static void BufferGrow(struct Buffer *buf)
+{
+    size_t new_cap = buf->cap == 0 ? 64 : buf->cap * 2;
+    buf->data = realloc(buf->data, new_cap);
+    buf->cap = new_cap;
+}
+
+// BufferPush appends one byte, growing first if the buffer is full.
+static void BufferPush(struct Buffer *buf, char c)
+{
+    if (buf->len == buf->cap)
+        BufferGrow(buf);
+    buf->data[buf->len++] = c;
+}
+
 // LineLen is this program's own strlen: it walks a NUL-terminated string
 // and returns its length in bytes.
 static unsigned long LineLen(const char *s)
@@ -19,29 +55,27 @@ static unsigned long LineLen(const char *s)
     return n;
 }
 
-// CountStream reads f to EOF and accumulates counts into *out. Each line is
-// collected in a fixed buffer, so the longest line it can report is 255
-// bytes; lesson 004 lifts that ceiling.
+// CountStream reads f to EOF and accumulates counts into *out. The line
+// buffer lives on the heap now, so lines of any length are measured truly.
 static void CountStream(FILE *f, struct Counts *out)
 {
-    char line[256];
-    unsigned long len = 0;
+    struct Buffer line;
+    BufferInit(&line);
     int in_word = 0;
     int c;
 
     while ((c = fgetc(f)) != EOF) {
         ++out->bytes;
         if (c == '\n') {
-            line[len] = '\0';
-            unsigned long line_len = LineLen(line);
+            BufferPush(&line, '\0');
+            unsigned long line_len = LineLen(line.data);
             if (line_len > out->longest)
                 out->longest = line_len;
             ++out->lines;
-            len = 0;
+            line.len = 0;
             in_word = 0;
         } else {
-            if (len < sizeof line - 1)
-                line[len++] = (char)c;
+            BufferPush(&line, (char)c);
             if (isspace(c)) {
                 in_word = 0;
             } else if (!in_word) {
@@ -51,9 +85,9 @@ static void CountStream(FILE *f, struct Counts *out)
         }
     }
 
-    if (len > 0) {
-        line[len] = '\0';
-        unsigned long line_len = LineLen(line);
+    if (line.len > 0) {
+        BufferPush(&line, '\0');
+        unsigned long line_len = LineLen(line.data);
         if (line_len > out->longest)
             out->longest = line_len;
     }
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La suite des doublons *(predict-the-output)*

Prenez un fichier avec une seule ligne de 5000 caractères (`huge.txt` de la
prose). `BufferGrow` tourne plus d'une fois pendant la lecture de la ligne.
Avant d'exécuter quoi que ce soit, prédisez combien de fois il tourne et la
suite exacte des capacités par lesquelles le tampon passe. Ajoutez ensuite une
impression d'instrumentation à `BufferGrow`, exécutez, et accordez la vraie
suite avec votre prédiction — y compris ce qui arrive au tout premier envoi
et pourquoi la capacité finale n'est pas 5000.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-004/ex1.md)

### Exercice 2 — Un octet à la fois *(measure-the-performance)*

Changez la politique de croissance du doublonnement à « un octet de plus que
la fois précédente », recompilez, et fabriquez deux fichiers avec une seule
ligne chacun : 100 000 caractères
(`head -c 100000 /dev/zero | tr '\0' 'x' > big.txt`) et 1 000 000 de
caractères, pareillement. Chronométrez les deux politiques sur les deux
fichiers avec `time ./wordcount FICHIER`. La prose affirme que doubler copie
O(N) octets au total tandis que grandir d'un octet à la fois copie O(N²) —
vérifiez cette affirmation contre vos mesures. Si les chronométrages ne
montrent pas l'écart que la théorie promet, trouvez où la copie prédite est
passée, et dites ce que les chronométrages que vous avez mesurés prouvent et
ne prouvent pas.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-004/ex2.md)

### Exercice 3 — Qui possède les octets *(explain-in-prose)*

Écrivez l'histoire de la propriété de ce programme en deux paragraphes :
quelle mémoire appartient à qui, ce que chaque `realloc` a fait au bloc qui
le précédait, ce que `fclose` rend et ce qu'il ne rend pas, et exactement
quels blocs sont encore vivants quand `main` renvoie. Dites ensuite ce à quoi
la leçon 005 s'opposera, et pourquoi une commande ponctuelle peut se
l'offrir tandis qu'une boucle de jeu ne le peut pas. La construction
instrumentée de la solution affiche l'adresse de chaque bloc à mesure que le
tampon grandit ; servez-vous-en pour ancrer l'histoire dans une vraie sortie.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-004/ex3.md)

### Exercice 4 — L'entrée standard, revue *(extend-the-code)*

La promesse du premier exercice de la leçon 001 : une fois que le comptage
vit dans sa propre fonction, lire l'entrée standard quand aucun fichier n'est
nommé est presque gratuit. Faites fonctionner `./wordcount < story.txt` —
aucun argument veut dire lire `stdin` et afficher la ligne sans nom de fichier
— et dites pourquoi le changement est désormais plus petit que la boucle
qu'il remplace.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-004/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 003 — tampons de caractères : les chaînes à la main](lesson-003-char-buffers.md) ·
**Suivante :** [Leçon 005 — les fuites rendues visibles avec les sanitizers](lesson-005-leaks.md) ·
**Étiquette de code :** [`lesson-004`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-004)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-004-heap-buffers.md`, révision `bb8d4ab`.*

<!-- translation-source: book/lessons/part-0/lesson-004-heap-buffers.md @ bb8d4ab -->
