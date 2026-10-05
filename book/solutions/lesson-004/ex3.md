# Solution: exercise 3 — Who owns the bytes

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 004 — malloc and free: growing buffers on the heap](../../lessons/part-0/lesson-004-heap-buffers.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The story, grounded in the instrument's output. Each `CountStream` call owns
one `struct Buffer` on its stack frame and, behind it, one heap block that
`malloc` (via `realloc(NULL, …)`) created and every `BufferGrow` replaced.
`realloc` retires the old block when it moves and extends it when it can —
on these fixtures it never moved, which is why every growth printed the
same address (your numbers will differ; the sameness is the news):

```
$ ./wordcount huge.txt
grown to 64 at 0x55ec6d332490
grown to 128 at 0x55ec6d332490
...
grown to 8192 at 0x55ec6d332490
1 1 5001 5000 huge.txt
```

`fclose` gives back the `FILE` object and its stdio buffers — none of which
are ours. What is alive when `main` returns: exactly one block per file
named on the command line — the final capacity of each file's buffer,
reachable from nothing, because the stack frame that held the `Buffer` died
at `CountStream`'s return. That is the leak, and lesson 005 will name and
count it. A one-shot command can afford it — the operating system reclaims
the whole heap at exit; the damage is to principle and to anyone who copies
the pattern. A game loop that leaks one block per frame cannot: 60 frames a
second × 8 KB is a megabyte every two minutes, forever. Free what you
allocate; from lesson 005 on, the tools will insist.
