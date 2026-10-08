# Solution: exercise 2 — the curve, predicted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 093 — particle bursts and easing](../../lessons/part-5/lesson-093-bursts-easing.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The prediction, written down first.** `EaseOutQuad` is the settling
shape: `1 − (1 − t)²` of the travel at fraction `t` of the time — and
its ends are clamped, so the value sits at its target once `t` reaches
1. For the shipped spark row (`accel 400`, `range 64`):

- **Halfway through the settle** (`t = 0.5`): the fraction is
  `1 − 0.5² = 0.75` — the ease-out curve is three-quarters of the way
  out at half the time. So the spark is `0.75 × 64 = 48 px` out. Not
  32: that is the whole difference between a curve and a line.
- **At the end** (`t = 1`): exactly `64 px` — the row's range. The
  clamp at 1 makes the last frame the target itself, not a value that
  has merely gotten very close.

**The run** — the probe printing the eased value every frame at 17
digits — says exactly that:

```
engine: probe: spark traveled 4.582819037655689 at t 0.036
engine: probe: spark traveled 25.74903114634359 at t 0.227
engine: probe: spark traveled 48.857177534509155 at t 0.514
engine: probe: spark traveled 59.596867977757853 at t 0.738
engine: probe: spark traveled 63.646876510750438 at t 0.926
engine: probe: spark traveled 63.999110491731322 at t 0.996
engine: probe: spark traveled 64 at t 1.000
engine: spark settled at 384,232 — 64 px out, its row's range 64 (exact)
```

The frame nearest the halfway reads `48.86 px` at `t 0.514` — the
predicted `48` at `t 0.5`, a frame's worth of easing later (the run's
frames are ~43 ms of a 400 ms settle, so `t` lands `0.1` apart; the
printed `t` is rounded to three digits and the value is not). Every
intermediate sits where `64 × (1 − (1 − t)²)` puts it. And the last
frames answer the question the effect turns on: `63.999110491731322 at
t 0.996` — the value *approaching* — then `64 at t 1.000`, printed at
seventeen significant digits as **`64`**: not `63.99999999999999`, not
a rounding's width short. The last frame lands on the target, and the
run's own comparison says `exact`.

Why this matters beyond the arithmetic: a value that only approaches
its target (the hero's accel ease, lesson 085's exponential approach)
is right for *feel* — weight, momentum, never quite arriving. A value
that must *be* somewhere — a counter showing the real score, a fade
that ends fully open, a spark where the impact put it — needs the clamp.
The toolkit ships both vocabularies on purpose: the ease set for
arrivals, and the exponential for weight. Choosing which one a value
needs is part of the feel design, and now the engine has names for
both.
