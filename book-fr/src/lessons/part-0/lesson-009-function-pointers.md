# Leçon 009 — pointeurs de fonction : comparateurs et hooks

{{#include ../../stability-horizon.md}}

## Prose

Cette leçon apprend au kit à accepter des instructions en arguments. Une
fonction en C n'est pas seulement quelque chose que vous appelez — le nom
d'une fonction, utilisé là où une valeur est attendue, devient un pointeur sur
le code machine de cette fonction. Rangez-le dans une variable, passez-le à
une autre fonction, appelez-le plus tard avec la syntaxe d'appel ordinaire.
Le `sorted(key=...)` de Python, les blocs de Ruby et `&:method` — même
capacité, habituellement cachée derrière la syntaxe. En C c'est une valeur de
première classe avec un type honnête et laid.

Le type est l'endroit où C vous fait travailler pour l'obtenir. Lisez
celui-ci de l'intérieur vers l'extérieur :

```c
int (*cmp)(const struct Item *, const struct Item *)
```

`cmp` est un pointeur (`*cmp`, mis entre parenthèses pour que le `*` se lie au
nom) vers quelque chose que vous pouvez appeler avec deux arguments
`const struct Item *` et qui produit un `int`. Les parenthèses autour de
`*cmp` sont porteuses : écrivez `int *f(const struct Item *)` sans elles et
vous avez déclaré une *fonction* renvoyant `int *` à la place. Cette lecture
à l'envers est le prix d'une syntaxe de déclaration couvrant variables,
fonctions, tableaux et pointeurs ; quand cela devient trop douloureux, les
programmeurs C nomment le type —
`typedef int (*ItemCmp)(const struct Item *, const struct Item *);` — et
écrivent `ItemCmp` à partir de là. L'étape de code garde la forme brute
visible dans la signature de `DaSort` pour que vous continuiez à la lire.

Ce que le kit fait d'un tel pointeur, c'est trier. `DaSort` prend un
comparateur — une fonction qui définit un ordre en répondant « plus petit,
égal, ou plus grand » par un nombre négatif, zéro, ou un nombre positif — et
remet le tableau au `qsort` de libc depuis `<stdlib.h>`, qui est du C
standard et trie *n'importe quel* tableau étant donné son début, sa longueur,
la taille de ses éléments, et un tel comparateur. Deux comparateurs
conduisent le pilote : `CmpByKey` transmet à `strcmp` (qui renvoie déjà une
réponse négative/zéro/positive), et `CmpByValue` en calcule une avec le
classique tour à trois voies
`(a->value > b->value) - (a->value < b->value)` — soustraire deux booléens de
comparaison, parce que soustraire les *valeurs* elles-mêmes déborderait pour
des `long` assez grands.

Maintenant la partie honnête — pourquoi `DaSort` contient une fonction de
pont et un `g_cmp` statique de fichier. Le type de comparateur de `qsort`
lui-même est `int (*)(const void *, const void *)` : des pointeurs vers
*n'importe quel* élément, non typés, parce que `qsort` précède toute notion
de généricité en C. Notre comparateur typé est un type de pointeur de
fonction différent, et C n'autorise pas à passer l'un là où l'autre est
attendu — ni n'est-il sûr de transtyper entre eux et d'appeler à travers le
mauvais type : le code machine serait en désaccord sur ce qu'il pointe. Alors
`CmpBridge` est la seule traversée conforme : il reçoit des pointeurs
`void *`, les re-transtype en `const struct Item *`, et transmet au
comparateur quel qu'il soit que `DaSort` a reçu. Et `g_cmp` existe parce que
`qsort` ne passe aucun contexte au comparateur — il n'y a pas d'emplacement
« d'argument supplémentaire » (le `qsort_r` de POSIX en a un ; le C standard
non), aussi le comparateur en attente attend-il dans une variable statique de
fichier autour de l'appel à `qsort`. Une verrue connue de la bibliothèque
standard C, sûre ici parce que rien d'autre ne tourne en même temps, et qui
vaut la peine d'être reconnue à vue : quand une API C ne peut pas vous remettre
votre propre contexte, elle vous fera trouver un endroit où le garer.

Le second pointeur de fonction est un **hook de visite**, et il définit
l'itération : `DaEach` parcourt le tableau et appelle votre fonction `visit`
sur chaque élément. C'est le motif du callback dans sa forme la plus pure —
l'inversion de contrôle : notre boucle tourne, votre fonction est appelée à
l'intérieur. Le pilote fournit `PrintItem` et obtient son impression
gratuitement ; fournissez un hook différent et le même parcourt fait autre
chose. Remarquez ce qui varie et ce qui ne varie pas : le comparateur change
*comment* le tableau est ordonné, le hook change *ce qui arrive par élément*,
et `DaSort`/`DaEach` ne connaissent jamais la différence.

La commande de construction est inchangée (depuis `sandbox/ds-kit/`) :

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

## Étape de code

Un seul changement pour cette leçon : `ds-kit.c` gagne la machinerie des
pointeurs de fonction — `DaSort` prenant un comparateur et enveloppant
`qsort` à travers un pont, `DaEach` prenant un hook de visite — et le pilote
envoie ses dix éléments, les trie par clé, les trie par valeur, et imprime
chaque état. Son état final est étiqueté `lesson-009`.

```diff
diff --git a/sandbox/ds-kit/ds-kit.c b/sandbox/ds-kit/ds-kit.c
index 81f9e65..ff3ce5c 100644
--- a/sandbox/ds-kit/ds-kit.c
+++ b/sandbox/ds-kit/ds-kit.c
@@ -1,9 +1,10 @@
 // ds-kit.c — the data-structures kit of Part 0: a dynarray of Items that
-// grows by realloc with capacity doubling.
+// grows by realloc, sorts by comparator, and visits by hook.
 //
-// Lesson 008: realloc growth — len, cap, and why doubling wins.
+// Lesson 009: function pointers — comparators and callbacks.
 #include <stdio.h>
 #include <stdlib.h>
+#include <string.h>
 
 struct Item {
     char key[16];
@@ -44,6 +45,49 @@ void DaFree(struct DynArray *da)
     DaInit(da);
 }
 
+// -- the function-pointer machinery -------------------------------------
+//
+// qsort's own comparator type is int (*)(const void *, const void *), so a
+// bridge function converts and forwards. Which comparator to forward to
+// lives in g_cmp: qsort offers no way to pass extra context along.
+
+static int (*g_cmp)(const struct Item *, const struct Item *);
+
+static int CmpBridge(const void *pa, const void *pb)
+{
+    return g_cmp((const struct Item *)pa, (const struct Item *)pb);
+}
+
+void DaSort(struct DynArray *da, int (*cmp)(const struct Item *, const struct Item *))
+{
+    g_cmp = cmp;
+    qsort(da->items, da->len, sizeof *da->items, CmpBridge);
+    g_cmp = NULL;
+}
+
+void DaEach(const struct DynArray *da, void (*visit)(const struct Item *))
+{
+    for (size_t i = 0; i < da->len; ++i)
+        visit(&da->items[i]);
+}
+
+// -- the callbacks this driver supplies --------------------------------
+
+static int CmpByKey(const struct Item *a, const struct Item *b)
+{
+    return strcmp(a->key, b->key);
+}
+
+static int CmpByValue(const struct Item *a, const struct Item *b)
+{
+    return (a->value > b->value) - (a->value < b->value);
+}
+
+static void PrintItem(const struct Item *it)
+{
+    printf("%s %ld\n", it->key, it->value);
+}
+
 int main(void)
 {
     struct DynArray da;
@@ -58,10 +102,18 @@ int main(void)
         snprintf(item.key, sizeof item.key, "%s", keys[i]);
         item.value = i;
         DaPush(&da, item);
-        printf("len=%zu cap=%zu\n", da.len, da.cap);
     }
 
-    printf("first=%s last=%s\n", da.items[0].key, da.items[9].key);
+    printf("before:\n");
+    DaEach(&da, PrintItem);
+
+    DaSort(&da, CmpByKey);
+    printf("sorted by key:\n");
+    DaEach(&da, PrintItem);
+
+    DaSort(&da, CmpByValue);
+    printf("sorted by value:\n");
+    DaEach(&da, PrintItem);
 
     DaFree(&da);
     return 0;
```

## Exercices

Trois courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — L'ordre inversé *(predict-the-output)*

Dans `CmpBridge`, échangez les deux arguments de l'appel à `g_cmp` — passez
`pb` là où va `pa` et inversement — et ne touquez à rien d'autre. Avant de
l'exécuter, écrivez les dix lignes exactes que vous attendez sous
`sorted by key:`. Puis construisez, exécutez, et rendez compte du résultat :
la sortie est-elle simplement le bloc précédent inversé, ou quelque chose de
plus subtil, et pourquoi ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-009/ex1.md)

### Exercice 2 — Chercher par prédicat *(extend-the-code)*

Ajoutez une recherche au kit : `DaFind`, conduite par un prédicat — une
fonction qui répond à une question oui/non sur un élément — renvoyant un
pointeur vers le premier élément qui le satisfait, ou `NULL` si aucun ne le
fait. Le tableau doit rester inchangé. Servez-vous-en depuis le pilote pour
deux requêtes : imprimez l'élément dont la clé est `grape`, et montrez une
requête qui ne correspond à rien. Nommez le type de pointeur de fonction du
prédicat en entier au moins une fois, et assurez-vous que le pilote ne
touche jamais le pointeur renvoyé sans le vérifier.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-009/ex2.md)

### Exercice 3 — Pourquoi le pont existe *(explain-in-prose)*

Écrivez une courte explication — un paragraphe chacune — de (a) pourquoi on
ne peut pas donner `CmpByKey` directement à `qsort` : quel est le type
déclaré de son comparateur, et ce que C dit de la conversion entre types de
pointeurs de fonction incompatibles et de l'appel à travers le pointeur
converti ; (b) ce que `g_cmp` compense dans la conception de `qsort`, et dans
quelle situation ce contournement cesserait d'être sûr ; (c) ce que `DaEach`
achète qu'une simple boucle `for` sur `da.items` n'achète pas, et quand le
callback ne *vaut* pas le coup. Appliquez ensuite la vérification confirmante
de la solution, regardez le pont travailler sur de vrais appels, et
assurez-vous que votre explication correspond à ce qu'il imprime.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-009/ex3.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 008 — croissance de dynarray : realloc et capacité](lesson-008-dynarray.md) ·
**Suivante :** [Leçon 010 — void\* : la généricité et ses peines](lesson-010-void-pointer.md) ·
**Étiquette de code :** [`lesson-009`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-009)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-009-function-pointers.md`, révision `333e81a`.*

<!-- translation-source: book/lessons/part-0/lesson-009-function-pointers.md @ 333e81a -->
