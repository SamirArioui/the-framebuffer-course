# Solution: exercise 1 — One sample, two volumes

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 067 — effects as one-shots](../../lessons/part-3/lesson-067-effects.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is one probe: `assets/effect.wav` fired twice at the same
instant through the engine's own `MixerPlayEffect` — at 192 and at 48
of 256 — on a scratch mixer, with the mixed buffer read back frame by
frame and the channels' end reported from the channels themselves.

The prediction, before any run. The two presses walk the same pool the
lesson's do, and the pool is fresh: the first takes channel 1 and the
second channel 2. Each channel emits its own contribution of each
frame — the sample's frame times its volume over `AUDIO_VOLUME_FULL`,
truncated toward zero **before** the sum, lesson 064's order — and the
mix adds the two. With the sample's first frames `0 1229 2438 3607
4719 5757 6703 7544`, the contributions and their sums are:

| frame | at 192 of 256 | at 48 of 256 | mixed |
| --- | --- | --- | --- |
| 0 | 0 | 0 | 0 |
| 1229 | 921 | 230 | 1151 |
| 2438 | 1828 | 457 | 2285 |
| 3607 | 2705 | 676 | 3381 |
| 4719 | 3539 | 884 | 4423 |
| 5757 | 4317 | 1079 | 5396 |
| 6703 | 5027 | 1256 | 6283 |
| 7544 | 5658 | 1414 | 7072 |

Read one row twice. Frame 1229 at 192 is 921.75 and at 48 is 230.4375
— and the channel truncates both before adding, so the mix says 1151,
not 1152. The order of truncation and summing is visible in a single
frame's three numbers.

The rest of the prediction is the one-shot contract itself. The effect
is 8820 frames and the buffers are 735 — twelve buffers exactly — so
after thirteen buffers of the pair, both channels have played to their
end, gone inactive, and returned to the pool; the thirteenth buffer
holds silence, because silence is what idle channels leave behind.

The run's log:

```
engine: volumes: channel 1 at 192 of 256, channel 2 at 48 of 256
engine: volumes: first frames (summed): 0 1151 2285 3381 4423 5396 6283 7072
engine: volumes: buffer 13 is silence, and channels 1 and 2 are free again
```

Every frame in the middle line is the table's right-hand column, to the
digit. And the two volumes are what make these **different sounds**:
the same sample, the same length, the same start — and the mix holds
two different streams of numbers because each effect carries its own
volume into its channel. That is why `MixerPlayEffect` takes a volume
at all: not for balance at the mixer, but because the sound is the
sample *and* its volume.

One thing the probe cannot show is the difference a volume makes to
an ear. The bytes prove the streams differ; that one is three-quarters
scale and the other less than a fifth of it *sounds* like two things is
yours to check on hardware that makes sound.
