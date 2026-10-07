# Solution: exercise 1 — The Part 3 demo on your machine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 069 — the closing demo](../../lessons/part-3/lesson-069-demo.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The account says what the run did and what it cost; the instrument adds
*how long* it ran and what the two halves of it average to — one line
that turns the demo's counts into rates a machine can be compared by:

```
engine: demo: 466 frames measured, 466 buffers fed, 21 effects fired, 2 music wraps
engine: demo: 8.00 s run, 58.3 frames/s while awake, sound fed at 58.3 buffers/s
engine: frame budget — 466 frames, avg 2.129 ms, worst 3.612 ms (frame 115)
engine:   subsystem   avg ms    share
engine:   update       0.001       0%
engine:   render       1.403      66%
engine:     sprites    0.002       0%
engine:     text       0.008       0%
engine:     tilemap    0.974      46%
engine:   present      0.687      32%
engine:   total        2.129     100%
```

(That run: 466 frames in 8.00 seconds, undriven — 58.3 frames/s while
awake, and the sound fed at exactly the same 58.3 buffers/s. "While
awake" is the honest qualifier: the loop waits between news, so these
are rates of the run, not of the CPU. The two rates matching is the
paced wait's signature — an undriven loop feeds one buffer per frame,
one horizon apart.)

Now the port. On a real desktop the same demo gives different numbers,
and the differences are the write-up:

- **The present cost moves most.** Under Xvfb the copy is a few hundred
  microseconds against a virtual screen; on a desktop there is a real
  driver, a compositor, and a monitor's refresh in the way. Part 2's
  lesson 058 found the same shape — `present` is the row that belongs
  to the machine.
- **The tilemap row is yours.** `tilemap 0.974 ms` is CPU work: 1,536
  blits through a `-O0` build. It moves with your CPU and with the
  world's size; everything else in the render is noise beside it.
- **The audio numbers have two new questions only your hardware can
  answer.** On this machine the submit returns immediately — `null`
  takes the samples and drops them — so the phase reads as the mix's
  cost and the two rates above agree. On real hardware a submit can
  wait for room in the device's buffer. If yours does, it shows up
  exactly here: the sound's rate stays pinned near 60 buffers/s by the
  buffer horizon while the frames/s and the per-frame `audio` numbers
  grow around the wait. Compare your `frame N:` lines' `audio` column
  against the book's 0.028-0.034 ms — a phase that sometimes reads
  milliseconds is not a slower mix, it is a device pushing back.

Record the machine with the numbers (CPU, build flags, Xvfb or
desktop, and the ALSA device name — `aplay -l` tells you what is
really there). And the last question is yours alone: this authoring
machine has no speakers at all — `null` accepted every sample and made
no sound — so what the demo *sounds* like, music wrapping under a
burst of effects, is the one fact the book cannot print.
