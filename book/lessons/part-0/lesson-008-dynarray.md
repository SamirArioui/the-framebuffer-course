# Lesson 008 — dynarray growth: realloc and capacity

{{#include ../../stability-horizon.md}}

## Prose

This lesson gives the kit its first container. C's arrays have one size for
life: `struct Item items[10]` holds ten items on the stack and never an
eleventh. Python's `list` and Ruby's `Array` grow silently as you append; C
has no such thing built in, so every C program that needs one *builds* one —
and the shape it builds is almost always this: a heap block plus a length
plus a capacity. That is the whole of `struct DynArray` in the code step:
`items` points at the block, `len` counts the elements in use, `cap` counts
the slots the block provides. The gap between `len` and `cap` is exactly the
room that makes appends cheap.

`DaInit`, `DaPush`, and `DaFree` operate on that struct and follow one naming
convention (`Da` for dynarray) instead of any language mechanism — C structs
have no methods, remember, only functions that take a pointer to one.
`DaPush` takes a `struct Item` **by value**: the twenty-four bytes are copied
into the call and copied again into the array, which is honest about what
storing means here. When the array is full — `len == cap` — `DaPush` grows
the block with `realloc` before storing, doubling the capacity: zero to four,
four to eight, eight to sixteen.

`realloc` is the C library's "move or extend this heap block" call, and its
contract deserves memorizing. Given a block and a new size it returns a block
of that size holding the old contents up to the old size — but the returned
pointer may be **the same or different**. The block can be extended in place
if free space follows it; otherwise a fresh block is allocated, the bytes are
copied, and the old block is freed. Either way the old pointer must not be
used afterwards, and the new pointer must come from the return value. On
failure `realloc` returns `NULL` and — this is the sharp edge — the old block
is left **intact and still yours**. That is why `DaPush` stores the result in
a temporary `p` and only assigns on success. The one-liner
`da->items = realloc(da->items, ...)` loses the only pointer to the block the
moment failure happens: a leak of exactly the kind lesson 005 taught you to
see. (And `realloc(NULL, n)` is legal and means `malloc(n)` — that is how the
first growth works with `items` still `NULL`.)

Why double the capacity rather than grow by one slot? Because growth copies
the whole array, and the *schedule* of growths decides the total copying. If
capacity grows by one each time, push number *n* copies *n* elements, and the
copies sum to roughly n²/2 — a million appends copy half a trillion elements.
With doubling, the expensive pushes get rarer geometrically: every element is
copied at most about log n times, and the total copying for *n* pushes stays
under 2n elements no matter how many you do. Individual pushes still cost
O(n) at a growth — but the *average* cost over any long run is constant. That
is the amortized O(1) your Python `list` was quietly giving you all along.

`DaFree` releases the block and then calls `DaInit` again, so the array is
left empty, valid, and reusable — a small discipline that keeps the struct
out of the "freed but still holding stale numbers" state that crashes
programs later. The driver in the code step makes the whole story visible: it
appends ten items and prints `len` and `cap` after every push, so the growth
points show up as the lines where `cap` jumps.

The build command is unchanged from lesson 007 — same flags, same directory
(`sandbox/ds-kit/`):

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

## Code step

One change for this lesson: the dynarray replaces the layout experiments in
`ds-kit.c` — `struct DynArray` with `DaInit`, `DaPush`, and `DaFree`, growth
by `realloc` with capacity doubling, and a driver that appends ten items and
reports `len`/`cap` as it grows. Its end state is tagged `lesson-008`.

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

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — The growth schedule *(predict-the-output)*

Predict the machine before it speaks. Write down all eleven lines you expect
from `./ds-kit` — the ten `len=.. cap=..` lines and the final
`first=.. last=..` line — starting from an array with no capacity at all.
Then run it and account for every difference between your prediction and the
output, in particular which push triggers each reallocation and why it is
that push.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-008/ex1.md)

### Exercise 2 — The half-freed array *(fix-the-crash)*

A teammate wanted a reset that leaves the array reusable:

```c
void DaClear(struct DynArray *da)
{
    free(da->items);
    da->len = 0;
}
```

Add it to `ds-kit.c`, call it halfway through the driver's push loop, and keep
pushing afterwards. Built with lesson 005's sanitizer
(`gcc -std=c11 -O0 -g -Wall -Wextra -fsanitize=address ds-kit.c -o ds-kit`)
the next push reports a heap-use-after-free; built plain it may only look
like it works. Reproduce the sanitizer report, then fix `DaClear` so that
clearing and reusing an array is leak-free and use-after-free-free — and say
which line of the original made the failure possible.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-008/ex2.md)

### Exercise 3 — Doubling versus one-at-a-time *(measure-the-performance)*

Make the growth rule a switch and measure the difference. Add a
`#define GROW_BY_ONE 1` you can toggle by hand: when set, growth adds exactly
one slot instead of doubling. Also add run-total counters for reallocation
calls and for bytes copied out of old blocks into new ones, printed at exit.
Push 1,000,000 items under each rule, time both with `time ./ds-kit
> /dev/null`, and report the counters and the wall clocks. Explain the gap
between the two stories if they disagree.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-008/ex3.md)

### Exercise 4 — What `realloc` really promises *(explain-in-prose)*

Write a short explanation — a paragraph each — of (a) why capacity doubling
gives amortized O(1) pushes, deriving the total elements copied for *n*
pushes from an empty array under both growth rules; (b) exactly what
`realloc` guarantees when it fails and why `da->items = realloc(da->items,
...)` is a bug pattern even though it usually works; (c) when `realloc` can
return the same pointer and when it cannot. Then apply the confirming check
from the solution and see how often case (c) actually happens on your
machine.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-008/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 007 — structs: sizeof, alignment, and padding](lesson-007-struct-layout.md) ·
**Next:** [Lesson 009 — function pointers: comparators and hooks](lesson-009-function-pointers.md) ·
**Code tag:** [`lesson-008`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-008)
