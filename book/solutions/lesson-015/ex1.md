# Solution: exercise 1 — The rectangle off the left

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 015 — fill-rect onto a memory buffer](../../lessons/part-0/lesson-015-fill-rect.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The rectangle `(-3, 1, 6, 3)` hangs three columns off the left edge. The
left fold removes exactly those three columns: `rw += x` takes the width
from 6 to 3, and `x` moves to 0. The other three folds find nothing to do
(the right edge lands at 3 ≤ 8, the bottom at 4 ≤ 6), so the visible part
is `x=0 y=1 w=3 h=3` — and the instrumented run confirms it:

```
FillRect visible part: x=0 y=1 w=3 h=3
```

Row 1 therefore begins with the orange `(255, 128, 0)` three times:

```
row 1: FF 80 00 FF 80 00 FF 80 00 00 00 00 ...
```

The losing rectangle is the blue pixel at `(7, 5)`: the bottom-right
rectangle `(5, 4, 10, 10)` clips to `x=5 y=4 w=3 h=2`, which covers
`(7, 5)`, and it runs after the `PutPixel` call that planted the blue —
later writes overwrite earlier ones, which is all "layering" ever is in a
framebuffer.
