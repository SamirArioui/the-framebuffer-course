# Solution: exercise 2 — The four corners

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 045 — the clipped, transparent blit](../../lessons/part-2/lesson-045-blit.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch folds the clip check into one `CheckClip` function — draw at
`(x, y)`, recompute the landed rectangle the same way `BlitSprite` does,
read every landed pixel back, then sweep the rest of the frame for touched
pixels — and calls it at the four positions.

The predictions, from arithmetic alone. The sprite is 16×16; the landed
rectangle is the sprite's box crossed with the 640×480 frame, so the
count is `landed_width × landed_height`:

| Position | Sprite box | Lands inside | Predicted |
| -------- | ---------- | ------------ | --------- |
| `(-4, -4)` | x −4..11, y −4..11 | x 0..11, y 0..11 | 12 × 12 = **144** |
| `(632, 472)` | x 632..647, y 472..487 | x 632..639, y 472..479 | 8 × 8 = **64** |
| `(-100, 0)` | x −100..−85, y 0..15 | (none) | **0** |
| `(640, 480)` | x 640..655, y 480..495 | (none) | **0** |

The run agrees on all four:

```
engine: clip (-4,-4): 144 pixels landed, 0 wrong, 0 touched outside
engine: clip (632,472): 64 pixels landed, 0 wrong, 0 touched outside
engine: clip (-100,0): 0 pixels landed, 0 wrong, 0 touched outside
engine: clip (640,480): 0 pixels landed, 0 wrong, 0 touched outside
```

Note what the arithmetic is *not*: it is not "whatever overlaps". Position
`(640, 480)` sits exactly at the frame's outside corner — its box shares
no pixel with the frame at all, and a naive "draw what overlaps" loop that
started iterating at the sprite's origin would have run 256 times and
written every one of them out of bounds. The clip rectangle is what turns
"no overlap" into "no iterations".

The `0 touched outside` column is the wrap detector, and it is worth
imagining lit up. A copy loop that dropped pixels *after* computing their
destination — or that let the row index stride past the frame's width —
would write dropped pixels somewhere else in the buffer: the sprite's left
columns appearing at the frame's right edge, or the top rows smeared into
the next row's start. In those bugs, `wrong` can even read `0`: every
landed pixel is correct, and the corruption is entirely in pixels that were
never supposed to be touched. Only the outside-sweep catches that, because
it is the one check that asserts *absence*. That is the habit: when a
claim is "nothing happened", the check has to go looking for the nothing.
