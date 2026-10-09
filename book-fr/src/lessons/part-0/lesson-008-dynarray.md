# Leçon 008 — croissance de dynarray : realloc et capacité

{{#include ../../stability-horizon.md}}

## Prose

Cette leçon donne au kit son premier conteneur. Les tableaux C ont une taille
pour la vie : `struct Item items[10]` contient dix éléments sur la pile et
jamais un onzième. Le `list` de Python et l'`Array` de Ruby grandissent en
silence quand vous ajoutez ; C n'a rien de tel intégré, aussi chaque programme
C qui en a besoin en *construit* un — et la forme qu'il construit est presque
toujours celle-ci : un bloc de tas plus une longueur plus une capacité. C'est
tout le `struct DynArray` de l'étape de code : `items` pointe vers le bloc,
`len` compte les éléments en usage, `cap` compte les emplacements que le bloc
fournit. L'écart entre `len` et `cap` est exactement la marge qui rend les
ajouts bon marché.

`DaInit`, `DaPush` et `DaFree` opèrent sur cette structure et suivent une
seule convention de nommage (`Da` pour dynarray) au lieu d'un mécanisme de
langage — les structures C n'ont pas de méthodes, souvenez-vous, seulement
des fonctions qui en prennent un pointeur. `DaPush` prend un `struct Item`
**par valeur** : les vingt-quatre octets sont copiés dans l'appel et copiés à
nouveau dans le tableau, ce qui est honnête sur ce que stocker veut dire ici.
Quand le tableau est plein — `len == cap` — `DaPush` agrandit le bloc avec
`realloc` avant de stocker, en doublant la capacité : zéro à quatre, quatre à
huit, huit à seize.

`realloc` est l'appel de la bibliothèque C pour « déplacer ou étendre ce bloc
de tas », et son contrat mérite d'être mémorisé. Étant donné un bloc et une
nouvelle taille, il renvoie un bloc de cette taille contenant l'ancien contenu
jusqu'à l'ancienne taille — mais le pointeur renvoyé peut être **le même ou
différent**. Le bloc peut être étendu sur place si de l'espace libre le suit ;
sinon un bloc neuf est alloué, les octets sont copiés, et l'ancien bloc est
libéré. Dans les deux cas l'ancien pointeur ne doit plus être utilisé, et le
nouveau pointeur doit venir de la valeur de retour. En cas d'échec `realloc`
renvoie `NULL` et — c'est le tranchant — l'ancien bloc est laissé **intact et
toujours à vous**. C'est pourquoi `DaPush` stocke le résultat dans un
temporaire `p` et n'affecte qu'en cas de succès. Le une-ligne
`da->items = realloc(da->items, ...)` perd le seul pointeur vers le bloc dès
que l'échec arrive : une fuite de l'exacte espèce que la leçon 005 vous a
appris à voir. (Et `realloc(NULL, n)` est légal et veut dire `malloc(n)` —
c'est ainsi que la première croissance fonctionne avec `items` encore `NULL`.)

Pourquoi doubler la capacité plutôt que grandir d'un emplacement ? Parce que
la croissance copie tout le tableau, et le *calendrier* des croissances décide
la copie totale. Si la capacité grandit d'un à chaque fois, l'ajout numéro *n*
copie *n* éléments, et les copies somment environ n²/2 — un million d'ajouts
copient un demi-billion d'éléments. Avec le doublement, les ajouts coûteux
deviennent plus rares géométriquement : chaque élément est copié au plus
environ log n fois, et la copie totale pour *n* ajouts reste sous 2n éléments,
quel que soit leur nombre. Les ajouts individuels coûtent encore O(n) à une
croissance — mais le coût *moyen* sur toute longue série est constant. C'est
le O(1) amorti que le `list` de Python vous donnait discrètement tout le
temps.

`DaFree` libère le bloc puis rappelle `DaInit`, si bien que le tableau est
laissé vide, valide et réutilisable — une petite discipline qui garde la
structure hors de l'état « libéré mais portant encore des nombres périmés »
qui fait planter des programmes plus tard. Le pilote de l'étape de code rend
toute l'histoire visible : il ajoute dix éléments et imprime `len` et `cap`
après chaque envoi, si bien que les points de croissance apparaissent comme
les lignes où `cap` saute.

La commande de construction est inchangée depuis la leçon 007 — mêmes
options, même répertoire (`sandbox/ds-kit/`) :

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

## Étape de code

Un seul changement pour cette leçon : le dynarray remplace les expériences de
disposition dans `ds-kit.c` — `struct DynArray` avec `DaInit`, `DaPush` et
`DaFree`, croissance par `realloc` avec capacité doublée, et un pilote qui
ajoute dix éléments et rapporte `len`/`cap` à mesure qu'il grandit. Son état
final est étiqueté `lesson-008`.

```diff
diff --git a/sandbox/ds-kit/ds-kit.c b/sandbox/ds-kit/ds-kit.c
index 455e975..81f9e65 100644
--- a/sandbox/ds-kit/ds-kit.c
+++ b/sandbox/ds-kit/ds-kit.c
@@ -1,57 +1,68 @@
-// ds-kit.c — the data-structures kit of Part 0, starting with its element
-// type and the memory it lives in.
+// ds-kit.c — the data-structures kit of Part 0: a dynarray of Items that
+// grows by realloc with capacity doubling.
 //
-// Lesson 007: structs as laid-out memory — sizeof, offsetof, padding.
+// Lesson 008: realloc growth — len, cap, and why doubling wins.
 #include <stdio.h>
-#include <stddef.h>   // offsetof
-#include <stdalign.h> // alignof (C11)
+#include <stdlib.h>
 
 struct Item {
     char key[16];
     long value;
 };
 
-// The same three fields in two different orders.
-struct Scattered {
-    char tag;
-    long score;
-    char flag;
+struct DynArray {
+    struct Item *items;
+    size_t len;
+    size_t cap;
 };
 
-struct Compact {
-    char tag;
-    char flag;
-    long score;
-};
+void DaInit(struct DynArray *da)
+{
+    da->items = NULL;
+    da->len = 0;
+    da->cap = 0;
+}
+
+void DaPush(struct DynArray *da, struct Item item)
+{
+    if (da->len == da->cap) {
+        size_t newcap = da->cap ? da->cap * 2 : 4;
+        struct Item *p = realloc(da->items, newcap * sizeof *p);
+        if (p == NULL) {
+            fprintf(stderr, "DaPush: out of memory\n");
+            exit(1);
+        }
+        da->items = p;
+        da->cap = newcap;
+    }
+    da->items[da->len++] = item;
+}
+
+void DaFree(struct DynArray *da)
+{
+    free(da->items);
+    DaInit(da);
+}
 
 int main(void)
 {
-    printf("== scalars ==\n");
-    printf("sizeof(char) = %zu\n", sizeof(char));
-    printf("sizeof(long) = %zu\n", sizeof(long));
-
-    printf("== struct Item ==\n");
-    printf("sizeof(struct Item)  = %zu\n", sizeof(struct Item));
-    printf("alignof(struct Item) = %zu\n", alignof(struct Item));
-    printf("Item.key   offset %zu\n", offsetof(struct Item, key));
-    printf("Item.value offset %zu\n", offsetof(struct Item, value));
-    // key is 16 bytes, so the gap before value is its offset minus 16.
-    printf("padding before value: %zu bytes\n",
-           offsetof(struct Item, value) - 16);
-
-    printf("== same fields, two orders ==\n");
-    printf("struct Scattered { tag, score, flag }: sizeof %zu\n",
-           sizeof(struct Scattered));
-    printf("  tag   offset %zu\n", offsetof(struct Scattered, tag));
-    printf("  score offset %zu\n", offsetof(struct Scattered, score));
-    printf("  flag  offset %zu\n", offsetof(struct Scattered, flag));
-    printf("struct Compact { tag, flag, score }: sizeof %zu\n",
-           sizeof(struct Compact));
-    printf("  tag   offset %zu\n", offsetof(struct Compact, tag));
-    printf("  flag  offset %zu\n", offsetof(struct Compact, flag));
-    printf("  score offset %zu\n", offsetof(struct Compact, score));
-    printf("Scattered[10] = %zu bytes, Compact[10] = %zu bytes\n",
-           sizeof(struct Scattered[10]), sizeof(struct Compact[10]));
+    struct DynArray da;
+    DaInit(&da);
+
+    const char *keys[10] = {
+        "pear", "apple", "fig", "banana", "cherry",
+        "date", "elder", "grape", "kiwi", "lemon",
+    };
+    for (int i = 0; i < 10; ++i) {
+        struct Item item;
+        snprintf(item.key, sizeof item.key, "%s", keys[i]);
+        item.value = i;
+        DaPush(&da, item);
+        printf("len=%zu cap=%zu\n", da.len, da.cap);
+    }
+
+    printf("first=%s last=%s\n", da.items[0].key, da.items[9].key);
 
+    DaFree(&da);
     return 0;
 }
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le calendrier des croissances *(predict-the-output)*

Prédisez la machine avant qu'elle ne parle. Écrivez les onze lignes que vous
attendez de `./ds-kit` — les dix lignes `len=.. cap=..` et la dernière ligne
`first=.. last=..` — en partant d'un tableau sans aucune capacité. Puis
exécutez-le et rendez compte de chaque différence entre votre prédiction et
la sortie, en particulier quel envoi déclenche chaque réallocation et
pourquoi c'est cet envoi.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-008/ex1.md)

### Exercice 2 — Le tableau à moitié libéré *(fix-the-crash)*

Un collègue voulait une remise à zéro qui laisse le tableau réutilisable :

```c
void DaClear(struct DynArray *da)
{
    free(da->items);
    da->len = 0;
}
```

Ajoutez-la à `ds-kit.c`, appelez-la à mi-chemin de la boucle d'envois du
pilote, et continuez à envoyer après. Construit avec le sanitizer de la leçon
005 (`gcc -std=c11 -O0 -g -Wall -Wextra -fsanitize=address ds-kit.c -o ds-kit`),
l'envoi suivant rapporte un heap-use-after-free ; construit simplement, cela
peut seulement avoir l'air de fonctionner. Reproduisez le rapport du
sanitizer, puis corrigez `DaClear` pour que vider et réutiliser un tableau
soit sans fuite et sans usage-après-libération — et dites quelle ligne de
l'original a rendu l'échec possible.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-008/ex2.md)

### Exercice 3 — Doubler contre un-à-la-fois *(measure-the-performance)*

Faites de la règle de croissance un interrupteur et mesurez la différence.
Ajoutez un `#define GROW_BY_ONE 1` que vous pouvez basculer à la main : quand
il est défini, la croissance ajoute exactement un emplacement au lieu de
doubler. Ajoutez aussi des compteurs de total d'exécution pour les appels de
réallocation et pour les octets copiés des anciens blocs vers les nouveaux,
imprimés à la sortie. Envoyez 1 000 000 d'éléments sous chaque règle,
chronométrez les deux avec `time ./ds-kit > /dev/null`, et rapportez les
compteurs et les horloges. Expliquez l'écart entre les deux histoires si elles
ne sont pas d'accord.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-008/ex3.md)

### Exercice 4 — Ce que `realloc` promet vraiment *(explain-in-prose)*

Écrivez une courte explication — un paragraphe chacune — de (a) pourquoi le
doublement de capacité donne des envois O(1) amortis, en dérivant le total
d'éléments copiés pour *n* envois depuis un tableau vide sous les deux règles
de croissance ; (b) exactement ce que `realloc` garantit quand il échoue et
pourquoi `da->items = realloc(da->items, ...)` est un motif de bug même s'il
fonctionne habituellement ; (c) quand `realloc` peut renvoyer le même
pointeur et quand il ne le peut pas. Appliquez ensuite la vérification
confirmante de la solution et regardez à quelle fréquence le cas (c) arrive
réellement sur votre machine.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-008/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 007 — structures : sizeof, alignement et remplissage](lesson-007-struct-layout.md) ·
**Suivante :** [Leçon 009 — pointeurs de fonction : comparateurs et hooks](lesson-009-function-pointers.md) ·
**Étiquette de code :** [`lesson-008`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-008)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-008-dynarray.md`,
révision `f4168c6`.*

<!-- translation-source: book/lessons/part-0/lesson-008-dynarray.md @ f4168c6 -->
