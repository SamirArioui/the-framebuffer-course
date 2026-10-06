# Solution: exercise 2 — The step and the wall

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 056 — the mover that stops at walls](../../lessons/part-2/lesson-056-mover.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

Two predictions to write down first.

**The jump.** The log's odd pair —

```
engine: sprite at 18,156 (t=3.867)
engine: sprite blocked at 18,156 (t=4.528)
engine: sprite unblocked at 18,146 (t=4.568)
```

— is not a wall. The timestamps are the evidence: the frame at t=3.867
arrived 0.315 s after its predecessor, and the frame at t=4.528 came
**0.66 s** later. The mover steps `240 px/s × dt`, so that frame
attempted a step of `240 × 0.66 ≈ 158 pixels` — from y=156 clean past
the top of the map. `TileRectSolid` saw a rectangle outside the world,
the edge is solid, and the step was refused *whole* — including the 137
perfectly good pixels of it. The next frame (0.04 s, a normal step) ran
fine, which is why the position jumps 156 → 146 on the "unblocked"
line. The prediction to make before running anything: **the blocked
report at 156 is a rejected giant step, not a wall** — and the double
precision in the report is what separates "did not move" (156.00 →
156.00) from "moved less than a pixel" (which the integer report of the
lesson's end state would have called blocked too).

**The stop distance.** Driving left into the border wall, the sprite
cannot pass x = 16 — its 16-wide box would then overlap the wall's
pixels 0..15. But the mover moves in steps, not by millimeters: the
last *accepted* step lands wherever the walk's stride puts it, and the
first *rejected* one leaves it there. So the prediction is not "exactly
16.00" but **within one step of 16** — and the step size here is 240 ×
dt, which at the scripted input's frame spacing is 5–10 pixels. The
patch's fractional report shows what actually happened:

```
engine: sprite blocked at 17.77,232.00 (t=4.796)
engine: sprite unblocked at 17.77,232.00 (t=5.548)
```

**17.77** — 1.77 pixels short of the perfect 16.00. The number is not a
constant: it depends on where the steps' strides happen to land against
the wall (the earlier run stopped at 18.17 with a different input
timing). The wall is exact; the arrival is a stride's accident.

What *should* the mover do about steps too large to take? Three
answers, in the order games usually meet them:

1. **Clamp dt.** Cap the step at, say, 1/30 s worth of movement. Simple,
   and it bounds how wrong one frame can be — at the cost of the sprite
   moving in slow motion during a long stall (the world does not get its
   lost time back).
2. **Substep.** Split the intended move into tile-sized (or smaller)
   steps and apply them until the query refuses. The sprite slides up to
   the wall exactly — the giant step's good pixels are no longer thrown
   away. This is what a real mover wants, and it costs one small loop.
3. **Sweep.** Ask "how far can I go along this direction?" instead of
   "can I be here?" — the query becomes a ray or a swept rectangle. The
   right answer for fast movers (bullet, dash) that cannot be allowed to
   tunnel through walls even at substep granularity.

Part 4's hero will take option 2 with option 1 behind it — and the
frame-accounting habit from lesson 036 will show the mover's cost inside
the `update` phase when it does. The lesson's end state leaves the step
all-or-nothing *on purpose*: the behavior is honest, the report is
honest, and the limitation is now a measured, named thing instead of a
surprise.
