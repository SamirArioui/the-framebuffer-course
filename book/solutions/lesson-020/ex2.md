# Solution: exercise 2 — What the frame cap costs

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 020 — timing with `clock_gettime`](../../lessons/part-0/lesson-020-timing.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Setting the frame budget to zero makes the remainder negative every frame, so
the sleep never fires — the cap is off, and the one-line change keeps
`SleepSec` referenced so `-Wextra` stays quiet. On one machine the measured
numbers were: `./snek 100` takes 3.35 s capped and 0.001 s uncapped —
100 frames go by in about a millisecond when nothing holds them back. That is
the cap doing its job: it turns loop iterations into *time*.

The tick counts make the deeper point. Uncapped, `./snek 100000` finishes in
0.052 s and reports `0 ticks`, and `./snek 2000000` — two million frames —
finishes in 0.9 s and reports `9 ticks`, the same nine ticks a 30 fps run
collects in one second. Game time is real time: the accumulator converts
*elapsed* seconds into ticks, so frame count and tick count are completely
decoupled. That is what lets the snake move at the same speed on a fast
machine and a slow one. Compare `dt` in both runs too: capped it sits at
0.0334, uncapped it collapses to microsecond values — the machine is no
longer the thing pacing the loop.
