# Solution: exercise 3 — Float versus integer

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 016 — drawing lines onto the buffer](../../lessons/part-0/lesson-016-lines.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The benchmark draws a 512-pixel diagonal 20 000 times per rasterizer into
a 512×512 scratch buffer and times each loop with `clock()`. On this
machine, at `-O0`:

```
bresenham: 2.03 us/line
float:     1.60 us/line
pixels that disagree: 6
```

Repeat runs land in the same bands (Bresenham ≈ 2.0 µs, float ≈ 1.6 µs) —
the folklore that the float line is slower does not survive contact with a
modern CPU: both loops are memory-bound stores plus trivial arithmetic.
But the second measurement is the one that settles it: for the diagonal
`(0, 0)`–`(7, 5)` the two rasterizers disagree on six of the pixels they
touch — the float line truncates `(1, 0)`, `(4, 2)`, `(5, 3)` where
Bresenham, exact, lights `(1, 1)`, `(4, 3)`, `(5, 4)`. Different pixels
going backwards again, and a vertical line divides by zero. Timing says
"either"; geometry says Bresenham — a line rasterizer's product *is* its
pixels, and only the integer one computes the right ones. Keep this
exercise in your pocket as a measuring lesson: folklore about speed is a
hypothesis, and `clock()` is cheap.
