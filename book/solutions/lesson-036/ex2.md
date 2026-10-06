# Solution: exercise 2 — The frame budget

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 036 — frame time as measured data](../../lessons/part-1/lesson-036-frame-time.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

A measurement becomes a decision when it has a line to be on the wrong side
of. The **frame budget** is that line: how long a frame may take. The 60
fps line is `1/60 = 16.7 ms` — the number a game holds itself to when it
wants the window to feel alive.

The change puts the budget where the data is: `FRAME_BUDGET` next to
`FrameStats`, and the account counts every frame that fit inside it. The
exit summary grows one line:

```
$ DISPLAY=:99 xdotool key --delay 40 --repeat 8 --window <id> Right
engine: 17 frames — avg 1.070 ms (update 0.000, render 0.482, present 0.587)
engine: worst frame 1.842 ms (frame 2); present is 54% of the frame
engine: within the 16.7 ms budget: 17/17 frames
```

17 of 17 — which is the honest result for an engine drawing one square:
the worst frame measured here is under 2 ms, so the budget is not yet
interesting. It becomes interesting exactly the way the course grows:
Part 2 fills the framebuffer with a scene, and the number starts to move;
Part 5 turns this counter into the frame-budget report — which frames fit,
which did not, and where the time went when they did not.

That is also why the budget lives in the engine's record code and not in
the platform layer: how long a frame *may* take is the game's policy. What
a frame *did* take is measured on the platform's clock. The two meet in one
line of arithmetic, and the line is this exercise's.
