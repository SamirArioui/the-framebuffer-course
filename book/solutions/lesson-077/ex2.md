# Solution: exercise 2 — Which wall said no

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 077 — the mover on an entity](../../lessons/part-4/lesson-077-mover.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff changes the mover's answer, not its behavior: `MoveEntity`
returns a `MoveResult` — `landed_x`, `landed_y` — filled as each axis is
tried, and the run's state report prints the two components on
transitions instead of a single `blocked` flag. The walk keeps the
hero's result (it knows which slot is the hero's) and the report is the
only reader.

The runs, from this lesson's end state plus the patch. Driving Left
into the map's left wall, then Down into the bottom row:

```
engine: hero: x refused, y landed (t=2.005)
engine: hero: x landed, y landed (t=2.030)
...
engine: hero: x refused, y landed (t=5.303)
```

and driving Down into the bottom row on its own (the earlier run):

```
engine: hero: x landed, y refused (t=2.003)
engine: hero: x landed, y landed (t=2.029)
```

The two refusals read differently — `x refused` at the left wall, `y
refused` at the floor — and that is the whole answer to the exercise's
question. The position comparison the run used before (`hero.x == was_x
&& hero.y == was_y`) knows only *that* the hero did not move: it cannot
tell a wall on the left from a floor below, it cannot tell a refusal
from a frame with no request at all (that is why the old report carried
a `move_x != 0 || move_y != 0` guard), and it cannot see the case where
one axis landed and the other did not — the *slide* — without more
comparisons. Every one of those distinctions is exactly what the mover
knew at the moment it made them.

That is the typed answer's value in one line: **the producer of a
decision reports it, instead of the consumer re-deriving it**. The
mover's result is the fact; the position is evidence. When lesson 081
attributes the update's cost, or Part 5's hero animates only while
`landed_x` is false against a wall, they read the same field — no second
implementation of "did it hit anything" appears anywhere.

Two details worth keeping. The result is a plain struct of two bools —
this engine's typed values are named fields, not exceptions or error
codes, and it composes with `EntityResult`'s shape (a value that says
what happened). And the mover's *behavior* did not change at all: same
one-axis shape, same queries, same slide — the exercise widened the
answer, never the rule.

One honest note on the runs above: the scripted presses wake two frames
each, so the report flickers between `refused` and `landed` as the
presses alternate with their releases. On a held key the transitions
are the ones a player's hand produces: `x landed, y landed` until the
wall, then `y refused` for as long as they push.
