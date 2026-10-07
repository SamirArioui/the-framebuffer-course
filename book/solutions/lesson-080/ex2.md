# Solution: exercise 2 — The gate, answered

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 080 — the vertical slice](../../lessons/part-4/lesson-080-slice.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is the gate's left column: the run names the services it
composes, one line per group — the same move lesson 069's demo made
when Part 3 wanted its acceptance table. The lines are not decoration;
they are a claim per line, and each one names the group of finished
services the slice uses.

The run, from this lesson's end state plus the patch:

```
engine: data: the archetype table loaded whole — definitions carrying their rows' values
engine: entities: one fixed store, created from the table, walked once per frame
engine: mover: the map's collision queries, one axis at a time
engine: view: the camera's clamped follow, the blit at each entity's position
engine: time: the game-time scale on the update's step, the record wall-clock at any scale
engine: sound: samples on channels through the one mixer
```

Now the first half of the answer: **name a line of the slice's frame
that is not a finished service.** Walk the frame with those six groups
open. The startup loads the table and the art (071-073) and asks for its
hero by name (`TableFind`, 073). The update reads polled input (032),
writes the hero's request (076), walks the store (075), and steps each
entity through `MoveEntity` (077) at `request × speed × dt` where `dt`
is `GameTimeStep` (078). The camera's clamp is lesson 054's arithmetic.
The render blits every entity through the camera (045, 076). The record
measures the phases and the step (036, 079). There is no line that is
*new*, and that is evidence rather than politeness because of how it is
checkable: every line maps to a lesson number, and a lesson number maps
to a tagged state — you can `git checkout lesson-077 -- src/` and see
the mover arrive. A claim of the form "this was already finished" is
falsifiable; a claim of the form "this is clean code" is not.

The second half: **if the slice had needed one new piece of engine
code**, that piece would be the service speaking up. The failure modes
are recognizable. A `CameraFollow(camera, hero)` helper means the
camera's rule was never actually settled (it is game arithmetic today,
and that is the right place — but if the game needed it twice, it is a
service). A special case in the walk for the hero's input means the
entity model is wrong: per-entity work should be *expressed once*, and
a branch on "is this the hero" is the per-type code the iteration
requirement exists to prevent (Part 5 would grow one branch per enemy
type). A new field on `Entity` the table does not declare means the
data model is drifting from the format — the fix is a column, not a
field.

What you would do about it before Part 5 starts: **put it in the
service, not the game** — extend the entity's contract, the table's
format, or the mover's result (lesson 077's exercise did exactly this),
and re-verify the slice before building on it. The gate exists to catch
that gap *now*, while the cost of fixing it is one lesson, rather than
in L12 when twelve lessons stand on it. The closing review
(`plan/part4-review.md`) records this change's own answer: the slice
invented nothing, and the only thing it *removed* was the demo's
scaffolding — the kill check and the lifetime scripts lessons 074-075
needed and the game does not.

One more thing the six lines make visible: the groups are not the
files. `entities` spans `table.*`, `entity.*`, and the walk in the
game; `time` spans `gametime.*` and the record in `frame.*`. A service
is what the *game* can rely on, and the acceptance list is written in
those terms on purpose — it is the list Part 5's lessons will quote.

Nothing here touches the slice, the services, or the game's loop: the
patch is six lines beside the identity line the lesson already prints.
