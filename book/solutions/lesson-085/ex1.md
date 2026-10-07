# Solution: exercise 1 — The curve, predicted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 085 — hero movement](../../lessons/part-5/lesson-085-hero-movement.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The probe prints `hero.move_x` once a frame — the eased fraction of full
speed — so the curve is a column of numbers.

**The prediction.** From rest (`move = 0`), holding a direction gives
`move += (1 − move) × dt/HERO_TIME` each frame. At 60 fps,
`dt = 1/60 ≈ 0.016667`, and with `HERO_TIME = 0.12` the ease per frame is
`k = dt/HERO_TIME ≈ 0.13889`. So:

| frame | `HERO_TIME = 0.12` | `HERO_TIME = 0.24` |
| ----- | ------------------ | ------------------ |
| 1 | `0 + 1×0.1389 = 0.139` | `0 + 1×0.0694 = 0.069` |
| 2 | `0.139 + 0.861×0.1389 = 0.258` | `0.069 + 0.931×0.0694 = 0.134` |
| 3 | `0.258 + 0.742×0.1389 = 0.361` | `0.134 + 0.866×0.0694 = 0.194` |

**Does it reach full speed in three frames?** No — not even close. After
three frames the hero is at 14%, 26%, 36% of full speed (and 7%, 13%,
19% with the doubled constant). The ease is a geometric approach —
`move` closes a fixed *fraction* of the remaining gap each frame — so it
gets asymptotically near full speed but never quite arrives in a fixed
count. In practice it reads as "up to speed" around 5–7 frames
(`HERO_TIME = 0.12`) or 10–14 (`HERO_TIME = 0.24`).

**Which reaches full speed sooner, and by how much?** `HERO_TIME = 0.12`
— roughly twice as fast as `0.24`, frame for frame, because the ease per
frame `k = dt/HERO_TIME` is twice as large. The doubled constant halves
the speed of the whole curve: after any frame `n`, `0.24` is about
halfway through what `0.12` had reached.

On a machine that runs at 60 fps, the probe's column matches those six
numbers to three decimals. (On the authoring machine's headless loop —
about a frame a second — `dt` is huge, `k` clamps to 1, and the probe
prints `0.000` then `1.000`: the same curve, sampled in one step. That is
why the exercise says to check it at 60 fps.) The shape is the point:
`HERO_TIME` is the number of seconds the curve takes to matter, and it is
the knob the "weight" of the hero lives in.
