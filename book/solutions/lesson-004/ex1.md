# Solution: exercise 1 — The doubling sequence

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 004 — malloc and free: growing buffers on the heap](../../lessons/part-0/lesson-004-heap-buffers.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction: eight growths, capacities 64, 128, 256, 512, 1024, 2048,
4096, 8192. The instrument confirms it exactly:

```
$ ./wordcount huge.txt
cap 64
cap 128
cap 256
cap 512
cap 1024
cap 2048
cap 4096
cap 8192
1 1 5001 5000 huge.txt
```

Three reconciliation points. The first push grows from `cap = 0` to 64 —
that `realloc(NULL, 64)` is a `malloc` in disguise, so the first block and
the later ones are the same mechanism. The count is eight because the line
needs 5001 stored bytes (5000 characters plus the NUL `BufferPush` adds
before measuring) and the sequence only passes 4096 on the way to 8192.
And the final capacity is not 5000 because capacity is headroom, not fit:
doubling overshoots by up to a factor of two, the buffer ends at 8192 with
3191 bytes of slack, and that slack is exactly what buys the O(log N)
growth count. A grow-to-exact policy would end at 5001 and cost a
reallocation for nearly every byte — exercise 2 measures that trade.
