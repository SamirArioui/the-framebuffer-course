# Solution: exercise 4 — Standard input, revisited

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 004 — malloc and free: growing buffers on the heap](../../lessons/part-0/lesson-004-heap-buffers.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The change is a five-line branch where lesson 001 had a counting loop:
call `CountStream` with `stdin` — the `FILE *` that is already open when
the program starts — accumulate into a zeroed `struct Counts`, and print
the row without a file name:

```
$ ./wordcount < story.txt
2 4 21 11
$ ./wordcount story.txt
2 4 21 11 story.txt
```

It works through a pipe too: `ls | ./wordcount`. This is the promise from
lesson 001's first exercise kept: back then the counting loop lived inside
`main` and stdin support meant writing the loop again; now the loop is a
function and the caller is one call plus a `printf`. The rules from that
exercise carry over unchanged — with file arguments present, stdin is
ignored entirely, which is what `wc` does and what keeps the program
predictable in pipelines. One subtlety: the stdin row drops the last
column because there is no name to print, and the `usage` error disappears
with the arguments — no files no longer means "no input", it means
"whatever the pipe has".
