# Solution: exercise 1 — the volume follows the distance

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 095 — audio integration](../../lessons/part-5/lesson-095-audio.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The loudness is one function in `sound.cpp` and one extra fact on every
event: **how far away it happened**. `Loudness` scales the sound's base
volume by the falloff — full at the hero's ear, nothing at `FADE`
pixels — and it works in *squared* distance, so no square root runs to
mix a sound (the same rule the AI's distance checks follow):

```cpp
double d2 = dx * dx + dy * dy;
double f2 = FADE * FADE;
if (d2 >= f2)
    return 0;
return (int)(base * (1.0 - d2 / f2));
```

The events carry `(dx, dy)` from the hero: the hero's own shot passes
`(0, 0)` — a shot at the player's ear — the enemies' shots their
shooter's offset, the hits and deaths the point they happened at. A
sound at the falloff's edge does not fire at all (and says so:
`sound: … — silent at N px`), so an event across the map costs the pool
nothing.

The run — a scratch fight with two armed bags at known distances (24 px
and 150 px from the hero) — reports exactly the falloff's arithmetic:

```
engine: fire: near -> bolt (damage 0, range 160)
engine: sound: shot -> channel 1 (volume 63 of 256)
engine: fire: far -> bolt (damage 0, range 160)
engine: sound: shot -> channel 2 (volume 42 of 256)
engine: fire: hero -> bolt (damage 1, range 160)
engine: sound: shot -> channel 3 (volume 64 of 256)
engine: hit: bolt hits hero — damage 0, health 3 -> 3
engine: sound: hit -> channel 4 (volume 127 of 256)
```

- The hero's own shot: `volume 64` — the blip's full quarter, distance
  zero.
- The near bag (24 px): `64 × (1 − (24/256)²) = 63.4` → **63**.
- The far bag (150 px): `64 × (1 − (150/256)²) = 42.0` → **42**.
- The hit on the hero (a few px): `128 × (1 − ε) = 127` — the thud's
  full half, essentially at the player's ear.

and the hit that lands across the fight is quieter still: `hit ->
channel 3 (volume 92 of 256)` is the far bag being shot at ~135 px
(`128 × (1 − (135/256)²) ≈ 92`). Two identical blips, two different
loudnesses — the mixer needed nothing new for any of this; the volume
knob on every channel was already there, waiting for the game to say
how loud each moment is.

One tuning note for the reader's own game: `FADE = 256` px was chosen
so the falloff *bites inside this game's combat range* (the enemies
fire within 160 px). A falloff longer than the world is not a falloff.
