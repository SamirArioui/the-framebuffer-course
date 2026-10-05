# Solution: exercise 1 — Counting the flush

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 022 — the double-buffered character grid](../../lessons/part-0/lesson-022-double-buffer.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction: the first flush rewrites the world — 20 rows × 40 columns =
800 cells. Every flush after that writes only what *changed*, and with the
marker as the only moving thing the answer is either 2 (its old cell emptied,
its new cell filled) or 0 (a frame that fell between ticks and changed
nothing at all). The instrumented run of `./snek 10` confirms it exactly:
`flush cells=800`, then a pattern of `0, 0, 2` as ticks land every third
frame.

The counts are the double-buffer invariant made visible: `front` is a claim
about the terminal, `back` is the scene, and the flush pays only for the
difference. Note the `front_valid` escape hatch — before the first draw there
*is* no claim, so every cell counts as changed (and the screen is cleared
once). The counter is a throwaway instrument; what stays is the habit of
asking "how much work did that actually do?" — which is also the question
behind the flicker and bandwidth arguments for double buffering in the first
place.
