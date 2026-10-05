# Lesson 019 — the game loop

{{#include ../../stability-horizon.md}}

## Prose

This lesson starts the last program of Part 0: `snek`, a terminal snake game
that grows lesson by lesson until, at the end of the part, it is a real game
in one C file. Today it is nothing but the skeleton every game in the world
has — the game loop — and the machinery that makes a game loop testable. The
point is not the snake. The point is the loop shape you will meet again in
every engine, including the one this course is about to build.

A game is not a script that starts, does a thing, and exits. It is a loop
that runs until the player leaves: it reads what the player did, advances the
world by one step, draws the result, and repeats. Python and Ruby game
libraries wrap this in a `run()` call or a framework; C games spell it out.
Here it is, in `main`: `while (running) { ProcessInput(); Update(); Render(); }`.
Three phases, in that order, once per iteration of the loop. One iteration is
one *frame*, and the `frame` counter names it.

The order is not decoration. `ProcessInput` collects what happened —
keystrokes, and eventually more — and nothing else. `Update` advances the
simulation: positions, scores, collisions, lives. `Render` reads the current
state and turns it into something visible, and *only* that. Two rules follow,
and every engine enforces them: **update never draws**, and **render never
advances the state**. Break the first and the game's speed depends on the
frame rate; break the second and one frame can tick the world twice — in this
program the symptom would be a frame counter that reports more updates than
the loop ran. Keeping the phases apart also keeps the loop testable: today's
program prints its per-frame state instead of drawing anything, and you can
predict its exact output before running it.

State in a Python game lives in object attributes; here it lives in file-scope
variables. The `static` keyword gives the three phases shared access to
`running`, `frame`, and `max_frames` while keeping those names out of every
other file — and in a one-file program it simply reads as "this is the game's
state". `running` is the loop's condition: a plain `int` flag the phases
consult. `Update` sets it to zero when the frame budget is exhausted, which is
also the loop's only exit — for now there is no keyboard to quit with, so the
program must be told how long to live.

That budget comes from the command line: `./snek FRAMES`. Real games run
forever, which makes them awkward to test; bounding the frame count turns the
loop into something a terminal can exercise non-interactively — every run of
`./snek 3` is the same three frames, and its output is checkable. The parse is
`strtoul`, the unsigned-integer sibling of `atoi`, and it is worth the extra
argument: `strtoul` converts the leading digits of the string and sets `end`
to point at the first character it did *not* consume, so the test `*end !=
'\0'` rejects strings like `12x` that a plain `atoi` would quietly truncate to
`12`. A zero frame count is rejected too: the loop must run at least once.
(There is one string family that slips past this validation and makes the
program appear to hang — exercise 2 is waiting with it.)

One stream decision to notice: the per-frame trace goes to **stderr**, and so
does the final `done after N frames` line. stdout stays empty on purpose.
Lesson 001 split results from diagnostics, and the split is about to matter:
from lesson 022 on, stdout carries the game's display — the frame the terminal
shows — while the trace is *about* the run, the kind of thing you want
visible when testing and invisible when redirecting. `./snek 3 2>/dev/null`
should therefore print nothing at all.

Finally, the command that builds all of this, from inside `sandbox/snek/`:

```
gcc -std=c11 -O0 -g -Wall -Wextra snek.c -o snek
```

The flags are the ones you already know: `-std=c11` pins the language
version, `-Wall -Wextra` turn on warnings (still curriculum, still taken
seriously), `-O0 -g` keep the binary debuggable for `gdb`, and `-o snek`
names the program. Run `./snek 3` and you should see three frame lines and
the exit summary.

## Code step

One change for this lesson: the whole of `snek.c`, committed together with
this prose. Its end state is tagged `lesson-019`.

```diff
diff --git a/sandbox/snek/snek.c b/sandbox/snek/snek.c
new file mode 100644
index 0000000..a5bb4a5
--- /dev/null
+++ b/sandbox/snek/snek.c
@@ -0,0 +1,54 @@
+// snek.c — a terminal snake game, grown lesson by lesson.
+//
+// Lesson 019: the game loop — ProcessInput, Update, Render, frame by frame.
+#include <errno.h>
+#include <stdio.h>
+#include <stdlib.h>
+
+static int running;              /* the loop runs while this is true */
+static unsigned long frame;      /* frames since the loop started */
+static unsigned long max_frames; /* stop after this many frames */
+
+static void ProcessInput(void)
+{
+    // No keyboard yet — lesson 021 teaches the terminal.
+}
+
+static void Update(void)
+{
+    ++frame;
+    if (frame >= max_frames)
+        running = 0;
+}
+
+static void Render(void)
+{
+    fprintf(stderr, "frame=%lu\n", frame);
+}
+
+int main(int argc, char **argv)
+{
+    if (argc != 2) {
+        fprintf(stderr, "usage: %s FRAMES\n", argv[0]);
+        return 1;
+    }
+
+    char *end;
+    errno = 0;
+    max_frames = strtoul(argv[1], &end, 10);
+    if (*end != '\0' || max_frames == 0) {
+        fprintf(stderr, "%s: FRAMES must be a positive integer, got '%s'\n",
+                argv[0], argv[1]);
+        return 1;
+    }
+
+    running = 1;
+    while (running) {
+        ProcessInput();
+        Update();
+        Render();
+    }
+
+    fprintf(stderr, "done after %lu frames\n", frame);
+    return 0;
+}
```

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — The life of `strtoul` *(predict-the-output)*

Predict the exact output of each of these runs — all of it, error lines
included:

```
./snek 3
./snek 0
./snek abc
./snek 12x
./snek
```

Then run each one and reconcile every difference between prediction and
reality. To check the *mechanism*, add one instrumenting line right after the
`strtoul` call — `fprintf(stderr, "parsed=%lu end='%s'\n", max_frames, end);`
is enough — rerun the five cases, and explain each result in terms of where
`end` points and what the validation test sees.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-019/ex1.md)

### Exercise 2 — The string that never ends *(fix-the-crash)*

`./snek -5` and `./snek 99999999999999999999` both sail past the validation
and then print frame lines forever — the program appears to hang. Find out
what `strtoul` actually returns for each of those strings and why `*end !=
'\0'` cannot see it, then fix the parse so both are rejected with the
program's usual error message. A real rejection, not a cap on the frame
count.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-019/ex2.md)

### Exercise 3 — Pack the state *(extend-the-code)*

The three phases reach their state through file-scope variables. Put
`running`, `frame`, and `max_frames` into a `struct Game`, and give each phase
a `struct Game *g` parameter; `main` declares one instance and passes its
address. No behavior changes — `./snek 3` must produce the same output as
before. This is the C idiom behind the C++ classes that arrive at the end of
Part 0: the struct is the object, the phases are its methods, and the pointer
is `this`.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-019/ex3.md)

### Exercise 4 — Why three phases *(explain-in-prose)*

Explain in prose why a game loop is `ProcessInput(); Update(); Render();` and
not any other order or grouping: what each phase owns, why `Update` must not
draw and `Render` must not advance the state, and what the trace would show if
`Render` were called twice per iteration. Before you write, make the program
agree with your explanation: add one `fprintf` at the top of each phase
printing the phase's name, run `./snek 2`, and check that the order you see is
the order you are about to argue for.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-019/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 018 — the optimizer and undefined behavior](lesson-018-optimizer-ub.md) ·
**Next:** [Lesson 020 — timing with `clock_gettime`](lesson-020-timing.md) ·
**Code tag:** [`lesson-019`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-019)
