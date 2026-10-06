# Solution: exercise 1 — Quarter volume, to the digit

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 063 — one channel](../../lessons/part-3/lesson-063-channel.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is two things: the channel's starting volume at
`AUDIO_VOLUME_FULL / 4` — 64 of 256 — and the probe, one line per fill,
naming what the channel produced and whether it is still playing after
it.

The prediction, before any run. At volume 64 the scaling is
`(frame * 64) / 256` — the sample's frame divided by four, truncated
toward zero:

| sample frame | × 64 / 256 | emitted |
| --- | --- | --- |
| 0 | 0 | 0 |
| 513 | 128.25 | 128 |
| 1024 | 256 | 256 |
| 1531 | 382.75 | 382 |
| 2032 | 508 | 508 |
| 2525 | 631.25 | 631 |
| 3009 | 752.25 | 752 |
| 3480 | 870 | 870 |

Then the fills. `CHUNK_FRAMES` is 735, and 22050 / 735 = 30 exactly:

| fill | sample frames | channel after the fill | the buffer |
| --- | --- | --- | --- |
| 1–29 | 735 each | active | the sample's frames at volume 64 |
| 30 | 735 | active | the sample's last frames — the cursor reaches `frame_count`, and nothing has looked at it yet |
| 31 | 0 | inactive | all silence — the channel goes off at the fill's first frame |
| 32 … | 0 | inactive | silence, every buffer after |

Fill 31 is where the end is observed: its **first** frame is the first
output frame past the sample's end, and that frame writes silence and
takes the channel inactive — as does every frame after it in that
buffer. The end report lands with feed 31's submit and says the
sample's own numbers.

The run's log, from the probe:

```
engine: channel: playing 22050 frames at volume 64 of 256
engine: channel: first frames at that volume: 0 128 256 382 508 631 752 870
engine: fill: 735 sample frames, channel active
...
engine: fill: 735 sample frames, channel active
engine: fill: 0 sample frames, channel inactive
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
engine: fill: 0 sample frames, channel inactive
```

The first fill line repeats thirty times, exactly as predicted. Every
number reconciles: the emitted frames match the table row for row — 128
from 128.25, 382 from 382.75, the truncation twice over — the thirty
active fills are 30 × 735 = 22050 = `frame_count`, and the fill past
the end carries nothing but zeros. The easy prediction to lose is the
one in the table's third row: the fill that spends the sample's final
frame comes back with the channel *still active*. `cursor ==
frame_count` is not yet the end; the end is the first output frame that
finds it so.

Now the boundary question. A 1000-frame sample: fill 1 spends 735
frames and leaves the channel active; fill 2 spends 265 (frames
735–999) and at its 266th frame — the first output frame past the end —
the channel goes inactive, with the rest of that buffer silence. The
report then says `1000 frames fed in 2 buffers`. The end falls
mid-fill, and neither the fill, the probe, nor the report had to know
that in advance: `frame_count` says when, and the channel goes inactive
exactly there.
