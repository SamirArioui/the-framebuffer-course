# Solution: exercise 3 — Offsets, by hand

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 013 — raw bytes and pixel formats](../../lessons/part-0/lesson-013-raw-bytes.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

With stride `8 * 3 = 24`, the formula `y * 24 + x * 3` gives
`(0, 0) → 0`, `(1, 0) → 3`, `(7, 5) → 141`, and `(3, 2) → 57`. The
instrumented run on stderr agrees for the three pixels the scene draws:

```
PutPixel(0,0) -> offset 0
PutPixel(1,0) -> offset 3
PutPixel(7,5) -> offset 141
```

The forgotten-stride version computes `y * w + x * 3`, so row 1's leftmost
pixel lands at offset `1 * 8 = 8` instead of `24` — inside row 0. Row 0
draws correctly (its `y * w` term is zero, which is exactly why the bug
hides), and every later row writes into the rows above it. A rectangle
drawn that way folds downward into itself: the picture looks like the
buffer was squeezed. The lesson is that a stride is not decoration — it is
what makes row *y*'s bytes disjoint from every other row's.
