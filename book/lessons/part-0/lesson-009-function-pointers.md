# Lesson 009 — function pointers: comparators and hooks

{{#include ../../stability-horizon.md}}

## Prose

This lesson teaches the kit to accept instructions as arguments. A function
in C is not just something you call — a function's name, used where a value
is expected, becomes a pointer to that function's machine code. Store it in a
variable, pass it to another function, call it later with ordinary call
syntax. Python's `sorted(key=...)`, Ruby's blocks and `&:method` — same
capability, usually hidden behind syntax. In C it is a first-class value with
an honest, ugly type.

The type is where C makes you work for it. Read this one from the inside
out:

```c
int (*cmp)(const struct Item *, const struct Item *)
```

`cmp` is a pointer (`*cmp`, parenthesized so the `*` binds to the name) to
something you can call with two `const struct Item *` arguments that produces
an `int`. The parentheses around `*cmp` are load-bearing: write
`int *f(const struct Item *)` without them and you have declared a *function*
returning `int *` instead. This backwards reading is the price of one
declaration syntax covering variables, functions, arrays, and pointers; when
it gets too painful, C programmers name the type —
`typedef int (*ItemCmp)(const struct Item *, const struct Item *);` — and
write `ItemCmp` from then on. The code step keeps the raw form visible in
`DaSort`'s signature so you keep reading it.

What the kit does with such a pointer is sort. `DaSort` takes a comparator —
a function that defines an ordering by answering "less, equal, or greater"
with a negative number, zero, or a positive number — and hands the array to
libc's `qsort` from `<stdlib.h>`, which is standard C and sorts *any* array
given its start, its length, its element size, and such a comparator. Two
comparators drive the driver: `CmpByKey` forwards to `strcmp` (which already
returns a negative/zero/positive answer), and `CmpByValue` computes one with
the classic three-way trick
`(a->value > b->value) - (a->value < b->value)` — subtracting two comparison
booleans, because subtracting the *values* themselves would overflow for
large enough `long`s.

Now the honest part — why `DaSort` contains a bridge function and a
file-static `g_cmp` at all. `qsort`'s own comparator type is
`int (*)(const void *, const void *)`: pointers to *any* element, untyped,
because `qsort` predates any notion of generics in C. Our typed comparator is
a different function-pointer type, and C does not allow passing one where the
other is expected — nor is it safe to cast between them and call through the
wrong type: the machine code would disagree about what it is pointing at. So
`CmpBridge` is the one conforming crossing: it receives `void *` pointers,
casts them back to `const struct Item *`, and forwards to whatever comparator
`DaSort` was given. And `g_cmp` exists because `qsort` passes the comparator
no context — there is no "extra argument" slot (POSIX's `qsort_r` has one;
standard C does not), so the pending comparator waits in a file-static
variable around the `qsort` call. A known wart of the C standard library,
safe here because nothing else runs concurrently, and worth recognizing on
sight: when a C API cannot hand you your own context, it will make you find
somewhere to park it.

The second function pointer is a **visit hook**, and it defines iteration:
`DaEach` walks the array and calls your `visit` function on each element.
That is the callback pattern in its purest form — inversion of control: our
loop runs, your function gets called inside it. The driver supplies
`PrintItem` and gets its printing for free; supply a different hook and the
same walk does something else. Notice what varies and what does not: the
comparator changes *how* the array is ordered, the hook changes *what happens
per element*, and `DaSort`/`DaEach` never know the difference.

The build command is unchanged (from `sandbox/ds-kit/`):

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

## Code step

One change for this lesson: `ds-kit.c` gains the function-pointer machinery —
`DaSort` taking a comparator and wrapping `qsort` through a bridge, `DaEach`
taking a visit hook — and the driver pushes its ten items, sorts them by key,
sorts them by value, and prints each state. Its end state is tagged
`lesson-009`.

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

## Exercises

Three short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The reversed order *(predict-the-output)*

In `CmpBridge`, swap the two arguments in the call to `g_cmp` — pass `pb`
where `pa` goes and vice versa — and leave everything else alone. Before you
run it, write down the exact ten lines you expect under `sorted by key:`.
Then build, run, and account for the result: is the output merely the
previous block reversed, or something subtler, and why?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-009/ex1.md)

### Exercise 2 — Searching by predicate *(extend-the-code)*

Add a search to the kit: `DaFind`, driven by a predicate — a function that
answers a yes/no question about one item — returning a pointer to the first
item that satisfies it, or `NULL` if none does. The array must stay
unchanged. Use it from the driver for two queries: print the item whose key
is `grape`, and show a query that matches nothing. Name the predicate's
function-pointer type in full at least once, and make sure the driver never
touches the returned pointer without checking it.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-009/ex2.md)

### Exercise 3 — Why the bridge exists *(explain-in-prose)*

Write a short explanation — a paragraph each — of (a) why `qsort` cannot be
handed `CmpByKey` directly: what its comparator's declared type is, and what
C says about converting between incompatible function-pointer types and
calling through the converted pointer; (b) what `g_cmp` is compensating for
in `qsort`'s design, and in what situation this workaround would stop being
safe; (c) what `DaEach` buys that a plain `for` loop over `da.items` does
not, and when the callback is *not* worth it. Then apply the confirming
check from the solution, watch the bridge work on real calls, and make sure
your explanation matches what it prints.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-009/ex3.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 008 — dynarray growth: realloc and capacity](lesson-008-dynarray.md) ·
**Next:** [Lesson 010 — void*: genericity and its pain](lesson-010-void-pointer.md) ·
**Code tag:** [`lesson-009`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-009)
