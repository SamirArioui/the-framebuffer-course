# Solution: exercise 1 — The shake that decays

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 054 — the camera](../../lessons/part-2/lesson-054-camera.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch turns the square wave into steps: the shake runs 30 frames and
its amplitude drops 6 → 4 → 2 → 0 in thirds, while the sign keeps
alternating. The report prints the additive whenever it *changes*, so
the log is the effect's whole path rather than one line per frame. Space
pressed, then input held so the frames run:

```
engine: camera additive 6,0 (shake starts)
engine: camera additive 6,0
engine: camera additive -6,0
engine: camera additive 6,0
engine: camera additive -6,0
engine: camera additive 6,0
engine: camera additive -4,0
engine: camera additive 4,0
engine: camera additive -4,0
engine: camera additive 4,0
engine: camera additive -4,0
engine: camera additive 4,0
engine: camera additive -4,0
engine: camera additive 4,0
engine: camera additive -2,0
engine: camera additive 2,0
engine: camera additive -2,0
engine: camera additive 2,0
engine: camera additive -2,0
engine: camera additive 2,0
engine: camera additive -2,0
engine: camera additive 2,0
engine: camera additive 0,0
engine: camera additive 0,0 (at rest)
```

The path is exactly the design: six for the first third, four for the
second, two for the last, alternating sign every frame — then exactly
`0,0`. (The `0,0` line before "at rest" is the change report firing as
the amplitude's last step lands; the parenthesized line is the run
saying where the hook lives.)

What decay buys the *feel*: a constant shake reads as a bug — the screen
vibrates and keeps vibrating. A decaying shake reads as an *event* —
something happened, and its force spent itself. The eye tracks the
envelope (the falling amplitude) more than the oscillation; the envelope
is what makes the effect feel like consequence rather than noise. This
is the shape Part 4's toolkit generalizes (easing curves instead of
steps, duration instead of frame counts) — but the *idea* is here in
four lines.

What the *report* buys: the log above is a complete trace of the hook's
state machine, one line per change. When the juice toolkit later drives
this field from three different effects at once, the same report will
show which effect wrote what and when — and a shake that never returns
to zero (the classic bug) is visible at a glance as a log that ends
without the parenthesized line. Report the hook, not just the frame.

One more thing the exercise surfaces: the shake only runs while frames
run — and frames happen when news happens (lesson 034's pacing note).
Holding a key during the shake is what made the 30 frames complete in
the transcript above; an idle machine would stretch those 30 frames
across minutes. A time-based decay (`shake_seconds` counted on the
platform clock instead of frame counts) is what a real effect wants —
and it is a fine next step from here.
