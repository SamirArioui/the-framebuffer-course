# Lesson 063 — one channel

{{#include ../../stability-horizon.md}}

## Prose

Lesson 062 played the sample with playback state scattered through the
run: a cursor beside the loop, a fill of the stream buffer, a
subtraction and a minimum bounding it. This lesson names the unit those
pieces were already making and moves it into `src/audio.h`: **a channel
is the unit of playback** — what is playing, where it is in the sample,
and how loud. It is a small struct of plain values, not a device. The
mixer batch starts here: lessons 063 through 065 grow this one struct
into a mix of many channels and a pool that hands them out.

### Four plain values

`Channel` in `src/audio.h` is four fields: `sample` — what it is
playing, or 0; `cursor` — where it is in the sample, the next frame to
read; `volume` — how loud, on the scale fixed below; `active` — playing
now.

The cursor is worth reading twice. It is a position in **sample
frames** — not bytes, not seconds. `ChannelFill` advances it one frame
per output frame, and a frame is `sample.channels` values wide, the
stride the loader checked in lesson 061. From the cursor's number alone
one can say how much of the sample has been spent.

The two functions are the rest of the channel. `ChannelPlay` is the
whole of "start a sound": four assignments — the sample, the cursor to
its first frame, the volume, the active flag. No device is touched,
nothing is allocated, and the call writes through one channel's
reference only — starting playback on one channel cannot disturb any
other channel, because there is no path from the call to one.
`ChannelFill` is lesson 062's feed fill with the state in the struct:
while the channel is playing it writes the sample's frames scaled by
the volume, one for one, cursor advancing; past the sample's end,
silence.

### Volume as fixed point

`AUDIO_VOLUME_FULL` is 256. The scale runs 0 to 256 — 0 is silence, 256
is full scale, and 128 is exactly half of the scale. The scaling is one
expression, in integer arithmetic: `(frame * volume) /
AUDIO_VOLUME_FULL`.

The run's own numbers show it working. The sample's first frames are 0
513 1024 1531 2032 2525 3009 3480; the channel, started at volume 128,
reports what it emits:

```
engine: channel: playing 22050 frames at volume 128 of 256
engine: channel: first frames at that volume: 0 256 512 765 1016 1262 1504 1740
```

Every emitted frame is the sample's frame × 128 / 256, truncated toward
zero: 513 becomes 256 (from 256.5), 1531 becomes 765 (from 765.5), 2525
becomes 1262, 3009 becomes 1504. The exact halves — 1024 becomes 512,
2032 becomes 1016, 3480 becomes 1740 — are the same arithmetic landing
on integers: half of the scale is exact, half of a frame is truncated.

Why integers. This is the part's design decision D5: the mix must be
arithmetic a learner can follow byte for byte. A `double` volume is the
rejected alternative — same behavior (513 × 0.5 is 256.5 there too),
one more format question: what the fraction is in binary, what the cast
to `short` does with it. Integers keep the whole computation at
`(frame * volume) / 256`, and paper and machine agree.

### The end of a sample

`frame_count` is the fact that ends playback — the same fact that
bounded lesson 062's fill. `ChannelFill` writes the sample's frames
while the cursor is inside the sample; the first output frame past the
end takes the other branch: silence in the buffer, and the channel goes
**inactive again**. The "again" is the point — the channel returns to
the state `ChannelPlay` found it in, and a channel in that state is
free. That is what makes a channel reusable, and it is exactly what
lesson 065's pool keys on: inactive means the channel can carry another
sound.

The exact frame where this happens is not the sample's last frame, and
the gap is worth one careful look. The fill that spends the sample's
final frame leaves the channel *still active* — its cursor sits at
`frame_count`, and nothing has looked at that yet. The look happens at
the first output frame of the next fill, where the cursor's condition
fails. This asset's arithmetic is round — 30 buffers of 735 frames are
22050 — so the end is observed with the first frame of the fill after
the thirtieth; lesson 062's short file (1000 frames) would end
mid-fill, at the 266th frame of the second buffer. The invariant holds
either way: **the channel goes inactive at the first output frame past
the sample's end, wherever that falls.** And what the run reports —
`22050 frames fed in 30 buffers` — counts what the cursor spent, the
sample's own count whatever the buffer boundaries do.

### One channel cannot overflow the format

Look at the emitted frame once more: `(frame * volume) / 256`, with
volume at most `AUDIO_VOLUME_FULL`. At full scale the sample passes
through unchanged; every smaller volume only makes it quieter. A single
channel's output cannot leave −32768..32767 — not even at the sample's
extremes: −32768 × 256 / 256 is −32768, and the intermediate multiply
(peaking near 8.4 million) is far inside `int`.

That is why this lesson has no clamp. Clamping is a *summing* problem:
two channels at full scale, each emitting 20000, produce 40000 between
them, and the format has no room for it. Only summing channels can
exceed the range — which is exactly why the clamp arrives with the mix,
in lesson 064, and not a lesson before it.

The guarantee rests on one assumption: the volume is
0..`AUDIO_VOLUME_FULL`. A channel at 512 would emit twice the sample
and leave the format by itself. `ChannelPlay` does not enforce the
range — the scale is the contract, held by whoever sets a volume, the
way `Sample`'s fields are held by the loader's refusal.

### What this run verified, and what it did not

All numbers from real runs of this lesson's end state on this machine,
against ALSA's `null` device:

- **The channel starts, says so, and scales byte-exact.** The startup
  lines name the channel's state — `playing 22050 frames at volume 128
  of 256` — and the frames it emits, 0 256 512 765 1016 1262 1504 1740,
  are `(frame * 128) / 256` truncated, digit for digit.
- **The channel spends exactly the sample.** The end line lands with
  the sample's own numbers:

  ```
  engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
  ```

  30 × 735 = 22050 = `frame_count`, now counted off the channel's
  cursor.
- **The build carries exactly one warning:** `DrawScene` defined but
  not used in `src/main.cpp` — the Part 2 wart carried forward on
  purpose — and this lesson adds none. `tools/check-boundary.sh` still
  passes: the channel is engine code and makes no OS call.

What this run cannot verify is the part that needs ears: no speaker has
made a sound — `null` takes the samples and discards them. "Half as
loud as lesson 062" is a claim about your machine and your ears. What
this lesson claims is that the frames were halved, and the run prints
them.

## Code step

One change for this lesson, three files: `src/audio.h` and
`src/audio.cpp` grow the channel — `Channel`, `ChannelPlay`,
`ChannelFill`, and the `AUDIO_VOLUME_FULL` scale — and `src/main.cpp`
plays the sample on one channel instead of feeding it inline. The feed
fills the stream from `ChannelFill`; how much of the sample was spent is
read off the channel's cursor; the end is named when the channel goes
inactive. The gate, the paced wait, and the horizon are lesson 062's,
untouched. Its end state is tagged `lesson-063`.

```diff
diff --git a/src/audio.cpp b/src/audio.cpp
index 82223bd..eaf730b 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -187,4 +187,34 @@ SampleResult LoadSample(Arena &arena, const char *path)
     return result;
 }
 
+void ChannelPlay(Channel &channel, const Sample &sample, int volume)
+{
+    channel.sample = &sample;
+    channel.cursor = 0;
+    channel.volume = volume;
+    channel.active = true;
+}
+
+void ChannelFill(Channel &channel, short *out, int frame_count)
+{
+    for (int i = 0; i < frame_count; ++i) {
+        if (channel.active && channel.sample &&
+            channel.cursor < channel.sample->frame_count) {
+            /* One sample frame, scaled to the channel's volume. A frame
+               is sample.channels values wide; the engine's stream is one
+               channel wide, so it takes the frame's first value. */
+            int frame =
+                channel.sample->frames[channel.cursor *
+                                       channel.sample->channels];
+            out[i] = (short)((frame * channel.volume) / AUDIO_VOLUME_FULL);
+            channel.cursor += 1;
+        } else {
+            /* The sample's end — the fact frame_count carries. Silence
+               from here, and the channel is free again. */
+            channel.active = false;
+            out[i] = 0;
+        }
+    }
+}
+
 } /* namespace engine */
diff --git a/src/audio.h b/src/audio.h
index f05c1b0..aed24e6 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -67,6 +67,31 @@ struct SampleResult {
    what the engine keeps is its copy, and a refused load keeps nothing. */
 SampleResult LoadSample(Arena &arena, const char *path);
 
+/* Volume in fixed point: AUDIO_VOLUME_FULL is full scale and 0 is
+   silence. The scale runs 0-256 so the mix is integer arithmetic a
+   learner can follow byte for byte. */
+constexpr int AUDIO_VOLUME_FULL = 256;
+
+/* Lesson 063: one channel — the unit of playback. What it is playing,
+   where it is in the sample, and how loud: three plain values, not a
+   device. A channel plays its sample to its end and then is free again. */
+struct Channel {
+    const Sample *sample; /* what it is playing, or 0 */
+    int cursor;           /* the next sample frame to read */
+    int volume;           /* 0..AUDIO_VOLUME_FULL, fixed point */
+    bool active;          /* playing now */
+};
+
+/* Starts `sample` playing on this channel at `volume`, from its first
+   frame. Playing on one channel leaves every other channel alone. */
+void ChannelPlay(Channel &channel, const Sample &sample, int volume);
+
+/* Fills `out` with `frame_count` frames of this channel's output: the
+   sample's frames at the channel's volume, one for one, advancing the
+   cursor. Past the sample's end the channel writes silence and is
+   inactive again — frame_count is the fact that says when. */
+void ChannelFill(Channel &channel, short *out, int frame_count);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index fbfd37d..732ac22 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -162,6 +162,22 @@ int Run(void)
         std::printf(" %d", (int)sample.frames[i]);
     std::printf("\n");
 
+    /* Lesson 063: the sample starts playing on one channel, at half
+       volume — the fixed-point scale is 0-256, so 128 is half. The
+       scaling is checkable byte for byte: at half volume every frame the
+       channel emits is the sample's frame halved, truncated. */
+    Channel channel = {};
+    ChannelPlay(channel, sample, AUDIO_VOLUME_FULL / 2);
+    std::printf("engine: channel: playing %d frames at volume %d of %d\n",
+                sample.frame_count, channel.volume, AUDIO_VOLUME_FULL);
+    std::printf("engine: channel: first frames at that volume:");
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+        std::printf(" %d",
+                    (int)(sample.frames[i * sample.channels] *
+                          channel.volume) /
+                        AUDIO_VOLUME_FULL);
+    std::printf("\n");
+
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
     double last = started;
@@ -222,7 +238,6 @@ int Run(void)
        has reached, what has been fed, and whether the end has been
        named. The sample's frame_count is the fact that says when the
        sample ends; nothing here assumes how long it is. */
-    int sample_cursor = 0;      /* the next frame the feed takes */
     int sample_fed = 0;         /* frames of sample handed to the device */
     int sample_feeds = 0;       /* feeds that carried sample frames */
     bool sample_end_named = false;
@@ -327,32 +342,22 @@ int Run(void)
            where no buffer was due — so the phase accounts for all of the
            frame's audio work.
 
-           Lesson 062: the stream is the sample. One buffer is filled from
-           the sample's frames where the sample has them and with silence
-           beyond its end — silence is a stream too, and the device keeps
-           getting its buffers. frame_count is the fact that says when the
-           sample ends; the fill never runs past it. */
+           Lesson 063: the stream is the channel's output. One buffer is
+           what the channel produces — its sample's frames at its volume,
+           and silence past the sample's end. frame_count is the fact that
+           says when the sample ends; the channel never runs past it. */
         double t_audio = platform::Now();
-        if (audio.output && sample.frames && t_audio >= next_feed) {
-            /* Each frame is sample.channels values wide — one here, the
-               loader refuses anything else — and the engine's stream is
-               one channel wide, so a frame is its first (only) channel. */
-            int left = sample.frame_count - sample_cursor;
-            int take = left < CHUNK_FRAMES ? left : CHUNK_FRAMES;
-            for (int i = 0; i < take; ++i)
-                stream[i] =
-                    sample.frames[(sample_cursor + i) * sample.channels];
-            for (int i = take; i < CHUNK_FRAMES; ++i)
-                stream[i] = 0;
+        if (audio.output && t_audio >= next_feed) {
+            int before = channel.cursor;
+            ChannelFill(channel, stream, CHUNK_FRAMES);
+            int take = channel.cursor - before;
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
-                sample_cursor += take;
                 sample_fed += take;
                 if (take > 0)
                     sample_feeds += 1;
-                if (!sample_end_named &&
-                    sample_cursor == sample.frame_count) {
+                if (!sample_end_named && !channel.active) {
                     /* The end, named in the sample's own numbers: what was
                        fed before silence, and how many buffers carried it. */
                     sample_end_named = true;
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Quarter volume, to the digit *(predict-the-output)*

The run starts the channel at 128 of 256 — half the scale — and the
first frames it emits are the sample's halved. Change the channel's
starting volume to 64 and predict its behavior to the digit before
running anything: the first eight frames it emits from the tone's first
eight (0 513 1024 1531 2032 2525 3009 3480), truncation included; the
fill sequence — how many fills carry sample frames, how many frames
each carries, and where exactly the channel goes inactive: which fill,
which frame of it, and what that fill's buffer holds frame by frame;
and exactly what the end report will say. Give the fill a probe — one
line per fill, naming how many sample frames it carried and whether
the channel is active after it — run at the new volume, and reconcile
every number with the scaling, the cursor, and `frame_count`. Finish
with the boundary the fill exposes: for a sample of 1000 frames
instead, where would the channel go inactive, and what would the
report count?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-063/ex1.md)

### Exercise 2 — The fade to silence *(extend-the-code)*

Volume is live state: the channel's fields are read at fill time, so
playback can change loudness while it runs. Make the run start the
sample at full volume (`AUDIO_VOLUME_FULL`) and fade it — every time
the loop feeds the device, the channel's volume drops by 16 of 256
(never below zero), and the run names the new volume each time it
drops. Predict before running: the fills that carry sound and the exact
fill where the channel's output becomes pure silence; what the channel
is doing from that fill on — when its playback actually ends, and what
the end report says at the fade's volumes; and what the report's
`frames fed` counts once the frames it feeds are silence. Then run, and
reconcile the fade's log line by line with the fill sequence and the
end report.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-063/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 062 — the sample's playback facts](lesson-062-playback.md) ·
**Next:** [Lesson 064 — the mix](lesson-064-mix.md) ·
**Code tag:** [`lesson-063`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-063)
