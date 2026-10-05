# Solution: exercise 2 — Counting what survived

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 015 — fill-rect onto a memory buffer](../../lessons/part-0/lesson-015-fill-rect.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The clipped rectangles are `3×3`, `2×2`, `3×2`, and `3×2`, so the
hand-computed counts are 9, 4, 6, and 6 — and the program agrees:

```
rect off the left:        9 pixels
rect off the top-right:   4 pixels
rect off the bottom-right: 6 pixels
rect fully inside:        6 pixels
rect entirely outside:    0 pixels
pixels written: 25
```

The change itself is small: `FillRect` returns `rw * rh` after the write
loops, and the empty path returns 0. The return value is computed *after*
the fold, which is why it counts what landed on the buffer and not what
was asked for — `(5, 4, 10, 10)` asked for 100 pixels and wrote 6. That
distinction is exactly what makes the return useful: it is the visible
area, and the entirely-outside rectangle reports 0 without touching a
single byte. Counting after clipping is also how a game budgets its draw
calls — the pixels that survived are the pixels the machine has to pay
for.
