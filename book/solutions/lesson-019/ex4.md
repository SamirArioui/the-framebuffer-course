# Solution: exercise 4 — Why three phases

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 019 — the game loop](../../lessons/part-0/lesson-019-game-loop.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The instrumented run of `./snek 2` prints, in order: `ProcessInput`, `Update`,
`Render`, `frame=1`, then the same three phases again and `frame=2`. Every
iteration walks the cycle exactly once, input first and drawing last.

That order is the argument. `ProcessInput` runs first so the update step sees
every key the player pressed *this* frame — read-then-simulate, never
simulate-then-read. `Update` owns the state: if it drew anything, the
simulation would depend on how often the render happened, and the game would
speed up on a fast monitor. `Render` observes and formats — if it advanced the
state, the trace would betray it: a loop running N iterations would report
more than N frames, and the double-`Render` experiment shows the shape of that
bug immediately, with two `frame=` lines per iteration while `Update` still
runs once. Input, simulation, presentation: one owner per responsibility, and
a frame counter that counts iterations, not work.

The duplication of `fprintf` calls in the patch is throwaway instrumentation —
delete it once the trace has made its point.
