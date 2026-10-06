# Solution: exercise 2 — The map as characters

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 052 — the tilemap asset format](../../lessons/part-2/lesson-052-tilemap.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch builds each row as a string of the cells' kind characters —
`TileAt` per cell, the kind table mapping index back to character — and
draws it with `DrawText`, one glyph row per map row. The world as ASCII
art, rendered by the engine.

Now the check, and the prompt's question about sharpness. First
considered: *slots with ink*. Run it and the answer is unhelpful — every
map character in this file (`#`, `.`, `w`) is a glyph with ink, so every
slot of every row reports ink: a check that reads `20 of 20` for a row
whose content is `#..................#`. It verifies almost nothing.

The sharper check counts **ink pixels** and compares against the file's
arithmetic, glyph by glyph. In this font the `#` glyph carries 22 ink
pixels and the `.` glyph carries 4 (a 2×2 dot). So the predictions are
arithmetic before any run:

- **Row 0** — `####################`: 20 × 22 = **440**
- **Row 1** — `#..................#`: 2 × 22 + 18 × 4 = **116**

The run agrees exactly:

```
engine: map view: row 0 draws 440 ink pixels
engine: map view: row 1 draws 116 ink pixels
```

Why pixel counts are the sharper check: they compose the *three* facts
that must all be true — the file's characters, the font's glyphs, and the
drawing path's fidelity — into one number per row. A row transposed with
its neighbor, a glyph cut one pixel off, a blit that draws the key color:
all three change the pixel count; almost nothing changes "has ink".
Presence checks are for existence; counts are for correctness.

The habit generalizes past this exercise: when a check can count
something, count it. Lesson 044's byte sums, lesson 045's `130 + 126 =
256`, the map check's `164 + 72 + 4 = 240` — the counts close the
arithmetic, and a count that closes is evidence a presence check can
never give.

And the debug view itself is worth keeping in your engine. A map drawn as
characters is crude next to lesson 053's tile drawing — but it renders
with zero tile art, works at any map size, and answers "is the file what
I think it is?" before a single tile sprite exists. Debug views are not
throwaway when they check data instead of pictures.
