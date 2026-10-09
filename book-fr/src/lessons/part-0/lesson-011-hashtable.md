# Leçon 011 — la table de hachage : hachage, seaux, recherche

{{#include ../../stability-horizon.md}}

## Prose

Cette leçon complète le kit et fait de `ds-kit` son programme final : un
compteur de fréquences de mots qui lit un fichier texte nommé sur la ligne de
commande et imprime la fréquence de chaque mot, triée par clé. Le composant
qui le rend possible est la dernière structure de données de la partie 0 — la
**table de hachage** — et elle est assemblée presque entièrement à partir de
pièces que vous avez déjà construites.

Le problème qu'elle résout est la recherche. Un tableau trouve l'élément *i*
en un pas mais ne trouve *par clé* qu'en marchant — O(n). Gardez le tableau
trié et la recherche binaire trouve les clés en O(log n), mais chaque
insertion re-gagne cet ordre. Une table de hachage répond à « trouver par clé
» en O(1) en moyenne et insère tout aussi bon marché. L'astuce est de
*calculer* où vit une clé au lieu de la chercher : une **fonction de hachage**
transforme la clé en un nombre, le nombre choisit un **seau**, et la valeur
attend dans ce seau.

Regardez `struct HashTable` et l'échafaudage de l'astuce est visible :
`chains` est un tableau de seaux, `nbuckets` les compte, `len` compte les
entrées stockées. Chaque seau contient une **chaîne** d'entrées — et une
chaîne n'est rien d'autre que le `DynArray` générique du kit, contenant des
`struct Item` (la paire `key[16]`/`value` qui est l'élément du kit depuis la
leçon 007). La table de hachage est le dynarray utilisé une fois par seau,
plus une fonction de hachage et une politique de recherche. Les programmes C
sont construits à partir d'exactement cette sorte de réutilisation.

La fonction de hachage ici est **FNV-1a**, un des petits classiques : partez
d'un accumulateur 32 bits sur la base de décalage `2166136261u`, et pour
chaque octet de la clé faites un ou exclusif de l'octet dans l'accumulateur,
puis multipliez par le premier `16777619u`. Le ou exclusif mélange un octet
dedans ; la multiplication étale son influence sur tout l'accumulateur (cet
étalement est ce qui fait que `cat` et `act` atterrissent loin l'un de
l'autre malgré leurs lettres communes). Le type est `uint32_t` de
`<stdint.h>` exprès : l'arithmétique non signée retombe modulo 2³² avec la
bénédiction de la norme — contrairement au débordement signé, qui est le
comportement indéfini de la leçon 006. Et la boucle consomme *chaque*
caractère : un hachage qui ignore un octet traite `catalog` et `cataloo`
comme la même clé pour toujours. (FNV-1a est rapide et bien distribué, pas
cryptographique — personne ici ne se défend contre un attaquant qui choisit
les clés.)

`HtLookup` montre ce qu'est une recherche : hacher la clé, prendre
`hash % nbuckets` pour choisir le seau, puis marcher dans la chaîne de ce
seau en comparant les clés avec `strcmp`. La comparaison est la partie à
laquelle réfléchir : des hachages égaux ne veulent **pas** dire des clés
égales — des clés différentes qui atterrissent dans le même seau est une
**collision**, et la chaîne est la façon de gérer les collisions. `HtPut`
cherche d'abord : une clé existante met à jour la valeur de l'entrée ; une
nouvelle clé est copiée dans la chaîne comme un `struct Item` neuf. `HtGet`
cherche et remet un pointeur vers la valeur à l'intérieur de la table — `NULL`
quand la clé est absente — ce qui fait que la boucle de comptage tient en
trois lignes.

Pourquoi est-ce O(1) *en moyenne* ? Parce que la chaîne moyenne a une
longueur `len / nbuckets` — le **facteur de charge** de la table — et qu'une
recherche marche une chaîne. Avec 1024 seaux et quelques milliers de mots,
les chaînes font une poignée d'entrées, et « marcher une poignée » est une
constante. La moyenne cache le pire cas pourtant : si chaque clé hachait vers
le même seau, chaque recherche marcherait chaque clé — O(n), le tableau dont
nous sommes partis. Les vraies tables re-hachent dans plus de seaux quand le
facteur de charge grandit ; celle-ci fixe son nombre de seaux à 1024 et le
dit — une simplification qui vaut d'être nommée, pas cachée.

Le pilote est maintenant le programme final. `CountWords` découpe le fichier
en mots avec un `fgetc` par caractère, pliant la casse et traitant tout ce qui
n'est pas alphanumérique comme un séparateur, et `CountWord` applique le motif
obtenir-ou-placer pour chaque mot trouvé. Puis `HtEntries` copie chaque
entrée hors des chaînes dans un dynarray, `DaSort` l'ordonne avec le
comparateur de la leçon 009, et `DaEach` imprime. Une verrue voyage avec :
la clé de `struct Item` est un 16 octets fixe, aussi un mot de plus de
quinze caractères est-il stocké tronqué — deux mots partageant leurs quinze
premiers caractères fusionnent. Le texte en langue naturelle ne le remarque
guère ; un vrai dictionnaire stockerait des clés `char *`, et maintenant vous
savez exactement où irait ce changement.

Commande de construction inchangée (depuis `sandbox/ds-kit/`) :

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

Exécutez-le avec `./ds-kit notes.txt` sur n'importe quel fichier texte.

## Étape de code

Un seul changement pour cette leçon : `ds-kit.c` gagne `struct HashTable` —
des seaux comme tableau de chaînes de dynarray, des clés de chaînes hachées
avec FNV-1a, `HtPut`/`HtGet` avec gestion des collisions par chaînage — et le
pilote devient le programme final : un compteur de fréquences de mots sur le
fichier nommé dans `argv`, imprimant les compteurs triés par clé. Son état
final est étiqueté `lesson-011`.

```diff
diff --git a/sandbox/ds-kit/ds-kit.c b/sandbox/ds-kit/ds-kit.c
index 1ba1cc4..3b41f76 100644
--- a/sandbox/ds-kit/ds-kit.c
+++ b/sandbox/ds-kit/ds-kit.c
@@ -1,10 +1,13 @@
-// ds-kit.c — the data-structures kit of Part 0: one generic dynarray that
-// stores any element type as raw bytes.
+// ds-kit.c — the data-structures kit of Part 0: a hashtable over the
+// generic dynarray, and a word-frequency driver.
 //
-// Lesson 010: void* — genericity, casting, and its silent failures.
+// Lesson 011: hashing, buckets, collisions — and lookup that is O(1) on
+// average.
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
+#include <stdint.h>
+#include <ctype.h>
 
 struct Item {
     char key[16];
@@ -68,10 +71,94 @@ void DaFree(struct DynArray *da)
     DaInit(da, da->elem_size);
 }
 
-// -- the callbacks this driver supplies --------------------------------
+// -- the hashtable ------------------------------------------------------
 //
-// Every one of them casts: the array is generic now, so the types are the
-// caller's job.
+// Buckets are an array of chains, and a chain is just a dynarray of
+// struct Item. FNV-1a turns a key into a 32-bit hash; the remainder modulo
+// nbuckets picks the bucket; the chain handles collisions.
+
+struct HashTable {
+    struct DynArray *chains;
+    size_t nbuckets;
+    size_t len;
+};
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
+
+// -- the driver: word frequencies ---------------------------------------
 
 static int CmpByKey(const void *pa, const void *pb)
 {
@@ -80,56 +167,66 @@ static int CmpByKey(const void *pa, const void *pb)
     return strcmp(a->key, b->key);
 }
 
-static int CmpLong(const void *pa, const void *pb)
-{
-    const long *a = (const long *)pa;
-    const long *b = (const long *)pb;
-    return (*a > *b) - (*a < *b);
-}
-
 static void PrintItem(const void *pe)
 {
     const struct Item *it = (const struct Item *)pe;
     printf("%s %ld\n", it->key, it->value);
 }
 
-static void PrintLong(const void *pe)
+static void CountWord(struct HashTable *ht, const char *word)
 {
-    const long *v = (const long *)pe;
-    printf("%ld\n", *v);
+    long *p = HtGet(ht, word);
+    if (p != NULL)
+        ++*p;
+    else
+        HtPut(ht, word, 1);
 }
 
-int main(void)
+static void CountWords(struct HashTable *ht, FILE *f)
 {
-    struct DynArray items;
-    DaInit(&items, sizeof(struct Item));
-
-    const char *keys[10] = {
-        "pear", "apple", "fig", "banana", "cherry",
-        "date", "elder", "grape", "kiwi", "lemon",
-    };
-    for (int i = 0; i < 10; ++i) {
-        struct Item item;
-        snprintf(item.key, sizeof item.key, "%s", keys[i]);
-        item.value = i;
-        DaPush(&items, &item);
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
     }
+    if (n > 0) {
+        word[n] = '\0';
+        CountWord(ht, word);
+    }
+}
 
-    DaSort(&items, CmpByKey);
-    printf("items sorted by key:\n");
-    DaEach(&items, PrintItem);
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
 
-    struct DynArray nums;
-    DaInit(&nums, sizeof(long));
-    long vals[5] = { 50, 30, 10, 40, 20 };
-    for (int i = 0; i < 5; ++i)
-        DaPush(&nums, &vals[i]);
+    struct HashTable ht;
+    HtInit(&ht, 1024);
+    CountWords(&ht, f);
+    fclose(f);
 
-    DaSort(&nums, CmpLong);
-    printf("numbers sorted:\n");
-    DaEach(&nums, PrintLong);
+    struct DynArray entries;
+    DaInit(&entries, sizeof(struct Item));
+    HtEntries(&ht, &entries);
+    DaSort(&entries, CmpByKey);
+    DaEach(&entries, PrintItem);
 
-    DaFree(&nums);
-    DaFree(&items);
+    DaFree(&entries);
+    HtFree(&ht);
     return 0;
 }
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Comptez dessus *(predict-the-output)*

Créez un fichier texte contenant exactement ces deux lignes :

```
The cat AND the dog.
Don't stop the bird!
```

Avant d'exécuter quoi que ce soit, écrivez chaque ligne de sortie que
`./ds-kit` imprimera pour ce fichier, dans l'ordre. Puis exécutez-le et
rendez compte de chaque ligne — y compris les deux mots que la ponctuation
fabrique silencieusement et la casse que le compteur plie en silence.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-011/ex1.md)

### Exercice 2 — Le classement *(extend-the-code)*

Ajoutez un mode `--top N` : `./ds-kit --top 3 file.txt` imprime seulement les
trois mots les plus fréquents, le plus fréquent en premier. Les mots à égalité
de compteur sortent dans l'ordre alphabétique, aussi la sortie est-elle
totalement déterminée. Le mode simple (sans `--top`) doit continuer à imprimer
exactement ce qu'il imprime maintenant. Trier par compteur veut dire que le
comparateur de la leçon 009 gagne une seconde politique — assurez-vous qu'une
égalité retombe sur la clé, sinon l'ordre n'est pas reproductible.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-011/ex2.md)

### Exercice 3 — La longueur de la marche *(explain-in-prose)*

Écrivez une courte explication — un paragraphe chacune — de (a) pourquoi la
recherche est O(1) *en moyenne* et quelle quantité exactement la moyenne cache
— exprimez la marche de chaîne attendue en fonction de `len` et `nbuckets` ;
(b) ce qu'est une collision, comment la chaîne la résout, et pourquoi
comparer les hachages ne peut jamais remplacer la comparaison des clés ; (c)
sous quelle entrée la table dégénère en recherches O(n) et ce que les vraies
conceptions de tables de hachage font pour y remédier. Appliquez ensuite la
vérification confirmante de la solution, lancez-la sur votre fichier texte, et
assurez-vous que votre explication prédit les comptes de sondage qu'elle
rapporte.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-011/ex3.md)

### Exercice 4 — Un seul seau *(measure-the-performance)*

Chronométrez la différence que font les seaux. Lancez le compteur sur un gros
texte — un livre de Project Gutenberg, `/usr/share/dict/words`, ou un fichier
généré de quelques centaines de milliers de mots — avec les 1024 seaux de la
table, puis avec un seul seau. Le patch de la solution permet de choisir le
nombre de seaux sur la ligne de commandne pour éviter de recompiler.
Rapportez les deux horloges et le nombre de mots distincts du fichier, et
expliquez le ratio entre les deux temps en fonction de la longueur moyenne
des chaînes.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-011/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 010 — void\* : la généricité et ses peines](lesson-010-void-pointer.md) ·
**Suivante :** [Leçon 012 — constructions multi-fichiers : unités de traduction et édition de liens](lesson-012-multi-file.md) ·
**Étiquette de code :** [`lesson-011`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-011)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-011-hashtable.md`,
révision `11ce1ee`.*

<!-- translation-source: book/lessons/part-0/lesson-011-hashtable.md @ 11ce1ee -->
