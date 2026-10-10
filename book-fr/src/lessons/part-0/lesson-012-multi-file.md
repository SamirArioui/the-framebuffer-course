# Leçon 012 — constructions multi-fichiers : unités de traduction et édition de liens

{{#include ../../stability-horizon.md}}

## Prose

Le kit est terminé — aujourd'hui il cesse d'être un seul fichier. `ds-kit.c`
a grandi à travers cinq leçons en un dynarray, une table de hachage, et un
pilote, et les fichiers uniques ne montent pas : personne ne peut tenir 300
lignes de préoccupations mêlées dans un seul écran, et chaque modification
recompile tout. Alors l'étape de code découpe le programme en `dynarray.h`,
`dynarray.c`, `hashtable.h`, `hashtable.c`, et `main.c` — à plat dans
`sandbox/ds-kit/`, sans sous-répertoires — et le programme est octet par
octet le même après. Ce qui change est comment il est *construit*, et cette
machinerie est le sujet de cette leçon : unités de traduction, fichiers objet,
tables de symboles, et l'éditeur de liens.

L'unité de travail du compilateur est l'**unité de traduction** : un fichier
`.c` plus tout ce que ses directives `#include` traînent avec eux.
`#include "dynarray.h"` n'est pas un import — c'est du collage textuel, fait
par le préprocesseur avant que le compilateur ne tourne jamais. `dynarray.c`
devient une unité de traduction contenant la structure et les prototypes de
l'en-tête suivis de l'implémentation ; `main.c` en devient une autre
contenant les deux en-têtes et le pilote. Crucialement, chaque unité de
traduction est compilée **seule** : le compilateur ne voit jamais deux
fichiers `.c` à la fois, et il vérifie chaque appel dans `hashtable.c` contre
les *déclarations* de `dynarray.h`, faisant confiance à quelqu'un, un jour,
pour fournir les définitions. Cette confiance est le rôle de l'en-tête : un
contrat contre lequel chaque fichier est vérifié, pas une comptabilité de
gestionnaire de fichiers.

Lancez `gcc -c` sur une unité de traduction et la sortie est un **fichier
objet** — du code machine qui n'est pas encore un programme :

```
$ gcc -std=c11 -O0 -g -Wall -Wextra -c dynarray.c hashtable.c main.c
$ file dynarray.o
dynarray.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), with debug_info, not stripped
```

*Relocatable* est le mot : le code est compilé mais ses adresses ne sont pas
finies, et les appels vers d'autres unités de traduction sont des questions
ouvertes. Chaque objet porte une **table de symboles** — sa liste de noms
qu'il définit et de noms dont il a besoin — et `nm` l'imprime :

```
$ nm hashtable.o
                 U DaAt
                 U DaFree
                 U DaInit
                 U DaPush
00000000000002dd T HtEntries
000000000000036a T HtFree
00000000000001a8 T HtGet
0000000000000000 t HtHash
0000000000000046 T HtInit
0000000000000101 t HtLookup
00000000000001eb T HtPut
                 U __stack_chk_fail
                 U exit
                 U free
                 U fwrite
                 U malloc
                 U snprintf
                 U stderr
                 U strcmp
```

Lisez les lettres, parce qu'elles sont tout le jeu. `T` est un symbole que
cet objet définit et exporte — les fonctions `Ht*` que toute unité de
traduction peut appeler. `t` (minuscule) est un symbole défini mais **non
exporté** : `HtHash` et `HtLookup` sont `static`, ce qui leur donne un *lien
interne* — visible seulement dans `hashtable.c`. `U` est un symbole dont cet
objet *a besoin* : les appels `Da*` promis-livrés par `dynarray.o`, et
`malloc`, `strcmp`, `stderr` et leurs semblables promis par la bibliothèque
C. Rien dans `hashtable.o` ne dit où ceux-là vivent. C'est le travail de
l'éditeur de liens.

L'**éditeur de liens** est le dernier étage de `gcc`, et il fait exactement
trois choses qui valent d'être retenues : il résout chaque `U` contre un `T`
quelconque — l'appel de `main.o` à `DaSort` contre la définition de
`dynarray.o`, `strcmp` contre libc — il fixe les adresses finales maintenant
que toutes les pièces sont adjacentes, et il enregistre `main` comme point
d'entrée. Jusqu'à ce qu'il tourne, il n'y a pas de programme ; quand il
échoue, il échoue sur des *noms*, pas sur du code. La construction en une
ligne que vous avez exécutée tout le temps cache cette couture —
`gcc *.c -o ds-kit` compile chaque unité de traduction et lie d'un même
souffle. La forme en deux temps la montre :

```
gcc -std=c11 -O0 -g -Wall -Wextra -c dynarray.c hashtable.c main.c
gcc dynarray.o hashtable.o main.o -o ds-kit
```

Les deux produisent le même programme à partir des mêmes cinq fichiers —
chaque exécution de cette leçon a été vérifiée contre les deux.

L'échec dont l'éditeur de liens est le plus célèbre est celui qui vaut la
peine d'être reproduit aujourd'hui. Mettez un auxiliaire simple (non
`static`) — disons `ReportOOM`, imprimant un message et sortant — dans
*les deux* `dynarray.c` et `hashtable.c`, et chaque fichier compile propre :
le compilateur ne voit qu'une définition par unité de traduction. La liaison
meurt ensuite :

```
/usr/bin/ld: hashtable.o: in function `ReportOOM':
hashtable.c:(.text+0x46): multiple definition of `ReportOOM'; dynarray.o:dynarray.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

(Avec `-g` dans les options, l'éditeur de liens peut nommer les lignes de
source fautives au lieu de décalages de code machine.) Deux fichiers
*exportent* chacun le même nom, et un nom externe ne peut être défini qu'une
fois par programme. La correction est le mot-clé de la leçon : donnez à
chaque fichier sa propre copie `static` — lien interne, pas de collision —
ou factorisez l'auxiliaire dans une seule unité de traduction et déclarez-le
dans un en-tête exactement une fois. `static` pour les auxiliaires utilisés
dans un seul fichier ; un en-tête partagé pour les auxiliaires réellement
utilisés entre fichiers. C'est toute la politique d'extériorité du C.

Un dernier détail de contrat que la découpe expose : les **gardes d'inclusion
(include guards)**. Puisque `#include` colle du texte, un en-tête collé deux
fois dans une même unité de traduction définirait `struct DynArray` deux fois
— une erreur dure de « redéfinition ». Le sandwich
`#ifndef DYNARRAY_H` / `#define DYNARRAY_H` / `#endif` rend le second collage
vide ; chaque en-tête du kit en porte un. (`#pragma once` fait le même
travail en une ligne et fonctionne sur chaque compilateur que vous
rencontrerez, mais la garde est l'orthographe portable.) Et regardez ce que
`hashtable.h` n'inclut *pas* : il mentionne `struct DynArray` seulement à
travers un pointeur, aussi une simple déclaration en avant —
`struct DynArray;` — suffit. Un type incomplet suffit pour les pointeurs ;
seul `hashtable.c`, qui marche les chaînes, inclut `dynarray.h` en entier.
Incluez ce que vous utilisez, déclarez ce à quoi vous ne faites que pointer.

Le kit est terminé. Il a enseigné la mémoire de la machine (007), la
croissance (008), les callbacks (009), les types effacés (010), le hachage
(011), et maintenant la construction elle-même — la boîte à outils sur
laquelle le moteur de la partie 1 s'appuiera chaque jour. Le prochain
programme de la partie 0 se tourne vers les octets bruts et les pixels.

Commandes de construction (depuis `sandbox/ds-kit/`) :

```
gcc -std=c11 -O0 -g -Wall -Wextra *.c -o ds-kit
```

ou, en montrant la couture :

```
gcc -std=c11 -O0 -g -Wall -Wextra -c dynarray.c hashtable.c main.c
gcc dynarray.o hashtable.o main.o -o ds-kit
```

## Étape de code

Un seul changement pour cette leçon : la découpe. `ds-kit.c` est supprimé et
ses parties deviennent `dynarray.h`/`dynarray.c` (le tableau générique),
`hashtable.h`/`hashtable.c` (la table — son en-tête déclarant en avant
`struct DynArray`), et `main.c` (le pilote ; le seul fichier avec `main`).
Son état final est étiqueté `lesson-012`.

```diff
diff --git a/sandbox/ds-kit/ds-kit.c b/sandbox/ds-kit/ds-kit.c
deleted file mode 100644
index 3b41f76..0000000
--- a/sandbox/ds-kit/ds-kit.c
+++ /dev/null
@@ -1,232 +0,0 @@
-// ds-kit.c — the data-structures kit of Part 0: a hashtable over the
-// generic dynarray, and a word-frequency driver.
-//
-// Lesson 011: hashing, buckets, collisions — and lookup that is O(1) on
-// average.
-#include <stdio.h>
-#include <stdlib.h>
-#include <string.h>
-#include <stdint.h>
-#include <ctype.h>
-
-struct Item {
-    char key[16];
-    long value;
-};
-
-struct DynArray {
-    void *data;
-    size_t len;
-    size_t cap;
-    size_t elem_size;
-};
-
-void DaInit(struct DynArray *da, size_t elem_size)
-{
-    da->data = NULL;
-    da->len = 0;
-    da->cap = 0;
-    da->elem_size = elem_size;
-}
-
-void DaPush(struct DynArray *da, const void *elem)
-{
-    if (da->len == da->cap) {
-        size_t newcap = da->cap ? da->cap * 2 : 4;
-        void *p = realloc(da->data, newcap * da->elem_size);
-        if (p == NULL) {
-            fprintf(stderr, "DaPush: out of memory\n");
-            exit(1);
-        }
-        da->data = p;
-        da->cap = newcap;
-    }
-    // Byte arithmetic on purpose: void* has no element size to step by.
-    char *slot = (char *)da->data + da->len * da->elem_size;
-    memcpy(slot, elem, da->elem_size);
-    ++da->len;
-}
-
-void *DaAt(struct DynArray *da, size_t i)
-{
-    char *base = da->data;
-    return base + i * da->elem_size;
-}
-
-void DaSort(struct DynArray *da, int (*cmp)(const void *, const void *))
-{
-    qsort(da->data, da->len, da->elem_size, cmp);
-}
-
-void DaEach(const struct DynArray *da, void (*visit)(const void *))
-{
-    const char *base = da->data;
-    for (size_t i = 0; i < da->len; ++i)
-        visit(base + i * da->elem_size);
-}
-
-void DaFree(struct DynArray *da)
-{
-    free(da->data);
-    DaInit(da, da->elem_size);
-}
-
-// -- the hashtable ------------------------------------------------------
-//
-// Buckets are an array of chains, and a chain is just a dynarray of
-// struct Item. FNV-1a turns a key into a 32-bit hash; the remainder modulo
-// nbuckets picks the bucket; the chain handles collisions.
-
-struct HashTable {
-    struct DynArray *chains;
-    size_t nbuckets;
-    size_t len;
-};
-
-static uint32_t HtHash(const char *s)
-{
-    uint32_t h = 2166136261u; // FNV-1a 32-bit offset basis
-    while (*s != '\0') {
-        h ^= (unsigned char)*s++;
-        h *= 16777619u; // FNV-1a 32-bit prime
-    }
-    return h;
-}
-
-void HtInit(struct HashTable *ht, size_t nbuckets)
-{
-    ht->chains = malloc(nbuckets * sizeof *ht->chains);
-    if (ht->chains == NULL) {
-        fprintf(stderr, "HtInit: out of memory\n");
-        exit(1);
-    }
-    ht->nbuckets = nbuckets;
-    ht->len = 0;
-    for (size_t b = 0; b < nbuckets; ++b)
-        DaInit(&ht->chains[b], sizeof(struct Item));
-}
-
-static struct Item *HtLookup(const struct HashTable *ht, const char *key)
-{
-    size_t b = HtHash(key) % ht->nbuckets;
-    struct DynArray *chain = &ht->chains[b];
-    for (size_t i = 0; i < chain->len; ++i) {
-        struct Item *it = (struct Item *)DaAt(chain, i);
-        if (strcmp(it->key, key) == 0)
-            return it;
-    }
-    return NULL;
-}
-
-long *HtGet(const struct HashTable *ht, const char *key)
-{
-    struct Item *it = HtLookup(ht, key);
-    return it != NULL ? &it->value : NULL;
-}
-
-void HtPut(struct HashTable *ht, const char *key, long value)
-{
-    struct Item *it = HtLookup(ht, key);
-    if (it != NULL) {
-        it->value = value;
-        return;
-    }
-    struct Item item;
-    snprintf(item.key, sizeof item.key, "%s", key); // keys longer than 15 are truncated
-    item.value = value;
-    size_t b = HtHash(key) % ht->nbuckets;
-    DaPush(&ht->chains[b], &item);
-    ++ht->len;
-}
-
-void HtEntries(const struct HashTable *ht, struct DynArray *out)
-{
-    for (size_t b = 0; b < ht->nbuckets; ++b) {
-        struct DynArray *chain = &ht->chains[b];
-        for (size_t i = 0; i < chain->len; ++i)
-            DaPush(out, DaAt(chain, i));
-    }
-}
-
-void HtFree(struct HashTable *ht)
-{
-    for (size_t b = 0; b < ht->nbuckets; ++b)
-        DaFree(&ht->chains[b]);
-    free(ht->chains);
-    ht->chains = NULL;
-    ht->nbuckets = 0;
-    ht->len = 0;
-}
-
-// -- the driver: word frequencies ---------------------------------------
-
-static int CmpByKey(const void *pa, const void *pb)
-{
-    const struct Item *a = (const struct Item *)pa;
-    const struct Item *b = (const struct Item *)pb;
-    return strcmp(a->key, b->key);
-}
-
-static void PrintItem(const void *pe)
-{
-    const struct Item *it = (const struct Item *)pe;
-    printf("%s %ld\n", it->key, it->value);
-}
-
-static void CountWord(struct HashTable *ht, const char *word)
-{
-    long *p = HtGet(ht, word);
-    if (p != NULL)
-        ++*p;
-    else
-        HtPut(ht, word, 1);
-}
-
-static void CountWords(struct HashTable *ht, FILE *f)
-{
-    char word[16];
-    size_t n = 0;
-    int c;
-    while ((c = fgetc(f)) != EOF) {
-        if (isalnum((unsigned char)c)) {
-            if (n + 1 < sizeof word)
-                word[n++] = (char)tolower((unsigned char)c);
-        } else if (n > 0) {
-            word[n] = '\0';
-            CountWord(ht, word);
-            n = 0;
-        }
-    }
-    if (n > 0) {
-        word[n] = '\0';
-        CountWord(ht, word);
-    }
-}
-
-int main(int argc, char **argv)
-{
-    if (argc != 2) {
-        fprintf(stderr, "usage: %s FILE\n", argv[0]);
-        return 1;
-    }
-    FILE *f = fopen(argv[1], "rb");
-    if (f == NULL) {
-        fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[1]);
-        return 1;
-    }
-
-    struct HashTable ht;
-    HtInit(&ht, 1024);
-    CountWords(&ht, f);
-    fclose(f);
-
-    struct DynArray entries;
-    DaInit(&entries, sizeof(struct Item));
-    HtEntries(&ht, &entries);
-    DaSort(&entries, CmpByKey);
-    DaEach(&entries, PrintItem);
-
-    DaFree(&entries);
-    HtFree(&ht);
-    return 0;
-}
diff --git a/sandbox/ds-kit/dynarray.c b/sandbox/ds-kit/dynarray.c
new file mode 100644
index 0000000..d1182f0
--- /dev/null
+++ b/sandbox/ds-kit/dynarray.c
@@ -0,0 +1,58 @@
+// dynarray.c — the generic dynarray's implementation.
+//
+// Lesson 012: one translation unit, one part of the kit. The header is
+// included first so the compiler checks this file against the contract.
+#include "dynarray.h"
+#include <stdio.h>
+#include <stdlib.h>
+#include <string.h>
+
+void DaInit(struct DynArray *da, size_t elem_size)
+{
+    da->data = NULL;
+    da->len = 0;
+    da->cap = 0;
+    da->elem_size = elem_size;
+}
+
+void DaPush(struct DynArray *da, const void *elem)
+{
+    if (da->len == da->cap) {
+        size_t newcap = da->cap ? da->cap * 2 : 4;
+        void *p = realloc(da->data, newcap * da->elem_size);
+        if (p == NULL) {
+            fprintf(stderr, "DaPush: out of memory\n");
+            exit(1);
+        }
+        da->data = p;
+        da->cap = newcap;
+    }
+    // Byte arithmetic on purpose: void* has no element size to step by.
+    char *slot = (char *)da->data + da->len * da->elem_size;
+    memcpy(slot, elem, da->elem_size);
+    ++da->len;
+}
+
+void *DaAt(struct DynArray *da, size_t i)
+{
+    char *base = da->data;
+    return base + i * da->elem_size;
+}
+
+void DaSort(struct DynArray *da, int (*cmp)(const void *, const void *))
+{
+    qsort(da->data, da->len, da->elem_size, cmp);
+}
+
+void DaEach(const struct DynArray *da, void (*visit)(const void *))
+{
+    const char *base = da->data;
+    for (size_t i = 0; i < da->len; ++i)
+        visit(base + i * da->elem_size);
+}
+
+void DaFree(struct DynArray *da)
+{
+    free(da->data);
+    DaInit(da, da->elem_size);
+}
diff --git a/sandbox/ds-kit/dynarray.h b/sandbox/ds-kit/dynarray.h
new file mode 100644
index 0000000..404f4de
--- /dev/null
+++ b/sandbox/ds-kit/dynarray.h
@@ -0,0 +1,24 @@
+// dynarray.h — the generic dynarray: any element type as bytes + a stride.
+//
+// Lesson 012: a header is a contract between translation units. Everything
+// here is what callers may rely on: the struct's shape and the Da* calls.
+#ifndef DYNARRAY_H
+#define DYNARRAY_H
+
+#include <stddef.h> // size_t
+
+struct DynArray {
+    void *data;
+    size_t len;
+    size_t cap;
+    size_t elem_size;
+};
+
+void DaInit(struct DynArray *da, size_t elem_size);
+void DaPush(struct DynArray *da, const void *elem);
+void *DaAt(struct DynArray *da, size_t i);
+void DaSort(struct DynArray *da, int (*cmp)(const void *, const void *));
+void DaEach(const struct DynArray *da, void (*visit)(const void *));
+void DaFree(struct DynArray *da);
+
+#endif // DYNARRAY_H
diff --git a/sandbox/ds-kit/hashtable.c b/sandbox/ds-kit/hashtable.c
new file mode 100644
index 0000000..faac93c
--- /dev/null
+++ b/sandbox/ds-kit/hashtable.c
@@ -0,0 +1,86 @@
+// hashtable.c — the hashtable's implementation: FNV-1a hashing, bucket
+// lookup, chained collision handling over the kit's own dynarray.
+//
+// Lesson 012: one translation unit. static keeps HtHash and HtLookup
+// private to this file — internal linkage, invisible to the linker.
+#include "hashtable.h"
+#include "dynarray.h"
+#include <stdio.h>
+#include <stdlib.h>
+#include <string.h>
+#include <stdint.h>
+
+static uint32_t HtHash(const char *s)
+{
+    uint32_t h = 2166136261u; // FNV-1a 32-bit offset basis
+    while (*s != '\0') {
+        h ^= (unsigned char)*s++;
+        h *= 16777619u; // FNV-1a 32-bit prime
+    }
+    return h;
+}
+
+void HtInit(struct HashTable *ht, size_t nbuckets)
+{
+    ht->chains = malloc(nbuckets * sizeof *ht->chains);
+    if (ht->chains == NULL) {
+        fprintf(stderr, "HtInit: out of memory\n");
+        exit(1);
+    }
+    ht->nbuckets = nbuckets;
+    ht->len = 0;
+    for (size_t b = 0; b < nbuckets; ++b)
+        DaInit(&ht->chains[b], sizeof(struct Item));
+}
+
+static struct Item *HtLookup(const struct HashTable *ht, const char *key)
+{
+    size_t b = HtHash(key) % ht->nbuckets;
+    struct DynArray *chain = &ht->chains[b];
+    for (size_t i = 0; i < chain->len; ++i) {
+        struct Item *it = (struct Item *)DaAt(chain, i);
+        if (strcmp(it->key, key) == 0)
+            return it;
+    }
+    return NULL;
+}
+
+long *HtGet(const struct HashTable *ht, const char *key)
+{
+    struct Item *it = HtLookup(ht, key);
+    return it != NULL ? &it->value : NULL;
+}
+
+void HtPut(struct HashTable *ht, const char *key, long value)
+{
+    struct Item *it = HtLookup(ht, key);
+    if (it != NULL) {
+        it->value = value;
+        return;
+    }
+    struct Item item;
+    snprintf(item.key, sizeof item.key, "%s", key); // keys longer than 15 are truncated
+    item.value = value;
+    size_t b = HtHash(key) % ht->nbuckets;
+    DaPush(&ht->chains[b], &item);
+    ++ht->len;
+}
+
+void HtEntries(const struct HashTable *ht, struct DynArray *out)
+{
+    for (size_t b = 0; b < ht->nbuckets; ++b) {
+        struct DynArray *chain = &ht->chains[b];
+        for (size_t i = 0; i < chain->len; ++i)
+            DaPush(out, DaAt(chain, i));
+    }
+}
+
+void HtFree(struct HashTable *ht)
+{
+    for (size_t b = 0; b < ht->nbuckets; ++b)
+        DaFree(&ht->chains[b]);
+    free(ht->chains);
+    ht->chains = NULL;
+    ht->nbuckets = 0;
+    ht->len = 0;
+}
diff --git a/sandbox/ds-kit/hashtable.h b/sandbox/ds-kit/hashtable.h
new file mode 100644
index 0000000..66c3f8b
--- /dev/null
+++ b/sandbox/ds-kit/hashtable.h
@@ -0,0 +1,31 @@
+// hashtable.h — the hashtable: hashed string keys, chained buckets.
+//
+// Lesson 012: a header is a contract. Note what is NOT here: dynarray.h.
+// The header only mentions struct DynArray through a pointer, so a forward
+// declaration is enough — callers that need the full type include
+// dynarray.h themselves.
+#ifndef HASHTABLE_H
+#define HASHTABLE_H
+
+#include <stddef.h> // size_t
+
+struct DynArray; // incomplete type: pointers to it are all this file needs
+
+struct Item {
+    char key[16];
+    long value;
+};
+
+struct HashTable {
+    struct DynArray *chains;
+    size_t nbuckets;
+    size_t len;
+};
+
+void HtInit(struct HashTable *ht, size_t nbuckets);
+long *HtGet(const struct HashTable *ht, const char *key);
+void HtPut(struct HashTable *ht, const char *key, long value);
+void HtEntries(const struct HashTable *ht, struct DynArray *out);
+void HtFree(struct HashTable *ht);
+
+#endif // HASHTABLE_H
diff --git a/sandbox/ds-kit/main.c b/sandbox/ds-kit/main.c
new file mode 100644
index 0000000..7e153f5
--- /dev/null
+++ b/sandbox/ds-kit/main.c
@@ -0,0 +1,81 @@
+// main.c — the driver: word frequencies over the file named on argv,
+// printed sorted by key.
+//
+// Lesson 012: the program itself. This is the only file with main.
+#include "dynarray.h"
+#include "hashtable.h"
+#include <stdio.h>
+#include <stdlib.h>
+#include <string.h>
+#include <ctype.h>
+
+static int CmpByKey(const void *pa, const void *pb)
+{
+    const struct Item *a = (const struct Item *)pa;
+    const struct Item *b = (const struct Item *)pb;
+    return strcmp(a->key, b->key);
+}
+
+static void PrintItem(const void *pe)
+{
+    const struct Item *it = (const struct Item *)pe;
+    printf("%s %ld\n", it->key, it->value);
+}
+
+static void CountWord(struct HashTable *ht, const char *word)
+{
+    long *p = HtGet(ht, word);
+    if (p != NULL)
+        ++*p;
+    else
+        HtPut(ht, word, 1);
+}
+
+static void CountWords(struct HashTable *ht, FILE *f)
+{
+    char word[16];
+    size_t n = 0;
+    int c;
+    while ((c = fgetc(f)) != EOF) {
+        if (isalnum((unsigned char)c)) {
+            if (n + 1 < sizeof word)
+                word[n++] = (char)tolower((unsigned char)c);
+        } else if (n > 0) {
+            word[n] = '\0';
+            CountWord(ht, word);
+            n = 0;
+        }
+    }
+    if (n > 0) {
+        word[n] = '\0';
+        CountWord(ht, word);
+    }
+}
+
+int main(int argc, char **argv)
+{
+    if (argc != 2) {
+        fprintf(stderr, "usage: %s FILE\n", argv[0]);
+        return 1;
+    }
+    FILE *f = fopen(argv[1], "rb");
+    if (f == NULL) {
+        fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[1]);
+        return 1;
+    }
+
+    struct HashTable ht;
+    HtInit(&ht, 1024);
+    CountWords(&ht, f);
+    fclose(f);
+
+    struct DynArray entries;
+    DaInit(&entries, sizeof(struct Item));
+    HtEntries(&ht, &entries);
+    DaSort(&entries, CmpByKey);
+    DaEach(&entries, PrintItem);
+
+    DaFree(&entries);
+    HtFree(&ht);
+    return 0;
+}
```

## Exercices

Trois courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — L'auxiliaire qui est entré en collision *(fix-the-crash)*

Le scénario `ReportOOM` de la plongée est désormais le vôtre. Ajoutez un
auxiliaire dans `dynarray.c` et `hashtable.c` — `void ReportOOM(const char *who)`,
imprimant `who` et « out of memory » sur `stderr` et sortant avec le statut 1
— comme fonction simple, non `static`, et servez-vous-en depuis `DaPush` et
`HtInit` à la place de leur gestion en ligne des manques de mémoire. Chaque
fichier compilera propre ; la liaison, non. Reproduisez la plainte de
l'éditeur de liens, puis corrigez l'échec pour que les deux fichiers gardent
leur auxiliaire et que le programme se lie — et dites laquelle des deux
corrections légales vous avez choisie et pourquoi.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-012/ex1.md)

### Exercice 2 — Statistiques de table *(extend-the-code)*

Donnez au kit une fonction de plus, ajoutée à travers le contrat de l'en-tête
de bout en bout : `HtStats`, déclarée dans `hashtable.h`, définie dans
`hashtable.c`, appelée depuis `main.c` après le comptage. Elle imprime sur
`stderr` une ligne unique avec le nombre d'entrées, le nombre de seaux,
combien de seaux sont vides, et la chaîne la plus longue — l'image du facteur
de charge de la leçon 011 en quatre nombres. Lancez-la sur un vrai fichier
texte et vérifiez les nombres contre ce que vous attendez.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-012/ex2.md)

### Exercice 3 — La lettre minuscule *(predict-the-output)*

`nm hashtable.o` montre `HtHash` comme un `t` minuscule tandis que `HtGet`
est un `T` majuscule. Prédisez ce que `nm` rapporte pour `HtHash` après que
vous avez supprimé le mot-clé `static` de sa définition — et si le programme
se construit et tourne toujours. Puis faites le changement d'un mot, relancez
`nm`, et réconciliez les deux sorties avec les règles de liaison de cette
leçon.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-012/ex3.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 011 — la table de hachage : hachage, seaux, recherche](lesson-011-hashtable.md) ·
**Suivante :** [Leçon 013 — octets bruts et formats de pixel](lesson-013-raw-bytes.md) ·
**Étiquette de code :** [`lesson-012`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-012)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-012-multi-file.md`, révision `e5cc4fc`.*

<!-- translation-source: book/lessons/part-0/lesson-012-multi-file.md @ e5cc4fc -->
