# Solution: exercise 1 — Five channels at the peak

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 064 — the mix](../../lessons/part-3/lesson-064-mix.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is two things: the run's channels — five on the tone at
`AUDIO_VOLUME_FULL`, instead of two at 128 and 64 — and the probe: the
whole sample through the engine's own `MixBuffer` on a scratch mixer,
frame by frame, every frame the clamp changes named (the first few) and
counted (all of them).

The prediction, before any run. At full volume a channel's contribution
is exact — `(frame * 256) / 256` is the frame itself — so the sum is
**five times the sample's frame**, and the clamp's threshold falls out
of the format's limit: 5v > 32767 clamps positive at v ≥ 6554, 5v <
−32768 clamps negative at v ≤ −6554. Anything at 6553 or less in
magnitude passes untouched.

| prediction | number |
| --- | --- |
| first eight mixed frames | 0 2565 5120 7655 10160 12625 15045 17400 |
| does the clamp touch them? | no — the largest, 17400, is far under 32767 |
| the tone's peak (frame 25, sample 8191) | sum 40955 → 32767; wrapped −24581 |
| the tone's trough (frame 75, sample −8191) | sum −40955 → −32768; wrapped 24581 |
| largest sum passing untouched | 32735 — the tone's frame 6547 |
| first frame the clamp changes | frame 15 (sample 6616, sum 33080) |
| frames changed in all | 9040 of 22050 |

The threshold is 6553, but the tone never lands on it: 5 × 6553 is
32765, the largest sum that *could* pass, and this tone's largest
passing frame is 6547 — the sine jumps from there straight to 6554,
which clamps. The count needs the same walk over the sample's data:
every frame at 6554 or more in magnitude clamps, and a half-second of
440 Hz has a lot of them.

The run's log:

```
engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: mix: five channels at volume 256 of 256
engine: mix: first frames (summed): 0 2565 5120 7655 10160 12625 15045 17400
engine: clamp: frame 15: sum 33080 -> out 32767, wrapped -32456
engine: clamp: frame 16: sum 34530 -> out 32767, wrapped -31006
engine: clamp: frame 17: sum 35840 -> out 32767, wrapped -29696
engine: clamp: frame 18: sum 37015 -> out 32767, wrapped -28521
engine: clamp: frame 19: sum 38040 -> out 32767, wrapped -27496
engine: clamp: frame 20: sum 38915 -> out 32767, wrapped -26621
engine: clamp: 9040 of 22050 frames clamped
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
```

Every number reconciles. The first eight are five times the sample's
frames, digit for digit — 513 × 5 = 2565, 3480 × 5 = 17400 — and none
of them clamps. Frame 15 is the first frame of the tone at 6554 or
more (its 6616), and the probe's six lines are the sine's climb toward
its peak at frame 25 — 8191 × 5 = 40955 — every one landing on 32767.
The count says how loud this mix is: 9040 of 22050 frames, 41% of the
sample, exceed the format and sit on a limit. And the end report is
untouched by all of it — `22050 frames fed in 30 buffers` — because it
counts channel 0's cursor and the five channels spend the same sample
at the same pace.

The wrapped column is the artifact the clamp replaces. At frame 15 a
wrapping accumulator would put −32456 in the stream — the sound's
ascent to its peak turns into a deep hole the moment it gets too loud —
and at the peak itself −24581, at the trough +24581. The loud passages
are exactly where wrapping distorts worst, and the clamp answers all of
them the same way: land on the limit.

Now the boundary. A sample at amplitude 1.0 — its peak 32767, the
format's own edge — carries **one** channel at full volume before the
clamp engages: one channel emits 32767 exactly and passes untouched,
two sum to 65534 and land on 32767. Lesson 063's claim is tight, then:
a channel alone never leaves the format, and full scale is exactly one
channel's room. The mix is where the room runs out.
