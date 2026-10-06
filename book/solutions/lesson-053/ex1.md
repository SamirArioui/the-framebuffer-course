# Solution: exercise 1 — The tile under the sprite

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 053 — tilemap drawing](../../lessons/part-2/lesson-053-tiles.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The inverse mapping is one division each way: the walk computes world
position from the cell (`x + cx × 16`), the probe computes the cell from
the world position (`(int)sprite_x / TILE_SIZE`). The startup report
checks it on the sprite's starting position — 312,232 — which is cell
`312 / 16 = 19`, `232 / 16 = 14`:

```
engine: tile under the sprite: cell 19,14 is kind '.' (solid 0)
```

Cell 19,14 in the demo's world is floor — non-solid, as the kind table
says. (If the arithmetic needed a proof it is working: put the sprite
over the water pool around cell 33,26 and the report names `w`.)

The outline is the same mapping drawn: the cell's rectangle is
`(cell_x × 16, cell_y × 16)` to `+15`, and one yellow `PutPixel` per
edge marks it. Drive the sprite with scripted input and the outline
follows — verified window-side at the outline's corner after a held
Right arrow, sprite at `596,232` (cell 37,14):

```
$ DISPLAY=:99 ./winread "the framebuffer engine" 592 224
winread: 592,224 -> r=255 g=255 b=0
```

Yellow at exactly `592 = 37 × 16`, `224 = 14 × 16` — the cell corner the
mapping predicted.

What this gives lesson 055 for free is worth spelling out, because it is
the whole collision system in embryo: **collision is this mapping plus
the kind table**. "Is this rectangle inside a wall?" becomes: find the
cells the rectangle covers (`world_x / TILE_SIZE` over its span), look up
each cell's kind (`TileAt`), ask the kind's `solid` flag. The
world→cell arithmetic, the bounds-safe `TileAt`, and the solidity data
are all already written and checked — 055 assembles them into the two
queries game logic runs. Nothing about collision needs to touch pixels,
the blitter, or the screen: it is data about data, which is exactly why
the spec wanted the map testable without a window.
