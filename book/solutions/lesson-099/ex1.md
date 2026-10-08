# Solution: exercise 1 — the sprites' draw gets the same medicine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 099 — pass 2a: fix the map's draw](../../lessons/part-5/lesson-099-map-draw.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

`BlitSpriteFrame` is `BlitSprite` with the sheet's frame column folded
into the source address, so the fix is the same fix: hoisted row
pointers (`src += 3`, `dst += 4` down the row, both bases computed
once) and the straight expand for a sheet with `key_count` zero — a
sheet with no transparent pixel anywhere certainly has none in the
frame being drawn from it. The key path keeps its per-pixel decision,
for the same reason the twin's does: for the hero and the shots the
transparency is real.

Then the measurement the exercise demands — the `sprites` row on this
machine's busy frames (the same 1,551-frame scenario, split into play
frames; the 50 frames where the most sprites drew):

```
                               twin untouched   twin fixed
  play frames, sprites avg       0.0063 ms      0.0047 ms
  busiest 50 frames              0.0202         0.0155
  worst single frame             0.164          0.150
```

The honest answer to "what it was ever worth": **about 25% of a row
that is 0.3% of the frame.** The medicine works — the busy frames fall
a quarter, the same shape of fall as the map's draw — and it is worth
`0.0016 ms` on the average play frame. If the game drew fifty times
more sprites (a bullet-hell on this engine), this would be the first
lever to pull. At the game's actual scale it is a rounding error.

Which is the real lesson of the exercise. The menu fixed the two
*measured* hotspots and left this loop alone not out of neglect but
because measurement said its row is noise — and when you spend an
optimization where measurement is silent, you earn complexity and
spend proof. Here the complexity was small and the proof is now
yours: the row moved, the frame did not.
