# Lesson 003 — char buffers: strings by hand

{{#include ../../stability-horizon.md}}

## Prose

This lesson gives `wordcount` a voice. From now on it prints

```
lines words bytes longest filename
```

for every file, all counted in one pass. Most of the new work is text — and
in C, text is not a type. It is bytes on the floor and a convention.

**A C string is bytes with a zero at the end.** Python strings know their
length — `len(s)` reads a field. A C string is just bytes in memory with one
rule: the sequence stops at a NUL byte, `'\0'`, the zero byte, and every
string-consuming routine works by *walking* until it finds one. `printf`'s
`%s`, the library's `strlen`, the library's `strcpy` — all walkers. The
`char *` from lesson 001 is "a pointer to the first byte of such a
sequence", and a string literal like `"hi"` really is three bytes in the
program: `h`, `i`, NUL. Two consequences follow. First, asking a string its
length costs a walk — you either walk it every time or remember the length
yourself. Second, forget the terminator and the walkers do not stop: they
stride past your data into whatever bytes follow — stale contents of the
same buffer, other variables, whatever the stack holds — and hand you
lengths and text that are lies. Exercise 2 is that bug, live.

**`char line[256]`** is an array: 256 contiguous bytes of storage named
`line`, subscripted from zero (`line[0]` is the first byte, the same
zero-based counting `argv` used). It lives in `CountStream`'s stack frame,
so it exists while that function runs and is gone when it returns. Two
numbers must never be confused: the *length* of the string currently in the
buffer — how many bytes were stored, tracked as `len` in the code — and the
*capacity*, `sizeof line`, which is 256 bytes and never moves. A string of
N characters needs N + 1 bytes because of the NUL, so this buffer holds
lines up to 255 characters and the last byte is reserved. The `if (len <
sizeof line - 1)` guard is that promise in code — every stored character
passes through it — and `line[len] = '\0'` closes the string before anyone
measures or prints it.

**`LineLen`** is this program's own `strlen`: it walks `s[n]` from `n = 0`
until `s[n]` is the NUL and returns the count. The parameter type
`const char *s` means "a string I promise not to modify" — the compiler
will complain if the body tries. The library has `strlen`, of course; the
point of writing this one is that afterwards "C string" is not a phrase you
have read about but a machine you have built.

**The single pass.** One `fgetc` at a time, and every byte feeds three
things: the byte counter; the line buffer, until `'\n'` arrives and the
completed line is measured, compared against the longest so far, counted,
and the buffer reset; and a two-state word machine. The `in_word` flag
remembers whether the previous byte was inside a word: whitespace (`isspace`
from the new `ctype.h` include — space, tab, newline and cousins) ends a
word, a non-whitespace byte after whitespace starts one, and starting one
is where `words` ticks up. Lines are counted on `'\n'` bytes, the same rule
as `wc -l`, and a final line without a trailing newline is still measured
for `longest` but not counted as a line — `wc` agrees. Cross-check the first
three columns against the real thing whenever you like:

```
$ printf 'hello world\nhi there\n' > story.txt
$ ./wordcount story.txt
2 4 21 11 story.txt
$ wc -l -w -c story.txt
 2  4 21 story.txt
```

`longest` is our own column: the byte length of the longest line, our
definition (GNU `wc`'s `-L` is a cousin that measures display width, so a
tab counts as several columns there and one byte here).

**Structs and pointers, gently.** `struct Counts` is a plain bundle of
named members — four counters under one name. `struct Counts counts = {0};`
declares one and zeroes every member in one stroke. `CountStream(f,
&counts)` passes two handles: the file, and `&counts`, the *address* of the
struct — exactly the kind of "pointer to a thing" that `FILE *` already is.
In the callee, the parameter is `struct Counts *out`, and `out->bytes` is
dereference-and-member in one symbol: "the `bytes` of the struct `out`
points to". The callee fills the caller's counters in place and returns
nothing (`void`): one call per file, four results, no copying. Pointers get
opened properly in lesson 004 and structs in lesson 007; today just hold
the shape.

**The ceiling, on purpose.** `line[256]` is a decision with a visible
price. A 300-character line is read in full — bytes, words, and lines all
count it correctly — but only its first 255 characters fit in the buffer, so
`longest` reports 255:

```
$ ./wordcount long.txt
1 1 301 255 long.txt
```

Nothing crashes and nothing is corrupted; the program reports a number it
cannot know. That is the fixed buffer's honest limitation — defined,
deliberate, and the reason lesson 004 gives the line buffer a heap that can
grow.

The build command is unchanged from lesson 002 — gdb stays useful against
this program as it grows:

```
gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount
```

## Code step

One change for this lesson: the byte counter becomes `struct Counts`, the
counting loop becomes the single-pass `CountStream` with a hand-managed
`char line[256]` buffer and a `LineLen` helper, and the output row grows
its columns — committed together with this prose. Its end state is tagged
`lesson-003`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index cd34962..a7e8a96 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,15 +1,62 @@
-// wordcount.c — count bytes in every file named on the command line.
+// wordcount.c — count lines, words, bytes, and the longest line in every
+// file named on the command line.
 //
-// Lesson 002: gdb — breakpoints, stepping, and stack frames.
+// Lesson 003: char buffers — strings by hand.
+#include <ctype.h>
 #include <stdio.h>
 
-static unsigned long CountBytes(FILE *f)
+struct Counts {
+    unsigned long lines, words, bytes, longest;
+};
+
+// LineLen is this program's own strlen: it walks a NUL-terminated string
+// and returns its length in bytes.
+static unsigned long LineLen(const char *s)
 {
-    unsigned long bytes = 0;
+    unsigned long n = 0;
+    while (s[n] != '\0')
+        ++n;
+    return n;
+}
+
+// CountStream reads f to EOF and accumulates counts into *out. Each line is
+// collected in a fixed buffer, so the longest line it can report is 255
+// bytes; lesson 004 lifts that ceiling.
+static void CountStream(FILE *f, struct Counts *out)
+{
+    char line[256];
+    unsigned long len = 0;
+    int in_word = 0;
     int c;
-    while ((c = fgetc(f)) != EOF)
-        ++bytes;
-    return bytes;
+
+    while ((c = fgetc(f)) != EOF) {
+        ++out->bytes;
+        if (c == '\n') {
+            line[len] = '\0';
+            unsigned long line_len = LineLen(line);
+            if (line_len > out->longest)
+                out->longest = line_len;
+            ++out->lines;
+            len = 0;
+            in_word = 0;
+        } else {
+            if (len < sizeof line - 1)
+                line[len++] = (char)c;
+            if (isspace(c)) {
+                in_word = 0;
+            } else if (!in_word) {
+                in_word = 1;
+                ++out->words;
+            }
+        }
+    }
+
+    if (len > 0) {
+        line[len] = '\0';
+        unsigned long line_len = LineLen(line);
+        if (line_len > out->longest)
+            out->longest = line_len;
+    }
 }
 
 int main(int argc, char **argv)
@@ -26,9 +73,11 @@ int main(int argc, char **argv)
             continue;
         }
 
-        unsigned long bytes = CountBytes(f);
+        struct Counts counts = {0};
+        CountStream(f, &counts);
 
-        printf("%lu %s\n", bytes, argv[i]);
+        printf("%lu %lu %lu %lu %s\n", counts.lines, counts.words,
+               counts.bytes, counts.longest, argv[i]);
         fclose(f);
     }
 
```

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — Three hundred characters *(predict-the-output)*

Build a file with exactly one 300-character line:
`printf '%300s\n' '' | tr ' ' 'x' > long.txt`. Before running anything,
predict the exact output row of `./wordcount long.txt` — every column — and
predict how it will differ from `wc -l -w -c long.txt` plus the true length
of the line. Then run both, reconcile every column of your prediction with
the real output, and say precisely which part of the program's rules
produced the number that surprised you.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-003/ex1.md)

### Exercise 2 — The quoted line lies *(fix-the-crash)*

A teammate added a feature: quote the file's longest line at the end of
each file. Their attempt compiles and runs, but on `story.txt` the counts
row is right while the quote reads `"hi thererld"` — and on a file without a
trailing newline it quotes garbage or nothing at all. Find every way the
attempt goes wrong, and fix the feature so the quote is always the file's
longest line, properly terminated, for every input. Their additions to
`CountStream`:

```c
    char longest_text[128];               /* at the top of the function */

            for (unsigned long i = 0; i < len; ++i)
                longest_text[i] = line[i];   /* in the newline branch */

    fprintf(stderr, "longest line: \"%s\"\n", longest_text);  /* at the end */
```

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-003/ex2.md)

### Exercise 3 — The longest word *(extend-the-code)*

Add a fifth column, `wlongest`, printed between `longest` and the file name:
the length in bytes of the longest whitespace-delimited word in the file.
The single pass already knows where words start and end. Then check the
column on fixtures whose answers you can verify by eye — and explain what it
reports for `long.txt`, and why that number differs from what `longest`
reports for the same file.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-003/ex3.md)

### Exercise 4 — The debugger on your machine *(port-to-your-own-machine)*

Take lesson 002's debugger session to the machine you actually own: build
this lesson's program there, break on `LineLen`, run on a fixture of your
own making, and single-step the length walk until you can say exactly what
it counts and why. If your machine does not ship gdb, translate the session
to its debugger (on macOS the toolchain gives you lldb) and report the one
command you could not translate. The instrumented build in the solution is
a portable way to confirm what your stepping told you.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-003/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 002 — gdb: breakpoints, stepping, stack frames](lesson-002-gdb.md) ·
**Next:** [Lesson 004 — malloc and free: growing buffers on the heap](lesson-004-heap-buffers.md) ·
**Code tag:** [`lesson-003`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-003)
