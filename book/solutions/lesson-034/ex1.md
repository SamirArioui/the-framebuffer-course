# Solution: exercise 1 — Eight directions

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 034 — the first interactive frame](../../lessons/part-1/lesson-034-first-frame.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diagonal is not a fifth movement — it is the two axes read at the same
time. The fix is to stop treating the four keys as four exclusive moves:
read each one into a direction component (`dx`, `dy`), then move once by the
result. Two held keys give `dx=1, dy=1` and the marker steps on the
diagonal; opposite keys cancel (`dx=0`); one key behaves exactly as before.

Scripted, with a Right+Down chord repeated five times:

```
$ DISPLAY=:99 xdotool key --delay 50 --repeat 5 --window <id> Right+Down
engine: marker at 308,228
engine: marker at 316,228
engine: marker at 324,236
engine: marker at 324,244
engine: marker at 332,244
engine: marker at 340,252
engine: marker at 340,260
```

Watch the states each frame found: `(316,228)` moved on x alone,
`(324,236)` on both axes at once — a diagonal step — and `(324,244)` on y
alone. The chord arrives as interleaved presses and releases, so not every
frame finds both keys down; every frame reads the state it finds and moves
accordingly. Hold two arrows on a real keyboard and the auto-repeat keeps
both keys down between frames — a steady diagonal.

The component form also answers the question the four-if form cannot: what
happens when both left and right are down? `dx` sums to zero — the marker
stays put. Deterministic input resolution falls out of the arithmetic
instead of needing its own rules.
