# Solution: exercise 2 — Starvation on real speakers

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 060 — the stream's shape](../../lessons/part-3/lesson-060-stream.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch adds the instrument the exercise asks for: a probe in
`src/platform_alsa.cpp` — the file allowed to know the device — that
times every `snd_pcm_writei` and accumulates what the device made the
loop wait. Three statics beside the staging buffer (no allocation; the
language law of lesson 026 binds this layer too), a clock read around
each write, and one report at `CloseAudioOutput`, after the drain:

```
platform: audio: 176 writes, 0.360 ms waiting on the device (worst 0.012 ms)
```

That is a real three-second idle run of the lesson's end state plus this
patch, against ALSA's `null` device — and it is the control the port is
measured against. Read it carefully before leaving this machine:

- **176 writes is the feed count**, one per horizon — the probe
  cross-checks the paced wait's own account of the run. If this number
  and the frame log's feeding frames ever disagree, one of them is
  lying.
- **0.360 ms total, 0.012 ms worst — the device never pushed back.**
  `null` accepts and discards; what little is there is the staging copy
  inside the call, not a device asking the loop to wait. A queue that
  never fills cannot starve and cannot apply back-pressure. This is
  exactly the gap the lesson's honesty section names.

Now the port, and what the probe should show there:

- **The device.** `libasound2-dev` installed, `./build.sh`, then run
  with `ALSA_DEVICE` unset (`default`) or pointed at real hardware —
  `aplay -l` lists what exists. Run it once idle and once busy: the
  sprite driven into a wall (a held arrow key), the camera shaking
  (space), for a fixed interval each.
- **Back-pressure is the number to compare.** On hardware, writes wait
  when the device's buffer is full: expect the total and worst waits to
  be *non-zero*, and the busy run's waits to grow against the idle
  run's. The worst single wait is the most telling figure — when the
  loop is late feeding, the write blocks for the room it needs, and a
  wait approaching one horizon (16.7 ms at the lesson's buffer size) is
  the device telling you the queue ran nearly dry. `null` shows none of
  this; hardware shows all of it.
- **The ears are the other instrument.** The tone is a continuous
  half-second buffer looped forever at 440 Hz; a fed stream is a steady
  note, and starvation is unmistakable — stutter, gaps, or the note
  dropping out under load and returning when the run calms. The probe
  records the cause in milliseconds; the stutter is the same fact at a
  human scale. Report both, and where in the deadline bookkeeping it
  showed: a stutter with low waits means the *loop woke late* (check the
  frame log's gaps against the horizon); a stutter with high worst waits
  means the *device held the loop* — the back-pressure the paced wait
  exists to respect.
- **The failure paths still print as they are.** A missing device is
  lesson 059's typed failure and the run continues without sound; a
  device that opens but refuses the samples is named once and the run
  drops to silence, its wait unbounded again. Neither is a crash.

One note on the environment override, since a port meets it:
`ALSA_DEVICE` is read inside the ALSA file and is not part of the seam's
contract — a second OS's file picks its device its own way, and a second
OS's `SubmitSamples` is where its own probe would live. The engine
behaves identically whatever is opened; only the numbers differ.

Then report what you found: the device, idle against busy, the waits,
what you heard, and whether the deadline bookkeeping predicted it. The
`null` run's `0.360 ms` is the floor; everything above it is your
machine teaching you what "the device consumes at its own rate" really
means.
