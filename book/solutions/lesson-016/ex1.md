# Solution: exercise 1 — The vertical line through everything

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 016 — drawing lines onto the buffer](../../lessons/part-0/lesson-016-lines.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The line runs from `y = -2` to `y = 9` at `x = 4`; the vertical clip cuts
both ends to the buffer and the instrumented run confirms the endpoints:

```
DrawLine clipped -> (4,0)-(4,5)
```

(the first two lines are the other two scene lines). Bresenham then lights
`(4, 0)` through `(4, 5)` — one pixel per row — so every row's bytes at
offsets 12–14 become `FF 00 FF`, the magenta channel triple. The stolen
pixels: `(4, 2)` was white (the inside rectangle) and `(4, 3)` was both
white *and* the yellow diagonal's — the line runs last and wins all six.
Note what clipping bought: the loop stepped exactly six pixels, not
twelve, and not one of them needed a bounds check. A vertical line is
also the case the naive float slope dies on; Bresenham treats it as just
another line (`dx = 0`, only the `e2 <= dx` branch ever fires).
