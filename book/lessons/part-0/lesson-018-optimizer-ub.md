# Lesson 018 — the optimizer and undefined behavior

{{#include ../../stability-horizon.md}}

## Prose

Lesson 017 shipped a fill loop written naively on purpose and promised
that this lesson would find out what the optimizer does with it. The bill
arrives today. This is the most important lesson in Part 0's back half:
**a program that works at `-O0` is not a correct program**, and the gap
between "works" and "correct" is undefined behavior.

First, the new build flag. `-O2` means "optimize the code aggressively":
the compiler reorders, inlines, eliminates, and — the point of today —
*proves things* about your program and acts on those proofs:

```
gcc -std=c11 -O2 -g -Wall -Wextra paint.c -o paint
```

Build the lesson-017 program that way and gcc objects before you even run
it:

```
paint.c: In function ‘ClearBuffer’:
paint.c:165:10: warning: iteration 2147483647 invokes undefined behavior [-Waggressive-loop-optimizations]
  165 |         i++;
      |         ~^~
paint.c:162:14: note: within this loop
  162 |     while (i >= 0) {          /* keep going while the offset is positive */
      |            ~~^~
```

That is the compiler pointing at the exact loop lesson 017 flagged: the
fill that stops only when its counter turns negative. Run the `-O2`
binary the way lesson 017 ran it — with `timeout`, which runs a command
with a time limit and kills it if the limit expires (its exit code 124
means "the time was up"):

```
$ timeout 5 ./paint
$ echo $?
124
```

No output, no file, nothing: the program entered `ClearBuffer` and never
left. The `-O0` build of the *same source* runs fine and prints
`clear ended at offset -2147483648`. The source did not change. The
machine did not change. One compiler flag changed what the program *is*.

Here is why. Signed integer overflow in C is **undefined behavior**:
when `i++` would pass `INT_MAX`, the standard imposes *no requirements at
all* on what happens next. Not "it wraps around" — nothing. The critical
corollary is what this licenses the compiler to do: if overflow never has
defined consequences, then **the compiler may assume it never happens**,
and rewrite the program under that assumption. gcc looks at
`while (i >= 0) { … i++; }` and reasons: `i` starts at 0 and only
increases, so `i` can only stop being non-negative by overflowing — which
may not happen — so *the loop condition is always true* — so the exit is
unreachable — so it may be deleted. The loop is compiled as endless,
which is exactly the hang you measured. Undefined behavior does not mean
"the program crashes" or "the program wraps"; it means the compiler may
do anything at all — including deleting your code — and be right by the
standard's rules. (Exercise 2 shows that even an explicit guard against
the wrap gets deleted on the same reasoning.)

Why did `-O0` look fine? Luck. At `-O0` the compiler emits plain machine
arithmetic, the hardware wraps `i` to `INT_MIN`, and that wrapped negative
value accidentally delivers the exit the loop needed. The wrap is a fact
about x86 arithmetic, not about C — the language never promised it, so
the program was broken from the moment it was written; `-O0` just hid it.
That is the habit this lesson exists to build: when behavior differs
between optimization levels, the bug is *never* the optimizer.

The fix is small and total: give the loop a real bound. `ClearBuffer`
becomes `for (int i = 0; i < nbytes; i++)` — the loop now ends because
`i` reaches `nbytes`, which is defined behavior at every optimization
level. The proof is in the runs: build with both commands, run both, and
the outputs are identical (`clear covered 144 bytes` …
`wrote paint.bmp (8x6, 24 bpp)`), and both builds write the same valid
`paint.bmp` — `file` still reports `PC bitmap, Windows 3.x format,
8 x 6 x 24 … cbSize 198`. No warnings, at either level.

Take three general rules away from this. **One:** undefined behavior is
not a rare crash; it is a voided contract — the compiler gets to assume
your worst case cannot occur and optimize accordingly. **Two:** the
guards you write against "impossible" cases are deleted along with the
impossibility; the only real fix is code whose defined behavior covers
every input. **Three:** treat every warning about "undefined behavior" as
a bug report, not a style note — gcc's `-Waggressive-loop-optimizations`
caught this one at build time before the hang ever ran. Speed is
explicitly *not* today's subject: `-O2` is here as a correctness
instrument, and optimizing for performance happens much later in the
course, with the right tools for it.

Build command for the fixed program — the one the rest of the course
assumes:

```
gcc -std=c11 -O0 -g -Wall -Wextra paint.c -o paint
```

## Code step

One change for this lesson: `ClearBuffer` gets a real loop bound, and the
naive fill is gone. Committed together with this prose; its end state is
tagged `lesson-018`.

```diff
diff --git a/sandbox/paint/paint.c b/sandbox/paint/paint.c
index 3aeb7bd..c1ef4ec 100644
--- a/sandbox/paint/paint.c
+++ b/sandbox/paint/paint.c
@@ -1,10 +1,8 @@
-// paint.c — Lesson 017: writing a real image file by hand.
+// paint.c — Lesson 018: the optimizer and undefined behavior.
 //
-// A pixel buffer is bytes (lesson 013), a file header is pinned bytes
-// (lesson 014), rectangles fold-clip (lesson 015), lines rasterize
-// (lesson 016).  Now the buffer becomes a real file: WriteBmp emits the
-// 54-byte header plus bottom-up, padded rows — and a small scene lands
-// in paint.bmp.
+// The lesson-017 ClearBuffer relied on signed overflow to stop.  This is
+// the fix: the loop gets a real bound, so -O0 and -O2 agree on what the
+// program does — and both write the same valid BMP.
 #include <stdio.h>
 #include <stdlib.h>
 
@@ -152,19 +150,14 @@ static void DrawLine(unsigned char *px, int w, int h,
     }
 }
 
-// ClearBuffer — fill the buffer with one byte value.  This fill is
-// written naively on purpose: its stop condition is the offset turning
-// negative, a stop that only a wrapping counter can deliver.
-// Lesson 018 finds out what the optimizer does with it.
+// ClearBuffer — fill the buffer with one byte value.  The loop bound is
+// the buffer size: no wraparound, no undefined behavior, so every
+// optimization level does the same thing.
 static void ClearBuffer(unsigned char *px, int nbytes, unsigned char v)
 {
-    int i = 0;
-    while (i >= 0) {          /* keep going while the offset is positive */
-        if (i < nbytes)       /* clip: never write past the buffer */
-            px[i] = v;
-        i++;
-    }
-    printf("clear ended at offset %d\n", i);
+    for (int i = 0; i < nbytes; i++)
+        px[i] = v;
+    printf("clear covered %d bytes\n", nbytes);
 }
 
 // BuildBmpHeader — lay out the 54-byte BMP header field by field.
```

## Exercises

Four short drills. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Predict the wreckage *(predict-the-output)*

Before running a single command, write down your predictions for the
lesson-017 program built at `-O2`: (a) the exact words of the warning gcc
prints at build time (which line of which function does it blame?), and
(b) for `timeout 5 ./paint` — what does the run print, and what is
`echo $?` afterwards? Then apply this lesson's solution patch (it swaps
the naive fill back in, with one marker `fprintf` at its entry so you can
see how far the run gets), build it both ways, run both, and reconcile
every prediction — especially the marker's appearance in the hung run.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-018/ex1.md)

### Exercise 2 — The guard that gets deleted *(explain-in-prose)*

A wrap *guard* seems like it should save the naive loop: stop explicitly
when the offset turns negative. Rewrite `ClearBuffer` as an unbounded
loop — `for (;;) { … i++; if (i < 0) break; }` — keeping the explicit
break as the only exit. Predict what the `-O2` build now does with that
break, run both builds, and explain in your own words why the guard does
not save the program: what exactly is the compiler entitled to conclude
about `i`, and on whose authority? Then state what *would* have saved it.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-018/ex2.md)

### Exercise 3 — Review a colleague's fill *(fix-the-crash)*

A colleague sends you their gradient fill to add to `paint.c`, "the same
style as the old clear":

```c
static void FillGradient(unsigned char *px, int w, int h)
{
    int i = 0;
    while (i >= 0) {
        if (i < w * h * 3)
            px[i] = (unsigned char)((i / 3) * 255 / (w * h - 1));
        i++;
    }
}
```

Called after `ClearBuffer`, it produces the same story as lesson 017:
fine at `-O0`, endless at `-O2` (confirm both before fixing anything).
Fix the function properly — a real bound, defined behavior everywhere,
the gradient drawn correctly — add the call to `main`, and verify the
program behaves identically at `-O0` and `-O2` now.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-018/ex3.md)

### Exercise 4 — Prove the fix *(extend-the-code)*

The lesson's claim is that the fixed program is boring: same behavior at
`-O0` and `-O2`, same valid `paint.bmp` from both. Make the program
check the first half of that itself: right after `ClearBuffer`, verify
that every byte of the buffer holds the background value and print the
count of bytes that do not. Then do the rest by hand and report: run both
builds into two files and `diff` them, and run `file` on the `paint.bmp`
of each. What does each check actually prove — and which class of bug can
`diff` of the outputs *not* catch?

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-018/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 017 — writing a real image file by hand](lesson-017-image-file.md) ·
**Next:** [Lesson 019 — the game loop](lesson-019-game-loop.md) ·
**Code tag:** [`lesson-018`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-018)
