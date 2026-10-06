# Lesson 005 — leaks made visible with sanitizers

{{#include ../../stability-horizon.md}}

## Prose

Lesson 004 ended in deliberate debt: `CountStream` allocates a heap block
per file and never gives it back, with a comment promising that lesson 005
would make the leak visible. Today is that day. The tool is a *sanitizer* —
a compiler mode that rewrites the program so that its own memory mistakes
cannot hide — and the payoff is a bug report you did not have to write.

**What a sanitizer is.** Building with `-fsanitize=address` makes gcc
instrument the program: nearly every memory access gains a check, and a
runtime library replaces the memory machinery underneath. `malloc` and
`free` are no longer the C library's — they are the runtime's, and each
allocation is recorded (with the call stack that requested it), fenced with
*redzones* of poisoned memory, and tracked in a *shadow memory* map that
says, for every byte of the program's memory, whether touching it is legal.
Out-of-bounds writes land in redzones, use-after-free touches memory the
runtime has poisoned, and — the part today needs — *LeakSanitizer* rides
along on Linux and audits the heap when the program exits: every block
still allocated, with no pointer to it left anywhere, is a leak, and the
report is the recorded stack of whoever asked for it.

**The build command** grows two flags. From inside `sandbox/wordcount/`:

```
gcc -std=c11 -O0 -g -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer wordcount.c -o wordcount
```

- `-fsanitize=address` is the instrumentation above. It costs speed (about
  1.6× on this program as written — exercise 2 measures it) and
  memory (the shadow map, the redzones, a quarantine of freed blocks), and
  it buys certainty about every memory access the program makes.
- `-fno-omit-frame-pointer` keeps one register reserved for the frame
  pointer in every function. Compilers like to reuse that register for
  data; when they do, walking the call stack requires per-function unwind
  tables and tracebacks get spotty. For debugging builds we keep the chain
  of frames intact so every report's stack trace is complete. It costs
  almost nothing and it is why the tracebacks below are trustworthy.

**The report.** Here is the lesson-004 state — the leaking program, exactly
as `lesson-004` keeps it — built with that command and run on two files:

```
$ ./wordcount story.txt long.txt
2 4 21 11 story.txt
1 1 301 300 long.txt

=================================================================
==102059==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 576 byte(s) in 2 object(s) allocated from:
    #0 0x7658364fc778 in realloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:85
    #1 0x620bd3f3f4b8 in BufferGrow .../sandbox/wordcount/wordcount.c:36
    #2 0x620bd3f3f594 in BufferPush .../sandbox/wordcount/wordcount.c:44
    #3 0x620bd3f3f8d9 in CountStream .../sandbox/wordcount/wordcount.c:78
    #4 0x620bd3f3fdec in main .../sandbox/wordcount/wordcount.c:111
    ...

SUMMARY: AddressSanitizer: 576 byte(s) leaked in 2 allocation(s).
```

Read it like a receipt. `Direct leak of 576 byte(s) in 2 object(s)` — two
blocks, one per file (`story.txt`'s 64-byte block plus `long.txt`'s
512-byte block; the intermediate blocks died at each `realloc`, as they
should). *Direct* means nothing points at these blocks at all; *indirect*
would mean they are reachable only through other leaked blocks. The stack
trace is not something the program printed — it is the recorded call stack
of the allocation, and it names the crime scene precisely: `BufferGrow` did
the allocation, from `BufferPush`, from `CountStream`, once per file. The
process exits with status 1: a leak is a failing grade. (Addresses, paths
and the process id differ on your machine; the shape does not. On a
terminal the counts print before the report; through a pipe they can vanish
entirely — the sanitizer ends the process before stdio's buffer flushes,
the buffering story from lesson 001.)

**The fix** is one function and one call. `BufferFree` hands the block back
with `free` and resets the struct so the buffer cannot be used to touch
freed memory afterwards. `free(NULL)` is explicitly legal — freeing an
empty buffer is a no-op — and calling `free` twice on the same block is
undefined behavior of its own kind, no matter how harmless the second call
looks. The
discipline the fix encodes is the one worth keeping for the rest of the
course: every allocation has exactly one owner, every owner frees on every
path out, and a function that takes a buffer in takes responsibility for
its end. Rebuild, and the same run is silent — counts only, exit status 0.
The pattern for the rest of Part 0: when a lesson touches memory bugs, the
build command brings the tool that makes them visible, and the state is not
done until the tool is quiet.

One more flag to know for the day you want both checkers at once: the
sanitizers compose, and `-fsanitize=address,undefined` is what lesson 006
builds with.

## Code step

One change for this lesson: `BufferFree` is added and every path out of
`CountStream` frees the buffer exactly once, so the sanitizer run is clean —
committed together with this prose. Its end state is tagged `lesson-005`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index cf8a0fb..e63016f 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,7 +1,7 @@
 // wordcount.c — count lines, words, bytes, and the longest line in every
 // file named on the command line.
 //
-// Lesson 004: malloc and free — growing buffers on the heap.
+// Lesson 005: leaks made visible with sanitizers.
 #include <ctype.h>
 #include <stdio.h>
 #include <stdlib.h>
@@ -17,10 +17,8 @@ struct Buffer {
     size_t len, cap;
 };
 
-// BufferInit prepares an empty buffer. The first push allocates.
-//
-// NOTE: nothing in this lesson ever frees the buffer's memory. We are
-// leaking this on purpose; lesson 005 makes it visible.
+// BufferInit prepares an empty buffer. The first push allocates; the
+// buffer's memory is owned here and released by BufferFree.
 static void BufferInit(struct Buffer *buf)
 {
     buf->data = NULL;
@@ -28,6 +26,17 @@ static void BufferInit(struct Buffer *buf)
     buf->cap = 0;
 }
 
+// BufferFree gives the buffer's memory back to the heap. Every path out of
+// CountStream must call it exactly once. free(NULL) is legal, so freeing an
+// empty buffer is safe.
+static void BufferFree(struct Buffer *buf)
+{
+    free(buf->data);
+    buf->data = NULL;
+    buf->len = 0;
+    buf->cap = 0;
+}
+
 // BufferGrow makes room for more bytes, doubling the capacity each time so
 // that pushing N bytes costs O(log N) reallocations instead of N.
 static void BufferGrow(struct Buffer *buf)
@@ -91,6 +100,8 @@ static void CountStream(FILE *f, struct Counts *out)
         if (line_len > out->longest)
             out->longest = line_len;
     }
+
+    BufferFree(&line);
 }
 
 int main(int argc, char **argv)
```

## Exercises

Three short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Three verdicts *(predict-the-output)*

With this lesson's state built under the sanitizer, predict what
LeakSanitizer will say — if anything — for three runs: `./wordcount
nope.txt` (a file that does not exist), `./wordcount empty.txt`, and
`./wordcount story.txt`. For each: does a report appear, what does it
claim, and what is the exit status? Then comment out the program's single
free — the solution shows the smallest way — and check every prediction
against the real verdicts.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-005/ex1.md)

### Exercise 2 — What the certainty costs *(measure-the-performance)*

Measure the sanitizer's price. Make a 20 MB text file
(`yes 'the quick brown fox' | head -c 20000000 > med.txt`), build this
lesson's program twice — plain (`gcc -std=c11 -O0 -g -Wall -Wextra …`) and
with the sanitizer command from the prose — and time both on the file with
`time ./wordcount med.txt`. If the two numbers are closer than the prose's
"factor of two" promise, apply the block-read change from the solution and
re-measure to unmask the signal. Report the ratios, and one sentence on
where the extra time goes.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-005/ex2.md)

### Exercise 3 — How the sanitizer knows *(explain-in-prose)*

Explain the mechanism in two paragraphs: where the report's stack trace
comes from (the program never printed it), what the runtime must be doing
to `malloc` and `free` to catch a leak at exit, and why the same program
under the plain build shows no sign of the bug at all. The instrumented
build in the solution prints every free as it happens; use it to check the
story you write.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-005/ex3.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 004 — malloc and free: growing buffers on the heap](lesson-004-heap-buffers.md) ·
**Next:** [Lesson 006 — undefined behavior and buffer overflows](lesson-006-undefined-behavior.md) ·
**Code tag:** [`lesson-005`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-005)
