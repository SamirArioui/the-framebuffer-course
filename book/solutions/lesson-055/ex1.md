# Solution: exercise 1 — The policy is yours

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 055 — tile kinds and solidity](../../lessons/part-2/lesson-055-collision.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch writes the same two queries again with the opposite
out-of-bounds answer — `TilePointFree` / `TileRectFree`, differing in
exactly one respect: coordinates outside the map answer *free*, so only
cells the map actually has can report a collision. The four
out-of-bounds cases under both policies:

```
engine: policy: point left of the map — outside-solid solid, outside-free free
engine: policy: point past the right edge — outside-solid solid, outside-free free
engine: policy: rect leaving the map — outside-solid solid, outside-free solid
engine: policy: rect entirely past the edge — outside-solid solid, outside-free free
```

The prediction, then the reconciliation. A query that lies *entirely*
outside the map is decided by the policy alone — so the two points and
the far-away rectangle flip. But **`rect leaving the map` does not
flip**: `(−8, 100, 16, 16)` reaches from x = −8 to x = 7, and its
in-map part — pixels x = 0..7 — is the map's border column, which is
wall. The in-map cells answer *solid* under **every** policy; the policy
only decides the part of the query that leaves the map.

That is the deeper point about out-of-bounds handling: it is not a
global behavior but the answer to *one part* of a query. A rectangle
straddling the edge is two questions at once — "does the inside part hit
anything?" (the map's data answers) and "is the outside part allowed?"
(the policy answers) — and the reported collision is `inside-something
OR outside-not-allowed`. The `TileRectFree` patch makes this literal: it
clamps the rectangle to the map first (answering the second question by
throwing the outside away) and then asks the cells (the first question).

Which policy does your game want?

- **Outside is solid** — the world is an island; nothing leaves. Right
  for a bounded arena, a dungeon, any game where the map *is* the
  world. This course's demo uses it.
- **Outside is free** — the map is a place, not a boundary. Right for
  scrolling worlds with void around them, for editors, for games where
  leaving the map is allowed and handled elsewhere (a "lost" state, a
  wrap, an invisible wall drawn as art).

Neither is more correct; a game that picks neither — an unchosen
default, whatever `TileAt`'s bounds check happens to return — has
already picked one by accident. That is the exercise's real answer:
**the policy is yours, so name it.**
