# Solution: exercise 1 — The corner, predicted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 077 — the mover on an entity](../../lessons/part-4/lesson-077-mover.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is a scratch corner: the hero's entity copied, parked at
`736,464` — the map's bottom-right interior — and three forced steps of
20 pixels per axis through `MoveEntity` itself. The corner is real map
data: cell 47 (to the right) is `#` and row 31 (below) is `#`, while up
and left are the open rows the hero has been walking all along.

The predictions, before any run. **Down-Right** (+20, +20): the x step
would put the rectangle on cell 47 — refused; the y step would put it on
row 31 — refused. Nothing moves: `736,464`. **Down-Left** (−20, +20):
the x step lands (716 is open), the y step is still refused at the
bottom row: `716,464`. **Up-Right** (+20, −20): the x step is refused at
cell 47, the y step lands: `736,444`.

The runs, from this lesson's end state plus the patch:

```
engine: corner: Down-Right -> 736,464
engine: corner: Down-Left  -> 716,464
engine: corner: Up-Right   -> 736,444
```

Exactly as predicted — and note what the three answers look like
together: the corner refuses *components*, not requests. A diagonal
into a corner does not bounce or stop dead; each axis takes what the
world offers.

Now the ordering question. The mover tries **x first, then y** — and the
y test runs at the *updated* x when the x step landed. That matters at a
corner: in the Down-Left case the y test ran at x 716, not at 736. Had
the x step slid the entity past the corner's edge — a wall ending
mid-move — the y step could land at the new x where it would have been
refused at the old one. That is the "sliding around a corner" case: the
order decides whether the hero slips around the corner's lip or sticks
to it. Y-first would answer the same situations differently.

Neither order is *right* in the abstract; the choice is part of the
mover's contract and this engine's is x-first, the same as lesson 056's
inline mover was. What matters is that it is one function with one
order — so every entity in the game slides around corners the same way,
and the day the feel needs the other order it changes in one place.

Nothing here touches the mover, the walk, or the game's loop: the probe
is one copied entity and three calls beside the run's own.
