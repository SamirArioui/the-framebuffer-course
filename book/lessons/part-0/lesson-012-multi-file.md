# Lesson 012 — multi-file builds: translation units and linking

{{#include ../../stability-horizon.md}}

## Prose

The kit is finished — today it stops being one file. `ds-kit.c` has grown
through five lessons into a dynarray, a hashtable, and a driver, and single
files do not scale: nobody can hold 300 lines of mixed concerns in one
scroll, and every edit recompiles everything. So the code step splits the
program into `dynarray.h`, `dynarray.c`, `hashtable.h`, `hashtable.c`, and
`main.c` — flat in `sandbox/ds-kit/`, no subdirectories — and the program is
byte-for-byte the same afterward. What changes is how it is *built*, and that
machinery is the subject of this lesson: translation units, object files,
symbol tables, and the linker.

The compiler's unit of work is the **translation unit**: one `.c` file plus
everything its `#include` directives drag in. `#include "dynarray.h"` is not
an import — it is textual pasting, done by the preprocessor before the
compiler ever runs. `dynarray.c` becomes one translation unit containing the
header's struct and prototypes followed by the implementation; `main.c`
becomes another containing both headers and the driver. Crucially, each
translation unit is compiled **alone**: the compiler never sees two `.c`
files at once, and it checks every call in `hashtable.c` against the
*declarations* in `dynarray.h`, trusting that someone, someday, provides the
definitions. That trust is the header's role: a contract each file is
checked against, not a file manager's bookkeeping.

Run `gcc -c` on a translation unit and the output is an **object file** —
machine code that is not yet a program:

```
$ gcc -std=c11 -O0 -g -Wall -Wextra -c dynarray.c hashtable.c main.c
$ file dynarray.o
dynarray.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), with debug_info, not stripped
```

*Relocatable* is the word: the code is compiled but its addresses are not
final, and calls into other translation units are open questions. Each
object carries a **symbol table** — its list of names it defines and names it
needs — and `nm` prints it:

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

Read the letters, because they are the whole game. `T` is a symbol this
object defines and exports — the `Ht*` functions any translation unit may
call. `t` (lowercase) is a symbol defined but **not exported**: `HtHash` and
`HtLookup` are `static`, which gives them *internal linkage* — visible only
inside `hashtable.c`. `U` is a symbol this object *needs*: the `Da*` calls
promise-delivered by `dynarray.o`, and `malloc`, `strcmp`, `stderr` and
friends promised by the C library. Nothing in `hashtable.o` says where those
live. That is the linker's job.

The **linker** is the last stage of `gcc`, and it does exactly three things
worth remembering: it resolves every `U` against some `T` — `main.o`'s call
to `DaSort` against `dynarray.o`'s definition, `strcmp` against libc — it
fixes the final addresses now that all the pieces are adjacent, and it
records `main` as the entry point. Until it runs, there is no program; when
it fails, it fails on *names*, not on code. The one-line build you have been
running all along hides this seam — `gcc *.c -o ds-kit` compiles each
translation unit and links in one breath. The two-step form shows it:

```
gcc -std=c11 -O0 -g -Wall -Wextra -c dynarray.c hashtable.c main.c
gcc dynarray.o hashtable.o main.o -o ds-kit
```

Both produce the same program from the same five files — every run in this
lesson was checked against both.

The failure the linker is most famous for is the one worth reproducing
today. Put a plain (non-`static`) helper — say `ReportOOM`, printing a
message and exiting — into *both* `dynarray.c` and `hashtable.c`, and each
file compiles clean: the compiler only sees one definition per translation
unit. The link then dies:

```
/usr/bin/ld: hashtable.o: in function `ReportOOM':
hashtable.c:(.text+0x46): multiple definition of `ReportOOM'; dynarray.o:dynarray.c:(.text+0x0): first defined here
collect2: error: ld returned 1 exit status
```

(With `-g` in the flags the linker can name the offending source lines
instead of machine-code offsets.) Two files each *export* the same name, and
an external name may be defined only once per program. The fix is the
lesson's keyword: give each file its own `static` copy — internal linkage,
no collision — or factor the helper into one translation unit and declare it
in a header exactly once. `static` for helpers used in one file; a shared
header for helpers genuinely used across files. That is the entire
externality policy of C.

One last contract detail the split exposes: **include guards**. Since
`#include` pastes text, a header pasted twice into one translation unit would
define `struct DynArray` twice — a hard "redefinition" error. The
`#ifndef DYNARRAY_H` / `#define DYNARRAY_H` / `#endif` sandwich makes the
second paste empty; every header in the kit carries one. (`#pragma once`
does the same job in one line and works on every compiler you will meet, but
the guard is the portable spelling.) And look at what `hashtable.h` does
*not* include: it mentions `struct DynArray` only through a pointer, so a
bare forward declaration — `struct DynArray;` — is enough. An incomplete
type suffices for pointers; only `hashtable.c`, which walks the chains,
includes `dynarray.h` in full. Include what you use, declare what you only
point at.

The kit is done. It taught the machine's memory (007), growth (008),
callbacks (009), erased types (010), hashing (011), and now the build itself
— the toolkit Part 1's engine will lean on every day. Part 0's next program
turns to raw bytes and pixels.

Build commands (from `sandbox/ds-kit/`):

```
gcc -std=c11 -O0 -g -Wall -Wextra *.c -o ds-kit
```

or, showing the seam:

```
gcc -std=c11 -O0 -g -Wall -Wextra -c dynarray.c hashtable.c main.c
gcc dynarray.o hashtable.o main.o -o ds-kit
```

## Code step

One change for this lesson: the split. `ds-kit.c` is deleted and its parts
become `dynarray.h`/`dynarray.c` (the generic array), `hashtable.h`/
`hashtable.c` (the table — its header forward-declaring `struct DynArray`),
and `main.c` (the driver; the only file with `main`). Its end state is tagged
`lesson-012`.

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

## Exercises

Three short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The helper that clashed *(fix-the-crash)*

The deep dive's `ReportOOM` scenario is now yours. Add a helper to both
`dynarray.c` and `hashtable.c` — `void ReportOOM(const char *who)`, printing
`who` and "out of memory" to `stderr` and exiting with status 1 — as a plain,
non-`static` function, and use it from `DaPush` and `HtInit` in place of
their inline out-of-memory handling. Every file will compile clean; the link
will not. Reproduce the linker's complaint, then fix the failure so both
files keep their helper and the program links — and say which of the two
legal fixes you chose and why.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-012/ex1.md)

### Exercise 2 — Table statistics *(extend-the-code)*

Give the kit one more function, added through the header contract end to
end: `HtStats`, declared in `hashtable.h`, defined in `hashtable.c`, called
from `main.c` after counting. It prints to `stderr` a single line with the
number of entries, the number of buckets, how many buckets are empty, and
the longest chain — the load-factor picture of lesson 011 as four numbers.
Run it on a real text file and check the numbers against what you expect.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-012/ex2.md)

### Exercise 3 — The lowercase letter *(predict-the-output)*

`nm hashtable.o` shows `HtHash` as a lowercase `t` while `HtGet` is an
uppercase `T`. Predict what `nm` reports for `HtHash` after you delete the
`static` keyword from its definition — and whether the program still builds
and runs. Then make the one-word change, run `nm` again, and reconcile the
two outputs with the linkage rules of this lesson.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-012/ex3.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 011 — the hashtable: hashing, buckets, lookup](lesson-011-hashtable.md) ·
**Next:** [Lesson 013 — raw bytes and pixel formats](lesson-013-raw-bytes.md) ·
**Code tag:** [`lesson-012`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-012)
