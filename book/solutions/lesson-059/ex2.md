# Solution: exercise 2 — Hear it on real speakers

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 059 — sound as samples](../../lessons/part-3/lesson-059-samples.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch adds one report to the success path of `OpenAudioOutput`, in
`src/platform_alsa.cpp` — the file allowed to know the device's name.
The name is OS business: it is printed where it is known and never
crosses the seam, so `platform.h` stays as it is and lesson 042's
boundary check keeps printing its account unchanged. On the authoring
machine the run now says what it opened:

```
platform: audio output: null (44100 Hz, 2 device ch)
engine: tone: 22050 frames at 44100 Hz, first frames: 0 513 1024 1531
engine: tone played
```

That is the whole verification this machine can do — `null` accepts and
discards the frames through the same open/submit/close calls a real
device gets. The rest of the exercise happens on your hardware:

- **The build.** `libasound2-dev` installed (the README prerequisite),
  then `./build.sh`. The link line wants `-lasound`; nothing else about
  the build changes.
- **The device.** With `ALSA_DEVICE` unset, `DeviceName` answers
  `default` — whatever your machine means by that. `aplay -l` lists what
  exists; `ALSA_DEVICE=<name> ./build/game` picks one on purpose (a USB
  headset, say), and the new report line records the choice in the log.
- **The listening.** The tone is half a second of A4 — 440 Hz at
  amplitude 0.25 — submitted at startup, before the frame loop, and
  `CloseAudioOutput` drains before closing, so every submitted frame is
  played even if the window closes immediately after. Expect one quiet,
  steady, clearly pitched note lasting about half a second.
- **The failure paths, as they print.** No usable output is the typed
  failure — ALSA's own line, then `engine: no audio output on this
  machine` / `engine: continuing without sound` — and the run continues
  to a clean close. A device that opens but would not take the frames
  prints `engine: the output would not take the samples`. Neither is a
  crash; both are named conditions.
- **The volume subtlety.** The run sets no mixer volume: amplitude 0.25
  is a quarter of the *format's* range, not a quarter of your speakers'.
  `alsamixer` is where a mysteriously quiet machine gets loud.

Then report what you heard — the device line, the duration, the pitch,
the volume — and anything that differs from what the lesson predicts. A
steady A4 about half a second long is the expected result; if what you
heard is not that (wrong speed, click at the start or end, silence
despite `tone played`), the discrepancy is more interesting than a pass.
The authoring checks end where the `null` device ends; this exercise is
where "does it actually sound right?" finally gets a witness.

One note on the environment override, since a port will meet it:
`ALSA_DEVICE` is read inside the ALSA file and is not part of the seam's
contract — a second OS's file picks its device its own way. The engine
behaves identically whatever is opened; only the report line differs.
