# Solution: exercise 2 — One knob, not three

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 078 — the game-time scale](../../lessons/part-4/lesson-078-game-time.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is a probe that makes the answer checkable: at every scale
transition it reports what the update *did* on that frame — how many
entities the walk visited and how much wall clock the update took. If
the update were being skipped or slowed, these numbers would show it.

The runs, from this lesson's end state plus the patch — the demo's
script turning the knob at three, five, and seven seconds:

```
engine: game-time: at scale 0.25 — 8 entities walked, update 0.019 ms of wall clock
engine: game-time: at scale 0.00 — 8 entities walked, update 0.018 ms of wall clock
engine: game-time: at scale 1.00 — 8 entities walked, update 0.019 ms of wall clock
```

Eight entities at every scale, the update costing the same ~0.019 ms of
*wall clock* at every scale. The pause's frame walked its eight
entities exactly like the play frames did; the only thing that was zero
was the step the walk multiplied into motion.

**What the update still does at scale 0**, then: it reads input
(polled state, lesson 032 — the pause screen's keys are what unpauses),
it walks the store (the per-entity work still runs; the step it moves
things by is zero), it draws the whole scene, it feeds the audio (a
paused game keeps its music — the mixer does not know about game time
and should not), it presents, and it measures. What it does *not* do is
advance the simulation. A pause screen needs all of those: without
input it could never be left, without drawing it would freeze a frame
mid-present, without measurement the frame budget would lie about what
a paused game costs.

**What breaks if the scale reached the platform clock**: two things,
both named in this change's specs. The frame record would measure
*scaled* durations — a paused game would report `total 0.000 ms` for
frames that took 2 ms of machine time, and the frame-budget table (and
lesson 081's attribution) would become fiction; that is the "the scale
does not reach the seam" scenario. And the seam's contract would change
under the platform layer — `Now()` would stop being the monotonic
measurer lesson 035 fixed, and every consumer of it (the paced wait's
schedule, the report's timestamps) would inherit game time without
asking for it. The engine's answer is the one this lesson implements:
the clock measures, the step scales, and the two never meet.

**The case against**, honestly: a game that wants pause to be *total*
would find this design too weak — a pause that also silences the music,
stops the rendering (for battery), and freezes a cutscene's real-time
animation is not "a scale of 0", it is "the loop stopped", and that is
a different mechanism (the run's own `paused` flag deciding what the
frame does). This engine keeps pause as a scale because its pause
screen — Part 5's L1 — wants the world stopped and the *presentation*
alive. A design is "right" for the game it serves; the knob is one
because this game's two needs (pause, hitstop) are one need seen at two
positions.

Nothing here touches the scale's service, the update's shape, or the
frame record: the probe is one report beside the lesson's own.
