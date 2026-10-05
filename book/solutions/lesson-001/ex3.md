# Solution: exercise 3 — One character at a time

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 001 — argv and file input: your first `gcc` command](../../lessons/part-0/lesson-001-first-program.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

On one machine the 100 MB file took about 0.15 s through `fgetc` and about
0.01 s through `fread` — an order of magnitude. Your numbers will differ;
the ratio will not.

Both loops do the same arithmetic, so the time is not in counting. Every
`fgetc` call is a function call that checks the stream's buffer state and
returns one byte; done 100 million times, that per-byte overhead is the
program. `fread` moves 4096 bytes per call and lets the C library's
buffering do its job in bulk — under the hood both are reading the file
through the same system calls, but the block version asks for a kilobyte at
a time and barely calls anything per byte.

This is the shape of most performance work in C: measure first (that is what
`time` is for here), change one thing, measure again. The counting result is
unchanged — the block loop still handles the short read at the end of the
file, because `fread` returns the number of elements it actually got, and a
return of `0` is the loop's end. Keeping the `ferror` check from exercise 2
is optional here, but the habit is cheap.
