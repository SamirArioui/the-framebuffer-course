# Solution: exercise 2 — The clock that lies

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 035 — the platform clock](../../lessons/part-1/lesson-035-clock.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Lesson 020's "two clocks" drill, revisited behind the seam. The instrument
adds the second clock — `CLOCK_REALTIME`, the wall clock — next to the
monotonic one, and the self-check prints both:

```
engine: clock never backwards over 100000 samples, finest step 20 ns
engine: monotonic 37725.274523 vs wall 1791253957.846222
```

The numbers already tell two different stories. The monotonic reading is
37,725 seconds — a little over ten hours, which is how long this machine
has been running; the clock counts from an arbitrary starting point and
nothing else. The wall reading is 1,791,253,957 seconds — seconds since
1970-01-01, the number the OS uses to tell you what time it is.

Now the explanation, in the engine's terms. The frame's step is
`dt = now − last`. With the monotonic clock, `dt` is always the real
elapsed time, because nothing can move the clock except time passing — the
self-check's 100,000 samples never went backwards, and no administrator,
NTP sync, or daylight-saving change can make them.

With the wall clock, `dt` inherits every adjustment the system makes:

- **the clock steps backwards** (NTP correcting, a manual date change) —
  `dt` goes *negative*, and the marker moves backwards at 240 pixels per
  second;
- **the clock steps forward** — `dt` is suddenly minutes wide, and one
  frame moves the marker across the whole screen;
- **both** are silent: the engine cannot tell an adjustment from a frame.

None of those are hypothetical — lesson 020 watched a wall clock disagree
with a monotonic one on this very machine. The monotonic clock is not a
preference; it is the only one of the two whose differences are *durations*.
(The wall clock is still the right clock for timestamps a human will read —
which is a different question than "how long did this frame take".)
