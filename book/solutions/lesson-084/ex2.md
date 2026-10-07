# Solution: exercise 2 — No tunneling

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 084 — tile collision](../../lessons/part-5/lesson-084-collision.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The bug is real and it is about *sampling*. `MoveEntity` checks only the
destination of the whole step: `next_x = entity.x + dx`, then "is the
rectangle at `next_x` solid?". On a short frame `dx` is a couple of
pixels and that is fine. On a long frame — the machine stutters, the
debugger pauses, the window is dragged — `dx = speed × dt` can be tens
of pixels. If a thin wall (one tile, like the map's `#` border) sits in
the gap between `entity.x` and `entity.x + dx`, the destination is clear
floor on the *far* side, the test passes, and the entity is teleported
through the wall. It never "landed on" the solid tile, so the collision
never fired. That is tunneling.

The fix is to stop sampling the endpoints and start sampling the path.
Split the move into sub-steps small enough that the entity cannot cross
a whole thin wall in one, and run the same one-axis rule in each:

```cpp
int steps = (int)(big / MAX_STEP) + 1;   // big = the larger of |dx|,|dy|
for (int i = 0; i < steps; ++i) { /* one-axis rule on sx, sy */ }
```

`MAX_STEP` (4 pixels here) must be smaller than the thinnest wall — a
tile is 16 pixels, so 4 leaves room. The one-axis rule is kept *inside*
each sub-step, so the slide still works exactly as before: within every
sub-step an axis can be refused while the other moves.

To see it, do what the exercise says: raise the hero's `speed` in
`assets/entities.txt` (say, `240` → `2400` — data, no rebuild), rebuild,
and drive the hero straight at the one-tile `#` border. Without the
patch, a long enough frame sends `dx` past the border in one step and the
hero pops out the far side (or off the map). With the patch, the hero is
refused at the border's face every time — the sub-steps walk it up to the
wall and stop, because each 4-pixel hop is tested and the hop that would
land on `#` is refused.

Two things to keep straight. The sub-stepping fixes *overshoot*, not the
one-axis rule — it is a loop around the same resolution, so the slide and
the corner behavior are unchanged. And it costs a little: a step that
used to be one `TileRectSolid` call per axis becomes `steps` of them. On
a normal frame `steps` is 1 and the cost is identical to before; it only
grows on the long frames where it is doing exactly the work needed to not
tunnel. If you want the honest number, the frame record's `entities` row
(lesson 081) will show the walk's cost with the patch in place — measure
it rather than guessing.
