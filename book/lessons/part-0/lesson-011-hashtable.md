# Lesson 011 — the hashtable: hashing, buckets, lookup

{{#include ../../stability-horizon.md}}

## Prose

This lesson completes the kit and turns `ds-kit` into its final program: a
word-frequency counter that reads a text file named on the command line and
prints how often each word occurs, sorted by key. The component that makes it
possible is the last data structure of Part 0 — the **hashtable** — and it is
assembled almost entirely from parts you have already built.

The problem it solves is lookup. An array finds element *i* in one step but
finds *by key* only by walking — O(n). Keep the array sorted and binary
search finds keys in O(log n), but every insert re-earns that order. A
hashtable answers "find by key" in O(1) on average and inserts just as
cheaply. The trick is to *compute* where a key lives instead of searching for
it: a **hash function** turns the key into a number, the number picks a
**bucket**, and the value waits in that bucket.

Look at `struct HashTable` and the trick's scaffolding is visible: `chains`
is an array of buckets, `nbuckets` counts them, `len` counts stored entries.
Each bucket holds a **chain** of entries — and a chain is nothing but the
kit's own generic `DynArray`, holding `struct Item` (the `key[16]`/`value`
pair that has been the kit's element since lesson 007). The hashtable is the
dynarray used once per bucket, plus a hash function and a lookup policy. C
programs are built out of exactly this kind of reuse.

The hash function here is **FNV-1a**, one of the small classics: start a
32-bit accumulator at the offset basis `2166136261u`, and for each byte of
the key xor the byte into the accumulator, then multiply by the prime
`16777619u`. Xor mixes a byte in; multiplication spreads its influence
across the whole accumulator (that spreading is what makes `cat` and `act`
land far apart despite sharing letters). The type is `uint32_t` from
`<stdint.h>` on purpose: unsigned arithmetic wraps modulo 2³² with the
standard's blessing — unlike signed overflow, which is lesson 006's undefined
behavior. And the loop consumes *every* character: a hash that ignores one
byte treats `catalog` and `cataloo` as the same key forever. (FNV-1a is fast
and well-distributed, not cryptographic — nobody here is defending against an
attacker choosing keys.)

`HtLookup` shows what a lookup is: hash the key, take `hash % nbuckets` to
pick the bucket, then walk that bucket's chain comparing keys with `strcmp`.
The comparison is the part to think about: equal hashes do **not** mean equal
keys — different keys landing in the same bucket is a **collision**, and the
chain is how collisions are handled. `HtPut` looks up first: an existing key
updates the entry's value; a new key is copied into the chain as a fresh
`struct Item`. `HtGet` looks up and hands back a pointer to the value inside
the table — `NULL` when the key is absent — which is what makes the counting
loop three lines long.

Why is this O(1) *on average*? Because the average chain has length
`len / nbuckets` — the table's **load factor** — and lookup walks one chain.
With 1024 buckets and a few thousand words, chains are a handful of entries
long, and "walk a handful" is a constant. The average hides the worst case,
though: if every key hashed to the same bucket, every lookup walks every key
— O(n), the array we started with. Real tables rehash into more buckets when
the load factor grows; this one fixes its bucket count at 1024 and says so —
a simplification worth naming, not hiding.

The driver is now the final program. `CountWords` tokenizes the file with one
`fgetc` per character, folding case and treating anything non-alphanumeric as
a separator, and `CountWord` applies the get-or-put pattern for each word
found. Then `HtEntries` copies every entry out of the chains into one
dynarray, `DaSort` orders it with the comparator from lesson 009, and `DaEach`
prints. One wart rides along: `struct Item`'s key is a fixed 16 bytes, so a
word longer than fifteen characters is stored truncated — two words sharing
their first fifteen characters merge. Natural-language text barely notices; a
real dictionary would store `char *` keys, and now you know exactly where
that change would go.

Build command unchanged (from `sandbox/ds-kit/`):

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

Run it as `./ds-kit notes.txt` on any text file.

## Code step

One change for this lesson: `ds-kit.c` gains `struct HashTable` — buckets as
an array of dynarray chains, string keys hashed with FNV-1a, `HtPut`/`HtGet`
with chained collision handling — and the driver becomes the final program: a
word-frequency counter over the file named on `argv`, printing counts sorted
by key. Its end state is tagged `lesson-011`.

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

## Exercises

Four short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Count on it *(predict-the-output)*

Create a text file containing exactly these two lines:

```
The cat AND the dog.
Don't stop the bird!
```

Before running anything, write down every output line `./ds-kit` will print
for that file, in order. Then run it and account for each line — including
the two words the punctuation quietly manufactures and the case the counter
quietly folds.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-011/ex1.md)

### Exercise 2 — The leaderboard *(extend-the-code)*

Add a `--top N` mode: `./ds-kit --top 3 file.txt` prints only the three most
frequent words, the most frequent first. Words tied on count come out in
alphabetical order, so the output is fully determined. The plain mode (no
`--top`) must keep printing exactly what it prints now. Sorting by count
means the comparator from lesson 009 gets a second policy — make sure a tie
falls back to the key, or the ordering is not reproducible.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-011/ex2.md)

### Exercise 3 — The walk's length *(explain-in-prose)*

Write a short explanation — a paragraph each — of (a) why lookup is O(1) on
*average* and what quantity exactly the average hides — express the expected
chain walk in terms of `len` and `nbuckets`; (b) what a collision is, how the
chain resolves it, and why comparing hashes can never replace comparing keys;
(c) under what input the table degrades to O(n) lookups and what real
hashtable designs do about it. Then apply the confirming check from the
solution, run it over your text file, and make sure your explanation predicts
the probe counts it reports.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-011/ex3.md)

### Exercise 4 — One bucket *(measure-the-performance)*

Time the difference buckets make. Run the counter over a large text — a book
from Project Gutenberg, `/usr/share/dict/words`, or a generated file of a few
hundred thousand words — with the table's 1024 buckets, and then with a
single bucket. The solution's patch lets the bucket count be chosen on the
command line so no recompiling is needed. Report both wall clocks and the
number of distinct words in the file, and explain the ratio between the two
times in terms of the chains' average length.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-011/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 010 — void*: genericity and its pain](lesson-010-void-pointer.md) ·
**Next:** [Lesson 012 — multi-file builds: translation units and linking](lesson-012-multi-file.md) ·
**Code tag:** [`lesson-011`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-011)
