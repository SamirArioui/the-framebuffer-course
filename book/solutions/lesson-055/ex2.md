# Solution: exercise 2 — The rectangle at the boundary

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 055 — tile kinds and solidity](../../lessons/part-2/lesson-055-collision.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The pillar occupies cells (8,6) and (9,7) and their neighbors —
world pixels x = 128..159, y = 96..127. A rectangle is *half-open*:
it covers `x .. x+w−1`, `y .. y+h−1` — the left/top edge included,
the right/bottom edge excluded. That single decision is the arithmetic
behind all six predictions:

| Rectangle | Covers | Cells | Answer |
| --------- | ------ | ----- | ------ |
| (128, 96, 16, 16) | x 128..159, y 96..127 | (8,6) only | **solid** — exactly the pillar |
| (160, 96, 16, 16) | x 160..175 | (10,6) | **free** — beside it, not touching |
| (159, 96, 1, 16) | x 159 only | (9,6) | **solid** — one pixel *into* the pillar |
| (160, 128, 16, 16) | x 160..175, y 128..143 | (10,8) | **free** — diagonal, not touching |
| (159, 127, 1, 1) | the pillar's corner pixel | (9,7) | **solid** |
| (160, 128, 1, 1) | the pixel past the corner | (10,8) | **free** |

The run confirms every one:

```
engine: boundary check: 6 of 6 edges behave as documented
```

Now the prompt's last question — which single character decides the
one-pixel cases. It is the **`− 1`** in the cell conversion:

```c++
    int cx1 = (x + w - 1) / TILE_SIZE;
    int cy1 = (y + h - 1) / TILE_SIZE;
```

`x + w` would convert the rectangle's *exclusive* edge — the first
pixel it does **not** cover — as if it were covered. The one-pixel
rectangle `(159, 96, 1, 16)`: with the `− 1`, its right edge is
`159 + 1 − 1 = 159` → cell 9 → pillar → solid. Without it, the right
edge computes `160` → cell 10 — and the check would ask two cells,
one of which the rectangle never touches. In the free directions the
bug hides (asking an extra cell only makes collisions *more* likely),
and in the tight directions it misreports every boundary. This is the
off-by-one that ships in games as "you can't quite touch that wall" or
"you collide with the wall beside the door" — and it is why the table
above exists.

The habit: when a check can sit exactly on a boundary, put it there.
Interior cases pass under every implementation; only the one-pixel
cases distinguish a correct rectangle from a nearly-correct one — and
"nearly correct" collision is the kind of bug players feel long before
tests find it.
