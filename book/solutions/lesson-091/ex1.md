# Solution: exercise 1 — the breath between waves

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 091 — waves](../../lessons/part-5/lesson-091-waves.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The intermission is a timer in the game's own state (`wave_wait`) and
one branch in the wave fight: on a clear, the next wave does not spawn
— the fight announces it and starts breathing; each frame the breath
runs out **by the frame's game-time step**, so the pause freezes it and
a hitstop slows it, like every step of the simulation. `GameWaves` is
the wave fight's function; it just gains the frame's `dt` to do the
counting with.

The run, with the wave lines stamped by the play clock:

```
engine: wave 1 begins — 2 enemies (t=0.000)
engine: wave 1 cleared — wave 2 incoming (t=0.333)
engine: wave 2: spawns bat at 312,232 — speed 20, health 1, chase
...
engine: wave 2 begins — 5 enemies (t=2.662)
engine: wave 2 cleared — wave 3 incoming (t=3.697)
```

The clear at `t=0.333` announces wave 2; wave 2 begins at `t=2.662` —
a breath of 2.3 seconds, the two-second target plus the frame the
change lands in (the same one-frame horizon every event in this engine
has). The second breath repeats it: cleared at `3.697`, the next wave
spawning about two seconds later.

**Why game time and not wall time.** The breath is part of the
*simulation* — the fight's rhythm — so it must stretch when the game
slows and stop when the game stops. A wall-time breath would tick away
during a pause: you would unpause straight into wave 2 with no breath
at all, and a hitstop (which exists to make a moment land) would do
nothing to it. The counter `wave_wait -= dt` is exactly the rule
lesson 078 laid down for everything the world does — and, unlike the
feedback hooks of lesson 086 (which must *end* while the game is
stopped and therefore run on wall time), a breath that waits while
paused is precisely right. The two clocks' split is the game's, and
this is a simulation-side timer on the simulation's clock.
