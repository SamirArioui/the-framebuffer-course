# Solution: exercise 1 — Draw an outline

{{#include ../../stability-horizon.md}}

*Sample solution for the [sample lesson](../../lessons/sample-lesson.md). A
solution ships as a diff against the lesson's end state plus a short
walkthrough — never a full listing.*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

An outline is four filled rectangles, one per side, so the natural fix reuses
`FillRectangle` instead of writing pixels again. That reuse is the point: every
side then goes through the clipping branch of `FillRectangle`, which is exactly
the behavior the prompt asked to keep. The top and bottom sides span the full
width at `thickness` tall; the left and right sides span the full height,
including the corners, so no gap appears where sides meet. When `thickness`
grows past the rectangle's own size the sides overlap and clipping folds them
back into the framebuffer — nothing writes out of bounds.

If you wrote the four sides as raw pixel loops instead, check that your loops
carry the same clipping: that is the only requirement the prompt makes.
