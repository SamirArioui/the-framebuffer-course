# Solution: exercise 1 — The corner and the wall

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 084 — tile collision](../../lessons/part-5/lesson-084-collision.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The probe records where each entity was, moves it through `MoveEntity`,
and then reports **which axis actually moved** — `x moved/stopped, y
moved/stopped`. That one line is the whole answer made visible: the
one-axis rule shows up as one axis moving and the other stopping.

**The two predictions.** `MoveEntity` takes the x step first (checked
against the entity's *original* y), then the y step (checked against the
*updated* x). So:

- **(a) Diagonal into a flat wall that blocks only x** (say, up-and-left
  into a wall on the left): x is refused (its destination is solid), but
  y is still free — so **it slides** along the wall, on the **y** axis.
  The probe reads `x stopped, y moved`.
- **(b) Diagonal into a corner where both axes are solid**: x is refused
  and y is refused — so **it stops entirely**. The probe reads `x
  stopped, y stopped`. Neither axis wins; there is nowhere to go.

The order matters in the in-between case: because x is resolved against
the *original* y and y against the *updated* x, x gets first refusal. If
both steps would be individually legal but not together, x still moves
first and y is then judged against the new x. So in a diagonal squeeze,
**x is the axis that wins** — it is resolved first.

The run behind the lesson already shows both shapes in the hero's own
position trace — the slide first:

```
engine: hero at 312,232
engine: hero at 210,232 (t=3.447)     <- x moved 312->210, y frozen: slid along a wall
engine: hero blocked at 206,228 (t=6.477)   <- both axes refused: stopped in the corner
```

`312 → 210` at a constant `y = 232` is the slide: one axis refused, the
other free. `blocked at 206,228` is the corner: both refused. With the
probe applied, the same run spells it out per step — a stretch of `x
stopped, y moved` (or `x moved, y stopped`, depending on the wall) while
the hero glides along the wall, then `x stopped, y stopped` the moment
it is wedged in the corner.

The lesson to carry forward: there is no separate "slide" behavior and
no separate "stop" behavior. There is one rule — resolve each axis on
its own — and a wall versus a corner is just how many of the two axes it
refuses. That is also why the order is not arbitrary: swapping the two
steps would not change the slide, but it would change which axis wins
when only one of the two can be honored.
