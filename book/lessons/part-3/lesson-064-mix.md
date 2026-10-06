# Lesson 064 — the mix

{{#include ../../stability-horizon.md}}

## Prose

Lesson 063 gave the engine one channel: what is playing, where it is,
how loud. One channel fills a buffer and the buffer is the stream. This
lesson adds the many, and the idea is one sentence: **many channels,
one stream** — every output frame is the sum of every active channel's
next frame at its volume, clamped to the format's range instead of
wrapped around it. The mixer batch's middle lesson is the sum itself;
lesson 065 hands the channels out.

### The mix, per output frame

`MixBuffer` in `src/audio.cpp` is two loops. The outer one walks the
output frames; inside it, a 32-bit accumulator — `int sum` — collects
every active channel's next frame at its volume, one pull per channel.
Then the clamp, and the frame is written as a `short`. That is the
whole mix: pull, add, clamp, write — `frame_count` times.

The run makes the sum checkable by hand. Two channels play the same
tone — lesson 059's tone, from `assets/tone.wav` — at volumes 128 and
64 of 256, and the startup prints the first frames of the sample beside
the first frames of the mix:

```
engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: mix: channel 0 at volume 128, channel 1 at volume 64 of 256
engine: mix: first frames (summed): 0 384 768 1147 1524 1893 2256 2610
```

Every mixed frame is two contributions added: 513 becomes 256 at volume
128 and 128 at volume 64 — 384. 2525 becomes 1262 and 631 — 1893. Look
at 1531 once more: its contributions are 765 and 382, and the mix says
**1147**. Scaling the frame once at 192 of 256 would have printed 1148.
The channels truncate **before** the sum, each contribution on its
own — the mix adds what the channels emitted, not what they might have
emitted in one step, and the run's number says which order it used.

### Clamped, never wrapped

A sum of loud channels leaves the format. Two channels at full scale,
each emitting 20000, produce 40000 between them, and `short` has no
room for it. What happens next is the lesson's most important
distinction. A **wrap** truncates the sum to 16 bits and lets it re-
enter from the other side: 40000 becomes −25536, the loudest peak of
the sound turns into a deep hole — the worst artifact in audio. A
**clamp** lands on the limit: 40000 becomes 32767, the loudest the
format can say. Clamped, never wrapped — that is the spec's own
scenario for the mix. The clamp is two limits, in the same numbers the
samples use: `SAMPLE_LIMIT_HI` at 32767, `SAMPLE_LIMIT_LO` at −32768.

The clamp was checked through the engine's own `MixBuffer` — a scratch
probe driving a real `Mixer`, `ChannelPlay`, and `MixBuffer`, not part
of the repo — summing known frames and reading the output back:

```
one channel at the tone's peak             sum   8191 -> out   8191
four channels at the peak                  sum  32764 -> out  32764
five channels at the peak (over)           sum  40955 -> out  32767
five channels at the trough                sum -40955 -> out -32768
three at full scale positive               sum  98301 -> out  32767
15 idle channels change nothing            byte-identical
```

The tone's peak is 8191 because `assets/tone.wav` is lesson 059's tone
at amplitude 0.25. Four channels at that peak land on 32764 — under the
limit by three counts, still the true sum. The fifth is what forces the
clamp. And the trough clamps to −32768, not to some positive: the clamp
holds each side of the range on its own side.

### Why the sum fits in 32 bits

The clamp can only do its job if it sees the true sum, so the
accumulator is 32 bits wide on purpose. Sixteen channels of 16-bit
samples peak at 16 × 32768 = 524288 — far inside `int`, whatever the
volumes make of them. No intermediate can wrap on the way to the clamp;
the clamp is the only thing between the sum and the format, and the
cast to `short` after it is safe exactly because the clamp ran first.

This is also where lesson 063's last section pays off. One channel
alone cannot overflow the format — the volume never exceeds
`AUDIO_VOLUME_FULL`, so a channel emits at most the sample it plays.
Only summing can push a frame out of range. That is why this lesson has
the clamp and lesson 063 did not: clamping is not a channel's problem,
it is the mix's.

### Silence contributes nothing

Most channels are silent most of the time — not yet started, or played
to their end. An idle channel contributes **nothing at all**: its pull
adds nothing to the accumulator, and the mix writes the stream exactly
once per output frame, from the sum. That is different from writing
zeroes. Lesson 063's fill wrote `0` into every frame of its buffer past
the sample's end; here nothing is ever written on a channel's behalf —
silence is the absence of a contribution, not a value. The check is
behavioral and exact: fifteen idle channels beside one playing channel
produce a byte-identical stream.

### The fill became a pull

Say this one out loud, because it is the lesson's real move. Lesson
063's `ChannelFill` filled one channel's whole buffer — the channel
wrote its output, frame by frame, into `out`. A mix cannot work that
way: summing per output frame needs each channel's **one** next frame,
all at the same instant. So `ChannelFill` became `ChannelFrame` — now
private to `src/audio.cpp` — and the direction of the call flipped. A
channel no longer fills a buffer; the mix **pulls** one frame from each
channel and adds them up. The struct is untouched; the shape of what it
does with it is not — the same habit as the earlier refactors: the unit
did not change, the code caught up with what it had to become.

### Sixteen channels, decided up front

`Mixer` is sixteen copies of lesson 063's `Channel` in one array —
`AUDIO_MIXER_CHANNELS = 16`, a plain struct of plain values. The count
is fixed at compile time and `MixerInit` walks it once, every channel
idle and free. Nothing is allocated while sound plays: no channel is
created when a sound starts, none is destroyed when one ends. This is
the arena habit applied to the mix — **capacity is a decision, not an
event** — and the number sixteen is exactly that decision, made up
front. How a busy run hands the sixteen out is lesson 065's; here they
are only a pool, and the mix does not care which of them are playing.

### What this run verified, and what it did not

All numbers from real runs of this lesson's end state on this machine,
against ALSA's `null` device:

- **The mix sums byte-exact.** The two channels' frames — 384, 768,
  1147, 1524, 1893, 2256, 2610 — are each contribution truncated and
  then added, digit for digit, beside the sample's own frames.
- **The clamp lands on the limit, and idle channels change nothing.**
  The probe above drove the engine's own `MixBuffer` at deliberate
  overload: 40955 becomes 32767, −40955 becomes −32768, and the sums
  that fit — 8191 and 32764 — pass untouched. Fifteen silent channels
  beside one playing one: byte-identical.
- **The sample still ends where it always did.** The end line is
  lesson 062's, unchanged by the mix:

  ```
  engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
  ```

- **The build carries exactly one warning:** `DrawScene` defined but
  not used in `src/main.cpp` — the Part 2 wart carried forward on
  purpose — and this lesson adds none. `tools/check-boundary.sh` still
  passes: the mix is engine code and makes no OS call.

What this run cannot verify is the part that needs ears: no speaker has
made a sound — `null` takes the samples and discards them. "Two sounds
at once" is a claim about your machine and your ears. What this lesson
claims is that the frames were summed and clamped, and the run prints
them.

## Code step

One change for this lesson, three files: `src/audio.h` and
`src/audio.cpp` grow the mixer — `Mixer`, `MixerInit`, `MixBuffer`, and
the `AUDIO_MIXER_CHANNELS` pool — and lesson 063's `ChannelFill`
becomes `ChannelFrame`, the private per-frame pull the mix sums.
`src/main.cpp` plays the tone on two channels at volumes 128 and 64 and
prints the sum by hand, the same arithmetic the mix performs. The
horizon, the paced wait, and the feed's gate are lesson 060's,
untouched. Its end state is tagged `lesson-064`.

```diff
diff --git a/src/audio.cpp b/src/audio.cpp
index eaf730b..58c3af8 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -195,25 +195,62 @@ void ChannelPlay(Channel &channel, const Sample &sample, int volume)
     channel.active = true;
 }
 
-void ChannelFill(Channel &channel, short *out, int frame_count)
+namespace {
+
+/* One channel's next output frame at its volume — and the cursor moves.
+   An idle channel, or one whose sample has ended, contributes nothing:
+   silence is not a value here, it is the absence of a contribution. */
+int ChannelFrame(Channel &channel)
+{
+    if (channel.active && channel.sample &&
+        channel.cursor < channel.sample->frame_count) {
+        /* One sample frame, scaled to the channel's volume. A frame is
+           sample.channels values wide; the engine's stream is one
+           channel wide, so it takes the frame's first value. */
+        int frame = channel.sample->frames[channel.cursor *
+                                           channel.sample->channels];
+        channel.cursor += 1;
+        return (frame * channel.volume) / AUDIO_VOLUME_FULL;
+    }
+
+    /* The sample's end — the fact frame_count carries. */
+    channel.active = false;
+    return 0;
+}
+
+/* The format's range, in the same numbers the samples use. */
+constexpr int SAMPLE_LIMIT_HI = 32767;
+constexpr int SAMPLE_LIMIT_LO = -32768;
+
+} /* namespace */
+
+void MixerInit(Mixer &mixer)
+{
+    for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c) {
+        mixer.channels[c].sample = 0;
+        mixer.channels[c].cursor = 0;
+        mixer.channels[c].volume = 0;
+        mixer.channels[c].active = false;
+    }
+}
+
+void MixBuffer(Mixer &mixer, short *out, int frame_count)
 {
     for (int i = 0; i < frame_count; ++i) {
-        if (channel.active && channel.sample &&
-            channel.cursor < channel.sample->frame_count) {
-            /* One sample frame, scaled to the channel's volume. A frame
-               is sample.channels values wide; the engine's stream is one
-               channel wide, so it takes the frame's first value. */
-            int frame =
-                channel.sample->frames[channel.cursor *
-                                       channel.sample->channels];
-            out[i] = (short)((frame * channel.volume) / AUDIO_VOLUME_FULL);
-            channel.cursor += 1;
-        } else {
-            /* The sample's end — the fact frame_count carries. Silence
-               from here, and the channel is free again. */
-            channel.active = false;
-            out[i] = 0;
-        }
+        /* The 32-bit accumulator for this output frame: sixteen channels
+           of 16-bit samples cannot overflow it, so the clamp below sees
+           the true sum and not a wrapped one. */
+        int sum = 0;
+        for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
+            sum += ChannelFrame(mixer.channels[c]);
+
+        /* Clamped, never wrapped: a sum past the range lands on the
+           limit rather than jumping to the opposite extreme. */
+        if (sum > SAMPLE_LIMIT_HI)
+            sum = SAMPLE_LIMIT_HI;
+        if (sum < SAMPLE_LIMIT_LO)
+            sum = SAMPLE_LIMIT_LO;
+        out[i] = (short)sum;
     }
 }
 
diff --git a/src/audio.h b/src/audio.h
index aed24e6..2b33c0f 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -86,11 +86,24 @@ struct Channel {
    frame. Playing on one channel leaves every other channel alone. */
 void ChannelPlay(Channel &channel, const Sample &sample, int volume);
 
-/* Fills `out` with `frame_count` frames of this channel's output: the
-   sample's frames at the channel's volume, one for one, advancing the
-   cursor. Past the sample's end the channel writes silence and is
-   inactive again — frame_count is the fact that says when. */
-void ChannelFill(Channel &channel, short *out, int frame_count);
+/* The mixer's fixed set of channels. */
+constexpr int AUDIO_MIXER_CHANNELS = 16;
+
+/* The mixer: one fixed pool of channels, decided up front — nothing is
+   allocated while sound plays. */
+struct Mixer {
+    Channel channels[AUDIO_MIXER_CHANNELS];
+};
+
+/* Every channel idle and free. */
+void MixerInit(Mixer &mixer);
+
+/* The mix: `frame_count` frames of stream, each one the sum of every
+   active channel's next frame at its volume, clamped to the format's
+   range. Clamped, never wrapped — a sum past the range lands on the
+   limit instead of jumping to the opposite extreme. Idle and ended
+   channels contribute nothing at all. */
+void MixBuffer(Mixer &mixer, short *out, int frame_count);
 
 } /* namespace engine */
 
diff --git a/src/main.cpp b/src/main.cpp
index 732ac22..25ce44c 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -162,20 +162,23 @@ int Run(void)
         std::printf(" %d", (int)sample.frames[i]);
     std::printf("\n");
 
-    /* Lesson 063: the sample starts playing on one channel, at half
-       volume — the fixed-point scale is 0-256, so 128 is half. The
-       scaling is checkable byte for byte: at half volume every frame the
-       channel emits is the sample's frame halved, truncated. */
-    Channel channel = {};
-    ChannelPlay(channel, sample, AUDIO_VOLUME_FULL / 2);
-    std::printf("engine: channel: playing %d frames at volume %d of %d\n",
-                sample.frame_count, channel.volume, AUDIO_VOLUME_FULL);
-    std::printf("engine: channel: first frames at that volume:");
-    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+    /* Lesson 064: two channels playing the same sample at different
+       volumes, so the mix is a real sum and the numbers are checkable —
+       each output frame is the two contributions added, then clamped. */
+    Mixer mixer;
+    MixerInit(mixer);
+    ChannelPlay(mixer.channels[0], sample, AUDIO_VOLUME_FULL / 2);
+    ChannelPlay(mixer.channels[1], sample, AUDIO_VOLUME_FULL / 4);
+    std::printf("engine: mix: channel 0 at volume %d, channel 1 at volume %d of %d\n",
+                mixer.channels[0].volume, mixer.channels[1].volume,
+                AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: first frames (summed):");
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i) {
+        int v = sample.frames[i * sample.channels];
         std::printf(" %d",
-                    (int)(sample.frames[i * sample.channels] *
-                          channel.volume) /
-                        AUDIO_VOLUME_FULL);
+                    (v * mixer.channels[0].volume) / AUDIO_VOLUME_FULL +
+                        (v * mixer.channels[1].volume) / AUDIO_VOLUME_FULL);
+    }
     std::printf("\n");
 
     double sprite_x = 312.0, sprite_y = 232.0;
@@ -342,22 +345,22 @@ int Run(void)
            where no buffer was due — so the phase accounts for all of the
            frame's audio work.
 
-           Lesson 063: the stream is the channel's output. One buffer is
-           what the channel produces — its sample's frames at its volume,
-           and silence past the sample's end. frame_count is the fact that
-           says when the sample ends; the channel never runs past it. */
+           Lesson 064: the stream is the mix. One buffer is every active
+           channel's next frames summed and clamped — silence where no
+           channel has anything to say. frame_count is the fact that says
+           when a sample ends; no channel ever runs past it. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            int before = channel.cursor;
-            ChannelFill(channel, stream, CHUNK_FRAMES);
-            int take = channel.cursor - before;
+            int before = mixer.channels[0].cursor;
+            MixBuffer(mixer, stream, CHUNK_FRAMES);
+            int take = mixer.channels[0].cursor - before;
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
                 sample_fed += take;
                 if (take > 0)
                     sample_feeds += 1;
-                if (!sample_end_named && !channel.active) {
+                if (!sample_end_named && !mixer.channels[0].active) {
                     /* The end, named in the sample's own numbers: what was
                        fed before silence, and how many buffers carried it. */
                     sample_end_named = true;
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Five channels at the peak *(predict-the-output)*

The run's two channels peak at 6142 — the clamp is the one piece of the
mix this run never shows working. Put five channels at
`AUDIO_VOLUME_FULL` on the tone and predict, before running anything:
the first eight mixed frames to the digit, and whether the clamp changes
any of them; the tone's peak frame and its trough frame — each one's
sum and what the stream holds there — and what a wrapping accumulator
(the sum truncated to the format's 16 bits) would put in the stream at
those two frames instead; the largest sum that still passes the clamp
untouched; and which frame of the sample the clamp changes first, with
how many of its 22050 frames it changes in all. Give the run a probe —
the whole sample through the engine's own `MixBuffer` on a scratch
mixer, five channels at full volume, the frames the clamp changes named
and counted — and reconcile every number with the sum, the threshold,
and the clamp. Finish with the boundary: a sample at amplitude 1.0 —
its peak 32767 — how many channels at full volume does one peak carry
before the clamp engages, and what does one channel alone do?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-064/ex1.md)

### Exercise 2 — What the mix costs *(measure-the-performance)*

The mix walks all sixteen channels for every output frame whether or
not any of them is playing — the pool is fixed, and so is the walk.
Measure what that costs as channels are added: time `MixBuffer` on its
own with 0, 1, 2, 4, 8, and 16 channels active — enough buffers per
configuration to see past the clock's granularity — and report the cost
per buffer and the cost per output frame at each count. Put the numbers
against the frame budget: the 16.7 ms frame, and the audio phase the
frame log reports at a feed. Say what the measurements show — which
part of the mix grows with the active count and which part does not —
and what your probe has to do to keep the channels playing while it
times. Finish with what a probe like this cannot hide about itself.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-064/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 063 — one channel](lesson-063-channel.md) ·
**Next:** [Lesson 065 — channel allocation](lesson-065-allocation.md) ·
**Code tag:** [`lesson-064`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-064)
