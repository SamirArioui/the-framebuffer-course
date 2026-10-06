# Solution: exercise 2 — Hear it stop

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 062 — the sample's playback facts](../../lessons/part-3/lesson-062-playback.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch is the log's evidence: the stream's account at the run's close,
printed beside the frame-budget table. The counters ride with the feed —
buffers that carried sample frames, buffers that carried none — and the
end's timestamp is taken where the end is named. Two lines, both from a
real three-second run against ALSA's `null` device:

```
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
engine: sample: 22050 frames in 30 buffers, then 145 buffers of silence over 2.5 s
```

Read them as the port's baseline. The first says the channel played
exactly the sample's frame count — 30 buffers × 735 frames = 22050 =
`frame_count`, no frame missing and none invented. The second says the
run did **not** stop when the sound did: 145 silence buffers over 2.5
seconds, one every horizon, until the close. The device kept getting its
buffers; silence is a stream. (The `else` branch of the account matters
too: close the window early and it reports the run ending before the
sample did — a partial playback, honestly counted.)

Now the port, and the question `null` cannot answer. Take the demo to a
machine with real sound hardware — `libasound2-dev` installed,
`./build.sh`, then run with `ALSA_DEVICE` unset (`default`) or pointed at
what `aplay -l` lists. What the numbers predict before you listen:

- **The duration: exactly half a second.** 22050 frames at 44100 Hz is
  0.500 s, and thirty horizons of 16.7 ms are the same half second in
  buffers. If what you hear is materially longer or shorter, the device's
  own rate is not 44100 and the log's frame times will show where the
  difference entered.
- **The pitch: 440 Hz, A4**, at amplitude 0.25 — a quiet tone, a quarter
  of the format's range, so expect a soft note rather than a beep.
- **The stop — the actual subject.** The sample does not end at rest: its
  last four frames are −2032, −1531, −1024, −513, where the wrap (lesson
  059's fact that made lesson 060's looping seamless) would have been 0.
  Silence begins at 0, so the speaker steps from −513 to rest — about
  1.6% of full scale, a small step and not a fall. Listen at the stop:
  is it clean, or is there a faint tick? Report what you hear. A tick
  would not be a bug in the loader or the feed; it would be this sample's
  own ending, and it is exactly the kind of question that only ears on
  hardware can close.

And the honesty, kept exactly where this course keeps it: no speaker on
this machine has made a sound — `null` takes samples and discards them,
and every number above is the engine's side of the schedule. The account
the patch prints is device-independent bookkeeping; what the device did
with those buffers is your machine's story to tell. If you also want the
device's side in numbers, lesson 060's exercise 2 probe measures the
waits `snd_pcm_writei` imposes — same run, both instruments.

The failure paths are unchanged and worth one run each on the port:
point `ALSA_DEVICE` at a name that does not exist and the run reports the
typed failure and continues without sound; a device that opens but
refuses the samples is named once and the run drops to silence, its wait
unbounded again. Neither is a crash — and the sample account at the close
will show zero buffers fed, which is what "no sound" looks like in the
log.
