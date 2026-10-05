# Solution: exercise 1 — Counting to the wall

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 023 — the state machine: title, play, death](../../lessons/part-0/lesson-023-state-machine.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction, from the geometry alone: the snake spawns three segments at
row `GRID_ROWS / 2` — row 10 — with the head at column 20, and one arrow-up
turns it toward the top wall. Each tick climbs one row, and death comes when
the *next* head would land on the border — the interior stops at row 2, so
rows 9, 8, … 2 are eight legal moves and the ninth move reaches row 1 and
dies. The state trace flips from `state=play` to `state=dead` at **tick 9** —
and the last legal cell, `at=2,20`, is where the body stays frozen on the
death screen.

The instrumented run agrees exactly: `head 9,20` down to `head 2,20` are the
eight moves that happen; `head 1,20` is the move that does not — the wall
check in `AdvanceSnake` rejects it and switches state instead. Note the
space bar in `printf ' \033[A'` was essential: the up-arrow only steers
while `state == PLAY`, so the run must first leave the title screen. The
death tick is a pure function of geometry and the fixed timestep — the frame
number is not, since frames carry no game time on their own.
