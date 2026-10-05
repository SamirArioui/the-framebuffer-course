# Lesson 002 — gdb: breakpoints, stepping, stack frames

{{#include ../../stability-horizon.md}}

## Prose

This lesson's subject is not `wordcount`; it is the machinery that runs your
program when your program is wrong. Until now the only observation tool has
been `fprintf`: print a value, guess, print again. That works right up to the
moment the bug is a crash inside code you did not write, or a value that is
correct at one end of a function and wrong at the other. A debugger replaces
guesswork with observation: it starts the program, stops it wherever you say,
and shows you the machine's state exactly as it is at that moment.

Start with the shape of a call. When `main` calls `CountBytes`, control does
not teleport — the machine pushes a *stack frame* onto the call stack: the
arguments being passed (`f`), the callee's local variables (`bytes`, `c`),
and the return address saying where inside `main` to resume. When
`CountBytes` returns, its frame is popped and `main` continues from exactly
there. Python and Ruby show you this chain frozen at the worst possible
moment: the traceback printed with an exception, innermost frame first. C has
the same chain at every instant of every run, crash or not, and `backtrace`
prints it on demand. The debugging payoff: each frame is its own scope.
`bytes` and `c` live in `CountBytes`'s frame, `i` and `argv` live in `main`'s,
and asking about a variable really means asking *in which frame*.

That is why this lesson's code step exists. The counting loop that lived in
`main` now lives in its own function, `static unsigned long CountBytes(FILE
*f)`, called once per file — not for tidiness, for the debugger. A loop
inside `main` has no frame of its own: there is nothing to break *in*, no
boundary between "the counting code" and everything else `main` does, and
nothing for a backtrace to name. A function gives that code a name to break
on, arguments to inspect, and a frame of its own to show up between `main`
and the C library in a stack trace. The `static` keyword is a storage-class
detail worth keeping: it gives the function *internal linkage*, meaning the
name is visible inside this file and nowhere else — in a one-file program it
is mostly a note to the reader that this is not an interface.

Now the build command grows two flags. From inside `sandbox/wordcount/`:

```
gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount
```

- `-g` writes *debug information* into the executable: the mapping from
  machine instructions back to source positions, the names and stack
  locations of variables, the type information `print` needs to format its
  output. Without `-g` gdb can still run the program and stop at addresses,
  but every question in this lesson comes back "no debugging symbols".
  Debug info costs disk space and nothing at run time; release binaries
  leave it out.
- `-O0` disables optimization. Optimization is the compiler rearranging,
  merging, and deleting instructions to make the program faster — useful,
  and fatal to debugging, because at higher settings "the next source line"
  stops meaning anything: instructions from several lines interleave, and
  some lines vanish entirely. `-O0` compiles the source the way it is
  written, so stepping through it is stepping through *it*. The cost is a
  slower binary — that is why release builds optimize, and the optimizer
  returns as real content later in Part 0.

Make two fixtures — `printf 'hi\n' > a.txt` and `printf 'hello\n' > b.txt` —
and start the debugger with `gdb ./wordcount`. gdb prints a banner and some
advice; on its first run Ubuntu's gdb also asks whether to enable
*debuginfod*, a service for downloading source and debug files on demand.
Answer `n` (or `set debuginfod enabled off` in `~/.gdbinit`) and it will not
ask again. The `(gdb)` prompt is the debugger's own REPL; `quit` leaves it.

The first skill is *stopping somewhere on purpose*:

```
(gdb) break CountBytes
Breakpoint 1 at 0x11d9: file wordcount.c, line 8.
(gdb) run a.txt b.txt
...
Breakpoint 1, CountBytes (f=0x5555555592a0) at wordcount.c:8
8	    unsigned long bytes = 0;
(gdb) backtrace
#0  CountBytes (f=0x5555555592a0) at wordcount.c:8
#1  0x00005555555552d2 in main (argc=3, argv=0x7fffffffdaf8) at wordcount.c:29
```

`break CountBytes` sets *breakpoint 1* on the function — stop when control
reaches it. `run` starts the program; when the breakpoint hits, gdb stops
before the first line of the function body has executed and shows you where
you are. `backtrace` (or `bt`) prints the frame chain: frame `#0` is where
the program is now, frame `#1` its caller, and so on up to `main`. Note what
each frame shows you: the function name, its arguments with values (`argc=3`
because two files plus the program name were on the command line), and where
in its source it is paused. The addresses are garbage-collection-free
reality: `f` is the pointer `fopen` returned, the number in frame `#1` is a
return address. The exact numbers depend on your machine; the shape does not.

The second skill is *asking a stopped program questions*:

```
(gdb) info locals
bytes = 93824992247200
c = 0
(gdb) next
10	    while ((c = fgetc(f)) != EOF)
(gdb) print bytes
$1 = 0
(gdb) next
11	        ++bytes;
(gdb) next
10	    while ((c = fgetc(f)) != EOF)
(gdb) print bytes
$2 = 1
```

`info locals` lists the current frame's local variables — and notice what it
shows for `bytes` at the exact moment the breakpoint hit: garbage, because
the line `unsigned long bytes = 0;` has not run yet. Uninitialized memory is
not random; it is whatever bits were already in that stack slot (on this run
they happened to be a pointer value). C does not clear variables for you,
and a debugger is honest enough to show you. `next` executes one source line
and stops again; `print` evaluates an expression in the current frame and
numbers the result `$1`, `$2`, … so you can refer back to it. Stepping the
loop twice turns `$1 = 0` into `$2 = 1`, one counted byte at a time.

`step` looks like `next` but differs in the one way that matters — it
follows calls *into* the called function instead of over them:

```
(gdb) step
0x00007ffff7c8f062 in _IO_getc (fp=0x5555555592a0) at ./libio/getc.c:37
warning: 37	./libio/getc.c: No such file or directory
(gdb) finish
0x00005555555551f4 in CountBytes (f=0x5555555592a0) at wordcount.c:10
10	    while ((c = fgetc(f)) != EOF)
Value returned is $3 = 105
```

Stepping into the `fgetc` call lands inside the C library — and gdb, with no
source for glibc installed, can tell you where you are but not show you the
code (`disassemble` would). `finish` runs until the current frame returns,
drops you back in the caller, and reports the value the frame returned
(`105` is the second byte of `a.txt`: `i`).
When you step into somewhere uninteresting, `finish` is the way back. After
that, `continue` resumes the program until the next breakpoint or the end,
and `quit` leaves gdb (it will ask about killing the running program; the
answer is `y`).

Two last pieces of vocabulary. `next`, `step`, `print`, `info`, `continue`,
and `quit` work from any stopping point — a breakpoint hit, a signal, the end
of `finish` — the *stopping point* and the *current frame* are what commands
act on, and `up`/`down` change the current frame without moving the program.
And when a session has to be reproducible, gdb takes its commands as
arguments: `gdb -batch -ex "break CountBytes" -ex "run a.txt" ./wordcount`
runs the same commands one after another and exits — the form used to check
the outputs quoted in this book. (On macOS the toolchain ships *lldb* instead
of gdb; same job, different spellings — exercise 4 of lesson 003 comes back
to this.)

All transcripts in this lesson were captured with gcc 13.3.0 and gdb 15.1 on
x86-64 Linux; the exact numbers will differ on your machine and the shapes
will not.

## Code step

One change for this lesson: the counting loop moves out of `main` into
`static unsigned long CountBytes(FILE *f)`, called once per file, committed
together with this prose. Its end state is tagged `lesson-002`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index a74772e..cd34962 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,8 +1,17 @@
 // wordcount.c — count bytes in every file named on the command line.
 //
-// Lesson 001: argv, file input, and the first gcc command.
+// Lesson 002: gdb — breakpoints, stepping, and stack frames.
 #include <stdio.h>
 
+static unsigned long CountBytes(FILE *f)
+{
+    unsigned long bytes = 0;
+    int c;
+    while ((c = fgetc(f)) != EOF)
+        ++bytes;
+    return bytes;
+}
+
 int main(int argc, char **argv)
 {
     if (argc < 2) {
@@ -17,10 +26,7 @@ int main(int argc, char **argv)
             continue;
         }
 
-        unsigned long bytes = 0;
-        int c;
-        while ((c = fgetc(f)) != EOF)
-            ++bytes;
+        unsigned long bytes = CountBytes(f);
 
         printf("%lu %s\n", bytes, argv[i]);
         fclose(f);
```

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — The second hit looks the same *(predict-the-output)*

Set a breakpoint on `CountBytes`, `run a.txt b.txt`, and `continue` once —
you are now stopped at the second call. Before typing `backtrace`, write
down exactly what it will show: how many frames, which function names, which
arguments with which values, and whether `f` will hold the same address as
in the first hit or a different one. Then run it and reconcile every line of
the real output with your prediction — and explain the one that surprised
you. (An instrumenting print of your own, to tell the two calls apart, is
allowed and useful.)

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-002/ex1.md)

### Exercise 2 — The file that closes twice *(fix-the-crash)*

A teammate decided that the function which reads a file should also close it,
and added one `fclose(f);` inside `CountBytes` just before its `return` —
but left `main`'s `fclose` alone. `./wordcount a.txt` now counts correctly
and then dies. Find the crash in gdb (`run`, then `backtrace`, then walk up
to the first frame that is our code rather than the C library) and say in one
sentence why the program aborts. Then fix the program so the file is closed
exactly once, leaving the ownership rule obvious to the next reader.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-002/ex2.md)

### Exercise 3 — Conditions see one frame *(explain-in-prose)*

With two files on the command line, try to stop only on the second call:
`break CountBytes if i == 2`. gdb refuses. Explain why `i` is not visible
from that breakpoint, why `break CountBytes if bytes == 0` *is* accepted but
will (typically) never fire at the entry to `CountBytes`, and what
`set $hits = 0` followed by `break CountBytes if ++$hits == 2` does instead —
including what kind of thing `$hits` is and where it lives. Two paragraphs.

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-002/ex3.md)

### Exercise 4 — A counter in two scopes *(extend-the-code)*

Add a call counter to the program: one file-scope variable that `CountBytes`
increments on entry, and one line in `main`, after the last file, reporting
how many times the function ran. Then verify in gdb that `print` can see the
counter from both the `CountBytes` frame and the `main` frame, while `i` can
only be printed from `main`. Report what you saw and what the difference
between the two variables is.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-002/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** [Lesson 001 — argv and file input: your first `gcc` command](lesson-001-first-program.md) ·
**Next:** [Lesson 003 — char buffers: strings by hand](lesson-003-char-buffers.md) ·
**Code tag:** [`lesson-002`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-002)
