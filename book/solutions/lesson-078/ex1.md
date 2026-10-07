# Solution: exercise 1 — Hitstop that ends

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 078 — the game-time scale](../../lessons/part-4/lesson-078-game-time.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is the hitstop as a *fireable* thing: SPACE sets the scale to a
quarter and puts a deadline in the run's bookkeeping — `hitstop_until`
— and the update checks the deadline every frame and restores full
speed. The shake rests for the exercise (its SPACE demo yields the
key); everything else in the run is untouched.

Now the question the call forces: **a duration of what?** The deadline
is `platform::Now() + 0.2` — **wall time** — and that is the right
choice here, for two reasons, the second of which settles it:

1. *Feel is measured the way it is experienced.* Hitstop exists to be a
   moment the player feels — 0.2 s of their real attention — so it
   lasts 0.2 s of real time. (A game-time countdown at scale 0.25 would
   last 0.8 s of real time: the slowdown would stretch the very effect
   that caused it.)
2. *A game-time countdown cannot end anything while the game is
   stopped.* At scale 0 the game step is 0, so a timer advanced by game
   time never advances — a hitstop scheduled in game time would end
   exactly never. Every effect that has to *restore* the scale has to
   run on a clock the scale does not reach. That is the same argument
   lesson 079 makes about the frame record, and it is why the seam's
   wall clock stays the engine's one measurer.

The run, from this lesson's end state plus the patch — one SPACE press
at the start of the run:

```
engine: game-time: hitstop fired — scale 0.25 for 0.2 s of wall time
engine: game-time: hitstop over — scale 1.00
```

The scale came back on its own, about 0.2 s of wall clock later (the
next frame after the deadline — this machine's loop only runs when
input news arrives, so "on its own" means "on the next frame whose
clock has passed the deadline"). The hero's steps crawl for the
hitstop's window and return to full afterward; the run's own scale
script then turns the knob again at three seconds, which is the demo's
business and not the hitstop's.

The design detail worth stealing for Part 5: the hitstop's state is one
number (`hitstop_until`) and the restore is a comparison. When L11 adds
screenshake beside it, the same shape works — a deadline in wall time,
checked in the update — and neither effect needs a second clock, a
callback, or a pause in the loop.

One thing this exercise deliberately leaves open: what happens if a
hitstop is fired *while another is running*? Here the deadline is
simply pushed later and the scale re-set. That is a policy choice —
refresh, ignore, or stack — and it belongs to the game, not the
service, exactly like the store's refusal policy did.

Nothing here touches the scale's service, the walk, or the frame
record: the patch is one fireable effect and its deadline.
