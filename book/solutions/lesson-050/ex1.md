# Solution: exercise 1 — The font dump

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 050 — the bitmap font as an asset](../../lessons/part-2/lesson-050-font.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch draws all 96 glyphs back where the cut found them —
`BlitSprite` at `32 + (k % 16) * 8, 32 + (k / 16) * 8`, the sheet's own
arithmetic run backwards — then sweeps the whole dump with `GetPixel`
and compares every pixel against the glyph sprite's bytes, key pixels
checked for *not* having written. The run:

```
engine: font dump: 96 glyphs drawn, 0 pixel mismatches
```

**Zero mismatches over 6,144 pixels** (96 × 64): the cut is lossless.
The sheet's pixels and the glyph sprites' pixels agree everywhere, which
is the claim the loader makes and the only one that matters downstream —
lesson 051's text will trust these sprites completely.

Now the diagnosis question, and it is the reason the dump is worth
drawing at all. A nonzero count is not just "a bug" — *where* the
mismatches land says which line is wrong:

- **Mismatches in every glyph, same pattern each time** — the cut's inner
  arithmetic is wrong *systematically*: the source stride (the sheet's
  width) or the destination stride (the cell width) is off. Every glyph
  is cut the same wrong way, so every glyph fails the same way. The
  first glyph you check (`A`) already shows it; the dump just confirms
  it is not a one-off.
- **Mismatches in exactly one glyph** — the *cell mapping* is wrong for
  one index: `cell_x = k % 16` / `cell_y = k / 16` is per-glyph
  arithmetic, and a mistake there (a `+1`, a swapped modulo) moves one
  cell — the glyph gets a neighbor's pixels. The dump localizes it: the
  broken glyph's *position in the dump* names the `k` that failed.

That two-level reading — systematic vs. localized — is the same habit
the byte-level checks of lesson 044 trained: when a check fails, the
*shape of the failure* is data. One more shape worth naming: mismatches
in a whole *row* of the dump would point at the row arithmetic
(`k / FONT_COLS`), and mismatches only in the last glyph of each row at
the modulo. The dump is a 96-cell scoreboard, and every bug pattern has
a signature on it.
