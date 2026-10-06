# Solution: exercise 1 — The Part 2 demo on your machine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 057 — the closing demo](../../lessons/part-2/lesson-057-demo.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The account says what the frames cost; the instrument adds *how long* the
run measured and what that averages to — one line that turns a per-frame
account into a rate you can compare across machines:

```
engine: 26 frames — avg 2.068 ms (update 0.022, render 1.436 incl. sprites 0.001, text 0.007, tilemap 0.963, present 0.610)
engine: 4.77 s measured, 5.4 frames/s while awake
```

(That run: 26 frames in 4.77 seconds of *running*, driven by scripted
input. "While awake" is the honest qualifier — the engine waits between
news, so this is a rate of frames, not of CPU.)

Now the port. On a real desktop the same demo gives different numbers,
and the differences are the write-up:

- **The present cost moves most.** Under Xvfb the copy is a few hundred
  microseconds against a virtual screen; on a desktop there is a real
  driver, a compositor, and a monitor's refresh in the way. Part 1's
  lesson 043 found the same shape — present is the part of the frame
  that belongs to the machine.
- **The tilemap row is yours.** `tilemap 0.963 ms` is CPU work: 1,536
  blits through a `-O0` build. It will move with your CPU and with the
  world's size — the exercise's last question (resize the world: edit
  `assets/map.txt` to 64×64 cells) turns it into a scaling curve on your
  machine.
- **The named phases are the point.** With the render split into
  `sprites / text / tilemap`, comparing machines means comparing *rows*,
  not just totals. If your render is 3× the book's but your tilemap row
  is 3× too, the renderer is not slower — the walk is.

Record the machine with the numbers (CPU, build flags, whether it was
Xvfb or a desktop). The book's column and yours are two honest
measurements of the same code; the shape — tilemap dominating render,
sprites and text nearly free, present owning a machine-dependent share —
is what should survive the trip.
