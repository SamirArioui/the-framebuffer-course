# Solution: exercise 2 — The crashing rectangle

{{#include ../../stability-horizon.md}}

*Sample solution for the [sample lesson](../../lessons/sample-lesson.md). A
solution ships as a diff against the lesson's end state plus a short
walkthrough — never a full listing.*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

`FillRectangleFast` writes `fb->pixels[y * fb->width + x]` with `x` and `y`
taken straight from the rectangle. The moment the rectangle leaves the
framebuffer — hanging off the right edge, or with a negative corner — the
computed index leaves the pixel buffer with it: past the end of a row (and past
the end of the buffer at the bottom), or before its start at the top. That is
an out-of-bounds write, and the crash lands wherever the write lands, far from
the loop that caused it.

The fix copies the shape of the clipping branch of `FillRectangle`: compute the
four bounds once, fold them into the framebuffer, and let the loops run over the
clipped range. The inner loop is untouched, so the function keeps the speed the
variant was after — the clipping work happens once per rectangle instead of
once per pixel.
