# Solution: exercise 2 — The worst frame's column

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 058 — the frame-budget table](../../lessons/part-2/lesson-058-budget.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The prediction first, from the log. The header names the worst frame
(`worst 4.162 ms (frame 17)` in the lesson's run, `worst 2.768 ms
(frame 9)` in this one); find that frame's line and read its phases. In
this run, frame 9:

```
frame 9: update 0.028 ms, render 1.317 ms (sprites 0.001, text 0.007, tilemap 0.887), present 1.422 ms, total 2.768 ms
```

The render is unremarkable — 1.317 ms against a 1.5 ms average, actually
*below* it. The worst frame is a **presentation event**: `present 1.422
ms`, more than double the 0.699 ms average. The prediction to write
before checking: the worst frame is not the one that drew the most; it
is the one whose copy to the window was slowest.

The patch keeps the worst frame's record (`if (frame.total >
worst_frame.total) worst_frame = frame;`) and prints its attribution
under the table:

```
engine: frame budget — 26 frames, avg 2.230 ms, worst 2.768 ms (frame 9)
engine:   worst frame 9: update 0.028, render 1.317 (sprites 0.001, text 0.007, tilemap 0.887), present 1.422
```

Exactly the prediction. Now *why* the worst frame's shape differs from
the average's — three reasons, all visible in this engine's history:

- **Different work, not just more of it.** The average frame is
  render-dominated (the tilemap walk every frame); the worst frame is
  present-dominated. Averages blur phases together; the worst frame is a
  single moment where one phase misbehaved. This is why the budget keeps
  the phases apart.
- **Firsts are expensive.** Worst frames cluster early (Part 1's lesson
  036 found frame 1 worst: first touches of the framebuffer, cold
  caches, the window's first real present) — and at any point where the
  system meets something new: a resized window, a freshly scrolled
  region, the OS scheduling the process somewhere else.
- **The present is not ours.** Lesson 031's contract makes the present
  synchronous — the copy is done when it returns — so its worst case
  includes whatever the window system was doing. The engine can make its
  own phases predictable; the present's tail belongs to the machine.

The habit the exercise is after: **never read a budget's average
without its worst row.** A game that runs at 2.2 ms average with a 2.8
ms worst frame is a different product from one with a 40 ms worst frame
and the same average — the player feels the tail, not the mean. Part 5's
frame-budget report will keep both columns, and the three-pass menu's
"fix top-2 hotspots" step reads the averages first and the worst frames
immediately after.
