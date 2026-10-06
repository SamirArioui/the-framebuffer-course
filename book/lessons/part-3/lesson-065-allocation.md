# Lesson 065 — channel allocation

{{#include ../../stability-horizon.md}}

## Prose

Lesson 064 gave the mixer sixteen channels and left one question open:
who plays where. Sixteen slots, and many more sounds over a run — every
sound that starts needs one of them, at the instant it starts. This
lesson is the allocator. The pool is fixed; a sound takes the **first
free effect channel**; and when every effect channel is busy the mixer
**steals the oldest one** — the sound that started earliest gives up
its channel to the sound that just arrived.

### The first free channel

`MixerPlay` in `src/audio.cpp` answers one request: start this sample
at this volume, and say which channel it landed on. The walk is
deliberately plain — from the channel after the music channel to the
pool's end, take the first one that is not active. `ChannelPlay` starts
the sound there exactly as lesson 063 did, the channel is stamped with
the mixer's order, and the channel's number is the return value. The
caller learns where the sound is; the mixer already knew.

First free is the whole policy when there is room, and it is enough.
There is no queue to wait in and no search for a better channel: at
most fifteen checks, each one a flag. It also keeps the pool **dense**
— sounds cluster at the pool's low end and the walk stops at the first
one it finds — and the mix above does not care at all which of the
channels are playing.

The run makes the allocation printable. Sixteen effects — one more than
the pool's fifteen effect channels — go through `MixerPlay` at volume
16 of 256, low so that fifteen of them still sum inside the format:
each contributes at most 511 of the tone's 8191 peak, and fifteen of
those sum to at most 7665. The log is the allocator's own answers:

```
engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: mix: effect  1 -> channel  1
engine: mix: effect  2 -> channel  2
...
engine: mix: effect 15 -> channel 15
engine: mix: effect 16 -> channel  1 (the oldest was stolen)
engine: mix: music channel 0 is reserved and was never stolen
engine: mix: 30 buffers of sound, then silence
```

Effects 3 to 14 fill in exactly as the pattern shows:
`effect  N -> channel  N`, one line per effect. The first fifteen
presses take the first free channel and find it at 1, 2, 3, and so on.
The sixteenth is the interesting line: the pool is full, and the answer
is channel 1.

### When they are all busy, the oldest goes

A full pool is not a failure — it is the case the allocator has a
documented answer for. The second walk finds the effect channel whose
`started` is smallest, and `ChannelPlay` restarts it with the new
sound: the cursor returns to the sample's first frame and the old sound
is gone from the mix. The run's effect 16 lands on channel 1 because
channel 1 holds the effect that started first — the one with order 1.

Note what always holds across a steal. The sound the player just asked
for is always the one that plays; the interruption falls on a sound
already in progress. With fifteen channels and sixteen sounds,
something must give — the policy chooses **which** sound gives, not
whether one does.

### Order, not time

"Oldest" needs a meaning, and the two new fields give it one:
`Channel.started` is when the channel's current sound began, and
`Mixer.order` is a counter of how many sounds have started. Every
start — free channel or steal — takes the next order and stamps the
channel with it. Smallest stamp is oldest sound; that is the whole
relation.

Why a counter and not a clock. What the policy compares is *order* —
started before, started after — and the starts themselves define that
relation; a timestamp would answer the same question with more
machinery and no better answer. The platform clock is an OS question
behind the seam, and this allocator never needs to ask it: one `long`
of starts, incremented at each one, is all "oldest" ever meant. (A
counter that never returns to a previous value is also what keeps the
comparison honest: a channel that is stolen is stamped anew, so the
next full pool moves on to the next-oldest instead of dropping the same
sound twice in a row.)

### Channel 0 is the music channel

`AUDIO_MUSIC_CHANNEL` — channel 0 — is reserved. The first-free walk
starts after it and the stealing walk only ever considers channels 1
through 15: effects take the pool's fifteen effect channels, and
channel 0 is never offered to an effect and never dropped. Lesson 066
puts the music there; this lesson only keeps it empty.

The reservation is worth its own sentence because it is a promise, not
a preference. Music is one long sound under everything else; effects
are the short bursts over it. If a busy second could steal channel 0,
the music would skip exactly when the game is most alive — the
background has to survive the busiest moment, so the busiest moment is
not allowed to touch it.

### Why a fixed pool

Nothing is allocated while sound plays. The channels exist because
`MixerInit` walked the pool once at startup; every start writes into
one of them. This is the arena habit applied to channels — **capacity
is a decision, not an event** — and the reason is the same as always: a
sound is needed at the instant the button is pressed. It cannot wait
for an allocation, and playing it must never fail because the machine
is busy. With the pool, a sound never fails to start; when capacity
runs out, the answer is a policy, not an error.

### Why the oldest, and not something else

Two alternatives are legal, and this is where they fail.

**Refuse the new sound.** Nothing gets cut; every sound that is playing
plays to its end. But the player pressed the button and nothing
happened — the input's answer is silence, and the busy second is
exactly when feedback matters most. Refusal is a real policy for sounds
that must not be interrupted; exercise 2 makes it code so the sentence
can be checked instead of trusted.

**Steal the quietest.** Fair on paper — drop the sound contributing
least. But quiet is not a number this course has: the channel's volume
is a scale factor, not how loud the sound *is*, and perceptual loudness
is a subject the course has not taught. A policy built on a quantity
nobody has defined is a policy nobody can check.

The oldest effect is the sound the listener has heard the most of; it
has said most of what it has to say, and the new sound is what just
happened. That is the trade this policy makes, and it is not free — a
stolen sound is cut mid-sample, and a game that keeps sixteen effects
busy keeps cutting them. The claim is only that the cuts land on the
sounds least worth keeping.

### What this run verified, and what it did not

All numbers from real runs of this lesson's end state on this machine,
against ALSA's `null` device:

- **The allocation is checkable line by line.** Effects 1 to 15 land on
  channels 1 to 15; effect 16 lands on channel 1 with the oldest
  effect dropped; the music channel is named and untouched. Every line
  is the allocator's own return value printed — not a claim about
  sound, a claim about which channel each sound was given.
- **The sample's end is unchanged by all of it.** The end line is in
  the mix's own numbers now — several channels play, so there is no
  single channel's cursor to count; the run counts buffers that carried
  sound instead:

  ```
  engine: mix: 30 buffers of sound, then silence
  ```

  Thirty buffers of 735 frames are 22050 — the sample's length,
  exactly. The burst changed who plays, not how long the sound lasts.
- **The build carries exactly one warning:** `DrawScene` defined but
  not used in `src/main.cpp` — the Part 2 wart carried forward on
  purpose — and this lesson adds none. `tools/check-boundary.sh` still
  passes: the allocator asks no OS question — the clock it does not
  need is exactly why the boundary holds — and `openspec validate --all`
  reports 11 passed, 0 failed.

What this run cannot verify is the part that needs ears: no speaker has
made a sound — `null` takes the samples and discards them. "Which sound
gives way" is answered here in channels and counters; what the cut
sounds like on real speakers is yours to hear.

## Code step

One change for this lesson, three files: `src/audio.h` and
`src/audio.cpp` grow the allocator — `MixerPlay`, the `Channel.started`
stamp, the `Mixer.order` counter, and the `AUDIO_MUSIC_CHANNEL`
reservation — and `src/main.cpp` plays a scripted burst of sixteen
effects through it, one more than the pool's effect channels, so the
policy runs in front of the reader. The loop's end report is reworded
into the mix's own numbers — how many buffers carried sound — because
with several channels playing there is no single channel to count. The
mix itself is lesson 064's, untouched. Its end state is tagged
`lesson-065`.

```diff
diff --git a/src/audio.cpp b/src/audio.cpp
index 58c3af8..abe239b 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -231,7 +231,35 @@ void MixerInit(Mixer &mixer)
         mixer.channels[c].cursor = 0;
         mixer.channels[c].volume = 0;
         mixer.channels[c].active = false;
+        mixer.channels[c].started = 0;
     }
+    mixer.order = 0;
+}
+
+int MixerPlay(Mixer &mixer, const Sample &sample, int volume)
+{
+    /* The first free channel of the pool's effects — the music channel is
+       reserved, so the walk starts after it. */
+    for (int c = AUDIO_MUSIC_CHANNEL + 1; c < AUDIO_MIXER_CHANNELS; ++c) {
+        if (!mixer.channels[c].active) {
+            ChannelPlay(mixer.channels[c], sample, volume);
+            mixer.channels[c].started = ++mixer.order;
+            return c;
+        }
+    }
+
+    /* Every effect channel is busy. The policy is to steal the oldest —
+       the one that started earliest — because it is the sound the
+       listener has already heard the most of. The music channel is never
+       a candidate. */
+    int oldest = AUDIO_MUSIC_CHANNEL + 1;
+    for (int c = oldest + 1; c < AUDIO_MIXER_CHANNELS; ++c) {
+        if (mixer.channels[c].started < mixer.channels[oldest].started)
+            oldest = c;
+    }
+    ChannelPlay(mixer.channels[oldest], sample, volume);
+    mixer.channels[oldest].started = ++mixer.order;
+    return oldest;
 }
 
 void MixBuffer(Mixer &mixer, short *out, int frame_count)
diff --git a/src/audio.h b/src/audio.h
index 2b33c0f..af36e32 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -80,6 +80,7 @@ struct Channel {
     int cursor;           /* the next sample frame to read */
     int volume;           /* 0..AUDIO_VOLUME_FULL, fixed point */
     bool active;          /* playing now */
+    long started;         /* when this channel began, in the mixer's order */
 };
 
 /* Starts `sample` playing on this channel at `volume`, from its first
@@ -89,15 +90,28 @@ void ChannelPlay(Channel &channel, const Sample &sample, int volume);
 /* The mixer's fixed set of channels. */
 constexpr int AUDIO_MIXER_CHANNELS = 16;
 
+/* Channel 0 is the music channel: reserved, and never stolen. Effects
+   take the rest. */
+constexpr int AUDIO_MUSIC_CHANNEL = 0;
+
 /* The mixer: one fixed pool of channels, decided up front — nothing is
    allocated while sound plays. */
 struct Mixer {
     Channel channels[AUDIO_MIXER_CHANNELS];
+    long order; /* how many sounds have started, so "oldest" has a meaning */
 };
 
 /* Every channel idle and free. */
 void MixerInit(Mixer &mixer);
 
+/* Starts `sample` playing at `volume` and says which channel it landed
+   on. An effect takes the first free channel of the pool's effects
+   (everything but the music channel). When they are all busy the mixer
+   steals the **oldest effect channel** — the one that started earliest —
+   so the sound that has had the longest hearing is the one dropped. The
+   music channel is never stolen. */
+int MixerPlay(Mixer &mixer, const Sample &sample, int volume);
+
 /* The mix: `frame_count` frames of stream, each one the sum of every
    active channel's next frame at its volume, clamped to the format's
    range. Clamped, never wrapped — a sum past the range lands on the
diff --git a/src/main.cpp b/src/main.cpp
index 25ce44c..1e86986 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -162,24 +162,22 @@ int Run(void)
         std::printf(" %d", (int)sample.frames[i]);
     std::printf("\n");
 
-    /* Lesson 064: two channels playing the same sample at different
-       volumes, so the mix is a real sum and the numbers are checkable —
-       each output frame is the two contributions added, then clamped. */
+    /* Lesson 065: a scripted burst of effects — one more than the pool's
+       effect channels — so the allocation policy runs in front of the
+       reader: the first free channel for each sound, and then the oldest
+       effect channel stolen. Volume is low so sixteen of them still sum
+       inside the format. */
     Mixer mixer;
     MixerInit(mixer);
-    ChannelPlay(mixer.channels[0], sample, AUDIO_VOLUME_FULL / 2);
-    ChannelPlay(mixer.channels[1], sample, AUDIO_VOLUME_FULL / 4);
-    std::printf("engine: mix: channel 0 at volume %d, channel 1 at volume %d of %d\n",
-                mixer.channels[0].volume, mixer.channels[1].volume,
-                AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: first frames (summed):");
-    for (int i = 0; i < 8 && i < sample.frame_count; ++i) {
-        int v = sample.frames[i * sample.channels];
-        std::printf(" %d",
-                    (v * mixer.channels[0].volume) / AUDIO_VOLUME_FULL +
-                        (v * mixer.channels[1].volume) / AUDIO_VOLUME_FULL);
+    const int EFFECT_CHANNELS =
+        AUDIO_MIXER_CHANNELS - AUDIO_MUSIC_CHANNEL - 1;
+    for (int i = 0; i <= EFFECT_CHANNELS; ++i) {
+        int ch = MixerPlay(mixer, sample, AUDIO_VOLUME_FULL / 16);
+        std::printf("engine: mix: effect %2d -> channel %2d%s\n", i + 1, ch,
+                    i < EFFECT_CHANNELS ? "" : " (the oldest was stolen)");
     }
-    std::printf("\n");
+    std::printf("engine: mix: music channel %d is reserved and was never stolen\n",
+                AUDIO_MUSIC_CHANNEL);
 
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
@@ -241,8 +239,7 @@ int Run(void)
        has reached, what has been fed, and whether the end has been
        named. The sample's frame_count is the fact that says when the
        sample ends; nothing here assumes how long it is. */
-    int sample_fed = 0;         /* frames of sample handed to the device */
-    int sample_feeds = 0;       /* feeds that carried sample frames */
+    int sample_feeds = 0;       /* buffers that carried sound */
     bool sample_end_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
@@ -351,21 +348,28 @@ int Run(void)
            when a sample ends; no channel ever runs past it. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            int before = mixer.channels[0].cursor;
+            /* Did this buffer carry sound? Answered from the cursors: the
+               buffer carried sample frames exactly when some channel's
+               cursor moved during the mix. */
+            long before = 0;
+            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
+                before += mixer.channels[c].cursor;
             MixBuffer(mixer, stream, CHUNK_FRAMES);
-            int take = mixer.channels[0].cursor - before;
+            long after = 0;
+            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
+                after += mixer.channels[c].cursor;
+            bool any = after > before;
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
-                sample_fed += take;
-                if (take > 0)
+                if (any)
                     sample_feeds += 1;
-                if (!sample_end_named && !mixer.channels[0].active) {
-                    /* The end, named in the sample's own numbers: what was
-                       fed before silence, and how many buffers carried it. */
+                if (!sample_end_named && !any) {
+                    /* The end, named in the mix's own numbers: how many
+                       buffers carried sound before it ran out. */
                     sample_end_named = true;
-                    std::printf("engine: sample: %d frames fed in %d buffers — the sample's end; the stream is silence from here\n",
-                                sample_fed, sample_feeds);
+                    std::printf("engine: mix: %d buffers of sound, then silence\n",
+                                sample_feeds);
                 }
             } else {
                 /* A device that will not take the samples is named once,
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The twenty-first press *(predict-the-output)*

Every sound effect is a press of a button: the player causes sounds in
whatever order the game allows, and the allocator answers each press
with a channel. Before running anything, predict what this sequence
does to a freshly initialized mixer. Fifteen `MixerPlay` calls in a row
— effects 1 to 15. The sounds of effects 4 and 9 end. Three calls —
effects 16, 17, 18. The sound of effect 7 ends. Two calls — effects 19
and 20. Say for every call which channel the effect lands on; for every
call that meets a full pool, which sound is dropped and from which
channel; and when the sequence is done, what `mixer.order` holds. Then
the boundary: the twenty-first call — which sound goes, and why that
one. Give the run a probe — the same sequence through the engine's own
`MixerPlay` on a scratch mixer, every landing channel, every ending,
and every dropped sound printed from the allocator's own answers — and
reconcile every line with the walk and the counter.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-065/ex1.md)

### Exercise 2 — The policy that refuses *(extend-the-code)*

This lesson's pool steals the oldest effect channel when they are all
busy; the alternative it names and rejects is to refuse the new sound
instead. Make the rejected policy real rather than trusting the
sentence: a second allocator beside `MixerPlay` — same pool, same first
free effect channel — that starts the sound when there is room and
refuses it, by name, when every effect channel is busy. `MixerPlay`
itself stays untouched. Drive the lesson's sixteen-effect burst through
both allocators and report what the sixteenth press does under each.
Finish with one paragraph: which policy you would ship for a game's
sound effects, and what the player experiences under the one you leave
out.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-065/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 064 — the mix](lesson-064-mix.md) ·
**Next:** [Lesson 066 — music as a loop](lesson-066-music.md) ·
**Code tag:** [`lesson-065`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-065)
