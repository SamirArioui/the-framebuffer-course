# Solution: exercise 1 — The first eight frames

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 059 — sound as samples](../../lessons/part-3/lesson-059-samples.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch widens the probe from four frames to eight and adds one search
over the buffer: the loudest sample by magnitude, and the frame number it
first lands on. The run then prints:

```
engine: tone: 22050 frames at 44100 Hz, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: tone: peak 8191 at frame 25
engine: tone played
```

Reconciling the prediction — frame `i` is the scaling in `GenerateTone`
written out, `(short)(sin(2π · 440 · i / 44100) * 0.25 * 32767)`:

- **Frame 0 is exactly 0** — the sine starts at rest, no arithmetic
  needed. That first zero is the format's rest position, and it is why
  a tone that starts at its buffer's beginning starts silently.
- **Frames 1-3** (513, 1024, 1531) were on the page to check against,
  and they are the arithmetic: one step of phase is 440/44100 of a turn
  (about 0.0627 radians), and near zero the sine is nearly its own
  argument — hence steps of roughly 512 counts at the start.
- **Frames 4-7** (2032, 2525, 3009, 3480) are the prediction the check
  rewards. The steps keep shrinking — 501, 493, 484, 471 — as the wave
  bends toward its crest. If your prediction landed one count high
  (2033, 2526, 3481 …), that is the cast: `(short)` truncates toward
  zero, it does not round. The stored frame is always the exact product
  with its fraction cut.
- **The peak lands at frame 25.** A 440 Hz wave's quarter period is
  `44100 / (4 * 440)` = 25.06 frames, so the first crest falls between
  frames 25 and 26 — and frame 25 samples 0.999994 of that crest
  (8191.7 counts), which the cast truncates to 8191, while frame 26 has
  already fallen off the far side. The value is the one the prose
  derived: amplitude 0.25 puts the crest at `0.25 * 32767` = 8191.75
  counts, and no frame can store past 8191. The frame number is the
  quarter period rounded down — `25 / 44100` = 0.567 ms into the tone.

The troughs reach −8191 at the same magnitudes a half turn later (the
cast truncates toward zero on both sides of the wave, so the format's
−32768 never appears at this amplitude). The search runs over magnitude,
so it reports 8191 whichever side it finds first — and the positive
crest is first.

None of this needs a device: the probe prints before `SubmitSamples`,
which is the point of it — the numbers are checkable before they are
audible. And the wrap fact from the prose closes the arithmetic: frame
22050, one past the buffer's end, would be exactly frame 0 again — 220
whole cycles in 22050 frames. The crest recurs every 100.23 frames all
the way to that edge.
