# Solution: exercise 1 — The camera at the map's edges

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 080 — the vertical slice](../../lessons/part-4/lesson-080-slice.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is a five-frame probe: the hero's position forced to one edge
position per frame, *before* the update's own camera code runs — so the
clamp under test is the clamp the game uses, not a copy of it. The
report is the camera's own on-change line.

The predictions, before any run. The base is `clamp(hero + 8 − 320, 0,
128)`, `clamp(hero + 8 − 240, 0, 32)` — this map scrolls 128 pixels
across and 32 down. Work each one:

| hero | unclamped | base |
| ---- | --------- | ---- |
| `0,0` | −312, −232 | `0,0` — clamped, both axes |
| `400,232` | 88, 0 | `88,0` — following in x, clamped at the top |
| `734,480` | 422, 248 | `128,32` — clamped, both axes (the map's far corner) |
| `312,232` | 0, 0 | `0,0` — the exact boundary: the view centered on the hero |
| `734,232` | 422, 0 | `128,0` — clamped in x, clamped at the top |

The runs, from this lesson's end state plus the patch (the probe forces
the positions in an order that moves the camera every frame, so every
one reports):

```
engine: camera base 88,0 (t=0.000)
engine: camera base 128,32 (t=2.003)
engine: camera base 0,0 (t=2.018)
engine: camera base 128,0 (t=2.034)
engine: camera base 0,0 (t=2.049)
```

Exactly the five predictions, in the probe's order. Which answers are
the clamp talking and which the view following: `88,0` is the only one
where the view is *following* — 88 is the hero's x pushed to the frame's
center — and even it is clamped in y, because this map is only 32 pixels
taller than the frame. `128,32`, `128,0`, `0,0` are the clamp holding
the view inside the world: the hero at `734,480` is deep in the map's
bottom-right, and the camera shows the map's corner rather than the
void beyond it. And `312,232` is the hinge — the one position where
following and clamping agree exactly at zero.

That hinge is worth a moment, because it is what "reconciled against the
map's bounds" means. The camera is not following the hero blindly and it
is not pinned to the map; it is the *view* of the hero, bounded by the
world. When the hero walks past `312`, the view starts to move; when it
walks past `712` (`128 + 320 − 8`), the view stops and the hero keeps
going. Both transitions are in this map's arithmetic and both are in the
run's reports — which is why the slice's check is this reconciliation
and not a screenshot.

Nothing here touches the camera, the clamp, or the game's loop: the
probe is five forced positions and the update's own report.
