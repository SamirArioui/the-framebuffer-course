# Solution: exercise 2 — The sprite that wraps

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 046 — the sprite moves](../../lessons/part-2/lesson-046-movable-sprite.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch replaces the clamp with a wrap: when the sprite has traveled a
full sprite-width past an edge, it reappears just outside the opposite
one. Nothing is drawn while it is outside — the blit's clip handles that
for free, and the crossing frames draw whatever sliver is still on screen.

Scripted input held Right across the edge; the report shows the wrap
happening exactly at the boundary:

```
engine: sprite at 519,232 (t=2.673)
engine: sprite at 577,232 (t=2.915)
engine: sprite at 635,232 (t=3.156)
engine: sprite at 32,232 (t=3.398)
engine: sprite at 89,232 (t=3.638)
engine: sprite at 147,232 (t=3.879)
```

From `635` the sprite kept moving right: the visible sliver shrank until
the sprite was fully outside (`sprite_x` passed `640` and then the
`sprite_x < -sprite.width` rule parked it at `FRAME_WIDTH`), and it
re-entered from the left — `32`, then `89`, then `147` — with the same
velocity. The readback at a reported position on the far side confirms
the pixels made the trip: at `176,232`, the sprite's center pixel reads
`(220, 40, 40)` in the window — the same art, the other side of the
world.

Note the two or three frames where the sprite is *nowhere on screen*.
Wrapping at `±sprite.width` means the sprite must travel its own width
past the edge before reappearing; if you wrapped exactly at the edge
instead, the sprite would pop from "one pixel showing" to "one pixel
showing" on the other side. Both are policies; the prompt asked what the
clamp bought:

- **The clamp guarantees presence.** The object is always fully on
  screen — every pixel of it drawn, every position readable. That is why
  the lesson's readback checks could sample at `(x + 8, y + 8)` without
  thinking: with clamping, that point is always a sprite pixel.
- **The wrap trades that guarantee for continuity of motion** — a world
  with edges that are not walls. The cost is exactly the property the
  clamp bought: the sprite can be partially drawn (two objects' worth of
  bookkeeping if anything ever keys off "where is it"), and it can be
  entirely off-screen at the moment a screenshot or a check would look
  for it.

Neither is right in the abstract. The clamp is the right policy for an
object the player must always see (this course's hero); the wrap is the
right policy for a world that has no edges (a starfield, a scrolling
marquee). The lesson-046 code keeps the clamp for exactly the first
reason — and now you can change the policy in one four-line block,
because the blit's clipping makes both policies safe.
