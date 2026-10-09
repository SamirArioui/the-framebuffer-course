# Leçon 006 — comportement indéfini et débordements de tampon

{{#include ../../stability-horizon.md}}

## Prose

La fuite de la leçon 005 était un bug avec des règles : le programme n'abusait
de rien, il oubliait seulement quelque chose, et la norme promettait toujours
ce qu'elle promet. Le sujet d'aujourd'hui est l'autre sorte de bug — celle où
les règles elles-mêmes s'arrêtent. C'est le dernier et le plus important
concept de C de la première moitié de la partie 0, parce que toute la gestion
de mémoire d'un moteur de jeu est bâtie sur la connaissance de l'endroit où
elle vit.

**Ce que veut dire « indéfini ».** Pour certaines opérations, la norme C ne
spécifie pas une mauvaise réponse — elle ne spécifie *aucune* réponse : si un
programme en réalise une, son comportement est indéfini, et l'implémentation
est félicitée pour tout ce qui se produit. Pas « ça plante ». Pas « ça
retombe ». Python lève `OverflowError` ? Les entiers Python grandissent sans
borne. Ruby passe aux `Bignum`. C n'a pas cette transition : faites-le, et le
contrat est caduc. Les trois façons de sortir de la carte qui comptent pour ce
programme :

- **Débordement signé.** Une arithmétique `int` dont le vrai résultat ne tient
  pas dans le type. `2147483647 + 1` est indéfini — pas « retombe sur
  −2147483648 », qui est simplement ce que vous observerez pendant que
  personne ne regarde (l'exercice 1 regarde).
- **Accès hors limites.** Lire ou écrire un élément de tableau hors du
  tableau, ou déréférencer un pointeur hors de son objet. Un pas au-delà de
  la fin peut être *pointé* mais jamais touché. C'est le sort dont notre
  propre tampon a flirté dans la leçon 003 et que l'exercice 2 met en scène
  proprement.
- **Valeurs indéterminées.** Utiliser de la mémoire avant de l'écrire : la
  camelote de `info locals` de la leçon 002 était cela — une valeur qui
  n'existe que comme les bits laissés derrière.

À contraster avec le quatrième sort, qui est *défini* et faux quand même :
l'arithmétique `size_t` retombe modulo sa largeur. Aucune règle n'est violée ;
vous obtenez simplement un nombre qui ne veut rien dire. Notre doublement de
capacité `buf->cap * 2` marche sur cette ligne — il ne devient jamais
indéfini, il devient juste silencieusement petit, et l'écriture suivante fait
déborder la petite allocation qui en résulte.

**Les hypothèses du compilateur.** Voici pourquoi le comportement indéfini est
pire qu'une mauvaise valeur. Un compilateur qui optimise un programme peut
supposer que le comportement indéfini *n'arrive jamais*, puisque aucun
programme valide n'en contient, et raisonner librement à partir de là — des
branches peuvent être prouvées mortes, des vérifications supprimées, des
boucles réarrangées. À `-O0` vous observez le retournement ; aux réglages
supérieurs vous pouvez observer l'hypothèse elle-même. L'optimiseur a ses
propres leçons dans le programme `paint` plus loin dans la partie 0 ; pour
l'instant, la règle est : ne construisez pas sur ce que le comportement
indéfini « fait ».

**À quoi ressemblent les débordements sous les sanitizers.** Le sanitizer
d'adresses rend les cas spatiaux visibles au moment où ils se produisent. Un
fichier brouillon n'importe où (celui-ci fait quatre lignes de `main`)
suffit pour voir l'anatomie :

```c
#include <stdlib.h>

int main(void)
{
    char *p = malloc(4);
    p[4] = 'x';
    free(p);
    return 0;
}
```

```
==105939==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x502000000014 ...
WRITE of size 1 at 0x502000000014 thread T0
    #0 0x6386f4bb72db in main /tmp/opencode/work/demo_overflow.c:6
    ...
0x502000000014 is located 0 bytes after 4-byte region [0x502000000010,0x502000000014)
allocated by thread T0 here:
    #0 0x7e6cfd2fd9c7 in malloc ...
```

Lisez les deux moitiés : la ligne de faute — lecture ou écriture, de quelle
taille, dans quelle fonction — et la géographie — `0 bytes after 4-byte
region`, l'allocation exacte que vous avez franchie, avec la pile qui l'a
allouée. (Les adresses et le chemin dans ces trames sont ceux de cette
machine ; les vôtres différeront et la forme, non.) Le sanitizer de
comportement indéfini rapporte les cas non spatiaux sous forme de lignes
`runtime error:` nommant l'opération et le type qu'elle a fait déborder. Les
deux vérificateurs se composent, et à partir de la construction d'aujourd'hui
la commande les porte tous les deux :

```
gcc -std=c11 -O0 -g -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer wordcount.c -o wordcount
```

**Le durcissement.** L'étape de code de cette leçon convertit les trois modes
d'échec silencieux du tampon en modes bruyants et propres. `BufferAt` est
désormais la seule façon sanctionnée d'atteindre les octets du tampon : un
indice hors de l'allocation est vérifié avant l'accès, et à la place d'un
comportement indéfini le programme rapporte l'indice et s'arrête — « proprement »
voulant dire exactement ce que font les chemins d'erreur partout dans ce
programme : dire ce qui s'est mal passé, libérer ce que vous possédez
(`BufferFree` en sortant, pour que le sanitizer reste silencieux même sur le
chemin d'échec), et sortir avec un statut d'échec. `BufferGrow` vérifie son
doublement pour le retournement *après* l'avoir calculé — l'arithmétique non
signée retombe par définition, aussi la vérification lit-elle la valeur
retombée — et vérifie le retour de `realloc` pour `NULL`, le bug classique
`p = realloc(p, n)` où l'échec perd le seul pointeur vers l'ancien bloc.
Notre version garde l'ancien pointeur tant qu'un nouveau n'existe pas.

Le programme est inchangé en comportement pour chaque entrée qui
fonctionnait avant — et il se construit toujours avec la commande simple de
la leçon 002
(`gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount`), parce que
les sanitizers sont un mode de débogage, jamais une dépendance. Le
durcissement est du code ordinaire ; seuls les vérificateurs sont facultatifs.

## Étape de code

Un seul changement pour cette leçon : le tampon gagne une vérification de
débordement et un chemin d'échec de `realloc` qui tous deux sortent proprement
sur l'erreur, et `BufferAt` remplace l'indexation brute `data[i]` — commité
avec ce texte. Son état final est étiqueté `lesson-006`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index e63016f..e7d8151 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,7 +1,7 @@
 // wordcount.c — count lines, words, bytes, and the longest line in every
 // file named on the command line.
 //
-// Lesson 005: leaks made visible with sanitizers.
+// Lesson 006: undefined behavior and buffer overflows.
 #include <ctype.h>
 #include <stdio.h>
 #include <stdlib.h>
@@ -37,12 +37,37 @@ static void BufferFree(struct Buffer *buf)
     buf->cap = 0;
 }
 
+// BufferAt is the only sanctioned way to reach the buffer's bytes: it
+// turns an out-of-range index from undefined behavior into a clean error.
+static char *BufferAt(struct Buffer *buf, size_t i)
+{
+    if (i >= buf->cap) {
+        fprintf(stderr, "wordcount: buffer index %zu out of range\n", i);
+        BufferFree(buf);
+        exit(1);
+    }
+    return &buf->data[i];
+}
+
 // BufferGrow makes room for more bytes, doubling the capacity each time so
-// that pushing N bytes costs O(log N) reallocations instead of N.
+// that pushing N bytes costs O(log N) reallocations instead of N. Both
+// failure modes end the program cleanly: size_t arithmetic wraps around by
+// definition, so the doubling is checked for it, and realloc can fail.
 static void BufferGrow(struct Buffer *buf)
 {
     size_t new_cap = buf->cap == 0 ? 64 : buf->cap * 2;
-    buf->data = realloc(buf->data, new_cap);
+    if (new_cap < buf->cap) {
+        fprintf(stderr, "wordcount: buffer capacity overflow\n");
+        BufferFree(buf);
+        exit(1);
+    }
+    char *p = realloc(buf->data, new_cap);
+    if (p == NULL) {
+        fprintf(stderr, "wordcount: out of memory\n");
+        BufferFree(buf);
+        exit(1);
+    }
+    buf->data = p;
     buf->cap = new_cap;
 }
 
@@ -51,7 +76,8 @@ static void BufferPush(struct Buffer *buf, char c)
 {
     if (buf->len == buf->cap)
         BufferGrow(buf);
-    buf->data[buf->len++] = c;
+    *BufferAt(buf, buf->len) = c;
+    ++buf->len;
 }
 
 // LineLen is this program's own strlen: it walks a NUL-terminated string
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — La valeur qui n'est pas promise *(predict-the-output)*

Ajoutez un démonstrateur à `main` : un `int` contenant `2147483647`,
incrémenteé une fois, imprimé avec `%d`. Prédisez trois choses avant
d'exécuter quoi que ce soit : ce que `-fsanitize=undefined` dira de
l'incrément, quelle valeur l'impression montre sous la construction sanitizer,
et ce que la construction simple imprime. Exécutez ensuite les deux
constructions et accordez — y compris pourquoi « ça est retombé sur
−2147483648 » est une observation et non une promesse, et ce qu'un compilateur
a donc le droit de faire avec le code autour.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-006/ex1.md)

### Exercice 2 — La ligne vide qui lit à l'envers *(fix-the-crash)*

Un collègue a ajouté une petite fonctionnalité : rapporter le dernier
caractère de la dernière ligne du fichier. Sa tentative compile et
fonctionne sur `story.txt` — mais la construction sanitizer s'arrête en
catastrophe avec un rapport de heap-buffer-overflow dès qu'une ligne vide
apparaît sous du vrai contenu, et la construction simple fait comme si de
rien n'était. Ses ajouts à `CountStream` :

```c
    char last = '?';                        /* next to in_word */

            last = line.data[line.len - 1]; /* first line of the newline branch */

    fprintf(stderr, "last char: '%c'\n", last);   /* just before BufferFree */
```

Trouvez exactement ce que cette lecture fait quand la ligne est vide, puis
corrigez la fonctionnalité pour qu'elle rapporte le dernier caractère de la
dernière ligne qui en a un et n'atteigne jamais hors du tampon — l'accesseur
de la leçon est la façon sanctionnée. Confirmez avec les deux constructions
sur un fichier avec une ligne vide.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-006/ex2.md)

### Exercice 3 — Trois sorts *(explain-in-prose)*

Classez les modes d'échec de cette leçon dans les trois sorts du langage :
comportement indéfini (débordement signé, accès hors limites), défini mais
faux (retournement `size_t`), et indéterminé (lectures non initialisées).
Dites ensuite contre quel sort chacune des trois vérifications de durcissement
de l'étape de code défend, et pourquoi « ça a marché quand je l'ai testé »
n'est une preuve contre aucune d'elles. Deux paragraphes. La construction
instrumentée de la solution imprime chaque vérification de capacité au moment
où elle se produit.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-006/ex3.md)

### Exercice 4 — Le prix de la vérification *(measure-the-performance)*

`BufferAt` tourne une fois par octet envoyé. Mesurez ce que cela coûte :
chronométrez le programme de cette leçon sur le `med.txt` de 20 Mo de la leçon
005 sous la construction simple, puis appliquez la variante à indexation brute
de la solution — la vérification retirée — et chronométrez à nouveau. La
vérification est-elle abordable sur le budget de ce programme ? Dites où va
réellement le coût quand la construction est `-O0`, et ce que cela implique
pour les constructions qui éliminent la vérification par inline.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-006/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 005 — les fuites rendues visibles avec les sanitizers](lesson-005-leaks.md) ·
**Suivante :** [Leçon 007 — structures : sizeof, alignement et remplissage](lesson-007-struct-layout.md) ·
**Étiquette de code :** [`lesson-006`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-006)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-006-undefined-behavior.md`, révision `bb8d4ab`.*

<!-- translation-source: book/lessons/part-0/lesson-006-undefined-behavior.md @ bb8d4ab -->
