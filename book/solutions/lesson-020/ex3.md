# Solution: exercise 3 — Monotonic versus wall clock

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 020 — timing with `clock_gettime`](../../lessons/part-0/lesson-020-timing.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

The two clocks answer different questions. `CLOCK_REALTIME` is civil time —
seconds since the epoch, which is what a log file or a savegame timestamp
wants. `CLOCK_MONOTONIC` is time since some fixed point (roughly boot), and
the kernel guarantees it only ever moves forward at a steady rate. The
instrumented run shows both on this machine: `mono=17568.9…` against
`wall=1791233614.…` — uptime versus calendar.

The guarantee is the reason the game uses monotonic. Wall clock is *stepped*:
an NTP correction, a dual-boot fix, or `date -s` can move it forward or
backward while the game runs. A `dt` computed from `CLOCK_REALTIME` would then
report a negative or huge frame, and the accumulator would either swallow
ticks or explode the snake through the wall. Monotonic deltas are always the
true elapsed time, so `dt` stays honest and the fixed timestep stays stable —
the two clocks' deltas agree today, and only one of them promises to agree
tomorrow.
