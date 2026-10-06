# Solution: exercise 1 — Platform layer done, on your machine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 043 — the closing demo](../../lessons/part-1/lesson-043-demo.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The account says what the frames cost; the instrument adds *how long* the
run measured and what that averages to — one line that turns a per-frame
account into a rate you can compare across machines:

```
engine: 21 frames — avg 0.904 ms (update 0.000, render 0.439, present 0.465)
engine: worst frame 1.797 ms (frame 1); present is 51% of the frame
engine: 1.82 s measured, 11.5 frames/s while awake
```

(That run: 21 frames in 1.82 seconds of *running*, driven by scripted
input. "While awake" is the honest qualifier — the engine sleeps between
news, so the rate is a rate of frames, not of CPU.)

Now the port. On a real desktop, the same run gives different numbers, and
the differences are the write-up:

- **The present cost moves most.** Under Xvfb the copy is a few hundred
  microseconds against a virtual screen. On a desktop there is a real
  driver, possibly a compositor, and a real monitor's refresh in the way.
  The lesson-036 numbers were honest for *their* machine; yours are honest
  for yours.
- **The render cost barely moves.** `ClearBuffer` and the marker are
  engine arithmetic — that number is about your CPU, not your display.
- **The rate is your input's rate.** Frames happen when news happens
  (lesson 034's pacing note) — so a mechanical keyboard's auto-repeat, a
  slow touchpad, and a colleague mashing arrows all give different
  frame rates for the same engine.

Record the machine you measured on (that is what a measurement *is*), and
compare the shape rather than the values: present as a share of the frame,
worst frame against average, update always negligible. If your shape
differs — say, the present is 90% of the frame — that is not an error to
fix; it is the thing Part 5's frame-budget work will be about.
