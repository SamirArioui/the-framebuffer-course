# Solution: exercise 2 — Why the scale

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 082 — the game skeleton](../../lessons/part-5/lesson-082-skeleton.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is one field of instrumentation: the frame log prints the
game-time scale beside the step it produced. It makes the answer visible
— the freeze is not in the loop skipping work, it is in one number.

The run, from this lesson's end state plus the patch — the title, a play
frame, then pause:

```
frame 1: scale 0.00, step 0.000 ms, update 0.001 ms (entities 0.001), … tilemap 0.000), …
frame 3: scale 1.00, step 20.362 ms, update 0.002 ms (entities 0.002), … tilemap 0.992), …
frame 4: scale 0.00, step 0.000 ms, update 0.012 ms (entities 0.002), … tilemap 0.000), …
```

**Why the scale and not a skipped update.** `step` is game time — the
wall clock's step scaled (lesson 078). At `scale 0.00` the step is
`0.000`: the world advanced nothing. But look at `update 0.012 ms` on
that same paused frame. The update *ran*. It read the input, updated the
hero's request, timed the walk, printed its reports — it cost real wall
time and advanced the world by zero game-seconds. The freeze lives in
the *step*, not in the loop refusing to call the update. Now suppose the
loop instead did `if (paused) continue;` before the update:

- **The frame record would lose a phase.** Lesson 079's contract is that
  the record measures wall clock and is *not* scaled — a paused frame is
  still a frame, and the account still owes an `update` number for it.
  Skipping the call leaves the row empty; scaling the step keeps the row
  honest and shows the game standing still in `step`, not in `update`.
  The pair above — `update 0.012`, `step 0.000` — is the proof: the
  machine worked, the world did not.
- **The input latch would starve.** `KeyPressed` is an edge the event
  pump sets and the update consumes. If the update does not run on a
  paused frame, the press that arrived during pause is either lost or
  latched with no one to read it; the pause key would act twice or never.
  Running the update every frame is what lets Escape resume cleanly.
- **A fractional scale would be impossible.** Lesson 092's hitstop is
  the same knob at a fraction — `scale 0.25` slows the world, it does not
  freeze it. A `continue` is a binary: on or off. There is no
  "half-skipped" update. The scale is one number that already spans pause
  (0), hitstop (a fraction), and play (1); building the freeze as a
  special case would throw away the one knob the juice toolkit needs.

**The measurement question.** `update` costing real time while `step` is
zero proves the freeze is *downstream* of the update — in the step it
advances the world by — and not in the update being skipped. If the
freeze were a skipped update, `update` would read `0.000` too (there
would be nothing to measure). It does not: it reads `0.012 ms`. The
update runs; `GameTimeStep` multiplies its `dt` by the scale, and the
scale is zero, so the world moves nothing. That is the whole mechanism,
and the log's two columns — `scale 0.00`, `update 0.012` — are it,
measured rather than argued.

Nothing here changes the machine: the patch only adds the scale to the
line the frame log already prints. The answer was in the record all
along; this makes it a column.
