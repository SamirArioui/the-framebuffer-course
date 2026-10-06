# Solution: exercise 1 — The horizon, doubled

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 060 — the stream's shape](../../lessons/part-3/lesson-060-stream.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch widens the buffer one feed hands the device: `CHUNK_FRAMES`
becomes `AUDIO_RATE / 30` — 1470 frames, one thirtieth of a second — and
the comment beside it is re-derived rather than left stale, because the
cursor's divisibility is a fact about *both* constants. The startup
report follows the constant without further work:

```
engine: stream: 1470-frame buffers, horizon 33.3 ms; the loop feeds one when it is due
```

Measured, runs of the same three-second length against the `null`
device, before and after (frame counts vary by one run to run; the
cadence does not):

| | one sixtieth | one thirtieth |
| --- | --- | --- |
| frames in 2.985 s | 176 (frames 1–176) | 89 (frames 1–89) |
| cadence | 59.0 frames/s | 29.8 frames/s |
| median gap | 17 ms | 34 ms |
| horizon | 16.7 ms | 33.3 ms |
| `audio` per feeding frame | 0.004–0.038 ms (171 of 176 in 0.004–0.007) | 0.007–0.069 ms (83 of 89 in 0.007–0.010) |
| feeds | 176 | 89 |

Reconciling the two rows the exercise asks about:

- **The cadence is the horizon's.** Doubling the buffer halves the
  feed rate: 735 frames at 44100 Hz are 16.7 ms and 1470 frames are
  33.3 ms, and the measured gaps (17 ms and 34 ms) land just past each —
  the millisecond ceiling in `PumpEvents`' timeout, the same rounding the
  lesson's prose names. Each wake arrives with the buffer already due,
  the feed happens at once, and the next deadline is a horizon away. The
  frame count is arithmetic: 2.985 s of horizons.
- **A feed costs more, feeds happen less often.** The `audio` phase
  roughly doubles per feed — 0.005 ms typical at one sixtieth, 0.008–0.009
  at one thirtieth: twice the frames through the staging copy and into
  the device — while the number of feeds halves. Total audio work over
  the run is flat, which is the interesting part: 0.90 ms across 176
  feeds versus 0.91 ms across 89. Each feed costs roughly twice as much
  (the median goes 0.005 ms to 0.008 ms) and there are half as many of
  them, so the run pays the same for the same stream. The frames that
  fed nothing still read `audio 0.000 ms` — the gate is untouched, so a
  news-woken frame at either horizon queues nothing (the input run in
  the lesson's prose shows forty-three such frames).

And the wrap check: `TONE_FRAMES / CHUNK_FRAMES` is now 22050 / 1470 =
**15 exactly** — still a whole number of buffers per tone, so the cursor
still wraps at a buffer boundary and no feed splits across the tone's
end. The general rule falls out of the constants: with
`CHUNK_FRAMES = AUDIO_RATE / k` and `TONE_FRAMES = AUDIO_RATE / 2`, the
ratio is `k / 2` — a whole number whenever `k` is even, which is why 30
and 60 both work.

Pick an odd `k` and watch it break, which is the caution worth one
experiment: `AUDIO_RATE / 7` is 6300 frames, and 22050 / 6300 = 3.5. The
cursor's offsets stop landing on buffer boundaries — a feed would need a
split copy, part of the tone's tail and part of its head — and the
simple cursor arithmetic does worse than that: a start near the buffer's
end hands `SubmitSamples` a range that runs *past* the tone's last
frame. The modulus wraps the start of the copy, not the copy. The
divisibility comment is not decoration; it is the precondition for the
one-line feed.

Nothing here touches the seam, the gate, or the deadline bookkeeping —
which is the other lesson of the exercise: the horizon is one constant,
and the paced wait follows it with no other change.
