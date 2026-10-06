# Lesson 006 — undefined behavior and buffer overflows

{{#include ../../stability-horizon.md}}

## Prose

Lesson 005's leak was a bug with rules: the program misused nothing, it
merely forgot something, and the standard still promised what it always
promised. Today's subject is the other kind of bug — the kind where the
rules themselves stop. This is the last and most important C concept in
Part 0's first half, because everything in a game engine's memory
management is built on knowing where it lives.

**What "undefined" means.** For some operations the C standard does not
specify a wrong answer — it specifies *no* answer at all: if a program
performs one, its behavior is undefined, and the implementation is
congratulated for anything that happens. Not "it crashes". Not "it wraps".
Python raises `OverflowError`? Python integers grow without bound. Ruby
switches to `Bignum`. C has no such transition: do it, and the contract is
void. The three ways to fall off the map that matter to this program:

- **Signed overflow.** `int` arithmetic whose true result does not fit the
  type. `2147483647 + 1` is undefined — not "wraps to −2147483648", which
  is merely what you will observe while nobody is watching (exercise 1
  watches).
- **Out-of-bounds access.** Reading or writing an array element outside
  the array, or dereferencing a pointer outside its object. One step past
  the end may be *pointed at* but never touched. This is the fate our own
  buffer flirted with in lesson 003 and exercise 2 stages properly.
- **Indeterminate values.** Using memory before writing it: lesson 002's
  `info locals` garbage was this — a value that exists only as whatever
  bits were left behind.

Contrast the fourth fate, which is *defined* and still wrong: `size_t`
arithmetic wraps around modulo its width. No rules are broken; you simply
get a number that means nothing. Our capacity doubling `buf->cap * 2`
walks this line — it never becomes undefined, it just quietly becomes
small, and the next write overflows the small allocation that results.

**The compiler's assumptions.** Here is why undefined behavior is worse
than a wrong value. A compiler optimizing a program may assume undefined
behavior *never happens*, because no valid program contains it, and reason
freely from there — branches can be proven dead, checks can be deleted,
loops can be rearranged. At `-O0` you observe the wraparound; at higher
settings you may observe the assumption itself. The optimizer gets its own
lessons in the `paint` program later in Part 0; for now, the rule is: do
not build on what undefined behavior "does".

**How overflows look under the sanitizers.** The address sanitizer makes
the spatial cases visible at the moment they happen. A scratch file
anywhere (this one is four lines of `main`) is enough to see the anatomy:

```c
#include <stdlib.h>

int main(void)
{
    char *p = malloc(4);
    p[4] = 'x';
    free(p);
    return 0;
}
```

```
==105939==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x502000000014 ...
WRITE of size 1 at 0x502000000014 thread T0
    #0 0x6386f4bb72db in main /tmp/opencode/work/demo_overflow.c:6
    ...
0x502000000014 is located 0 bytes after 4-byte region [0x502000000010,0x502000000014)
allocated by thread T0 here:
    #0 0x7e6cfd2fd9c7 in malloc ...
```

Read the two halves: the fault line — read or write, of what size, in
which function — and the geography — `0 bytes after 4-byte region`, the
exact allocation you stepped past, with the stack that allocated it. (The
addresses and the path in those frames are this machine's; yours will
differ and the shape will not.) The
undefined-behavior sanitizer reports the non-spatial cases as `runtime
error:` lines naming the operation and the type it overflowed. The two
checkers compose, and from today's build onward the command carries both:

```
gcc -std=c11 -O0 -g -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer wordcount.c -o wordcount
```

**The hardening.** This lesson's code step converts the buffer's three
silent failure modes into loud, clean ones. `BufferAt` is now the only
sanctioned way to reach the buffer's bytes: an index outside the
allocation is checked before the access, and instead of undefined behavior
the program reports the index and stops — "cleanly" meaning exactly what
the error paths do everywhere in this program: say what went wrong, free
what you own (`BufferFree` on the way out, so the sanitizer stays quiet
even on the failure path), and exit with a failing status. `BufferGrow`
checks its doubling for wraparound *after* computing it — unsigned
arithmetic wraps by definition, so the check reads the wrapped value — and
checks `realloc`'s return for `NULL`, the classic `p = realloc(p, n)` bug
where failure loses the only pointer to the old block. Our version keeps
the old pointer until a new one exists.

The program is unchanged in behavior for every input that worked before —
and it still builds with the plain lesson-002 command
(`gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount`), because
the sanitizers are a debugging mode, never a dependency. The hardening is
ordinary code; only the checkers are optional.

## Code step

One change for this lesson: the buffer grows an overflow check and a
`realloc` failure path that both error out cleanly, and `BufferAt` replaces
raw `data[i]` indexing — committed together with this prose. Its end state
is tagged `lesson-006`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index e63016f..e7d8151 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,7 +1,7 @@
 // wordcount.c — count lines, words, bytes, and the longest line in every
 // file named on the command line.
 //
-// Lesson 005: leaks made visible with sanitizers.
+// Lesson 006: undefined behavior and buffer overflows.
 #include <ctype.h>
 #include <stdio.h>
 #include <stdlib.h>
@@ -37,12 +37,37 @@ static void BufferFree(struct Buffer *buf)
     buf->cap = 0;
 }
 
+// BufferAt is the only sanctioned way to reach the buffer's bytes: it
+// turns an out-of-range index from undefined behavior into a clean error.
+static char *BufferAt(struct Buffer *buf, size_t i)
+{
+    if (i >= buf->cap) {
+        fprintf(stderr, "wordcount: buffer index %zu out of range\n", i);
+        BufferFree(buf);
+        exit(1);
+    }
+    return &buf->data[i];
+}
+
 // BufferGrow makes room for more bytes, doubling the capacity each time so
-// that pushing N bytes costs O(log N) reallocations instead of N.
+// that pushing N bytes costs O(log N) reallocations instead of N. Both
+// failure modes end the program cleanly: size_t arithmetic wraps around by
+// definition, so the doubling is checked for it, and realloc can fail.
 static void BufferGrow(struct Buffer *buf)
 {
     size_t new_cap = buf->cap == 0 ? 64 : buf->cap * 2;
-    buf->data = realloc(buf->data, new_cap);
+    if (new_cap < buf->cap) {
+        fprintf(stderr, "wordcount: buffer capacity overflow\n");
+        BufferFree(buf);
+        exit(1);
+    }
+    char *p = realloc(buf->data, new_cap);
+    if (p == NULL) {
+        fprintf(stderr, "wordcount: out of memory\n");
+        BufferFree(buf);
+        exit(1);
+    }
+    buf->data = p;
     buf->cap = new_cap;
 }
 
@@ -51,7 +76,8 @@ static void BufferPush(struct Buffer *buf, char c)
 {
     if (buf->len == buf->cap)
         BufferGrow(buf);
-    buf->data[buf->len++] = c;
+    *BufferAt(buf, buf->len) = c;
+    ++buf->len;
 }
 
 // LineLen is this program's own strlen: it walks a NUL-terminated string
```

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — The value that is not promised *(predict-the-output)*

Add a demonstrator to `main`: an `int` holding `2147483647`, incremented
once, printed with `%d`. Predict three things before running anything: what
`-fsanitize=undefined` will say about the increment, what value the print
shows under the sanitizer build, and what the plain build prints. Then run
both builds and reconcile — including why "it wrapped to −2147483648" is
an observation and not a promise, and what a compiler is therefore allowed
to do with the code around it.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-006/ex1.md)

### Exercise 2 — The empty line that reads backwards *(fix-the-crash)*

A teammate added a small feature: report the last character of the file's
last line. Their attempt compiles and works on `story.txt` — but the
sanitizer build aborts with a heap-buffer-overflow report the moment an
empty line shows up below real content, and the plain build pretends nothing
happened.
Their additions to `CountStream`:

```c
    char last = '?';                        /* next to in_word */

            last = line.data[line.len - 1]; /* first line of the newline branch */

    fprintf(stderr, "last char: '%c'\n", last);   /* just before BufferFree */
```

Find out exactly what that read does when the line is empty, then fix the
feature so it reports the last character of the last line that has one and
never reaches outside the buffer — the lesson's accessor is the sanctioned
way. Confirm with both builds on a file with an empty line.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-006/ex2.md)

### Exercise 3 — Three fates *(explain-in-prose)*

Sort this lesson's failure modes into the language's three fates:
undefined behavior (signed overflow, out-of-bounds access), defined but
wrong (`size_t` wraparound), and indeterminate (uninitialized reads). Then
say which fate each of the code step's three hardening checks defends
against, and why "it worked when I tested it" is evidence against none of
them. Two paragraphs. The instrumented build in the solution prints every
capacity check as it happens.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-006/ex3.md)

### Exercise 4 — The price of the check *(measure-the-performance)*

`BufferAt` runs once per byte pushed. Measure what that costs: time this
lesson's program on lesson 005's 20 MB `med.txt` under the plain build,
then apply the raw-indexing variant from the solution — the check removed —
and time it again. Is the check affordable on this program's budget? Say
where the cost actually goes when the build is `-O0`, and what that implies
for builds that inline the check away.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-006/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 005 — leaks made visible with sanitizers](lesson-005-leaks.md) ·
**Next:** [Lesson 007 — structs: sizeof, alignment, and padding](lesson-007-struct-layout.md) ·
**Code tag:** [`lesson-006`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-006)
