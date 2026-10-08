# Solution: exercise 2 — the chaser's staircase

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 089 — enemy AI](../../lessons/part-5/lesson-089-enemy-ai.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The prediction.** The bat sits at `(560, 72)`, the hero at
`(312, 232)`: the delta is `(−248, +160)` — the bat is 248 px to the
right and 160 px above. `AiChase` writes the compass point of that
delta: `(−1, +1)` scaled by 1/√2 — down-left at 45°. The chaser walks
that diagonal until one of the two gaps closes. The y gap is the
shorter one (160 < 248), so it closes first: after 160 px of diagonal
the bat is at `(400, 232)` — level with the hero — and the delta is
now `(−88, 0)`, whose compass point is `(−1, 0)`. The last leg is
straight left, 88 px, into the hero. One diagonal, one straight leg —
that is the staircase: it turns exactly once, and only at an alignment.

**What the run shows.** The probe prints the direction whenever a
chaser's compass point turns — and it turns a *lot*:

```
ai: bat steers -0.707,0.707
ai: bat steers 0.707,0.707
ai: bat steers -0.707,0.707
ai: bat steers 0.707,0.707
```

The prediction's clean turn is a **wobble** in practice. When the bat
is nearly level with the hero, the delta's small component flips sign
between frames (a step of 6 px either side of zero), and the compass
point flips with it — the bat zigzags down the last leg instead of
running straight. The eight-lane world has no "mostly straight": a
direction is one of eight, every frame. (The zigs are small — the bat
ends at `311,176` against the hero at `312,232`, level to within a
pixel.) The ladder is not pure geometry either: the run's quoted path
bends where the map bends it — a step whose x is refused still moves
in y, because the mover resolves **x first** and the slide is the
one-axis rule of lesson 084. Which brings the last question:

**A wall across the diagonal.** The mover tries x, then y. A wall
across the diagonal refuses the x step first; the y step is still free,
so the bat slides *along* the wall in y — the staircase flattens
against the obstacle. And in this very run the bat ends wedged at
`(311, 176)`, held at the pillar's top edge below the hero: its y step
is refused (the pillar's column), and the wobble's x oscillation never
carries it far enough sideways to clear the pillar's column and
descend. That is the honest limit of a compass-point chaser with no
pathfinding — it slides, it corners, and it can wedge — and it is the
map's and the mover's answer, not the behavior's: the behavior only
writes requests.
