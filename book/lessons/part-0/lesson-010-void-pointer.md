# Lesson 010 — void*: genericity and its pain

{{#include ../../stability-horizon.md}}

## Prose

Last lesson the kit sorted one element type. This lesson it stores *any*
element type — and the price of that genericity becomes the lesson. The price
has a name in C: `void *`, a pointer whose pointee type has been erased.
Every pointer to data converts to `void *` and back without ceremony —
implicit going in, and only a cast to give the bytes their type back on the
way out. What it points at is unknown by design: you cannot dereference a
`void *`, cannot do arithmetic on it, cannot even know how many bytes it
covers. It is "some memory, somewhere" — a promise you must keep yourself.
(Two footnotes: function pointers do *not* convert to `void *` — they are a
separate family — and arithmetic on `void *` is a GCC extension; portable C
steps through `char *`, one byte at a time, which is exactly what `DaPush`
and `DaAt` now do.)

Look at what the generic `DynArray` became. It no longer contains `struct
Item *items`; it contains `void *data` — a block of bytes — plus `elem_size`,
the stride that says how many bytes each element occupies. `DaPush` takes the
element as `const void *` and `memcpy`s `elem_size` bytes of it into the next
slot. `DaAt` returns `void *` to the slot. `DaSort` hands `qsort` the block,
the count, the stride, and a comparator that is *already* `qsort`'s own
comparator type. And notice what disappeared from lesson 009: `CmpBridge`
and its `g_cmp` shim. `qsort`'s world was always `void *` — once our array
speaks `void *` too, the bridge is redundant. The casts did not vanish,
though; they moved into every comparator and every visit hook, where the
caller casts the `void *` back to the type the caller knows it stored.

The driver now proves the payoff: one `DaInit`/`DaPush`/`DaSort`/`DaEach`
implementation serves both an array of `struct Item` and an array of `long`.
No second array type, no duplicated sort. In Python this would be a `list`;
in Ruby, an `Array` — and the runtime would check every element's type at
every operation and raise a clean `TypeError` on the first mismatch.

C checks nothing. That is the pain. `elem_size` is a number you typed, not a
fact the compiler verified: initialize the array with `sizeof nums` — the
*struct* — instead of `sizeof(long)` — the element — and every `DaPush`
copies `sizeof(struct DynArray)` bytes from an eight-byte variable, reading
off the end of a stack slot. Cast a `long` slot to `const int *` and the same eight bytes get
reinterpreted as two four-byte integers. Both programs compile with
`-Wall -Wextra` and not one complaint, because from the compiler's view the
`void *` erased exactly the information it would have needed to object. The
failures land in lesson 006's territory: silent wrong output when the bytes
happen to look plausible, undefined behavior when they do not. The conventions
— "this array holds `struct Item`, see the `DaInit` call" — are now load-
bearing comments. This is C's genericity contract: one implementation, zero
checks, all responsibility yours.

Why does C do it this way at all? Because the standard library is built from
it: `qsort`, `memcpy`, `malloc`, `fwrite` all take `void *` and a size. C has
no generics of the modern kind — C11's `_Generic` can dispatch on a type name
but cannot abstract storage over one — so the language's uniform answer to
"store anything" is bytes plus a stride plus discipline. The alternative is
what the kit had before this lesson: a typed copy per element type. When
Part 1's engine arrives in C++, templates will make the compiler generate
those copies for you; until then, the stride is yours to keep honest.

The build command is unchanged (from `sandbox/ds-kit/`):

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

## Code step

One change for this lesson: `DynArray` generalizes to any element — `data`
as `void *` plus `elem_size`, `DaPush` copying through `memcpy`, `DaAt`
returning `void *` for the caller to cast — and the driver refactors onto the
generic array, adding a second array of `long` to show one implementation
serving two types. Its end state is tagged `lesson-010`.

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

## Exercises

Four short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The same bytes, differently *(predict-the-output)*

Add a small experiment to the driver: an array whose `elem_size` is
`sizeof(long)`, holding the two values 7 and 42, read back through a cast to
`const int *` — print *both* `int`s of each element, two lines of two
numbers. Predict the exact four numbers your machine will print before you
build; then run and explain where each number comes from, and what the
prediction would be on a machine with the opposite byte order.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-010/ex1.md)

### Exercise 2 — The wrong size *(fix-the-crash)*

A teammate wrote the numbers array like this, and the program dies under
lesson 005's sanitizer:

```c
    struct DynArray nums;
    DaInit(&nums, sizeof nums);
    long v = 7;
    DaPush(&nums, &v);
```

Build it with `-fsanitize=address` and read what the sanitizer says the
offending access is. Fix the initialization so the array really holds
`long`s, and explain why the mistake produced no compiler complaint even
with `-Wall -Wextra` — what would the compiler have needed to know?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-010/ex2.md)

### Exercise 3 — Removing in the middle *(extend-the-code)*

Add `DaRemoveAt` to the kit: given an index, it removes that element and
slides the later ones down one slot, shrinking `len` — no reallocation
needed. The slide is pure byte surgery with `memmove`, and the index must be
validated. Demonstrate it from the driver: push five `long`s and remove the
element at index 2, printing the survivors. Say, in a comment or a sentence,
why `memmove` and not `memcpy` for the slide.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-010/ex3.md)

### Exercise 4 — The contract, written down *(explain-in-prose)*

Write a short explanation — a paragraph each — of (a) two distinct ways a
`void *`-based container can silently corrupt data, one through `elem_size`
and one through a wrong cast, and why each escapes the compiler's checks;
(b) how the same two mistakes look in Python or Ruby — what replaces the
silent corruption there, and what that costs at runtime; (c) why the kit's
accessors return pointers *into* the block and what that implies about
`realloc` and about how long a `DaAt` result may be held. Then apply the
confirming check from the solution and see the byte-level machinery at work.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-010/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 009 — function pointers: comparators and hooks](lesson-009-function-pointers.md) ·
**Next:** [Lesson 011 — the hashtable: hashing, buckets, lookup](lesson-011-hashtable.md) ·
**Code tag:** [`lesson-010`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-010)
