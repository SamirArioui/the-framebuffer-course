# Solution: exercise 4 — Integrating over the fixed timestep

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 020 — timing with `clock_gettime`](../../lessons/part-0/lesson-020-timing.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The patch adds one state variable, one line of integration inside the tick
loop, and one field in the trace. `pos` advances by `speed * TICK_LEN` — one
cell per second of *game* time — and the run shows it landing exactly:
`pos=0.10` at the first tick, `0.90` at tick nine after 30 frames, `1.00` by
the tenth tick. Because the integration happens per tick and the timestep is
fixed, the position is the same function of game time on every machine and
every frame rate: two million uncapped frames and thirty capped ones both put
`pos` at 0.90 after nine ticks.

That is the pattern the snake will use. Compare integrating `speed * dt` per
*frame* instead: the position would then depend on how the frame times fell,
jitter accumulates into wobble, and a two-second stall teleports the snake
through a wall. Per-tick integration over a fixed step trades a little
smoothness — movement is quantized to 100 ms — for physics that cannot run
away from the clock. Lesson 022 turns this `pos` into a cell on screen.
