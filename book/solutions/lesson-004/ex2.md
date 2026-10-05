# Solution: exercise 2 — One byte at a time

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 004 — malloc and free: growing buffers on the heap](../../lessons/part-0/lesson-004-heap-buffers.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The policy change is one line — `buf->cap + 1` instead of the doubling
expression — and the measurements on one machine (gcc 13.3.0, x86-64) are:

```
grow-by-one, 100k:  real 0m0.001s
grow-by-one, 1M:    real 0m0.008s
doubling,   100k:   real 0m0.001s
doubling,   1M:     real 0m0.005s
```

At 100 000 characters the policies are indistinguishable; at 1 000 000 the
alternative is about 1.6× slower — a few milliseconds, not the catastrophe
the copy theory predicts. If every growth copied, one-byte-at-a-time would
move about N²/2 = 5×10¹¹ bytes and run for minutes; clearly it moved almost
nothing. Where did the copying go? Into `realloc`'s first contract clause:
when nothing sits above the block in the heap, it grows **in place** and
copies zero bytes. Exercise 3's address instrument is the evidence — every
growth on these fixtures prints the same block address. What the timings do
prove is the cost of a million `realloc` calls; what they do not prove is
the worst case, which appears as soon as the heap is fragmented enough that
the block must move. Theory bounds that case; measurement showed this one.
Measure first — including measuring that the theory is not currently
hurting you.
