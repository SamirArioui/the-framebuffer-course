# Leçon 010 — void\* : la généricité et ses peines

{{#include ../../stability-horizon.md}}

## Prose

La leçon passée, le kit triait un type d'élément. Cette leçon, il stocke
*n'importe quel* type d'élément — et le prix de cette généricité devient la
leçon. Le prix a un nom en C : `void *`, un pointeur dont le type pointé a
été effacé. Tout pointeur vers des données se convertit en `void *` et
inversement sans cérémonie — implicite à l'aller, et seulement un
transtypage pour redonner aux octets leur type au retour. Ce qu'il pointe est
inconnu par conception : vous ne pouvez pas déréférencer un `void *`, pas
faire d'arithmétique dessus, pas même savoir combien d'octets il couvre. C'est
« de la mémoire, quelque part » — une promesse que vous devez tenir
vous-même. (Deux notes de bas de page : les pointeurs de fonction ne se
convertissent *pas* en `void *` — ils sont une famille à part — et
l'arithmétique sur `void *` est une extension GCC ; le C portable enjambe par
`char *`, un octet à la fois, ce que font exactement `DaPush` et `DaAt`
maintenant.)

Regardez ce que le `DynArray` générique est devenu. Il ne contient plus
`struct Item *items` ; il contient `void *data` — un bloc d'octets — plus
`elem_size`, le pas qui dit combien d'octets occupe chaque élément. `DaPush`
prend l'élément en `const void *` et en `memcpy` `elem_size` octets dans
l'emplacement suivant. `DaAt` renvoie `void *` vers l'emplacement. `DaSort`
remet à `qsort` le bloc, le compte, le pas, et un comparateur qui est
*déjà* le type de comparateur de `qsort`. Et remarquez ce qui a disparu de la
leçon 009 : `CmpBridge` et son shim `g_cmp`. Le monde de `qsort` a toujours
été `void *` — une fois que notre tableau parle `void *` aussi, le pont est
redondant. Les transtypages n'ont pas disparu pourtant ; ils ont déménagé
dans chaque comparateur et chaque hook de visite, où l'appelant re-transtype
le `void *` vers le type qu'il sait avoir stocké.

Le pilote prouve maintenant le gain : une seule implémentation de
`DaInit`/`DaPush`/`DaSort`/`DaEach` sert à la fois un tableau de `struct Item`
et un tableau de `long`. Pas de second type de tableau, pas de tri dupliqué.
En Python ce serait un `list` ; en Ruby, un `Array` — et l'exécution
vérifierait le type de chaque élément à chaque opération et lèverait un
propre `TypeError` au premier désaccord.

C ne vérifie rien. C'est la peine. `elem_size` est un nombre que vous avez
tapé, pas un fait que le compilateur a vérifié : initialisez le tableau avec
`sizeof nums` — la *structure* — au lieu de `sizeof(long)` — l'élément — et
chaque `DaPush` copie `sizeof(struct DynArray)` octets depuis une variable de
huit octets, lisant au-delà d'un emplacement de pile. Transtypez un
emplacement `long` en `const int *` et les mêmes huit octets se réinterprètent
en deux entiers de quatre octets. Les deux programmes compilent avec
`-Wall -Wextra` et pas une plainte, parce que du point de vue du compilateur
le `void *` a effacé exactement l'information dont il aurait eu besoin pour
s'opposer. Les échecs atterrissent sur le territoire de la leçon 006 : sortie
fausse et silencieuse quand les octets ont l'air plausible, comportement
indéfini quand ils ne l'ont pas. Les conventions — « ce tableau contient des
`struct Item`, voir l'appel à `DaInit` » — sont désormais des commentaires
porteuses. C'est le contrat de généricité de C : une implémentation, zéro
vérification, toute responsabilité à vous.

Pourquoi C fait-il ainsi, après tout ? Parce que la bibliothèque standard est
bâtie dessus : `qsort`, `memcpy`, `malloc`, `fwrite` prennent tous un
`void *` et une taille. C n'a pas de généricité au sens moderne — le
`_Generic` de C11 peut dispatcher sur un nom de type mais ne peut pas
abstraire le stockage sur un — aussi la réponse uniforme du langage à «
stocker n'importe quoi » est des octets plus un pas plus une discipline.
L'alternative est ce que le kit avait avant cette leçon : une copie typée par
type d'élément. Quand le moteur de la partie 1 arrivera en C++, les
modèles feront générer ces copies par le compilateur pour vous ; d'ici là, le
pas est à vous de garder honnête.

La commande de construction est inchangée (depuis `sandbox/ds-kit/`) :

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

## Étape de code

Un seul changement pour cette leçon : `DynArray` se généralise à tout élément
— `data` en `void *` plus `elem_size`, `DaPush` copiant via `memcpy`, `DaAt`
renvoyant `void *` pour que l'appelant transtype — et le pilote se refactorise
sur le tableau générique, en ajoutant un second tableau de `long` pour montrer
une impléformation servant deux types. Son état final est étiqueté
`lesson-010`.

```diff
diff --git a/sandbox/ds-kit/ds-kit.c b/sandbox/ds-kit/ds-kit.c
index ff3ce5c..1ba1cc4 100644
--- a/sandbox/ds-kit/ds-kit.c
+++ b/sandbox/ds-kit/ds-kit.c
@@ -1,7 +1,7 @@
-// ds-kit.c — the data-structures kit of Part 0: a dynarray of Items that
-// grows by realloc, sorts by comparator, and visits by hook.
+// ds-kit.c — the data-structures kit of Part 0: one generic dynarray that
+// stores any element type as raw bytes.
 //
-// Lesson 009: function pointers — comparators and callbacks.
+// Lesson 010: void* — genericity, casting, and its silent failures.
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
@@ -12,86 +12,97 @@ struct Item {
 };
 
 struct DynArray {
-    struct Item *items;
+    void *data;
     size_t len;
     size_t cap;
+    size_t elem_size;
 };
 
-void DaInit(struct DynArray *da)
+void DaInit(struct DynArray *da, size_t elem_size)
 {
-    da->items = NULL;
+    da->data = NULL;
     da->len = 0;
     da->cap = 0;
+    da->elem_size = elem_size;
 }
 
-void DaPush(struct DynArray *da, struct Item item)
+void DaPush(struct DynArray *da, const void *elem)
 {
     if (da->len == da->cap) {
         size_t newcap = da->cap ? da->cap * 2 : 4;
-        struct Item *p = realloc(da->items, newcap * sizeof *p);
+        void *p = realloc(da->data, newcap * da->elem_size);
         if (p == NULL) {
             fprintf(stderr, "DaPush: out of memory\n");
             exit(1);
         }
-        da->items = p;
+        da->data = p;
         da->cap = newcap;
     }
-    da->items[da->len++] = item;
+    // Byte arithmetic on purpose: void* has no element size to step by.
+    char *slot = (char *)da->data + da->len * da->elem_size;
+    memcpy(slot, elem, da->elem_size);
+    ++da->len;
 }
 
-void DaFree(struct DynArray *da)
+void *DaAt(struct DynArray *da, size_t i)
 {
-    free(da->items);
-    DaInit(da);
+    char *base = da->data;
+    return base + i * da->elem_size;
 }
 
-// -- the function-pointer machinery -------------------------------------
-//
-// qsort's own comparator type is int (*)(const void *, const void *), so a
-// bridge function converts and forwards. Which comparator to forward to
-// lives in g_cmp: qsort offers no way to pass extra context along.
-
-static int (*g_cmp)(const struct Item *, const struct Item *);
-
-static int CmpBridge(const void *pa, const void *pb)
+void DaSort(struct DynArray *da, int (*cmp)(const void *, const void *))
 {
-    return g_cmp((const struct Item *)pa, (const struct Item *)pb);
+    qsort(da->data, da->len, da->elem_size, cmp);
 }
 
-void DaSort(struct DynArray *da, int (*cmp)(const struct Item *, const struct Item *))
+void DaEach(const struct DynArray *da, void (*visit)(const void *))
 {
-    g_cmp = cmp;
-    qsort(da->items, da->len, sizeof *da->items, CmpBridge);
-    g_cmp = NULL;
+    const char *base = da->data;
+    for (size_t i = 0; i < da->len; ++i)
+        visit(base + i * da->elem_size);
 }
 
-void DaEach(const struct DynArray *da, void (*visit)(const struct Item *))
+void DaFree(struct DynArray *da)
 {
-    for (size_t i = 0; i < da->len; ++i)
-        visit(&da->items[i]);
+    free(da->data);
+    DaInit(da, da->elem_size);
 }
 
 // -- the callbacks this driver supplies --------------------------------
+//
+// Every one of them casts: the array is generic now, so the types are the
+// caller's job.
 
-static int CmpByKey(const struct Item *a, const struct Item *b)
+static int CmpByKey(const void *pa, const void *pb)
 {
+    const struct Item *a = (const struct Item *)pa;
+    const struct Item *b = (const struct Item *)pb;
     return strcmp(a->key, b->key);
 }
 
-static int CmpByValue(const struct Item *a, const struct Item *b)
+static int CmpLong(const void *pa, const void *pb)
 {
-    return (a->value > b->value) - (a->value < b->value);
+    const long *a = (const long *)pa;
+    const long *b = (const long *)pb;
+    return (*a > *b) - (*a < *b);
 }
 
-static void PrintItem(const struct Item *it)
+static void PrintItem(const void *pe)
 {
+    const struct Item *it = (const struct Item *)pe;
     printf("%s %ld\n", it->key, it->value);
 }
 
+static void PrintLong(const void *pe)
+{
+    const long *v = (const long *)pe;
+    printf("%ld\n", *v);
+}
+
 int main(void)
 {
-    struct DynArray da;
-    DaInit(&da);
+    struct DynArray items;
+    DaInit(&items, sizeof(struct Item));
 
     const char *keys[10] = {
         "pear", "apple", "fig", "banana", "cherry",
@@ -101,20 +112,24 @@ int main(void)
         struct Item item;
         snprintf(item.key, sizeof item.key, "%s", keys[i]);
         item.value = i;
-        DaPush(&da, item);
+        DaPush(&items, &item);
     }
 
-    printf("before:\n");
-    DaEach(&da, PrintItem);
+    DaSort(&items, CmpByKey);
+    printf("items sorted by key:\n");
+    DaEach(&items, PrintItem);
 
-    DaSort(&da, CmpByKey);
-    printf("sorted by key:\n");
-    DaEach(&da, PrintItem);
+    struct DynArray nums;
+    DaInit(&nums, sizeof(long));
+    long vals[5] = { 50, 30, 10, 40, 20 };
+    for (int i = 0; i < 5; ++i)
+        DaPush(&nums, &vals[i]);
 
-    DaSort(&da, CmpByValue);
-    printf("sorted by value:\n");
-    DaEach(&da, PrintItem);
+    DaSort(&nums, CmpLong);
+    printf("numbers sorted:\n");
+    DaEach(&nums, PrintLong);
 
-    DaFree(&da);
+    DaFree(&nums);
+    DaFree(&items);
     return 0;
 }
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Les mêmes octets, autrement *(predict-the-output)*

Ajoutez une petite expérience au pilote : un tableau dont `elem_size` est
`sizeof(long)`, contenant les deux valeurs 7 et 42, relues à travers un
transtypage en `const int *` — imprimez *les deux* `int` de chaque élément,
deux lignes de deux nombres. Prédisez les quatre nombres exacts que votre
machine imprimera avant de construire ; puis exécutez et expliquez d'où vient
chaque nombre, et quelle serait la prédiction sur une machine avec l'ordre
des octets opposé.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-010/ex1.md)

### Exercice 2 — La mauvaise taille *(fix-the-crash)*

Un collègue a écrit le tableau de nombres comme cela, et le programme meurt
sous le sanitizer de la leçon 005 :

```c
    struct DynArray nums;
    DaInit(&nums, sizeof nums);
    long v = 7;
    DaPush(&nums, &v);
```

Construisez avec `-fsanitize=address` et lisez ce que le sanitizer dit de
l'accès fautif. Corrigez l'initialisation pour que le tableau contienne
vraiment des `long`, et expliquez pourquoi l'erreur n'a produit aucune
plainte du compilateur même avec `-Wall -Wextra` — qu'aurait eu besoin de
savoir le compilateur ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-010/ex2.md)

### Exercice 3 — Supprimer au milieu *(extend-the-code)*

Ajoutez `DaRemoveAt` au kit : étant donné un indice, il supprime cet élément
et fait glisser les suivants d'un emplacement vers le bas, en réduisant `len`
— aucune réallocation nécessaire. Le glissement est de la chirurgie d'octets
pure avec `memmove`, et l'indice doit être validé. Démontrez-le depuis le
pilote : envoyez cinq `long` et supprimez l'élément à l'indice 2, en
imprimant les survivants. Dites, dans un commentaire ou une phrase,
pourquoi `memmove` et pas `memcpy` pour le glissement.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-010/ex3.md)

### Exercice 4 — Le contrat, par écrit *(explain-in-prose)*

Écrivez une courte explication — un paragraphe chacune — de (a) deux façons
distinctes dont un conteneur basé sur `void *` peut corrompre silencieusement
des données, une via `elem_size` et une via un mauvais transtypage, et
pourquoi chacune échappe aux vérifications du compilateur ; (b) comment les
deux mêmes erreures se présentent en Python ou Ruby — ce qui remplace la
corruption silencieuse là-bas, et ce que cela coûte à l'exécution ; (c)
pourquoi les accesseurs du kit renvoient des pointeurs *à l'intérieur* du bloc
et ce que cela implique sur `realloc` et sur la durée de validité d'un
résultat de `DaAt`. Appliquez ensuite la vérification confirmante de la
solution et voyez la machinerie au niveau des octets à l'œuvre.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-010/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 009 — pointeurs de fonction : comparateurs et hooks](lesson-009-function-pointers.md) ·
**Suivante :** [Leçon 011 — la table de hachage : hachage, seaux, recherche](lesson-011-hashtable.md) ·
**Étiquette de code :** [`lesson-010`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-010)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-010-void-pointer.md`, révision `60d447b`.*

<!-- translation-source: book/lessons/part-0/lesson-010-void-pointer.md @ 60d447b -->
