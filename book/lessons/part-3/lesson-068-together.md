# Lesson 068 — music and effects together

{{#include ../../stability-horizon.md}}

## Prose

Lesson 066 gave the game its music — one sound that runs until told to
stop. Lesson 067 gave it effects — sounds that play once and give their
channels back. This lesson is the two of them in one run, at the same
time, through the same mixer. Nothing new is built here, and that is
the point: **this is the MVD's audio line — obligation O7 — delivered.**
The game's sound is music and effects routed as channels through one
mixer, summed by one `MixBuffer` into one stream. Part 5's integration
lesson wires this to the game's events. It does not redesign it.

### One mixer, one mix, one format

The run starts its music on the music channel, looping, at full volume,
and fires effects over it on the pool's channels at a quarter volume
each. The log names every route and every landing:

```
engine: mix: music   -> channel  0 (looping, volume 256 of 256)
engine: mix: effect  1 -> channel  1 (volume 64 of 256)
engine: mix: first frames (music + effect 1, summed): 0 584 1163 1732 2286 2820 3330 3813
engine: mix: effect  2 -> channel  2 (volume 64 of 256)
engine: mix: effect  3 -> channel  3 (volume 64 of 256)
engine: mix: effect  4 -> channel  1 (volume 64 of 256)
```

The mixed frames are the lesson in one line. Every output frame is the
music's next frame plus the effect's next frame at its volume — 277 and
307 make 584; 554 and 609 make 1163; 831 and 901 make 1732 — each
contribution truncated before the sum, lesson 064's order, exactly as
when only one kind of sound plays. The music's first frames and the
effect's are both in this lesson's facts lines; the sum above is
reconcilable frame by frame from them.

Look at the channels and see the two contracts at work at once. The
burst of effects takes the pool's first free channels — 1, 2, 3 — and
then gives them back: effect 4 lands on channel 1 again, the pool
returned it. Through all of it channel 0 is never offered, never
stolen, never disturbed. That is lesson 065's reservation under load,
holding exactly as promised.

### Nothing in the mix knows which is which

Here is the sentence this batch has been building toward: **music and
effects differ only in which channel they are on and whether their
cursor wraps.** Music is a channel with the loop flag set on the
reserved channel; an effect is a channel with the loop flag clear on a
pooled one. The mix pulls one frame per channel per output frame and
adds them up — it has no idea which contribution came from a melody and
which from a footstep. Volumes, clamping, the fixed pool, the steal
policy: all of it is shared, all of it already taught. A "music mixer"
and an "effects mixer" would be two machines to build, feed, and keep
in step; this engine has one, and one is enough.

The formats agree too, and this is not a coincidence. `assets/music.wav`
and `assets/effect.wav` are the same format — mono, 16-bit signed, at
`AUDIO_RATE`, in the container lesson 061 defined — because the mixer
sums samples and the format is one. There is no conversion anywhere in
the sound path: the loader refuses anything else typed, so what the mix
adds together is always the same kind of number.

### The music under the effects

The run keeps watching the looping cursor while the effects fire, and
the wrap lines show the two rhythms interleaving:

```
engine: loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300
engine: mix: effect 13 -> channel  1 (volume 64 of 256)
...
engine: loop: music wrapped on channel 0 — wrap 2, 265335 frames played, cursor 735 of 132300
...
engine: loop: music wrapped on channel 0 — wrap 3, 397635 frames played, cursor 735 of 132300
```

Twenty-nine effects fire in the quoted run — a burst of six, then one
a second — and the music wraps through all of them on its own
arithmetic: 133035, 265335, 397635 frames played, three full passes of
its 132300-frame loop and 735 frames into a fourth, cursor 735 at each
observed wrap. Not one effect touched channel 0 and not one changed the
music's cursor. The busiest second of this run is exactly the second
lesson 065 said the music has to survive, and it does, by construction:
the pool never had the option.

### The mix's cost, in the frame record

The `audio` phase has been measured inside every frame since lesson
060; with both kinds of sound playing it carries the whole sound path —
the mix and its submit. From the quoted run's frame log:

```
frame 1: update 0.001 ms, audio 0.033 ms, render 1.632 ms (sprites 0.001, text 0.006, tilemap 0.936), present 0.461 ms, total 2.127 ms
frame 2: update 0.001 ms, audio 0.034 ms, render 1.368 ms (sprites 0.001, text 0.008, tilemap 0.945), present 0.598 ms, total 2.001 ms
frame 4: update 0.001 ms, audio 0.072 ms, render 1.742 ms (sprites 0.001, text 0.007, tilemap 0.970), present 0.768 ms, total 2.583 ms
```

Over the run's 702 frames the phase averages 0.036 ms — floor 0.025,
worst 0.331 — against a 2.082 ms average frame. Reconcile that against
the work: one buffer is 735 output frames and the mixer walks all
`AUDIO_MIXER_CHANNELS` = 16 of them per output frame, so a buffer is
11 760 channel pulls plus 735 clamps and writes. 0.036 ms over that is
about 3 ns per channel pull and about 50 ns per output frame. The mix's
cost is **linear in the channel count and invisible in the frame** —
under two percent of an average frame at sixty frames a second. Every
frame of this run fed one buffer — the paced wait wakes the loop at the
buffer horizon — so there were no idle audio frames to compare against;
a frame that mixes nothing is essentially the two clock readings around
a test. On this machine the submit returns immediately (`null` takes
the samples and drops them), so the phase reads as the mix's cost; on
real hardware a submit can wait for room in the device's buffer, the
same way `present` carries the copy's sync. Exercise 1 makes the
measurement yours, on your machine.

### What this run verified, and what it did not

All numbers from real runs of this lesson's end state on this machine,
against ALSA's `null` device:

- **Music and effects mix together as documented.** The music loops on
  channel 0 while effects fire on channels 1, 2, 3 — and back to 1 as
  the pool returns it — and the mixed buffer's first frames are the two
  sounds' frames added: `0 584 1163 1732 2286 2820 3330 3813`,
  reconcilable from the facts lines to the digit. One stream, one mix,
  one format.
- **The music survives the busy second.** Three wraps are observed —
  133035, 265335, 397635 frames played — across twenty-nine effects,
  and channel 0 is never taken or stolen.
- **The format leaves the clamp out of it.** The busiest moment the
  script reaches — the music's peak 10442 plus three effects at a
  quarter of their peak 9770 — sums to at most 17768 of 32767. The
  clamp is there for when a game asks for more than the format holds;
  this run never asks.
- **The frame record measures it.** The `audio` phase — in the log
  since lesson 060 — averages 0.036 ms across 702 frames, reconciling
  with the buffer size and channel count at about 3 ns per channel
  pull, and about 1.7% of the average frame.
- **The build carries exactly one warning:** `DrawScene` defined but
  not used in `src/main.cpp` — the Part 2 wart carried forward on
  purpose — and this lesson adds none. `tools/check-boundary.sh` still
  passes: the run grew no OS question — and `openspec validate --all`
  reports 11 passed, 0 failed.

What this run cannot verify is the part that needs ears. No speaker has
made a sound — `null` takes the samples and discards them. Whether a
quarter-volume effect reads over full-volume music, and what the
combination is worth listening to, are questions for hardware that
makes sound. The bytes say the sum is what it should be; the hearing is
yours.

## Code step

One change for this lesson, one file: `src/main.cpp` — the run plays
the game's sound as the game has it, the music looping on the music
channel and effects firing over it on the pool's channels, every one of
them summed by the same `MixBuffer` into the one stream, with the
first-frames check and the wrap watch that make the togetherness
checkable. The mixer, the routes, and the assets are lessons 065-067's,
untouched: this lesson wires nothing new — it runs what is already
there, together. Its end state is tagged `lesson-068`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 9f825c3..a8264b5 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -193,28 +193,22 @@ int Run(void)
     PrintSample("music", music);
     PrintSample("effect", effect);
 
-    /* Lesson 067: the effect route's contract, in front of the reader.
-       Two effects of one sample at different volumes, fired together —
-       one sample, two different sounds — and then one more after they
-       have played to their end, so the pool's answer is visible: the
-       channel the first effect had comes back. The music is loaded and
-       silent here; lesson 068 starts it. */
+    /* Lesson 068: the game's sound as the game has it — the music
+       looping on the music channel and effects firing over it on the
+       pool's channels, every one of them summed by the same MixBuffer
+       into the one stream. Nothing in the mix knows which is which. */
     Mixer mixer;
     MixerInit(mixer);
-    int first_channel = MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: effect 1 -> channel %2d (volume %d of %d)\n",
-                first_channel, mixer.channels[first_channel].volume,
+    MixerPlayMusic(mixer, music, AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: music   -> channel %2d (looping, volume %d of %d)\n",
+                AUDIO_MUSIC_CHANNEL, mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
                 AUDIO_VOLUME_FULL);
-    int second_channel = MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL / 4);
-    std::printf("engine: mix: effect 2 -> channel %2d (volume %d of %d)\n",
-                second_channel, mixer.channels[second_channel].volume,
-                AUDIO_VOLUME_FULL);
-    std::printf("engine: mix: one sample, two volumes — two different sounds\n");
 
-    /* The script's one decision: when both effects have played to their
-       end, fire one more. Its channel is the pool's own answer. */
-    int third_channel = -1;
-    int effect_step = 0;
+    /* The run's rhythm: a burst of effects at the start — up to three in
+       flight — then one a second, all at a quarter volume so the music
+       and the busiest moment still sum inside the format. */
+    int effect_count = 0;
+    int music_wraps = 0;
 
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
@@ -272,13 +266,11 @@ int Run(void)
        last one — and the loop knows that without asking the platform. */
     double next_feed = platform::Now();
 
-    /* Lesson 067: the run's account of its sounds, in the mix's own
-       numbers — how many buffers have been fed and how many carried
-       sound, and whether the final silence has been named. Nothing here
-       assumes how long a sound is. */
+    /* Lesson 068: the run's bookkeeping — how many buffers have been
+       handed to the device, which drives the rhythm above. The mix's own
+       account of what it carried is the frame record's audio phase now,
+       measured like every other phase of the frame. */
     int feeds = 0;         /* buffers handed to the device */
-    int sound_feeds = 0;   /* buffers that carried sound */
-    bool silence_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -388,69 +380,49 @@ int Run(void)
            difference. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            /* Did this buffer carry sound? A channel speaks in it exactly
-               when it is active and has frames left to give: a one-shot at
-               its end gives none, and a looping channel at its end gives
-               everything again. */
-            bool any = false;
-            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c) {
-                const Channel &ch = mixer.channels[c];
-                if (ch.active && ch.sample && ch.sample->frame_count > 0 &&
-                    (ch.loop || ch.cursor < ch.sample->frame_count))
-                    any = true;
+            /* The run's rhythm, in the game's own terms: an effect every
+               fifth buffer through the opening burst, then one every
+               second — each one a MixerPlayEffect on the pool's channels,
+               over the music that keeps looping. */
+            bool fire = (feeds < 30 && feeds % 5 == 0) ||
+                        (feeds >= 30 && feeds % 30 == 0);
+            if (fire) {
+                int ch = MixerPlayEffect(mixer, effect,
+                                         AUDIO_VOLUME_FULL / 4);
+                effect_count += 1;
+                std::printf("engine: mix: effect %2d -> channel %2d (volume %d of %d)\n",
+                            effect_count, ch, mixer.channels[ch].volume,
+                            AUDIO_VOLUME_FULL);
             }
 
+            int music_before = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+
             MixBuffer(mixer, stream, CHUNK_FRAMES);
 
             if (feeds == 0) {
-                /* The mix's own bytes while the two effects play: every
-                   output frame is their two contributions added. */
-                std::printf("engine: mix: first frames (summed):");
+                /* The mix's own bytes with both kinds of sound in it:
+                   every frame is the music's and the effect's next frame
+                   added — the same sum either way. */
+                std::printf("engine: mix: first frames (music + effect 1, summed):");
                 for (int i = 0; i < 8 && i < CHUNK_FRAMES; ++i)
                     std::printf(" %d", (int)stream[i]);
                 std::printf("\n");
             }
 
-            /* The one-shot contract's second half, observed: a channel
-               whose sound has played to its end is free again. The run's
-               next effect is fired the moment the pair is done. */
-            if (effect_step == 0 && !mixer.channels[first_channel].active &&
-                !mixer.channels[second_channel].active) {
-                effect_step = 1;
-                std::printf("engine: effect: both ended in %d buffers — channels %d and %d are free again\n",
-                            (effect.frame_count + CHUNK_FRAMES - 1) /
-                                CHUNK_FRAMES,
-                            first_channel, second_channel);
-                third_channel =
-                    MixerPlayEffect(mixer, effect, AUDIO_VOLUME_FULL);
-                std::printf("engine: mix: effect 3 -> channel %2d (volume %d of %d)%s\n",
-                            third_channel,
-                            mixer.channels[third_channel].volume,
-                            AUDIO_VOLUME_FULL,
-                            third_channel == first_channel
-                                ? " — the pool returned the first effect's channel"
-                                : "");
-            } else if (effect_step == 1 && third_channel >= 0 &&
-                       !mixer.channels[third_channel].active) {
-                effect_step = 2;
-                std::printf("engine: effect: effect 3 ended in %d buffers — channel %d is free again\n",
-                            (effect.frame_count + CHUNK_FRAMES - 1) /
-                                CHUNK_FRAMES,
-                            third_channel);
+            if (mixer.channels[AUDIO_MUSIC_CHANNEL].active &&
+                mixer.channels[AUDIO_MUSIC_CHANNEL].cursor < music_before) {
+                /* The wrap: the cursor went backwards — the loop's own
+                   arithmetic, visible from outside the mixer. */
+                music_wraps += 1;
+                int cursor = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+                std::printf("engine: loop: music wrapped on channel %d — wrap %d, %ld frames played, cursor %d of %d\n",
+                            AUDIO_MUSIC_CHANNEL, music_wraps,
+                            (long)music_wraps * music.frame_count + cursor,
+                            cursor, music.frame_count);
             }
 
-            if (platform::SubmitSamples(audio.output, stream,
-                                        CHUNK_FRAMES)) {
-                if (any)
-                    sound_feeds += 1;
-                /* The account closes when the script is done, in the mix's
-                   own numbers: how many buffers carried sound. */
-                if (!silence_named && !any && effect_step == 2) {
-                    silence_named = true;
-                    std::printf("engine: mix: %d buffers of sound, then silence\n",
-                                sound_feeds);
-                }
-            } else {
+            if (!platform::SubmitSamples(audio.output, stream,
+                                         CHUNK_FRAMES)) {
                 /* A device that will not take the samples is named once,
                    not once per frame: the run closes the output and carries
                    on in silence — its wait unbounded again. */
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The audio phase, measured *(measure-the-performance)*

The `audio` phase has been in the frame record since lesson 060, and
this lesson's run mixes music and effects through it. Measure what the
mix costs, on your machine. From a real run of this lesson's end state,
take the `frame N:` log's `audio` numbers and report their floor, their
average, and their worst — and what the phase costs on a frame that fed
a buffer versus one that did not (if your run has none of the second
kind, say why, from the paced wait). Then reconcile the cost of a
feeding frame against the work one buffer is: 735 output frames times
the channels the mixer walks — per output frame and per channel frame.
Finish with the budget's question: is the mix's cost visible in the
average frame at sixty frames a second, and what would have to happen
for it to become visible?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-068/ex1.md)

### Exercise 2 — The music that ducks *(extend-the-code)*

When a loud effect fires over music, games lower the music for a moment
and bring it back — the duck. Make it real beside this lesson's run:
when an effect starts, the music channel's volume drops to half its own
for ten buffers and then climbs back to where it was over ten more.
Keep the arithmetic in the mixer's fixed-point volumes and leave the
routes and `MixerStop` untouched. Drive one duck on a scratch mixer and
report the music channel's volume at every buffer of it — the drop, the
floor, the climb — and, from the mix's own bytes, one frame of the
music at the duck's floor beside the same frame at full volume. Finish
with one sentence naming what the duck costs the mix.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-068/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 067 — effects as one-shots](lesson-067-effects.md) ·
**Next:** [Lesson 069 — the closing demo](lesson-069-demo.md) ·
**Code tag:** [`lesson-068`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-068)
