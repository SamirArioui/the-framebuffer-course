# Lesson 067 — effects as one-shots

{{#include ../../stability-horizon.md}}

## Prose

Lesson 066 gave the game its music: one sound that runs until told to
stop. This lesson is the other half — the sound effect, the burst that
plays to its end and is gone. Two sounds, two behaviours, and the
engine's route for each gets its contract written down where the code
can be checked against it. The route itself is not new. **Lesson 065's
allocator gets its name here — `MixerPlay` becomes `MixerPlayEffect` —
because it was always the effect path.** The walk, the stealing policy,
and the reserved music channel are exactly lesson 065's; the name now
says which route it is.

### Play once, then free

The one-shot contract, in full: the effect plays **once**, to the
sample's end — then its channel goes inactive and returns to the pool,
free for the next sound. Three clauses, and the third is the one that
matters. "Play once" is lesson 065's behaviour with the loop flag
clear; "to its end" is `frame_count` doing what it always did. But
"then free" is the contract's promise to the *next* sound: the channel
does not linger, does not fade, does not wait to be noticed. It goes
inactive, and the first-free walk takes it like any other.

That return is what makes effects cheap to fire. Nothing is allocated
when a button is pressed — the pool was decided at startup and the
channels just change hands — and nothing is freed either: the channel
that ended is the channel the next effect plays on. A game can fire
effects at any rate the pool can absorb, at the cost of one walk and
one struct write per sound. This is the arena habit again, in its
smallest form: capacity is a decision, and reuse is instant.

### The pool's second life

The run makes the return visible. Two effects of one sample fire
together, at different volumes; when both have played to their end,
one more fires. The log is the allocator's and the channels' own
answers:

```
engine: mix: effect 1 -> channel  1 (volume 256 of 256)
engine: mix: effect 2 -> channel  2 (volume 64 of 256)
engine: mix: one sample, two volumes — two different sounds
engine: mix: first frames (summed): 0 1536 3047 4508 5898 7196 8378 9430
engine: effect: both ended in 12 buffers — channels 1 and 2 are free again
engine: mix: effect 3 -> channel  1 (volume 256 of 256) — the pool returned the first effect's channel
engine: effect: effect 3 ended in 12 buffers — channel 1 is free again
engine: mix: 24 buffers of sound, then silence
```

Follow the channels. The pair takes the pool's first two free channels
— 1 and 2, lesson 065's walk. The effect is 8820 frames and the
buffers are 735, so twelve buffers carry it exactly; the pull past its
end is what sets the channel inactive, and the next buffer finds both
channels free. The third effect asks for a channel and
lands on **1** — the same channel the first effect had. The pool
returned it. That line is the contract's third clause happening in
front of the reader, and the account closes in the mix's own numbers:
24 buffers carried sound — twelve with the pair playing together,
twelve with the third effect — and then silence.

### Per-sound volume

Both effects of the pair play the same sample, start at the same
instant, and end at the same instant — and the mix holds two different
sounds. The difference is the `volume` each carries into its channel:
one plays the sample at full scale and one at a quarter of it. The
mixed buffer's first frames say so digit by digit:

```
engine: mix: first frames (summed): 0 1536 3047 4508 5898 7196 8378 9430
```

Every frame is two contributions added, each truncated toward zero
before the sum — lesson 064's order. Frame 1229 becomes 1229 at full
volume and 307 at a quarter — 1536. Frame 2438 becomes 2438 and 609 —
3047. The same sample, and the output is not the sample: it is the
sample **at a volume**, and two volumes are two different sounds. That
is why `MixerPlayEffect` takes a volume per call and not per mixer: the
loudness belongs to the sound the game just asked for, not to the
channel it happened to land on.

### The name, and what it does not change

Renaming is the smallest code step this course has had, and it is worth
saying what it is not. The allocator's walk is unchanged — first free
effect channel, oldest stolen under pressure. The reservation of the
music channel is unchanged. The mix is unchanged. What changes is that
the route now reads as what it is: `MixerPlayEffect` is the effect
path, `MixerPlayMusic` is the music path, and the difference between
them is the channel and the loop flag — not the arithmetic, not the
pool, not the sum. Lesson 065 taught the allocator before the naming
was due; this lesson closes the account.

### What this run verified, and what it did not

All numbers from real runs of this lesson's end state on this machine,
against ALSA's `null` device:

- **One-shot playback and channel return, as documented.** Effects 1
  and 2 land on channels 1 and 2 by the pool's own first-free walk,
  both end in twelve buffers — 8820 frames of 735, exactly — and the
  pool reports both channels free again. The third effect lands on
  channel 1: the pool returned the first effect's channel. Every
  channel number is the allocator's own return value printed.
- **Per-sound volume produces different output.** The same sample at
  full and at a quarter sums to `0 1536 3047 4508 5898 7196 8378 9430`
  — reconcilable frame by frame from the sample's first frames, each
  contribution truncated before the sum.
- **The account closes.** 24 buffers of 735 frames carried sound —
  twelve with the pair, twelve with the third effect — and then
  silence, counted by the run from the channels' own state.
- **The build carries exactly one warning:** `DrawScene` defined but
  not used in `src/main.cpp` — the Part 2 wart carried forward on
  purpose — and this lesson adds none. `tools/check-boundary.sh` still
  passes: the rename touches no seam and asks no OS question — and
  `openspec validate --all` reports 11 passed, 0 failed.

What this run cannot verify is the part that needs ears. No speaker has
made a sound: `null` takes the samples and discards them. Whether a
quarter-volume effect *sounds* like a different sound, and what an
interrupted one sounds like at all, are questions for hardware that
makes sound — the channels, the volumes, and the bytes are all
checked here.

## Code step

One change for this lesson, three files: `src/audio.h` and
`src/audio.cpp` name the effect route — `MixerPlay` becomes
`MixerPlayEffect`, and the header now states the one-shot contract in
full: plays once to the sample's end, its channel goes inactive and
returns to the pool, and the volume it takes is the sound's own — and
`src/main.cpp` drives that contract in front of the reader: two
effects of one sample at different volumes, then one more after they
end, so the pool's answer is a line of the log. The loop, the mix, and
the allocator's walks are lesson 065's and 066's, untouched. Its end
state is tagged `lesson-067`.

```diff
diff --git a/src/audio.cpp b/src/audio.cpp
index 0849b9d..a1f4e47 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -256,7 +256,7 @@ void MixerInit(Mixer &mixer)
     mixer.order = 0;
 }
 
-int MixerPlay(Mixer &mixer, const Sample &sample, int volume)
+int MixerPlayEffect(Mixer &mixer, const Sample &sample, int volume)
 {
     /* The first free channel of the pool's effects — the music channel is
        reserved, so the walk starts after it. */
diff --git a/src/audio.h b/src/audio.h
index e20f6d6..7576f59 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -112,13 +112,23 @@ struct Mixer {
 /* Every channel idle and free. */
 void MixerInit(Mixer &mixer);
 
-/* Starts `sample` playing at `volume` and says which channel it landed
-   on. An effect takes the first free channel of the pool's effects
+/* The effect route — lesson 065's allocator, named at last: it was
+   always the effect path. Starts `sample` playing at `volume` and says
+   which channel it landed on.
+
+   The one-shot contract, in full: the effect plays **once**, to the
+   sample's end — then its channel goes inactive and returns to the
+   pool, free for the next sound. Play once, then free; that is what
+   makes effects cheap to fire. `volume` is the sound's own, carried
+   into its channel, so two effects of one sample at different volumes
+   are different sounds.
+
+   An effect takes the first free channel of the pool's effects
    (everything but the music channel). When they are all busy the mixer
    steals the **oldest effect channel** — the one that started earliest —
    so the sound that has had the longest hearing is the one dropped. The
    music channel is never stolen. */
-int MixerPlay(Mixer &mixer, const Sample &sample, int volume);
+int MixerPlayEffect(Mixer &mixer, const Sample &sample, int volume);
 
 /* Starts `sample` playing as the run's music: on the music channel,
    looping, at `volume`. That is what the reserved channel was reserved
diff --git a/src/main.cpp b/src/main.cpp
index 10cdc13..9f825c3 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -193,28 +193,28 @@ int Run(void)
     PrintSample("music", music);
     PrintSample("effect", effect);
 
-    /* Lesson 066: the music as a loop and one effect as a one-shot in the
-       same run — the difference this lesson is about. The music takes the
-       music channel and runs until the run stops it; the effect takes a
-       pool channel and runs to its end. Both are the engine's format and
-       sum through the same mix. */
+    /* Lesson 067: the effect route's contract, in front of the reader.
+       Two effects of one sample at different volumes, fired together —
+       one sample, two different sounds — and then one more after they
+       have played to their end, so the pool's answer is visible: the
+       channel the first effect had comes back. The music is loaded and
+       silent here; lesson 068 starts it. */
     Mixer mixer;
     MixerInit(mixer);
-    MixerPlayMusic(mixer, music, AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: music  -> channel %2d (looping, volume %d of %d)\n",
-                AUDIO_MUSIC_CHANNEL, mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
+    int first_channel = MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: effect 1 -> channel %2d (volume %d of %d)\n",
+                first_channel, mixer.channels[first_channel].volume,
                 AUDIO_VOLUME_FULL);
-    int effect_channel = MixerPlay(mixer, effect, AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: effect -> channel %2d (one-shot, volume %d of %d)\n",
-                effect_channel, mixer.channels[effect_channel].volume,
+    int second_channel = MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL / 4);
+    std::printf("engine: mix: effect 2 -> channel %2d (volume %d of %d)\n",
+                second_channel, mixer.channels[second_channel].volume,
                 AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: the effect plays to its end; the music plays until the run stops it\n");
+    std::printf("engine: mix: one sample, two volumes — two different sounds\n");
 
-    /* The run's script: the music stops after this many buffers. 600
-       buffers of 735 frames are 441000 frames of music — three and a
-       third times around its 132300-frame loop. The stop is the run's
-       decision; the loop itself would go on. */
-    constexpr int MUSIC_STOP_FEEDS = 600;
+    /* The script's one decision: when both effects have played to their
+       end, fire one more. Its channel is the pool's own answer. */
+    int third_channel = -1;
+    int effect_step = 0;
 
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
@@ -272,14 +272,12 @@ int Run(void)
        last one — and the loop knows that without asking the platform. */
     double next_feed = platform::Now();
 
-    /* Lesson 066: the loop's own bookkeeping, in the run's numbers — how
-       many buffers have been fed and how many carried sound, how far the
-       music has played (its wraps and its cursor say), and whether the
-       silence after the stop has been named. Nothing here assumes how
-       long the loop is. */
+    /* Lesson 067: the run's account of its sounds, in the mix's own
+       numbers — how many buffers have been fed and how many carried
+       sound, and whether the final silence has been named. Nothing here
+       assumes how long a sound is. */
     int feeds = 0;         /* buffers handed to the device */
     int sound_feeds = 0;   /* buffers that carried sound */
-    int music_wraps = 0;   /* times the looping cursor returned to frame 0 */
     bool silence_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
@@ -390,59 +388,64 @@ int Run(void)
            difference. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            /* The run's script, one decision in it: at the 600th buffer
-               the run stops the music. A looping channel does not end on
-               its own, so ending it is the run's call — and this is the
-               call, made in front of the reader. */
-            if (feeds == MUSIC_STOP_FEEDS) {
-                MixerStop(mixer, AUDIO_MUSIC_CHANNEL);
-                std::printf("engine: mix: MixerStop ended the music on channel %d — %d frames in %d buffers, %d wraps, cursor %d of %d\n",
-                            AUDIO_MUSIC_CHANNEL, feeds * CHUNK_FRAMES, feeds,
-                            music_wraps,
-                            mixer.channels[AUDIO_MUSIC_CHANNEL].cursor,
-                            music.frame_count);
-            }
-
-            /* Does this buffer carry sound, and did the music wrap? Both
-               answered from the channels' own state: a channel active when
-               the mix starts speaks in this buffer, and a looping cursor
-               going backwards is the wrap. */
+            /* Did this buffer carry sound? A channel speaks in it exactly
+               when it is active and has frames left to give: a one-shot at
+               its end gives none, and a looping channel at its end gives
+               everything again. */
             bool any = false;
-            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
-                any = any || mixer.channels[c].active;
-            int music_before = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
-            bool effect_before = mixer.channels[effect_channel].active;
+            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c) {
+                const Channel &ch = mixer.channels[c];
+                if (ch.active && ch.sample && ch.sample->frame_count > 0 &&
+                    (ch.loop || ch.cursor < ch.sample->frame_count))
+                    any = true;
+            }
 
             MixBuffer(mixer, stream, CHUNK_FRAMES);
 
-            if (effect_before && !mixer.channels[effect_channel].active) {
-                /* The one-shot's end, named in the sample's own numbers:
-                   it played once, to its end, and its channel is free. */
-                std::printf("engine: loop: the effect ended on channel %d — %d frames in %d buffers, the one-shot played to its end\n",
-                            effect_channel, effect.frame_count,
-                            (effect.frame_count + CHUNK_FRAMES - 1) /
-                                CHUNK_FRAMES);
+            if (feeds == 0) {
+                /* The mix's own bytes while the two effects play: every
+                   output frame is their two contributions added. */
+                std::printf("engine: mix: first frames (summed):");
+                for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
+                    std::printf(" %d", (int)stream[i]);
+                std::printf("\n");
             }
-            if (mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
-                mixer.channels[AUDIO_MUSIC_CHANNEL].cursor < music_before) {
-                /* The wrap: the cursor went backwards — the loop's own
-                   arithmetic, visible from outside the mixer. */
-                music_wraps += 1;
-                int cursor = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
-                std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
-                            AUDIO_MUSIC_CHANNEL, music_wraps,
-                            (long)music_wraps * music.frame_count + cursor,
-                            cursor, music.frame_count);
+
+            /* The one-shot contract's second half, observed: a channel
+               whose sound has played to its end is free again. The run's
+               next effect is fired the moment the pair is done. */
+            if (effect_step == 0 && !mixer.channels[first_channel].active &&
+                !mixer.channels[second_channel].active) {
+                effect_step = 1;
+                std::printf("engine: effect: both ended in %d buffers — channels %d and %d are free again\n",
+                            (effect.frame_count + CHUNK_FRAMES - 1) /
+                                CHUNK_FRAMES,
+                            first_channel, second_channel);
+                third_channel =
+                    MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL);
+                std::printf("engine: mix: effect 3 -> channel %2d (volume %d of %d)%s\n",
+                            third_channel,
+                            mixer.channels[third_channel].volume,
+                            AUDIO_VOLUME_FULL,
+                            third_channel == first_channel
+                                ? " — the pool returned the first effect's channel"
+                                : "");
+            } else if (effect_step == 1 && third_channel >= 0 &&
+                       !mixer.channels[third_channel].active) {
+                effect_step = 2;
+                std::printf("engine: effect: effect 3 ended in %d buffers — channel %d is free again\n",
+                            (effect.frame_count + CHUNK_FRAMES - 1) /
+                                CHUNK_FRAMES,
+                            third_channel);
             }
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
                 if (any)
                     sound_feeds += 1;
-                if (!silence_named && !any) {
-                    /* The silence after the stop, named in the mix's own
-                       numbers: how many buffers carried sound before the
-                       run stopped the last sound. */
+                /* The account closes when the script is done, in the mix's
+                   own numbers: how many buffers carried sound. */
+                if (!silence_named && !any && effect_step == 2) {
                     silence_named = true;
                     std::printf("engine: mix: %d buffers of sound, then silence\n",
                                 sound_feeds);
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — One sample, two volumes *(predict-the-output)*

Per-sound volume means the same sample can be any number of different
sounds. Before running anything, predict the bytes. Fire
`assets/effect.wav` twice at the same instant through
`MixerPlayEffect` — one at volume 192 of 256 and one at volume 48 — on
a fresh mixer, and predict: the channel each effect lands on; the
first eight frames of the mixed buffer, contribution by contribution,
each contribution truncated toward zero before the sum; and what the
mix holds in the buffer after both effects have played to their end,
with the channels' state beside it. Then give the run a probe that
does exactly this on a scratch mixer and reconcile every frame of its
first eight with your numbers.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-067/ex1.md)

### Exercise 2 — The effect that must not stack *(extend-the-code)*

Some sounds are one event: a door asked to slam three times in the
same second should sound like one slam. Make that route real beside
`MixerPlayEffect` — `MixerPlayEffectSolo` — with the same pool, the
same per-sound volume, and one gate before it: if the sample is
already playing on some effect channel, the call refuses, by name,
instead of starting a second copy. Drive three presses of one effect
through it and report each press's channel or its refusal; then run
the same three presses through `MixerPlayEffect` and report the same.
Finish with one paragraph: which route a game's footstep sounds should
take, which route its door slams should take, and what the player
hears under the wrong choice.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-067/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 066 — music as a loop](lesson-066-music.md) ·
**Next:** [Lesson 068 — music and effects together](lesson-068-together.md) ·
**Code tag:** [`lesson-067`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-067)
