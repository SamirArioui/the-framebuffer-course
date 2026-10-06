# Lesson 004 — malloc and free: growing buffers on the heap

{{#include ../../stability-horizon.md}}

## Prose

Lesson 003 ended with a confession in the output: a 300-character line was
measured as 255, because `char line[256]` is a size fixed at compile time
and the program silently dropped what did not fit. Today the line buffer
moves to the only place in C where memory can grow while the program runs:
the heap. The 255 becomes 300 becomes 5000 becomes whatever the file
contains.

**Stack and heap.** Everything local so far — `line`, `counts`, `f`, `c` —
has lived on the stack: storage carved out when a function is called and
destroyed when it returns, with a size the compiler fixed in advance. The
heap is the other kind: a pool of memory the *process* owns and hands out
in whatever sizes you ask for, at whatever moment you ask. Python and Ruby
put every object there and send a garbage collector to tidy up; C gives you
the same memory and hands you the collector's job. Three verbs run it:
`malloc(n)` gives you a fresh block of `n` bytes, `realloc(p, n)` moves or
resizes an existing block to `n` bytes, and `free(p)` gives a block back.
Everything else about heap programming is bookkeeping over those three.

Two types appear with them. Sizes in C are `size_t` — the type `sizeof`
returns, wide enough for any object in memory; `printf` spells it `%zu`.
And `malloc` returns `void *`, a pointer with no pointee type, which C
quietly converts to whatever pointer you assign it to — `char *p =
malloc(n)` compiles as written. (C++ is stricter and wants a cast; the
engine at Part 1 will see that rule.)

**Realloc, in detail.** `realloc` is the growing-buffer workhorse, and its
contract has three clauses worth memorizing. It copies your bytes to the
new block if it must move — content is preserved. If it moves, the *old*
block is freed, so the old pointer is dead afterwards no matter what. And
`realloc(NULL, n)` is defined to behave exactly like `malloc(n)` — which is
why `BufferGrow` never needs a special first call. Failure is the fourth
clause: `realloc` returns `NULL` and leaves the old block untouched, and
the classic C bug is assigning `p = realloc(p, n)` and thereby dropping the
only pointer to the old block when that happens. This lesson's `BufferGrow`
takes the happy path and does not check; that carelessness is exactly what
lesson 006 stops, with the tools to see why.

**The Buffer.** `struct Buffer` is the shape nearly every dynamic container
in C takes: `char *data` — where the bytes live, `size_t len` — how many
are useful, `size_t cap` — how many fit. Three small functions own it:
`BufferInit` empties it (and allocates nothing yet — `data` is `NULL`, and
the first push pays for the first block), `BufferGrow` doubles the
capacity, and `BufferPush` appends one byte, growing first when the buffer
is full. Doubling is a policy with a cost analysis behind it: after N
pushes there have been about log₂(N) reallocations, and the total number of
bytes copied across all of them is proportional to N, not N². Exercise 2
asks you to measure the alternative.

One call out the door of `BufferGrow`: `realloc` hands back `void *` like
`malloc`, and each grown block *replaces* the previous one — the pointers
that exercise 3's instrument prints at each growth are the same story
`realloc`'s first clause tells.

**Ownership, and the leak we are keeping.** Here is the sentence this
lesson owes you: **we are leaking the buffer on purpose; lesson 005 makes
it visible.** `CountStream` allocates the line buffer on the heap and never
frees it. When `CountStream` returns, the `line` struct — a stack local —
vanishes, but the heap block it pointed at stays allocated, unreachable,
until the process exits. The operating system reclaims the whole heap at
exit, so a one-shot command like ours pays for this in nothing but
principle. A game loop that leaks every frame pays in the only currency a
game has. The reason to write it this way now: the fix is one function call
long, and the *tool that proves it* is the interesting half — lesson 005
runs this exact program under a leak sanitizer and shows you the report
before showing you the cure.

Ownership rules worth stating while the code is small: the block belongs to
the `Buffer`, one `Buffer` per `CountStream` call, `realloc` retires the
old block on every growth, `fclose` gives back the `FILE` object and its
internal buffers but knows nothing about ours, and at the end of the
program exactly one block per file is still alive.

**The ceiling, lifted.** Same program, same rows, honest numbers:

```
$ ./wordcount story.txt
2 4 21 11 story.txt
$ ./wordcount long.txt
1 1 301 300 long.txt
$ ./wordcount huge.txt
1 1 5001 5000 huge.txt
```

`long.txt` is lesson 003's 300-character line; `huge.txt` is one
5000-character line. Both are measured truly now. The build command is
unchanged:

```
gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount
```

## Code step

One change for this lesson: the fixed `char line[256]` becomes a
heap-growable `struct Buffer` with `BufferInit`, `BufferGrow`, and
`BufferPush`, grown by `malloc`/`realloc` with capacity doubling — and
deliberately never freed. Committed together with this prose; its end state
is tagged `lesson-004`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index a7e8a96..cf8a0fb 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,14 +1,50 @@
 // wordcount.c — count lines, words, bytes, and the longest line in every
 // file named on the command line.
 //
-// Lesson 003: char buffers — strings by hand.
+// Lesson 004: malloc and free — growing buffers on the heap.
 #include <ctype.h>
 #include <stdio.h>
+#include <stdlib.h>
 
 struct Counts {
     unsigned long lines, words, bytes, longest;
 };
 
+// Buffer is a growable byte buffer. data points at len bytes of useful
+// content with room for cap bytes in total.
+struct Buffer {
+    char *data;
+    size_t len, cap;
+};
+
+// BufferInit prepares an empty buffer. The first push allocates.
+//
+// NOTE: nothing in this lesson ever frees the buffer's memory. We are
+// leaking this on purpose; lesson 005 makes it visible.
+static void BufferInit(struct Buffer *buf)
+{
+    buf->data = NULL;
+    buf->len = 0;
+    buf->cap = 0;
+}
+
+// BufferGrow makes room for more bytes, doubling the capacity each time so
+// that pushing N bytes costs O(log N) reallocations instead of N.
+static void BufferGrow(struct Buffer *buf)
+{
+    size_t new_cap = buf->cap == 0 ? 64 : buf->cap * 2;
+    buf->data = realloc(buf->data, new_cap);
+    buf->cap = new_cap;
+}
+
+// BufferPush appends one byte, growing first if the buffer is full.
+static void BufferPush(struct Buffer *buf, char c)
+{
+    if (buf->len == buf->cap)
+        BufferGrow(buf);
+    buf->data[buf->len++] = c;
+}
+
 // LineLen is this program's own strlen: it walks a NUL-terminated string
 // and returns its length in bytes.
 static unsigned long LineLen(const char *s)
@@ -19,29 +55,27 @@ static unsigned long LineLen(const char *s)
     return n;
 }
 
-// CountStream reads f to EOF and accumulates counts into *out. Each line is
-// collected in a fixed buffer, so the longest line it can report is 255
-// bytes; lesson 004 lifts that ceiling.
+// CountStream reads f to EOF and accumulates counts into *out. The line
+// buffer lives on the heap now, so lines of any length are measured truly.
 static void CountStream(FILE *f, struct Counts *out)
 {
-    char line[256];
-    unsigned long len = 0;
+    struct Buffer line;
+    BufferInit(&line);
     int in_word = 0;
     int c;
 
     while ((c = fgetc(f)) != EOF) {
         ++out->bytes;
         if (c == '\n') {
-            line[len] = '\0';
-            unsigned long line_len = LineLen(line);
+            BufferPush(&line, '\0');
+            unsigned long line_len = LineLen(line.data);
             if (line_len > out->longest)
                 out->longest = line_len;
             ++out->lines;
-            len = 0;
+            line.len = 0;
             in_word = 0;
         } else {
-            if (len < sizeof line - 1)
-                line[len++] = (char)c;
+            BufferPush(&line, (char)c);
             if (isspace(c)) {
                 in_word = 0;
             } else if (!in_word) {
@@ -51,9 +85,9 @@ static void CountStream(FILE *f, struct Counts *out)
         }
     }
 
-    if (len > 0) {
-        line[len] = '\0';
-        unsigned long line_len = LineLen(line);
+    if (line.len > 0) {
+        BufferPush(&line, '\0');
+        unsigned long line_len = LineLen(line.data);
         if (line_len > out->longest)
             out->longest = line_len;
     }
```

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — The doubling sequence *(predict-the-output)*

Take a file with a single 5000-character line (`huge.txt` from the prose).
`BufferGrow` runs more than once while the line is read. Before running
anything, predict how many times it runs and the exact sequence of
capacities the buffer passes through. Then add one instrumenting print to
`BufferGrow`, run it, and reconcile the real sequence with your prediction —
including what happens to the very first push and why the final capacity is
not 5000.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-004/ex1.md)

### Exercise 2 — One byte at a time *(measure-the-performance)*

Change the growth policy from doubling to "one byte more than last time",
rebuild, and make two fixtures with a single line each: 100 000 characters
(`head -c 100000 /dev/zero | tr '\0' 'x' > big.txt`) and 1 000 000
characters, likewise. Time both policies on both files with
`time ./wordcount FILE`. The prose claims doubling copies O(N) bytes in
total while growing one byte at a time copies O(N²) — check that claim
against your measurements. If the timings do not show the gap the theory
promises, find out where the predicted copying went, and say what the
timings you measured do and do not prove.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-004/ex2.md)

### Exercise 3 — Who owns the bytes *(explain-in-prose)*

Write the ownership story of this program in two paragraphs: which memory
belongs to whom, what every `realloc` did to the block that came before it,
what `fclose` gives back and what it does not, and exactly which blocks are
still alive when `main` returns. Then say what lesson 005 will object to,
and why a one-shot command can afford it while a game loop cannot. The
instrumented build in the solution prints each block's address as the
buffer grows; use it to ground the story in real output.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-004/ex3.md)

### Exercise 4 — Standard input, revisited *(extend-the-code)*

The promise from lesson 001's first exercise: once the counting lives in
its own function, reading standard input when no files are named is nearly
free. Make `./wordcount < story.txt` work — no arguments means read `stdin`
and print the row without a file name — and say why the change is now
smaller than the loop it replaces.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-004/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 003 — char buffers: strings by hand](lesson-003-char-buffers.md) ·
**Next:** [Lesson 005 — leaks made visible with sanitizers](lesson-005-leaks.md) ·
**Code tag:** [`lesson-004`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-004)
