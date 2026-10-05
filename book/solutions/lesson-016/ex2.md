# Solution: exercise 2 — Rectangle outlines

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 016 — drawing lines onto the buffer](../../lessons/part-0/lesson-016-lines.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

`DrawRect` is four `DrawLine` calls — top, bottom, left, right — and the
outline `(-2, 1, 6, 4)` exercises every clipping case at once: the top and
bottom edges hang off the left, the right edge is fully inside, and the
left edge (`x = -2`, from `(−2, 1)` to `(−2, 4)`) is entirely outside the
buffer. The dump shows exactly that:

```
row 1: 60 60 60 60 60 60 60 60 60 60 60 60 FF 00 FF ...
row 2: FF 80 00 FF 80 00 FF FF FF 60 60 60 FF 00 FF ...
```

— the top edge clipped to `(0, 1)`–`(3, 1)`, the bottom to `(0, 4)`–`(3,
4)`, the right edge at `x = 3` rows 1–4 (here overwriting the yellow
diagonal's `(3, 2)`, last write winning again), and the left edge wrote
nothing: both its endpoints carry the same out-code, so `ClipLine` rejects
the segment outright. That rejection is the clip doing its job — an
invisible edge costs four `OutCode` calls, zero pixels, and zero
per-pixel checks. Composing shapes from clipped primitives means no
composite shape ever needs clip math of its own.
