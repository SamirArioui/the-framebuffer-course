# Solution: exercise 1 — The sample that is not thirty buffers

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 062 — the sample's playback facts](../../lessons/part-3/lesson-062-playback.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

First the file: `assets/tone.wav` cut to 44 + 2000 bytes — 1000 frames of
the same tone — with the `data` size field set to 2000 and the RIFF size
to 36 + 2000, so both of the container's claims follow the bytes and the
file is well-formed but short. (Those are lesson 061's byte edits: a
lying claim is a malformed file; an honest short file is just a short
sample.)

The prediction, before any run. `CHUNK_FRAMES` is 735 and the fill takes
`min(frame_count - cursor, CHUNK_FRAMES)`:

| feed | from the sample | of silence | where |
| --- | --- | --- | --- |
| 1 | 735 | 0 | frames 0–734 of the sample |
| 2 | 265 | 470 | frames 735–999, then zeros — the tail buffer |
| 3 … | 0 | 735 | silence, every buffer after |

Feed 2 is where `sample_cursor` reaches `frame_count`, so the end line
must say **1000 frames fed in 2 buffers** — the sample's frame count and
the number of buffers that carried any of it. The device's total is 1000
sample frames: exactly `frame_count`, with 470 frames of the second
buffer already silence.

The patch is the instrument: one line per feed, naming how many frames
came from the sample and how many from silence. Run against the short
file, the log reads:

```
engine: feed: 735 frames from the sample, 0 of silence
engine: feed: 265 frames from the sample, 470 of silence
engine: sample: 1000 frames fed in 2 buffers — the sample's end; the stream is silence from here
engine: feed: 0 frames from the sample, 735 of silence
engine: feed: 0 frames from the sample, 735 of silence
...
```

Every predicted number reconciles: 265 is 1000 − 735, 470 is 735 − 265,
the end line lands with feed 2's frame, and the 174 feeds that followed
in the three-second run are all pure silence. Nothing in the pacing moved
either — the gate still closed on the frames that woke early on window
news (they logged `audio 0.000 ms`), and the horizon is still 16.7 ms.

Now the question the tail buffer answers. A one-line feed —

```cpp
platform::SubmitSamples(audio.output, sample.frames + sample_cursor, CHUNK_FRAMES);
```

— hands the device 735 frames starting at frame 735, and only 265 of them
are the sample's. The other 470 are whatever the arena holds past the
sample's frames — another asset's bytes, the previous contents of the
reservation — **played as sound**, and the run would then call that
"the sample." The channel would play more than the sample's frame count
and misread memory to do it. The tail fill is what makes the spec's
sentence true for every file and not just for lengths that happen to
divide: 22050 / 735 = 30 exactly is *this asset's* arithmetic, and
1000's is 735 + 265 + silence. `frame_count` says when; the fill spends
exactly that many frames and not one more.
