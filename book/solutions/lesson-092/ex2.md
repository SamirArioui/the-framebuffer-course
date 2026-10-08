# Solution: exercise 2 — the pause that meets the hitstop

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 092 — hitstop and screenshake](../../lessons/part-5/lesson-092-hitstop-shake.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The prediction, written down first.** The game-time scale is the
state's scale *times* the hitstop's factor. Play is `1.00`; pause is
`0.00`; the hitstop is `0.25` while it lasts. So during the pause the
product is `0.00 × 0.25 = 0.00` — the world stands **still**, not
crawling: zero times anything is zero, and the pause wins outright. The
hitstop's countdown, though, is wall time (lesson 078's clock) — so it
keeps running through the pause and `hitstop rested` lands *while the
game is frozen*. And the resume: by then the hitstop is long gone, so
play resumes at full speed with nothing hanging over it.

**The run** — the scratch roster's killing blow, `Escape` pressed right
after the hit lands, the probe printing both factors and the hitstop's
own countdown every frame of the window:

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: feel: hitstop fired (0.25x, 0.30s)
engine: feel: shake fired (10 px, 0.50s)
frame 11: step 42.239 ms, …
engine: probe: state play, scale 1.00 x hitstop 0.25 = 0.25 (hitstop 0.26s left)
frame 12: step 11.024 ms, …
engine: probe: state play, scale 1.00 x hitstop 0.25 = 0.25 (hitstop 0.21s left)
frame 13: step 10.805 ms, …
engine: probe: state play, scale 1.00 x hitstop 0.25 = 0.25 (hitstop 0.17s left)
frame 14: step 10.738 ms, …
engine: probe: state play, scale 1.00 x hitstop 0.25 = 0.25 (hitstop 0.13s left)
frame 15: step 10.803 ms, …
engine: state play -> pause (the player paused)
engine: probe: state pause, scale 0.00 x hitstop 0.25 = 0.00 (hitstop 0.08s left)
frame 16: step 0.000 ms, …
engine: probe: state pause, scale 0.00 x hitstop 0.25 = 0.00 (hitstop 0.08s left)
frame 17: step 0.000 ms, …
engine: probe: state pause, scale 0.00 x hitstop 0.25 = 0.00 (hitstop 0.04s left)
engine: feel: hitstop rested — full speed again
frame 18: step 0.000 ms, …
engine: probe: state pause, scale 0.00 x hitstop 1.00 = 0.00 (hitstop 0.00s left)
```

Every piece of the prediction is there. Before the pause: the product
`1.00 × 0.25 = 0.25` and the step at `10.8 ms` against the run's paced
`43 ms` — the quarter-speed window of the lesson's own run. The pause
lands mid-hitstop: the probe's product flips to `0.00` (`0.00 × 0.25`),
the step goes to `0.000 ms`, and `hitstop rested — full speed again`
prints **between two paused frames** — the hook's wall clock ran on
while the world stood still, and rested on its own deadline with nobody
advancing anything. The shake did the same thing a few frames later in
the same run: `engine: feel: shake rested at 0,0`, its offset settling
to exactly zero while the game was still paused.

And the resume:

```
engine: state pause -> play (the player resumed)
frame 53: step 22.696 ms, …
frame 54: step 6.239 ms, …
frame 55: step 13.629 ms, …
frame 56: step 42.818 ms, …
frame 57: step 43.688 ms, …
```

Full paced speed again — no leftover slowdown. (Frames 53–55 are the
scripted driver's own jitter, frames arriving in a burst around the
resume keypress: their steps are small because their wall steps were
small, not because of any scale. The paced frames after are the run's
usual `43 ms`.)

**Why the product, and not an if-else.** A pause that checked "is a
hitstop running?" and special-cased it would be one more rule to keep
in step with the hooks. The product needs no rules: whatever the state
says, whatever the hitstop says, one multiplication settles it — and
the edge cases fall out for free. A hitstop cannot outlive itself under
a pause (its clock is the wall's). A pause cannot be undone by a
hitstop (zero times anything is zero). And the frame record stays
honest throughout — `step 0.000` is the world genuinely advancing
nothing, exactly as lesson 079's contract promised, at any scale.
