# Solution: exercise 1 — The diagonal is too fast

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 035 — the platform clock](../../lessons/part-1/lesson-035-clock.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The bug: four independent `if`s add `MARKER_SPEED * dt` to *each* active
axis, so a diagonal frame moves the marker `speed × dt` along x **and** the
same along y — a total of `speed × √2 × dt` per second. The diagonal is 41
percent faster than every other direction, and the fastest way across the
screen is always the corner.

The fix is the direction vector treated as one direction: read the axes
into components, and divide by the vector's length before applying speed.
The four keys can only make axis-aligned or 45-degree movement, so the
length is 1 or √2 — no `sqrt` needed, and the constant is spelled out for
what it is.

Deterministic hold, one second each way (a helper that presses, waits
exactly 1000 ms, releases):

```
axis:     engine: marker at 548,228 (t=1.001)   moved 240 px
diagonal: engine: marker at 478,398 (t=1.002)   moved 170 px on each axis
```

Same time, same speed: the diagonal run's displacement is
`√(170² + 170²) ≈ 240` pixels — exactly the axis run's 240. The marker now
moves at `MARKER_SPEED` in *every* direction, which is what a speed
constant is supposed to mean.
