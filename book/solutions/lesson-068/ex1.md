# Solution: exercise 1 — The audio phase, measured

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 068 — music and effects together](../../lessons/part-3/lesson-068-together.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is a measurement, not a feature: two scratch mixers — one
idle, one as busy as the pool allows (the music looping on its channel
and every effect channel playing) — each mixed a thousand times over
through the engine's own `MixBuffer`, timed on the platform clock, the
same clock the frame record uses. The busy mixer's channels all loop so
that no measurement accidentally includes a channel ending: the timing
measures the mix, nothing else. What the channels hold cannot matter to
the arithmetic — one pull per channel per output frame, whatever the
pull returns — and the numbers below are exactly where that claim gets
checked.

The run's log:

```
engine: cost: idle mixer, 1000 buffers of 735 frames x 16 channels: 20.558 ms total, 20.6 us per buffer
engine: cost: busiest mix, 1000 buffers of 735 frames x 16 channels: 37.143 ms total, 37.1 us per buffer, 3 ns per channel frame
```

Reconcile the busy line by hand. One buffer is 735 output frames, each
one pulling from all `AUDIO_MIXER_CHANNELS` = 16 channels — 11 760
channel frames per buffer. 37.1 µs per buffer over 11 760 channel
frames is about 3.1 ns each, the printed 3. Per *output* frame the cost
is 37.1 µs / 735 ≈ 50 ns — sixteen pulls, one clamp, one `short`
written.

The idle line is the finding. An idle mixer is cheaper but not free:
the walk still runs, 16 channels × 735 frames, and every channel
answers zero — 20.6 µs per buffer, about 28 ns per output frame. The
mix pays for its **channel count**, not for its loudness. That is the
shape of the fixed pool's cost: decided up front, paid every buffer,
tiny.

Now the frame record, which has carried the `audio` phase since lesson
060. From this lesson's run — music looping, effects firing, one
buffer fed per frame — the log's own lines are:

```
frame 1: update 0.001 ms, audio 0.033 ms, render 1.632 ms (sprites 0.001, text 0.006, tilemap 0.936), present 0.461 ms, total 2.127 ms
frame 2: update 0.001 ms, audio 0.034 ms, render 1.368 ms (sprites 0.001, text 0.008, tilemap 0.945), present 0.598 ms, total 2.001 ms
frame 4: update 0.001 ms, audio 0.072 ms, render 1.742 ms (sprites 0.001, text 0.007, tilemap 0.970), present 0.768 ms, total 2.583 ms
```

Over the run's 702 frames the phase averages 0.036 ms, floors at 0.025
ms and worst-cases at 0.331 ms. Two facts to report with those. First,
in this run **every** frame fed a buffer — the paced wait wakes the
loop at the buffer horizon, so a frame that fed nothing did not occur;
a frame that does no audio work would show the phase's own floor,
essentially the two clock readings around an `if`. Second, the phase
wraps the mix *and* its submit, and on the `null` device the submit
returns immediately — so the phase reads as the mix's cost, and 0.036
ms is the probe's 37.1 µs seen from inside the frame.

The budget's question answers itself from there: 0.036 ms of the 2.082
ms average frame is about 1.7% — the mix is invisible in the budget at
60 frames a second. To make it visible you would need many more
channels (the cost is linear in the count) or a machine roughly two
orders of magnitude slower. On real hardware one more term joins the
phase: a submit can wait for room in the device's buffer, exactly as
`present` includes the copy's sync — that part is not this machine's to
measure.

What this measurement cannot tell you is what the mixed bytes sound
like. Timings are exact here; the hearing is yours.
