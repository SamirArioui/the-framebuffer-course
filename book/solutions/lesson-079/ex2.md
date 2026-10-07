# Solution: exercise 2 — What a paused game costs

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 079 — measurement is not scaled](../../lessons/part-4/lesson-079-wall-clock.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is a window account: the run counts every frame and sums its
wall-clock `total`, and each window closes when the knob turns — so the
report names the average frame at *each* scale, from the same run, on
the same machine, with nothing else changing.

The numbers, from a real run of this lesson's end state plus the patch
on the authoring machine (headless display, scripted presses, no sound
device):

```
engine: window (play): 33 frames at scale 1.00 averaged 1.791 ms of wall clock
engine: window (hitstop): 16 frames at scale 0.25 averaged 1.772 ms of wall clock
engine: window (pause): 32 frames at scale 0.00 averaged 1.780 ms of wall clock
engine: window (the run's last): 64 frames at scale 1.00 averaged 1.715 ms of wall clock
```

**Pause is not free.** The paused window averaged 1.780 ms a frame
against play's 1.791 ms — the same cost, to within the noise of the
measurement. Hitstop, a quarter-speed simulation, averaged 1.772 ms.
The scale is doing nothing to the frame's price, which is the point:
the frame is machine work, and the machine does not know the game is
standing still.

**Where the time goes** is in the record's phases (lesson 079's log
lines): `render` ~1.36 ms — the tilemap's walk and the blits, the same
scene drawn every frame — and `present` ~0.5 ms — the copy to the
window. `update`, the only phase the scale touches, is ~0.01 ms: the
part of the frame the game *could* make free is the part that already
almost is. A paused frame's cost is the cost of showing a paused game.

**What it would cost to also stop the presentation**: the render and
present rows, ~1.85 ms of the 1.78 ms frame — nearly all of it — plus
whatever the window system does with a window that never updates. The
game would save that and lose the pause screen: no menu drawn, no
fade-out presented, no way for the player to see what they paused. The
numbers make the trade explicit rather than a matter of taste.

**The design question.** If a pause screen wants to be cheaper, what
should it change? The **scale** — no: it is already zero and the cost
is not there. The **draw** — yes, and this is the real lever: draw
*less*, not nothing. A pause screen that draws the frozen world once
into a buffer and then draws only the menu's moving parts can drop the
tilemap's walk (the biggest row) from every frame; that is a rendering
decision, not a time decision. The **machine's schedule** — sometimes:
a pause on a battery-powered machine may drop the presentation's frame
rate (present every other frame, or only when the menu changes), which
is the loop pacing the platform layer already owns (lesson 060's paced
wait), not the game-time scale. All three levers exist; the scale is
the wrong one for this problem, and the measurement is what says so.

That is the exercise's real lesson: **the knob you have is not always
the knob that controls the cost you are paying**. The frame record's
wall-clock discipline is what turns "the game is paused" from a feeling
into a row of numbers that points at the right lever.

Nothing here touches the scale, the record, or the loop: the window
account is one struct and one line beside the lesson's own.
