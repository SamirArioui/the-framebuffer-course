# Solution: exercise 2 — Why wall-time

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 086 — feedback and animation](../../lessons/part-5/lesson-086-feedback-animation.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The probe prints the hitstop's own countdown in **wall seconds** beside
the game-time factor it is holding up — the two clocks, side by side.

```
engine: feel: hitstop 0.300 s of wall time left, scale factor 0.25
engine: feel: hitstop 0.200 s of wall time left, scale factor 0.25
...
engine: feel: hitstop rested — full speed again
```

The countdown falls by real seconds while the factor sits at `0.25`. That
is the point.

**Why not game time?** A hitstop's whole job is to *change* the game-time
scale — to `0.25` here. If the hitstop's own countdown ran on game time,
it would be multiplied by that same `0.25`: the countdown would advance a
quarter as fast, so a 0.4-second hitstop would take 1.6 real seconds —
and the more dramatic the slowdown, the longer it would last, which is
backwards. Worse, at a *full* pause (scale `0`) game time does not move
at all, so a hitstop's countdown on game time would freeze forever: the
one thing that must end *while the game is stopped* would never end.
Lesson 078 settled this clock — anything that has to finish while the
game is stopped (a hitstop, a screenshake under pause) runs on the wall
clock.

**What would happen to a screenshake on game time?** The same trap: pause
mid-shake and game time stops, so the shake would hang forever with the
camera stuck off-center — the offset would never return to `0,0`. On
wall-time it keeps decaying and settles at exactly zero even under pause.

**What the two rest lines prove.** `hitstop rested — full speed again`
and `shake rested at 0,0` — both hooks finished *on their own* while the
hitstop was still holding the scale at a fraction. The hitstop ended even
though game time was slowed to `0.25` the whole time it ran; if its clock
were game time, it could not have. Those two lines are the wall-time
choice, measured: the hooks' lives are in real seconds, independent of
the very scale they set.
