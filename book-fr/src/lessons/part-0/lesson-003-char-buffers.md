# Leçon 003 — tampons de caractères : les chaînes à la main

{{#include ../../stability-horizon.md}}

## Prose

Cette leçon donne une voix à `wordcount`. Désormais il affiche

```
lines words bytes longest filename
```

pour chaque fichier, le tout compté en une seule passe. L'essentiel du
travail nouveau est du texte — et en C, le texte n'est pas un type. Ce sont
des octets par terre et une convention.

**Une chaîne C est des octets suivis d'un zéro.** Les chaînes Python
connaissent leur longueur — `len(s)` lit un champ. Une chaîne C n'est que des
octets en mémoire avec une règle : la séquence s'arrête sur un octet NUL,
`'\0'`, l'octet zéro, et toute routine qui consomme des chaînes travaille en
*marchant* jusqu'à en trouver un. Le `%s` de `printf`, le `strlen` de la
bibliothèque, le `strcpy` de la bibliothèque — tous marcheurs. Le `char *` de
la leçon 001 est « un pointeur vers le premier octet d'une telle séquence »,
et un littéral de chaîne comme `"hi"` est vraiment trois octets dans le
programme : `h`, `i`, NUL. Deux conséquences suivent. D'abord, demander sa
longueur à une chaîne coûte une marche — soit vous la marchez à chaque fois,
soit vous mémorisez la longueur vous-même. Ensuite, oubliez le terminateur et
les marcheurs ne s'arrêtent pas : ils enjambent vos données jusque dans les
octets qui suivent — l'ancien contenu du même tampon, d'autres variables, ce
que la pile contient — et vous rendent des longueurs et du texte qui sont des
mensonges. L'exercice 2 est ce bug, en direct.

**`char line[256]`** est un tableau : 256 octets contigus de stockage nommés
`line`, indexés depuis zéro (`line[0]` est le premier octet, le même comptage
depuis zéro qu'utilisait `argv`). Il vit dans la trame de pile de
`CountStream`, donc il existe pendant que cette fonction tourne et disparaît
quand elle renvoie. Deux nombres ne doivent jamais être confondus : la
*longueur* de la chaîne actuellement dans le tampon — combien d'octets ont été
stockés, suivis comme `len` dans le code — et la *capacité*, `sizeof line`, qui
est de 256 octets et ne bouge jamais. Une chaîne de N caractères a besoin de
N + 1 octets à cause du NUL, ce qui fait que ce tampon contient des lignes
jusqu'à 255 caractères et que le dernier octet est réservé. La garde
`if (len < sizeof line - 1)` est cette promesse en code — chaque caractère
stocké la traverse — et `line[len] = '\0'` referme la chaîne avant que
quiconque ne la mesure ou ne l'affiche.

**`LineLen`** est le `strlen` de ce programme : il marche `s[n]` depuis `n = 0`
jusqu'à ce que `s[n]` soit le NUL et renvoie le compteur. Le type du
paramètre `const char *s` veut dire « une chaîne que je promets de ne pas
modifier » — le compilateur se plaindra si le corps essaie. La bibliothèque a
`strlen`, bien sûr ; l'intérêt d'écrire celui-ci est qu'ensuite « chaîne C »
n'est pas une expression que vous avez lue mais une machine que vous avez
construite.

**La passe unique.** Un `fgetc` à la fois, et chaque octet nourrit trois
choses : le compteur d'octets ; le tampon de ligne, jusqu'à l'arrivée de
`'\n'` où la ligne terminée est mesurée, comparée à la plus longue connue,
comptée, et le tampon remis à zéro ; et une machine à mots à deux états. Le
drapeau `in_word` se souvient si l'octet précédent était à l'intérieur d'un
mot : une espace blanche (`isspace` du nouvel include `ctype.h` — espace,
tabulation, retour à la ligne et cousins) termine un mot, un octet non
blanc après une espace blanche en commence un, et commencer un mot est l'endroit
où `words` s'incrémente. Les lignes sont comptées sur les octets `'\n'`, la
même règle que `wc -l`, et une dernière ligne sans retour à la ligne final est
quand même mesurée pour `longest` mais pas comptée comme ligne — `wc` est
d'accord. Recoupez les trois premières colonnes avec la vraie chose quand vous
voulez :

```
$ printf 'hello world\nhi there\n' > story.txt
$ ./wordcount story.txt
2 4 21 11 story.txt
$ wc -l -w -c story.txt
 2  4 21 story.txt
```

`longest` est notre propre colonne : la longueur en octets de la plus longue
ligne, notre définition (le `-L` de GNU `wc` est un cousin qui mesure la
largeur d'affichage, aussi une tabulation y compte-t-elle pour plusieurs
colonnes et un octet ici).

**Structures et pointeurs, tout doux.** `struct Counts` est un simple paquet
de membres nommés — quatre compteurs sous un seul nom.
`struct Counts counts = {0};` en déclare un et met chaque membre à zéro d'un
seul geste. `CountStream(f, &counts)` passe deux poignées : le fichier, et
`&counts`, l'*adresse* de la structure — exactement la sorte de « pointeur
vers une chose » qu'est déjà `FILE *`. Dans l'appelé, le paramètre est
`struct Counts *out`, et `out->bytes` est déréférencement-et-membre en un seul
symbole : « le `bytes` de la structure à laquelle `out` pointe ». L'appelé
remplit les compteurs de l'appelant sur place et ne renvoie rien (`void`) : un
appel par fichier, quatre résultats, aucune copie. Les pointeurs sont ouverts
proprement dans la leçon 004 et les structures dans la leçon 007 ; pour
aujourd'hui, tenez juste la forme.

**Le plafond, exprès.** `line[256]` est une décision avec un prix visible. Une
ligne de 300 caractères est lue en entier — octets, mots et lignes la
comptent tous correctement — mais seuls ses 255 premiers caractères tiennent
dans le tampon, aussi `longest` rapporte-t-il 255 :

```
$ ./wordcount long.txt
1 1 301 255 long.txt
```

Rien ne plante et rien n'est corrompu ; le programme rapporte un nombre qu'il
ne peut pas connaître. C'est la limite honnête du tampon de taille fixe —
définie, délibérée, et la raison pour laquelle la leçon 004 donne au tampon de
ligne un tas capable de grandir.

La commande de construction est inchangée depuis la leçon 002 — gdb reste
utile contre ce programme à mesure qu'il grandit :

```
gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount
```

## Étape de code

Un seul changement pour cette leçon : le compteur d'octets devient
`struct Counts`, la boucle de comptage devient la passe unique `CountStream`
avec un tampon `char line[256]` géré à la main et un auxiliaire `LineLen`, et
la ligne de sortie gagne ses colonnes — commités avec ce texte. Son état final
est étiqueté `lesson-003`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index cd34962..a7e8a96 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,15 +1,62 @@
-// wordcount.c — count bytes in every file named on the command line.
+// wordcount.c — count lines, words, bytes, and the longest line in every
+// file named on the command line.
 //
-// Lesson 002: gdb — breakpoints, stepping, and stack frames.
+// Lesson 003: char buffers — strings by hand.
+#include <ctype.h>
 #include <stdio.h>
 
-static unsigned long CountBytes(FILE *f)
+struct Counts {
+    unsigned long lines, words, bytes, longest;
+};
+
+// LineLen is this program's own strlen: it walks a NUL-terminated string
+// and returns its length in bytes.
+static unsigned long LineLen(const char *s)
 {
-    unsigned long bytes = 0;
+    unsigned long n = 0;
+    while (s[n] != '\0')
+        ++n;
+    return n;
+}
+
+// CountStream reads f to EOF and accumulates counts into *out. Each line is
+// collected in a fixed buffer, so the longest line it can report is 255
+// bytes; lesson 004 lifts that ceiling.
+static void CountStream(FILE *f, struct Counts *out)
+{
+    char line[256];
+    unsigned long len = 0;
+    int in_word = 0;
     int c;
-    while ((c = fgetc(f)) != EOF)
-        ++bytes;
-    return bytes;
+
+    while ((c = fgetc(f)) != EOF) {
+        ++out->bytes;
+        if (c == '\n') {
+            line[len] = '\0';
+            unsigned long line_len = LineLen(line);
+            if (line_len > out->longest)
+                out->longest = line_len;
+            ++out->lines;
+            len = 0;
+            in_word = 0;
+        } else {
+            if (len < sizeof line - 1)
+                line[len++] = (char)c;
+            if (isspace(c)) {
+                in_word = 0;
+            } else if (!in_word) {
+                in_word = 1;
+                ++out->words;
+            }
+        }
+    }
+
+    if (len > 0) {
+        line[len] = '\0';
+        unsigned long line_len = LineLen(line);
+        if (line_len > out->longest)
+            out->longest = line_len;
+    }
 }
 
 int main(int argc, char **argv)
@@ -26,9 +73,11 @@ int main(int argc, char **argv)
             continue;
         }
 
-        unsigned long bytes = CountBytes(f);
+        struct Counts counts = {0};
+        CountStream(f, &counts);
 
-        printf("%lu %s\n", bytes, argv[i]);
+        printf("%lu %lu %lu %lu %s\n", counts.lines, counts.words,
+               counts.bytes, counts.longest, argv[i]);
         fclose(f);
     }
 
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Trois cents caractères *(predict-the-output)*

Fabriquez un fichier avec exactement une ligne de 300 caractères :
`printf '%300s\n' '' | tr ' ' 'x' > long.txt`. Avant d'exécuter quoi que ce
soit, prédisez la ligne de sortie exacte de `./wordcount long.txt` — chaque
colonne — et prédisez en quoi elle différera de `wc -l -w -c long.txt` plus la
vraie longueur de la ligne. Puis exécutez les deux, accordez chaque colonne de
votre prédiction avec la sortie réelle, et dites précisément quelle partie des
règles du programme a produit le nombre qui vous a surpris.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-003/ex1.md)

### Exercice 2 — La ligne citée ment *(fix-the-crash)*

Un collègue a ajouté une fonctionnalité : citer la plus longue ligne du
fichier à la fin de chaque fichier. Sa tentative compile et tourne, mais sur
`story.txt` la ligne de comptes est correcte tandis que la citation lit
`"hi thererld"` — et sur un fichier sans retour à la ligne final elle cite de
la camelote ou rien du tout. Trouvez toutes les façons dont la tentative va
mal, et corrigez la fonctionnalité pour que la citation soit toujours la plus
longue ligne du fichier, proprement terminée, pour chaque entrée. Ses ajouts à
`CountStream` :

```c
    char longest_text[128];               /* at the top of the function */

            for (unsigned long i = 0; i < len; ++i)
                longest_text[i] = line[i];   /* in the newline branch */

    fprintf(stderr, "longest line: \"%s\"\n", longest_text);  /* at the end */
```

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-003/ex2.md)

### Exercice 3 — Le mot le plus long *(extend-the-code)*

Ajoutez une cinquième colonne, `wlongest`, imprimée entre `longest` et le nom
de fichier : la longueur en octets du mot délimité par espaces le plus long du
fichier. La passe unique sait déjà où les mots commencent et finissent.
Vérifiez ensuite la colonne sur des fichiers dont les réponses vous pouvez
contrôler à l'œil — et expliquez ce qu'elle rapporte pour `long.txt`, et
pourquoi ce nombre diffère de ce que `longest` rapporte pour le même fichier.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-003/ex3.md)

### Exercice 4 — Le débogueur sur votre machine *(port-to-your-own-machine)*

Transportez la session de débogueur de la leçon 002 sur la machine que vous
possédez réellement : construisez-y le programme de cette leçon, posez un
point d'arrêt sur `LineLen`, exécutez sur un fichier de votre cru, et avancez
pas à pas dans la marche des longueurs jusqu'à pouvoir dire exactement ce
qu'elle compte et pourquoi. Si votre machine ne livre pas gdb, traduisez la
session vers son débogueur (sur macOS, la chaîne d'outils vous donne lldb) et
rapportez la commande que vous n'avez pas pu traduire. La construction
instrumentée de la solution est une façon portable de confirmer ce que vos pas
à pas vous ont dit.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-003/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 002 — gdb : points d'arrêt, pas à pas, trames de pile](lesson-002-gdb.md) ·
**Suivante :** [Leçon 004 — malloc et free : faire grandir les tampons sur le tas](lesson-004-heap-buffers.md) ·
**Étiquette de code :** [`lesson-003`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-003)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-003-char-buffers.md`, révision `1772957`.*

<!-- translation-source: book/lessons/part-0/lesson-003-char-buffers.md @ 1772957 -->
