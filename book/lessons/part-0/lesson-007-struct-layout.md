# Lesson 007 — structs: sizeof, alignment, and padding

{{#include ../../stability-horizon.md}}

## Prose

This lesson starts the second program of Part 0: `ds-kit`, a small kit of data
structures that grows, lesson by lesson, into a word-frequency counter over a
text file. Today the kit is one type and a set of questions about it. The type
is the element the kit will store:

```c
struct Item {
    char key[16];
    long value;
};
```

If you come from Python or Ruby, a struct is not a class. It has no methods,
no references, no hidden header: a struct is a *typed span of memory*, and its
declaration is a layout recipe the compiler follows to decide which byte holds
which field. `item.key` and `item.value` with a dot are not attribute lookups
through a dict — they are offsets the compiler already computed. A struct is a
value: assigning one to another copies every byte, and passing one to a
function copies it again. Nothing is shared behind your back.

Ask the machine what the recipe produced and it answers at compile time.
`sizeof(struct Item)` is the total number of bytes the struct occupies;
`offsetof(struct Item, value)` — from `<stddef.h>` — is the byte distance from
the struct's start to that field; `alignof(struct Item)` — C11's `<stdalign.h>`
— is the alignment the struct demands of its address. The `ds-kit.c` of this
lesson prints exactly these observations, plus a comparison that is the real
subject of the day.

Because alignment is not the compiler's invention — it is the machine's rule.
Every type has an **alignment**: an address it must live at, a power of two.
An `int` wants a 4-byte boundary, a `long` an 8-byte boundary on this machine.
The rule, in full: each member sits at an offset that is a multiple of its own
alignment; the struct's alignment is the largest of its members'; the struct's
size is rounded up to a multiple of that alignment so that consecutive array
elements stay aligned too. When the recipe leaves a member wanting a spot the
previous member already occupies, the compiler inserts **padding** — bytes
that belong to no field.

That is what `struct Scattered` and `struct Compact` in the program
demonstrate: the same three fields, a `char`, a `long`, and a `char`, in two
orders. `Scattered` places `score` at offset 8 — seven padding bytes after
`tag` — and wastes another seven after `flag`, so one element is 24 bytes and
ten of them are 240. `Compact` puts both chars together first and pays only
the interior gap before `score`: 16 bytes an element, 160 for ten. Reordering
fields saved 40% without changing a single line of program logic. That is the
one optimization in this book you get for free.

Two cautions before you run. First, the machine decides the numbers: the
program prints `sizeof(struct Item)` as 24 here because `long` is 8 bytes on
this machine. On a 32-bit build, or on 64-bit Windows where `long` is 4 bytes,
the same source prints 20. Nothing broke — alignment is the *machine's* rule,
and the compiler just obeys the machine you asked. Second, padding is one
reason a struct's bytes are not a file format: `fwrite` a `Scattered` to disk
and read it back on a machine with different alignments and you read garbage.
Byte layouts are lesson 013's business; remember only this: structs are for
memory, formats are for bytes.

One build command, from inside `sandbox/ds-kit/`, with the flag set you
already know from `wordcount` — `-O0 -g` arrived with gdb, and the rest with
the first command you ever ran:

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

Run `./ds-kit` and read its observations alongside this text.

## Code step

One change for this lesson: the whole of `ds-kit.c`, committed together with
this prose. Its end state is tagged `lesson-007`.

```diff
diff --git a/sandbox/ds-kit/ds-kit.c b/sandbox/ds-kit/ds-kit.c
new file mode 100644
index 0000000..455e975
--- /dev/null
+++ b/sandbox/ds-kit/ds-kit.c
@@ -0,0 +1,57 @@
+// ds-kit.c — the data-structures kit of Part 0, starting with its element
+// type and the memory it lives in.
+//
+// Lesson 007: structs as laid-out memory — sizeof, offsetof, padding.
+#include <stdio.h>
+#include <stddef.h>   // offsetof
+#include <stdalign.h> // alignof (C11)
+
+struct Item {
+    char key[16];
+    long value;
+};
+
+// The same three fields in two different orders.
+struct Scattered {
+    char tag;
+    long score;
+    char flag;
+};
+
+struct Compact {
+    char tag;
+    char flag;
+    long score;
+};
+
+int main(void)
+{
+    printf("== scalars ==\n");
+    printf("sizeof(char) = %zu\n", sizeof(char));
+    printf("sizeof(long) = %zu\n", sizeof(long));
+
+    printf("== struct Item ==\n");
+    printf("sizeof(struct Item)  = %zu\n", sizeof(struct Item));
+    printf("alignof(struct Item) = %zu\n", alignof(struct Item));
+    printf("Item.key   offset %zu\n", offsetof(struct Item, key));
+    printf("Item.value offset %zu\n", offsetof(struct Item, value));
+    // key is 16 bytes, so the gap before value is its offset minus 16.
+    printf("padding before value: %zu bytes\n",
+           offsetof(struct Item, value) - 16);
+
+    printf("== same fields, two orders ==\n");
+    printf("struct Scattered { tag, score, flag }: sizeof %zu\n",
+           sizeof(struct Scattered));
+    printf("  tag   offset %zu\n", offsetof(struct Scattered, tag));
+    printf("  score offset %zu\n", offsetof(struct Scattered, score));
+    printf("  flag  offset %zu\n", offsetof(struct Scattered, flag));
+    printf("struct Compact { tag, flag, score }: sizeof %zu\n",
+           sizeof(struct Compact));
+    printf("  tag   offset %zu\n", offsetof(struct Compact, tag));
+    printf("  flag  offset %zu\n", offsetof(struct Compact, flag));
+    printf("  score offset %zu\n", offsetof(struct Compact, score));
+    printf("Scattered[10] = %zu bytes, Compact[10] = %zu bytes\n",
+           sizeof(struct Scattered[10]), sizeof(struct Compact[10]));
+
+    return 0;
+}
```

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — Padding in a fresh struct *(predict-the-output)*

Add a `struct Triple` holding exactly a `char a`, an `int b`, and a `char c`,
in that order, and print its `sizeof` along with the offsets of `b` and `c`.
Before you compile anything, write down the three numbers you expect on your
machine — your first instinct for the size is very likely wrong. Then build,
run, and reconcile every difference between prediction and output with the
alignment rule from this lesson, showing the arithmetic.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-007/ex1.md)

### Exercise 2 — The padding is real memory *(extend-the-code)*

Padding bytes live inside the object — prove it. Add a `DumpBytes` helper
that prints any memory region as space-separated two-digit hex bytes, and dump
exactly `sizeof(struct Scattered)` bytes of a `Scattered` instance whose three
members you have just assigned by hand. Which bytes belong to members, and
which are garbage? Run twice and check whether the garbage is stable. Then
`memset` the struct to zero before assigning the members, dump again, and
explain the difference.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-007/ex2.md)

### Exercise 3 — Why the machine insists *(explain-in-prose)*

Write a short explanation — a paragraph each — of (1) why the compiler places
`Scattered`'s `score` at offset 8 instead of right after `tag`; (2) what would
go wrong at the CPU level if it placed `score` at offset 1; (3) why `fwrite`-ing
`Scattered` structs to a file and reading them back on another machine is not
a portable format. Then apply the confirming check from the solution and make
sure your explanation agrees with the numbers it prints.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-007/ex3.md)

### Exercise 4 — The other machine's layout *(port-to-your-own-machine)*

On your machine `struct Item` is 24 bytes with `value` at offset 16, because
`long` is 8 bytes here. Predict `sizeof(struct Item)`, the offset of `value`,
and `sizeof(struct Scattered)` on a machine where `long` is 4 bytes — a 32-bit
build or 64-bit Windows. Verify wherever you can reach one: `gcc -m32` on a
system with 32-bit libraries, a friend's machine, a container. If no such
machine is reachable, verify by hand from the alignment rule and show your
work. The solution's check prints a machine fingerprint; run it on every
machine you touch and compare.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-007/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 006 — undefined behavior and buffer overflows](lesson-006-undefined-behavior.md) ·
**Next:** [Lesson 008 — dynarray growth: realloc and capacity](lesson-008-dynarray.md) ·
**Code tag:** [`lesson-007`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-007)
