# Solution: exercise 3 — Pause

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 023 — the state machine: title, play, death](../../lessons/part-0/lesson-023-state-machine.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

Pause is a state, not a flag — which is exactly what this lesson is about.
The patch adds `PAUSE` to `enum GameState` and its name to `state_names`, two
`p` rows in the key handling (one for each direction of the toggle, each
guarded by the state it leaves), and one line of drawing. `Update` needed no
change at all: the tick loop already advances the snake only `if (state ==
PLAY)`, so `PAUSE` freezes the simulation for free — the payoff of routing
all motion through one state check.

A run proves it: `( printf ' p'; sleep 1.5; printf 'p' ) | ./snek 75` shows
`state=pause … at=10,20` from frame 1 to about frame 45 — the head never
moves — then `state=play` and the position advances again after the second
`p`. Note what the trace also shows: `tick` keeps counting while paused. The
fixed timestep measures real time and real time does not stop; what pause
freezes is the *game's response* to it. Freezing the accumulator as well
would make pause wall-clock-exact at the cost of a subtler `Update` — both
designs are defensible, and knowing which clock stops is the part that
matters.
