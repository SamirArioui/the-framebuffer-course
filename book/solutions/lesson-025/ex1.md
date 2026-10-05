# Solution: exercise 1 — A third view

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 025 — the C++ subset: classes and vtables](../../lessons/part-0/lesson-025-cpp-subset.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

A new drawable is one class and one row. `TallyView` declares the same single
function the other views do, and its `Draw` formats the counters with
`snprintf` and paints them onto the bottom border row through the same
`Grid::Put` overload `GridView` uses. Registering it cost one object and one
`&tally` in `views` — `Render` never changed, because the loop never knew what
it was drawing. That is the whole point of the interface.

The traced run ends `done after 30 frames, 9 ticks`, and the bottom row of the
screen comes out as:

```
19|+-f=30 t=9-----------------------------+
```

Draw order is table order, and here it bit: the tally sits *after* `&playfield`
in `views`, so it paints over the border dashes `GridView::Draw` just placed —
the same z-order lesson 024's `Render` maintained by hand. One array row up or
down and the counters would be dashes again.
