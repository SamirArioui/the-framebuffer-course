# Solution: exercise 2 — What the row does not say

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 081 — the slice's cost in the frame budget](../../lessons/part-4/lesson-081-entities-row.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is the update's own number printed beside the entity work
inside it — the row's boundary, made visible as arithmetic.

The run, from this lesson's end state plus the patch:

```
engine: frame budget — 73 frames, avg 1.750 ms, worst 2.415 ms (frame 16)
engine: update: 0.010 ms a frame of wall clock, 0.001 ms of it entity work — the rest is input, the camera, and the reports
```

The update costs 0.010 ms a frame and 0.001 ms of it is the walk. The
other 0.009 ms is what the exercise asks about.

**What sits in `update` outside `entities`, in this game's frame**: the
polled input read (four `KeyDown` calls and the hero's request written),
the score's arithmetic, the mover's state report, the camera's clamp
and its on-change report, the shake's countdown, and the scale script's
comparison. Every one of those runs *once per frame*, not once per
entity — none of them would cost more if the store held fifty entities
instead of two. That is the boundary's justification in one line: the
row's job is to answer "what does holding entities cost?", and work
that does not grow with the entity count does not belong to the answer.

**The stress test.** The temptations are real, and they all resolve the
same way. Pathfinding is per-entity work — and belongs inside the
walk's body, timed by this row (its cost *is* the cost of holding
pathfinding entities). Animation is per-entity — same answer. Sound
triggers fired per entity — same. What about work that is per-entity
but cheap, like the facing update already in the walk? It is inside the
row already, and that is correct: the row is *the walk's* time, not "the
time of the parts of the walk someone considers interesting". And the
opposite case — a per-*frame* system that reads all entities (a wave
spawner, a collision grid)? That is not the walk; it is a game system,
and if it grows with the entity count the honest fix is to give *it* a
name of its own inside `update` (lesson 046's move, available again)
rather than to stretch this row past its definition.

**What a row of 0.001 ms cannot tell you.** It cannot tell you what
entity work costs when it is *real* work — two entities stepping through
a mover is not fifty entities running AI, and the per-entity number does
not transfer between them. It cannot tell you about *distributions* —
the average row hides the frame where every entity hit a wall at once.
And it cannot tell you about the machine: 0.001 ms here is a
measurement of this CPU at `-O0` under Xvfb, and the same game on a
player's laptop is a different number. What to measure instead, when you
want those answers: the *worst* frame's row (the record already names
the worst frame — read its line), the row at the game's real entity
counts under the game's real load, and all of it on the machines the
game targets.

That is the row's place in the discipline: not a verdict, a *column* —
read beside the frame count, the machine, and the game that produced
it.

Nothing here touches the record, the walk, or the budget's arithmetic:
the patch is one line beside the table the lesson already prints.
