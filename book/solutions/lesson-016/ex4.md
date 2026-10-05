# Solution: exercise 4 — The error term, watched

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 016 — drawing lines onto the buffer](../../lessons/part-0/lesson-016-lines.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The trace of the yellow diagonal `(0, 0)`–`(7, 5)` reads

```
step (0,0) err=2
step (1,1) err=4
step (2,1) err=-1
step (3,2) err=1
...
```

Each round, `e2 = 2 * err` is compared against `dy` and `dx`: the first
comparison decides whether `x` advances and the error is corrected by
`dy`; the second whether `y` advances and the error is corrected by `dx`.
So one round is "step right, and decide up or not" for shallow lines —
two integer adds and two compares. `err` is an integer throughout because
it starts as `dx + dy` (the scaled distance of the *first* pixel, chosen
so the very first comparison needs no special case) and is only ever
adjusted by the integer `dx` and `dy` — there is no division and no
rounding anywhere in the loop, which is exactly why there is no drift at
any coordinate size. Where would the float version diverge? At `(1, 1)`:
the ideal line sits at `y ≈ 0.71` when `x = 1`, and truncation throws that
away to `0` — while the trace shows Bresenham rounding to row 1 there, the
exact nearest-pixel choice.
