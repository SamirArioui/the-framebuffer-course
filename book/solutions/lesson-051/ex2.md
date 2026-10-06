# Solution: exercise 2 — The two kinds of nothing

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 051 — text on screen](../../lessons/part-2/lesson-051-text.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The prediction, from `FontGlyph`'s two halves — the index math and the
bounds check. For `' '`: byte 32, index 0 — the sheet's first cell. It
is *in* the sheet; the function returns a real sprite whose 64 pixels all
happen to be key-colored. For byte `200`: index 168, past `FONT_COUNT =
96` — the function returns `0`, and there is no sprite at all. The run:

```
engine: space: has a glyph; byte 200: no glyph
engine: "A B": 2 slots with ink, 1 without
```

Both answers as predicted, and `"A B"` reports the slot map the lesson's
rule produces: `A` draws, the space's slot is empty, `B` draws **at
`x + 2 × 8`** — its layout position, computed from its index, untouched
by the empty slot in the middle.

Now why the distinction matters even though the pixels agree. The two
nothings are different *facts about the font*:

- **A space is content that happens to be invisible.** The sheet says
  "character 32 exists and looks like nothing". If you recolor or redraw
  the sheet's first cell, the space becomes visible — it is art like any
  other cell.
- **A missing byte is a gap in the sheet.** The font simply has no
  opinion about that character. The layout reserves the slot (so the
  string's shape is stable) and the draw skips (so nothing false is
  shown) — and that is *all* the engine can honestly do.

The last prompt question — what if the font lacks a glyph you need — has
exactly one answer under this format: **the sheet is the font**. There is
no fallback, no synthesized glyph, no "draw a box" (a box would be a
glyph the font *does* have, like the sheet's DEL cell at 127, and the
engine would draw it only if the *character* were 127). To support a new
character you redraw the sheet — extend the grid, move the format's
constant, and every loaded sheet must follow. That is the cost of a
format small enough to define by hand, and the lesson-050 prose said as
much: later lessons extend by *reading more*, never by reinterpreting.

When lesson 058's frame-budget table prints its own numbers, it will
build its strings from digits and spaces — both in the sheet, both
unambiguous. If you ever find yourself typing a `µ` into a HUD string,
you now know exactly which two slots will be empty, and why.
