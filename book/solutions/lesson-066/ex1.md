# Solution: exercise 1 — The seam, predicted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 066 — music as a loop](../../lessons/part-3/lesson-066-music.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is one probe: a scratch mixer playing the music alone, fed
buffer by buffer through the engine's own `MixBuffer`, with the music
channel's cursor and the wrap count printed around the seam and the
frames the mix holds at it. The probe mixes the same buffers the run
mixes — one `MixBuffer` call per buffer — and simply stops counting
once past the seam.

The prediction, before any run. The buffers are 735 frames and the
loop is 132300 — exactly 180 buffers of 735, which is why the seam
lands on a buffer boundary rather than inside one. After `b` buffers
the cursor reads `b * 735` until that reaches `frame_count`: after
buffer 179 the cursor sits on 132300, the sample's end. The wrap is
**lazy** — it happens at the first pull past the end, not at the end
itself — so buffer 180 is where it happens: its first pull returns the
cursor to 0 and takes frame 0, and 735 pulls later the buffer leaves
the cursor at 735. That is the backwards step the run watches for, and
the wrap count goes to 1 there.

The frames at the seam follow from the same arithmetic, and the
channel plays alone at full volume, so the mix **is** the sample's own
frames. Buffer 179 holds the sample's frames 131565 to 132299 — its
last four are the file's last four — and buffer 180 holds frames 0 to
734. The file's first frames are its own facts, in the run's line:
`0 277 554 831 …`, and its last frame is `0`. The seam is therefore
`… -1 -1 0 0 | 0 277 …` — the last frame meets the first as 0 meets 0,
the one join in the file where nothing steps at all.

The run's log:

```
engine: seam: buffer 179: cursor 131565 -> 132300, wraps 0
engine: seam: buffer 179's last 4 frames: -1 -1 0 0
engine: seam: buffer 180: cursor 132300 -> 735, wraps 1
engine: seam: buffer 180's first 4 frames: 0 277 554 831
engine: seam: the sample's last 4 frames: -1 -1 0 0, and its first 4: 0 277 554 831
```

Every line reconciles with the cursor's arithmetic, and the last line
is the proof the mix needed no special seam handling: buffer 179's
last four frames are byte-identical to the sample's last four, and
buffer 180's first four to its first four. The wrap is one line of
cursor arithmetic inside the pull; nothing outside the channel knows
the boundary existed.

Two details earn their keep here.

**The wrap is lazy, and the cursor can sit at the end.** Between the
last pull of buffer 179 and the first pull of buffer 180 the cursor
equals `frame_count` — the same state a non-looping channel ends in.
The difference is only what the *next* pull does with it: end, or wrap.
That is the whole lesson in one cursor value.

**"Frames played" is the wraps and the cursor added.** The wrap
counted in the probe says how many full passes the loop has made, and
the cursor says how far into the current one it is: `wraps *
frame_count + cursor`. The lesson's own wrap line — `wrap 1, 133035
frames played, cursor 735` — is exactly `1 * 132300 + 735`, and the
buffer count agrees: 133035 is 181 buffers of 735.

What the probe cannot say is what the seam sounds like. It shows the
bytes the seam meets — `0` to `0` — and those are checkable; the
hearing of a loop's join is yours, on a machine that makes sound.
