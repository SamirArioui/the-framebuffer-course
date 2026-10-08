# Solution: exercise 2 — the cost per tile

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 099 — pass 2a: fix the map's draw](../../lessons/part-5/lesson-099-map-draw.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The probe sits at the end of `DrawTileMap`, after the walk: the window
of cells it computed, multiplied out, printed whenever the count
changes — so a run shows every distinct shape the walk takes as the
camera clamps along the map. From a real walk of this map:

```
engine: probe: map draw 1230 of 1536 cells at offset -8,0
engine: probe: map draw 1200 of 1536 cells at offset -128,0
engine: probe: map draw 1230 of 1536 cells at offset -123,0
engine: probe: map draw 1200 of 1536 cells at offset -133,0
```

The count moves between 1,200 and 1,230 as the camera's pixel offset
shifts the window a half cell — never near the walk's old 1,536. (The
offset passes 128 because the shake's additive offset joins the
camera's clamped base — lesson 092's hook, visible here too.)

Now the pair, and the per-tile arithmetic. The frame log's `tilemap`
row over the same run's 641 play frames is `0.549 ms`; the probe says
the walk was about 1,215 cells of a draw. So:

```
before (lesson-098):  0.981 ms / 1536 cells = 639 ns per tile
after  (this run):    0.549 ms / ~1215 cells = 450 ns per tile
```

The decomposition the exercise asks for — which lever owns which
number:

- **The count: 1,536 → ~1,215 cells (−21%)** is the *culling's* —
  `DrawTileMap` walking only the visible window. It is a flat cut,
  whatever the loop costs.
- **The per-tile cost: 639 → 450 ns (−30%)** is the *loop's* — the
  hoisted row pointers and the straight expand. The tiles here carry
  zero key pixels, so every tile takes the test-free path.

Together they are the measured 43% fall of the `tilemap` row
(`0.981 → 0.559 ms` on the lesson's run), and the two numbers verify
independently: a change in the count without a change in the per-tile
cost would have meant the loop rewrite did nothing; the reverse would
have meant the culling did. Measure the pair and neither lever can
hide behind the other.

One footnote for your own probes: this one prints on *change*, not per
frame — a per-frame print would land inside the `tilemap` phase it is
trying to measure (lesson 079's rule: the measurement must not inflate
the thing measured). Even so, the probe's own print is real work in
the render phase on the frames it fires; when you chase single-digit
microseconds, subtract your probe or park it outside the timed phase.
