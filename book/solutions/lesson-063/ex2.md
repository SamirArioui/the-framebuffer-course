# Solution: exercise 2 — The fade to silence

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 063 — one channel](../../lessons/part-3/lesson-063-channel.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is two things: the channel started at full volume
(`AUDIO_VOLUME_FULL`), and the fade — after every fill the volume drops
by 16 of 256, never below zero, with the run naming each new volume.

The prediction, before any run. The drop happens after the fill, so
fill n's buffer is at volume 256 − 16(n−1):

| fill | volume | the buffer |
| --- | --- | --- |
| 1 | 256 | the sample at full scale |
| 2 | 240 | each frame × 240 / 256, truncated |
| … | … | sixteen steps down |
| 16 | 16 | a whisper — each frame 1/16 of the sample's |
| 17–30 | 0 | silence — the channel still playing |
| 31 | 0 | silence, and the channel inactive from the fill's first frame |

So the sound runs out with fill 16 and the *playback* runs out at fill
30's last frame: fills 17 through 30 are pure silence with the cursor
still advancing — volume takes no part in the fill's condition, so the
channel keeps spending the sample at full speed while emitting zeros.
The end report is unchanged: `22050 frames fed in 30 buffers`, because
what it counts is the cursor's spending, not what a listener hears.

The run's log, from the fade's own lines:

```
engine: channel: playing 22050 frames at volume 256 of 256
engine: fade: volume 240 of 256
engine: fade: volume 224 of 256
...
engine: fade: volume 16 of 256
engine: fade: volume 0 of 256
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
```

Sixteen fade lines, 240 down to 0, and the seventeenth drop never
comes. Each line names the volume the *next* fill's buffer is at, so
the last line above follows the fill that ran at 16 of 256. The three
numbers that reconcile everything: 16 fills carried sound (11760
frames), 14 fills were silence from a playing channel (10290 frames),
and 11760 + 10290 = 22050 = `frame_count`. The end arrived where it
always does — fill 30 spends the sample's last frame, fill 31 notices
— and the fade never touched it.

Which answers the counting question. Once the frames it feeds are
silence, `frames fed` counts **sample frames consumed** — 22050 of
them, every one spent by the cursor, only the first 11760 of them
audible. The count does not move with the volume, and that is the proof
that it is the sample's frames being counted and not the sound.

The distinction the exercise is really about: `active` is a playback
position, not a loudness. A channel at volume 0 is silent and still
playing — its cursor walks the sample and its end arrives on schedule.
Only `frame_count` ends a channel, and only an ended channel is free
again — the fact lesson 065's pool keys on.

One honesty note, kept where this course keeps it: no speaker on this
machine has made a sound. The fade's sixteen steps over 16 fills — 267
ms at the horizon's 16.7 — are checked in the frames and the log; what
a tone sliding from full scale to silence over a quarter second sounds
like is a question for real hardware.
