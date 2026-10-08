# Solution: exercise 1 — the shake that settles

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 092 — hitstop and screenshake](../../lessons/part-5/lesson-092-hitstop-shake.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The shake's envelope is one division. `Feedback` grows `shake_total` —
the shake's whole life — recorded where `FeelShake` fires it, and the
drive in `FeelUpdate` scales the offset's magnitude by what is left:

```cpp
double left = feel.shake_total > 0.0 ? feel.shake / feel.shake_total : 0.0;
double mag = feel.shake_mag * left;
camera.add_x = ((int)(feel.shake * 40.0) & 1) ? (int)mag : -(int)mag;
```

The alternation is untouched (the sign still flips while the shake
runs) and the rest is untouched — the hook's own settle at exactly
`0,0` is the rule that had to hold, and it does. Only the *size* of the
swings shrinks as the shake's life runs out, so the camera comes back
to rest instead of snapping off a full swing.

The run — the scratch roster of the lesson's killing-blow excerpt, one
fragile standing kind, the probe printing the offset every frame the
shake drives it — shows the ramp (the death's 10 px shake):

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: feel: shake fired (5 px, 0.25s)
engine: feel: shake fired (10 px, 0.50s)
engine: probe: camera add -9,0 (shake 0.457s left)
engine: probe: camera add -8,0 (shake 0.414s left)
engine: probe: camera add -7,0 (shake 0.370s left)
engine: probe: camera add 6,0 (shake 0.327s left)
engine: probe: camera add 5,0 (shake 0.284s left)
engine: probe: camera add 4,0 (shake 0.241s left)
engine: probe: camera add 3,0 (shake 0.198s left)
engine: probe: camera add -3,0 (shake 0.155s left)
engine: probe: camera add -2,0 (shake 0.112s left)
engine: probe: camera add -1,0 (shake 0.069s left)
engine: probe: camera add 0,0 (shake 0.026s left)
engine: feel: shake rested at 0,0
```

`-9, -8, -7, 6, 5, 4, 3, -3, -2, -1, 0` — the swing falls from 9 px to
under a pixel across the shake's half-second, still alternating, and
the last frames of the shake move the camera by less than a pixel: the
shake eases into rest instead of cutting to it. Then the hook's own
rest line: `shake rested at 0,0` — exactly zero, the whole point.

Two things worth noticing in the numbers. The first probe line reads
`-9`, not `-10`: the frame the shake fired in already settled once, so
the first swing is one frame's decay in — the same frame-granularity
the lesson measured on the hitstop's deadline. And the probe's
`shake …s left` is the hook's own countdown, which still runs on wall
time and still ends the shake on its own — the settle changed how the
shake looks while it lasts, not when it ends, and not that it ends at
exactly zero.
