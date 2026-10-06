# Solution: exercise 2 — The file that closes twice

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 002 — gdb: breakpoints, stepping, stack frames](../../lessons/part-0/lesson-002-gdb.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Running the buggy variant on a terminal dies like this (the counts for
`a.txt` print first; through a pipe the abort can swallow the buffered
output):

```
3 a.txt
free(): double free detected in tcache 2

Program received signal SIGABRT, Aborted.
...
#8  0x00007ffff7cadeae in __GI___libc_free (mem=mem@entry=0x5555555592a0) at ./malloc/malloc.c:3398
#9  0x00007ffff7c85410 in _IO_deallocate_file (fp=0x5555555592a0) at ./libio/libioP.h:958
#10 _IO_new_fclose (fp=0x5555555592a0) at ./libio/iofclose.c:74
#11 0x0000555555555320 in main (argc=2, argv=0x7fffffffdaf8) at wordcount.c:33
```

The one-sentence answer: the file is closed twice — once in `CountBytes` and
once in `main` — and the second `fclose` frees a `FILE` object the first one
already released, which the C library's heap checker aborts on. Frames `#0`
through `#10` are the C library's signal and allocator machinery; walk up
until you reach `main`, and the deepest frame that is *your* code names the
guilty call. The fix is "close exactly once", and this patch takes the
teammate's rule seriously: `CountBytes` consumes the stream, so it closes it,
with the ownership rule in a comment where the next reader will trip over it.
Deleting the callee's `fclose` instead is an equally correct program — the
bug class is two closes, not which close. (Addresses in the trace vary per
machine and per run; the frame shapes do not.) Behavior is unchanged:
`./wordcount a.txt b.txt` still prints `3 a.txt` and `6 b.txt`.
