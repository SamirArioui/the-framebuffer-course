# Solution: exercise 1 — The HUD that counts

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 051 — text on screen](../../lessons/part-2/lesson-051-text.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch builds the line where the data is — `snprintf` into a stack
buffer, `X %d Y %d` with the sprite's position — and draws it at
startup with the same check the lesson's text claims use: slot count
from `TextWidth`, ink presence per slot from `GetPixel`.

The prediction, before the run. The sprite starts centered at `312,232`,
so the line is `"X 312 Y 232"` — eleven characters, of which three are
spaces. Spaces have glyphs (all key-colored), so they draw nothing:
**11 slots, 8 with ink.** The run agrees:

```
engine: hud "X 312 Y 232": 11 slots, 8 with ink
```

Now the question the format asks — the position crossing from `312` to
`99`. The line becomes `"X 99 Y 99"`: nine characters, nine slots. Two
slots *fewer*, and the digits move left: the number of digits is part of
the string, and the layout is per character, so a shorter string is a
narrower line. Nothing about `DrawText` resizes, right-aligns, or pads —
a HUD built from it reflows as its data changes width.

Which raises the ghost question: does the old, longer line leave pixels
behind when the new, shorter one draws? **`DrawText` draws; it never
erases.** In this engine the answer is safe because every frame starts
with `ClearBuffer` — the whole scene is redrawn 60 times a second and the
old HUD is wiped before anything else. But notice the dependency: a
renderer that stops clearing every pixel (a dirty-rectangle renderer, one
of Part 5's named optimization levers) would leave the old digits' ghosts
on screen, and a "text" system that knows about erasing would be needed.
The clear is doing text-erasing work today for free — worth knowing which
of your engine's behaviors are load-bearing for which others.

If you took the exercise further and drew the line every frame in the
render phase (inside the `text` timing), the account shows the cost
moving: the named `text` phase grows from the one label's 0.002 ms to
roughly double it for two lines — and it says so in numbers instead of
standing as a mystery inside `render`.
