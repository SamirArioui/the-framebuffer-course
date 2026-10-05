# Solution: exercise 4 — Why fold first

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 015 — fill-rect onto a memory buffer](../../lessons/part-0/lesson-015-fill-rect.md).*

## The diff

```diff
{{#include ex4.patch}}
```

## Walkthrough

The instrumented folds print

```
fold left:   x=0 rw=3
fold top:    y=0 rh=2
fold right:  rw=2
fold right:  rw=3
fold bottom: rh=2
```

— four cheap comparisons per rectangle (only the ones that fire print),
against a bounds check for every one of the `rw * rh` pixels in the
per-pixel scheme. At 10×10 that is 4 tests versus 100; at sprite scale the
ratio only grows. For (b), run the arithmetic before you run the code:
with the `x < 0` fold deleted, the left-hanging rectangle's first pixel at
`(x, y) = (-3, 1)` computes `off = 1 * 24 + (-3) * 3 = 15` — inside the
buffer, in the tail of row 0. The verified dump shows exactly that: row 0
grows `FF 80 00` at bytes 15–17, and every row bleeds into its neighbor —
silent corruption, no crash. Move the rectangle to `y = 0` and the offsets
go negative; AddressSanitizer then reports `heap-buffer-overflow ... WRITE
of size 1` before the buffer. For (c): a per-pixel check earns its keep
when many shape writers share one raw `PutPixel` and each shape's clip
math is complex (rotated sprites, polygons) — checking at the bottom is
then the cheap insurance that no shape can corrupt memory.
