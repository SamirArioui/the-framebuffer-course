# Solution: exercise 2 — The part's acceptance table

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 069 — the closing demo](../../lessons/part-3/lesson-069-demo.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch makes the demo name what it uses — one line per capability
group, printed before the first frame:

```
engine: drawing: one blit for sprites, glyphs, and tiles; clipping and transparency included
engine: text: strings laid out one slot per character, missing glyphs skipped
engine: world: the map loaded whole, drawn through the camera's summed offset; the mover gated by collision
engine: input: polled movement, one space tap shakes the camera
engine: sound: samples loaded whole, channels with cursors and volumes mixed per frame into one stream
engine: measurement: one record per frame, every phase named
```

Those six lines are the table's left column. The rest is yours to fill —
and the shape that makes it worth keeping is this (the evidence column
is from this lesson's runs: the demo above, and one run against a
truncated `music.wav` and a deleted `sprite.ppm` in a scratch copy of
`assets/`, so the repo's files were never touched):

| Spec scenario | Lesson | Evidence from a run |
| ------------- | ------ | ------------------- |
| Sound loads as whole bytes into the arena | 059, 061-062 | `music: 132300 frames at 44100 Hz, 1 channel, peak 10442, first frames: 0 277 554 831 …, last frame 0` |
| The WAV container is parsed by hand | 061 | the facts line's count/rate/channels are the header's own claims, checked against the bytes |
| Missing or malformed file is a typed failure | 059, 061-062 | `assets/music.wav: could not load (malformed)` on a truncated file; `assets/sprite.ppm: could not load` on a missing one |
| Playback stops where the sample ends | 062 | the effect's 8820 frames play to their end and its channel comes back — `effect 4 -> channel 1` |
| One channel plays at its volume | 063 | `mix: effect 1 -> channel 1 (volume 64 of 256)` |
| The mix sums per frame and clamps | 064 | `first frames (music + effect 1, summed): 0 584 1163 1732 2286 2820 3330 3813` — reconcilable from the two facts lines to the digit |
| Channels are allocated by a fixed policy | 065 | the burst takes 1, 2, 3 and returns 1; the music's channel 0 is never offered |
| Music loops at its channel's cursor | 066 | `loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300` |
| Effects are one-shots on pooled channels | 067 | twenty-five effects, channels freed and retaken at every end |
| Music and effects share one mixer | 068 | three wraps and twenty-five effects in one run; the summed bytes contain both |
| The device's output lives behind the seam | 060 | `stream: 735-frame buffers, horizon 16.7 ms`; `check-boundary.sh` still names no OS in engine code |
| The paced wait keeps the device fed | 060 | one buffer per horizon; `58.3 frames/s while awake, sound fed at 58.3 buffers/s` |
| The mix is measured, not guessed | 060 | `audio 0.028 ms` in every `frame N:` line, on the platform clock |

The last question — which rows break first when the engine changes — is
where the table earns its keep:

- **The byte-exact rows break first under any mixer change.** Touch the
  sum's order, the volume's fixed-point truncation, or the clamp and
  the `first frames` line stops reconciling immediately. That is
  deliberate: those rows are cheap to re-run and precise about what
  broke.
- **The schedule rows break silently.** A pacing or wrap change rarely
  crashes anything — the run just quietly stops matching its own
  arithmetic. The wraps' `frames played` and the two rates exist
  because "sounds fine" is not a check.
- **The typed-failure rows are the cheapest to re-run and the easiest
  to forget.** They only fire on broken files — which is exactly when
  you need them.
- **The account rows drift rather than break.** Numbers move with the
  machine and the device; what must not change is the *shape* — a phase
  per frame, a row per phase, sums and shares from real frames.

One limit the table must keep saying out loud: every evidence line
above came from ALSA's `null` device. The bytes are verified; the
hearing is not, and the table is not the place to pretend otherwise.
