# Lesson 001 — argv and file input: your first `gcc` command

{{#include ../../stability-horizon.md}}

## Prose

This lesson builds the first program of Part 0: `wordcount`, a tiny `wc`. It
takes file names on the command line and prints how many bytes each file
holds. Everything in Part 0 lives under `sandbox/` — throwaway programs, no
engine sources, built and thrown away by hand. The point today is not the
program; it is the machinery around it: how a C program starts, how it reads
files, and how `gcc` turns one text file into something you can run.

A C program starts at `main`. There is no `if __name__ == "__main__"` guard
and no import machinery deciding what runs: the linker records `main` as the
entry point, and the operating system starts execution there. `main` receives
the command line as two values: `argc`, the number of arguments, and `argv`,
the array of argument strings. Compare that with Python's `sys.argv` or Ruby's
`ARGV`: same idea, different ceremony. `argv[0]` is the program's own name —
the usage message in `main` prints it instead of hard-coding a name — and the
file names we care about are `argv[1]` through `argv[argc - 1]`. The loop that
walks them is an ordinary `for`; C has no iterators.

Two argument-related types look exotic from Python or Ruby. `char *` is what a
string is in C: a pointer to bytes — for today, read "a string"; lessons 003
and 004 open pointers up properly. `char **argv` is a pointer to those string
pointers, the array the shell hands us. And `FILE *f` from `fopen` is a handle
to an open file — an opaque pointer you must close yourself. C has no garbage
collector and no `with` block: `fopen` acquires a resource, `fclose` releases
it, and every path through the loop must do both.

Notice what `fopen` returns on failure and what `main` does about it. There
are no exceptions in C: failure is a value — `NULL` here — that you check.
The error message goes to `stderr` and the loop moves on to the next file,
which is why one missing file does not stop the run. Sending diagnostics to
`stderr` and results to `stdout` is a habit worth forming now: it is what lets
`./wordcount report.txt 2>/dev/null` keep the output clean while the complaint
disappears.

The reading loop is one `fgetc` call per byte, and its return type deserves a
pause. `fgetc` returns `int`, not `char`: a `char` cannot hold every byte
value *plus* the "end of file" signal, so the end marker `EOF` needs its own
room. The loop condition `while ((c = fgetc(f)) != EOF)` assigns and tests in
one stroke. Only when the loop ends does the program print its count with
`printf` — `%lu` for the `unsigned long` counter, `%s` for the file name —
and close the file.

Finally, the command that makes any of this exist. From inside
`sandbox/wordcount/`:

```
gcc -std=c11 -Wall -Wextra wordcount.c -o wordcount
```

Read it flag by flag — none of them are decoration:

- `gcc` runs the whole pipeline on `wordcount.c`: the **preprocessor** expands
  `#include <stdio.h>` into the declarations behind `printf`, `fopen`, and the
  rest; the **compiler** translates the program to assembly; the **assembler**
  turns that into machine code; the **linker** stitches the machine code to
  the C library's implementations and records `main` as the entry point.
- `-std=c11` pins the language version to C11, so the compiler agrees with
  this book about what the code means, today and on your machine.
- `-Wall -Wextra` turn on the compiler's warnings. Warnings are curriculum
  here: a warning is the compiler telling you that your program is legal but
  suspicious, and Part 0 takes every one of them seriously.
- `-o wordcount` names the output program instead of the default `a.out`.

Run it as `./wordcount file.txt file2.txt` — or `./wordcount` alone, to see
the usage message you get when `argc` is too small.

## Code step

One change for this lesson: the whole of `wordcount.c`, committed together
with this prose. Its end state is tagged `lesson-001`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
new file mode 100644
index 0000000..a74772e
--- /dev/null
+++ b/sandbox/wordcount/wordcount.c
@@ -0,0 +1,30 @@
+// wordcount.c — count bytes in every file named on the command line.
+//
+// Lesson 001: argv, file input, and the first gcc command.
+#include <stdio.h>
+
+int main(int argc, char **argv)
+{
+    if (argc < 2) {
+        fprintf(stderr, "usage: %s FILE...\n", argv[0]);
+        return 1;
+    }
+
+    for (int i = 1; i < argc; ++i) {
+        FILE *f = fopen(argv[i], "rb");
+        if (f == NULL) {
+            fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[i]);
+            continue;
+        }
+
+        unsigned long bytes = 0;
+        int c;
+        while ((c = fgetc(f)) != EOF)
+            ++bytes;
+
+        printf("%lu %s\n", bytes, argv[i]);
+        fclose(f);
+    }
+
+    return 0;
+}
```

## Exercises

Four short drills. Each ends with its solution — a diff against this lesson's
end state plus a walkthrough — after the prompt.

### Exercise 1 — Standard input *(extend-the-code)*

Right now `./wordcount` with no file arguments prints the usage message and
stops. Make it read standard input instead and print just the byte count, so
`./wordcount < notes.txt` works the way `wc` does.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-001/ex1.md)

### Exercise 2 — The silent directory *(fix-the-crash)*

`./wordcount .` prints `0 .` as if the directory were an empty file. Opening
a directory succeeds on this system, but reading it fails — and the failure
is invisible in the output. Make the program detect a read error after the
loop (look at what `ferror` reports about the stream) and print a
`cannot read` message for that file instead of a count.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-001/ex2.md)

### Exercise 3 — One character at a time *(measure-the-performance)*

Make a big file — `yes | head -c 100000000 > big.txt` — and time the program
on it with `time ./wordcount big.txt`. Then replace the `fgetc` counting loop
with a block read: a `char` buffer of a few kilobytes filled with `fread`,
counted in chunks. Time it again. How much faster is it, and what is the
machine doing differently?

> **Solution:** [ex3 — diff + walkthrough](../../solutions/lesson-001/ex3.md)

### Exercise 4 — Bytes, not characters *(predict-the-output)*

Create a file with exactly the five bytes `0x48 0x69 0x00 0x0A 0xFF` —
`printf 'Hi\0\n\xff' > five.bin` does it. Predict the exact output line
`./wordcount five.bin` prints, and predict how it differs from `wc -c
five.bin`. Then add a one-line `fprintf(stderr, "c=%d\n", c);` inside the
read loop, run it on that file, and reconcile what you see with your
prediction — especially the last value the loop reports.

> **Solution:** [ex4 — diff + walkthrough](../../solutions/lesson-001/ex4.md)

---

**Part:** [Part 0 — C foundations](../../index.md) ·
**Previous:** — ·
**Next:** [Lesson 002 — gdb: breakpoints, stepping, stack frames](lesson-002-gdb.md) ·
**Code tag:** [`lesson-001`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-001)
