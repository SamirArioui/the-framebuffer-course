# Solution: exercise 2 — Heavier to stop

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 085 — hero movement](../../lessons/part-5/lesson-085-hero-movement.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

One constant makes starting and stopping mirror images. Split it and the
hero can feel crisp off the line and weighty coming to rest:

```cpp
constexpr double HERO_ACCEL = 0.09; /* eases up to speed quickly */
constexpr double HERO_DECEL = 0.22; /* coasts to a stop */
```

The only question is which to use each frame — is the hero speeding up
or slowing down? The patch compares the *magnitudes*: the intent's
squared length against the current velocity's squared length.

```cpp
double tau = (intent2 > move2) ? HERO_ACCEL : HERO_DECEL;
```

- **Speeding up** — the intent is faster than the hero currently moves
  (pushing from rest toward full speed, or a slow turn into a faster one):
  use `HERO_ACCEL`, the quick one.
- **Slowing down** — the intent is zero (released) or slower than the
  current motion: use `HERO_DECEL`, the heavy one.

Comparing magnitudes is a clean, cheap rule and it falls straight out of
the ease already computing both vectors. It is not the only reasonable
one — you could test whether the intent *opposes* the velocity (a dot
product) to catch a hard turn and treat it like a stop — but for
"crisp start, heavy stop" the magnitude test is enough. Either way, the
choice is per-frame and needs no state.

Two invariants the exercise asked you to keep, and the patch does: the
**diagonal normalization** is untouched (the intent is still scaled by
`1/√2`, so the diagonal is still the straight-line speed — the constants
change *when* the hero reaches a speed, never *which* speed), and the
**frame-rate independence** is untouched (the ease is still `dt / tau`,
so the curve is the same shape at any frame rate — the constants are in
seconds, not per-frame steps).

The feel, by the numbers: with `HERO_ACCEL = 0.09` the hero closes on
full speed about a third sooner than the symmetric `0.12` did (the
acceleration curve is tighter), and with `HERO_DECEL = 0.22` it takes
nearly twice as long to come to rest (the deceleration curve is longer —
it coasts). Run it and hold-then-release: the up-ramp is visibly shorter
than the down-ramp. If the coast feels too floaty or the start too
sluggish, those two numbers are the whole tuning surface — they are the
hero's weight, and they are data-in-waiting (lesson 087 moves feel like
this into the hero's table row).
