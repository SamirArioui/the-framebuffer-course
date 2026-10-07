# Solution: exercise 1 — The row, at two sizes

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 081 — the slice's cost in the frame budget](../../lessons/part-4/lesson-081-entities-row.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is one line of context: the budget table prints how many
entities the walk visits per frame beside the table itself, so a row's
number can be read against its size. The measurement is yours to take
— rows added to `assets/entities.txt` (data, no rebuild of the engine's
logic) and the run's own table read at each size.

The measurements, from real runs of this lesson's end state plus the
patch on the authoring machine — the slice at three table sizes,
`ENTITY_CAP` raised to 256 for the largest (a scratch change, the
lesson's own 64 is restored after):

| entities walked per frame | `entities` row |
| ------------------------- | -------------- |
| 2 | 0.001 ms |
| 20 | 0.002 ms |
| 200 | 0.007 ms (two runs, the same to the digit) |

**Is the walk's cost linear?** Yes — with a floor. The shape is
`fixed + count × per-entity`: the first ~0.001 ms is the measurement's
own cost (the two `platform::Now()` calls and the loop's setup, paid
whether the store holds two entities or none), and the marginal cost is
about 30-50 nanoseconds an entity (from 20 to 200 entities the row
grows 0.005 ms over 180 entities ≈ 28 ns each). Ten times the entities
is a few times the row, not ten times the row, because at these sizes
the floor is most of the number.

**How many entities inside 0.1 ms?** At ~30 ns an entity, some three
thousand — and the number is a *machine* number: your CPU, your build
flags, your row. (This machine: `-O0 -g`, one Xvfb display, no sound.)
The exercise's real answer is the measurement you took, with your
machine named beside it — a budget table without its machine is a
rumor, lesson 069's phrasing.

**The reconciliation**, of the 200-entity run against its own log — the
lesson's discipline, done once here: the run's `frame N:` lines carry
`entities` in every record (`update 0.018 ms (entities 0.002), …`), and
averaging that column over the run's frames gives the table's
`entities 0.007 ms`. The row is the log; the log is the frames.

Two things the row is *not* saying in this exercise. It is not saying
the game is slow — 0.007 ms of a ~1.9 ms frame is 0.4%, and the
tilemap's walk still owns the render. And it is not a prediction for
Part 5: two hundred entities that each do one multiplication per frame
are not two hundred entities that each run an AI, and the row will
measure *those* just as honestly when they arrive. The instrument's
value is that it keeps answering.

Nothing here touches the walk, the record, or the game's loop: the
patch is one line beside the table the lesson already prints.
