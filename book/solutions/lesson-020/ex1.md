# Solution: exercise 1 — Predicting the ticks

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 020 — timing with `clock_gettime`](../../lessons/part-0/lesson-020-timing.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction: 30 frames at the 30 fps cap take about one second of wall
clock, and one second at the fixed 100 ms timestep is ten ticks. The real run
takes `real 0m1.004s` and ends `done after 30 frames, 9 ticks` — one tick
shy. The instrumented accumulator explains the missing tick and the
frame-by-frame picture. Each frame drains `dt` (about 0.0335 s) into
`tick_accum`, and whenever the accumulator reaches `TICK_LEN` (0.1 s) it is
subtracted and `tick` advances: the trace shows a tick firing every third
frame, each time leaving `accum` near zero. Frame 1 reports `dt=0.0000`,
because `prev` was sampled immediately before the loop — that is roughly one
frame's worth of time never fed to the accumulator, and 30 frames of real
time land just under ten ticks. Run 60 or 90 frames and the law of large
numbers smooths it: tick counts land within one of `frames * 10 / 30`.
