# Solution: exercise 1 — The second hit looks the same

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 002 — gdb: breakpoints, stepping, stack frames](../../lessons/part-0/lesson-002-gdb.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction: two frames — `CountBytes` on top, `main` below — with
`argc=3` in the caller (two files plus the program name), and the surprise:
`f` holds the *same* address in both hits. Real output at the second stop:

```
Breakpoint 1, CountBytes (f=0x5555555592a0) at wordcount.c:8
8	    unsigned long bytes = 0;
returning 3

Breakpoint 1, CountBytes (f=0x5555555592a0) at wordcount.c:8
8	    unsigned long bytes = 0;
#0  CountBytes (f=0x5555555592a0) at wordcount.c:8
#1  0x00005555555552f4 in main (argc=3, argv=0x7fffffffdab8) at wordcount.c:30
```

Why the same address: `main` closed the first file before opening the
second, and the C library handed the same freed heap chunk to the new
`fopen` — reuse you will see properly in lesson 004. The lesson for
debugging: the backtrace carries function names and argument *values*, and
both calls look identical in shape. Frame values cannot tell you which file
you are on; the instrumenting print can — `returning 3` on stderr proves the
first call already finished before the second stop.
