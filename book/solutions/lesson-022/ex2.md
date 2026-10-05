# Solution: exercise 2 — One off, twice

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 022 — the double-buffered character grid](../../lessons/part-0/lesson-022-double-buffer.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

C counts from zero; ANSI cursor addressing does not. `ESC [ r ; c H` uses
1-based coordinates — row 1 is the top line, row 1;col 1 is the top-left
corner — so the flush was painting the whole grid one cell down and to the
right of its coordinates, with `ESC [ 0 ; 0 H` as the giveaway: zero is not a
legal position at all, and most terminals clamp it to 1;1. The last row of
the grid escaped notice only because the terminal clamps the overflow too.

The fix is the classic pair of `+ 1`s at the boundary where C's zero-based
world is translated into the terminal's one-based one — `row + 1, col + 1` —
and nothing inside the program's own coordinates changes. After the fix the
first cell writes `ESC [ 1 ; 1 H`, the bottom-right writes `ESC [ 20 ; 40 H`
for a 20×40 grid, and the marker at internal (10, 20) writes `ESC [ 11 ; 21
H`. The lesson generalizes: every interface with its own coordinate or
indexing convention needs one translation point, kept obvious — off-by-one
bugs hide at exactly this kind of seam.
