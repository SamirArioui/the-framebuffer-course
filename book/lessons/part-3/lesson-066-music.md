# Lesson 066 — music as a loop

{{#include ../../stability-horizon.md}}

## Prose

Lesson 065 reserved a channel and left it empty. This lesson spends the
reservation: the game's music goes on `AUDIO_MUSIC_CHANNEL`, and the
music's most important fact is that it does not end. A sound effect is
a burst that plays to its end and is gone; music is the sound that
repeats — the same bars, over and over, for as long as the run lasts.
The mixer's job for both is the same sum. What differs is one bit of
state per channel, and the whole lesson is what that bit does to a
cursor.

### The loop is the cursor's arithmetic

`Channel` grows one field: `bool loop`. Everything else about playback
— sample, cursor, volume, active — is lesson 063's, untouched. The
wrap lives in the per-frame pull, `ChannelFrame` in `src/audio.cpp`,
and it is one line of arithmetic: when the cursor has reached the
sample's end and the channel loops, the cursor returns to the sample's
first frame and the pull takes that frame. The sample plays again from
its start.

Say this plainly, because it is the point: **looping needs no special
mixer**. There is no loop mode in `MixBuffer`, no second buffer, no
copy of the sample. The wrap is the cursor's own arithmetic inside the
pull — one branch that says where the next frame number comes from —
and the mix above sums the channel's next frame exactly as it always
did. A looping channel and a one-shot channel differ the way two
different cursor sequences differ: in where the numbers go.

The same branch is where lesson 065's behaviour is kept exact. When
`loop` is not set, the pull falls through to the end it always had: the
channel goes inactive at `frame_count`, the fact that says where a
sample ends. A one-shot is not a special case of a loop; a loop is a
channel that declines the ending. And a sample with no frames at all
has nothing to wrap to — the branch's other guard — so it ends here
like any other instead of wrapping forever.

### What makes a loop seamless

A loop is heard over and over, so its seam is heard over and over: a
step at the join is a click on every repeat. Smoothness is not the
mixer's to give — it is the **asset's own property**, and
`assets/music.wav` is built for it. Its melody is twelve notes of a
quarter second over three bass notes of a second, and every note's
frequency completes whole cycles over its own slot and fades to zero on
its own last frame. Every note boundary is therefore silence — and the
loop's boundary is no different: the file's last frame meets its first
as zero meets zero. The seam is not a special place in the waveform; it
is one more place where a note ended.

The run's byte-level check on both sounds, printed before anything
plays:

```
engine: music: 132300 frames at 44100 Hz, 1 channel, peak 10442, first frames: 0 277 554 831 1107 1381 1655 1927, last frame 0
engine: effect: 8820 frames at 44100 Hz, 1 channel, peak 9770, first frames: 0 1229 2438 3607 4719 5757 6703 7544, last frame 0
```

Both files are the format lesson 061 defined and lesson 062's loader
checks: mono, 16-bit signed, 44100 Hz, one `data` chunk of whole
frames. The music is 132300 frames — three seconds, exactly 180
buffers of the run's 735. The effect is 8820 — a fifth of a second,
exactly 12 buffers. Those are the two sounds of a game: one long and
repeating, one short and once.

The peaks are in the same line, and they are the mixing budget. The
music's loudest frame is 10442 and the effect's is 9770 — both well
under the format's 32767, and together at most 20212. That is why
these assets can play at full volume over each other without ever
asking the clamp for help: the sum has room before the limit.

### The music channel, playing

`MixerPlayMusic` in `src/audio.cpp` is the route music takes. It plays
the sample on `AUDIO_MUSIC_CHANNEL` at the given volume and sets the
channel's loop flag — the one thing `MixerPlay` never does. That is
what the reservation was for: the background is one long sound under
everything else, it takes the channel no effect is ever offered, and
nothing steals it. The run starts its music there and its effect
through the pool, and the two routes answer with their channels:

```
engine: mix: music  -> channel  0 (looping, volume 256 of 256)
engine: mix: effect -> channel  1 (one-shot, volume 256 of 256)
engine: mix: the effect plays to its end; the music plays until the run stops it
```

Note what `ChannelPlay` now guarantees on every route: a channel
starts **unlooped**. The loop flag is set where a sound is routed, not
carried over from whatever played on that channel before — a pooled
effect that happens to land on a channel that once played music is a
one-shot, because `MixerPlayEffect` does not ask for a loop.

### Runs until told

A looping channel does not end on its own, so someone must end it, and
that someone is the run: `MixerStop` stops a channel's sound now. The
run's script stops the music at the 600th buffer and the log shows the
whole story in the mixer's own numbers:

```
engine: loop: the effect ended on channel 1 — 8820 frames in 12 buffers, the one-shot played to its end
engine: loop: music wrapped on channel 0 — wrap 1, 133035 frames played, cursor 735 of 132300
engine: loop: music wrapped on channel 0 — wrap 2, 265335 frames played, cursor 735 of 132300
engine: loop: music wrapped on channel 0 — wrap 3, 397635 frames played, cursor 735 of 132300
engine: mix: MixerStop ended the music on channel 0 — 441000 frames in 600 buffers, 3 wraps, cursor 44100 of 132300
engine: mix: 600 buffers of sound, then silence
```

The wraps are observed, not assumed: the run watches the music
channel's cursor, and a cursor going **backwards** is the wrap. Each
wrap line is that observation with the arithmetic that explains it —
`frames played` is the full passes over the loop's 132300 frames plus
the cursor's place in the current one. Wrap 1 reads 133035 frames
played and cursor 735: `1 × 132300 + 735`, and 133035 is 181 buffers of
735 — the wrap is lazy, and happens on the first pull past the sample's
end, one buffer after the cursor sat on 132300 exactly.

The stop line is the claim this run was built to establish. At the
stop, the music had played **441000 frames in 600 buffers** — three and
a third times around its own 132300-frame length — was on its fourth
pass at cursor 44100, and was still active when `MixerStop` ended it.
Long past the sample's length, the channel was exactly where the loop's
arithmetic says it should be, three wraps counted on the way. And the
silence line closes the run's account in the mix's own numbers: 600
buffers carried sound, and then nothing — not because a sample ended,
but because the run stopped the last sound.

### The one-shot beside it, unchanged

The first line of that log is the other half of the contrast. The
effect — the same engine's same channels, the same `MixBuffer` — played
its 8820 frames in 12 buffers and ended. No run decision stopped it;
its sample ended it. That is exactly lesson 065's behaviour with the
loop flag clear, and it is exactly what a sound effect should be:
**plays to its end** where the music is **runs until told**. Music and
effects use the same channels, the same volumes, and the same mix; the
only difference between them is which channel, and whether the cursor
wraps.

### What this run verified, and what it did not

All numbers from real runs of this lesson's end state on this machine,
against ALSA's `null` device:

- **The loop continues and wraps as documented.** Three wraps are
  observed from the cursor's own arithmetic — 133035, 265335, 397635
  frames played — and at the run's scripted stop the music was still
  active, 441000 frames into a 132300-frame sample, cursor 44100. The
  music channel is active long past the sample's length because nothing
  about the sample's length ends it: the cursor wraps and the channel
  plays on.
- **`MixerStop` ends it.** One call at the 600th buffer and the channel
  is inactive: the next buffer carries no sound and the run names the
  silence — 600 buffers of sound, then silence.
- **The seam is checkable in bytes.** The run prints each sound's first
  frames and its last frame: the music's last frame is `0` and its
  first frames start at `0`. Exercise 1 walks the seam buffer by
  buffer.
- **The build carries exactly one warning:** `DrawScene` defined but
  not used in `src/main.cpp` — the Part 2 wart carried forward on
  purpose — and this lesson adds none. `tools/check-boundary.sh` still
  passes: the wrap is arithmetic on an `int` and asks no OS question —
  and `openspec validate --all` reports 11 passed, 0 failed.

What this run cannot verify is the part that needs ears. No speaker has
made a sound — `null` takes the samples and discards them. Whether the
loop's seam is inaudible on real hardware, and whether these notes are
worth hearing twice, are questions for a machine that makes sound;
here the claim stops at the bytes, and the bytes say the seam is zero.

## Code step

One change for this lesson, three files: `src/audio.h` and
`src/audio.cpp` grow the loop — `Channel.loop`, the wrap in the
per-frame pull, `MixerPlayMusic` on the music channel, and `MixerStop`
— and `src/main.cpp` plays the run's two sounds through them, watching
the looping cursor for wraps and stopping the music in front of the
reader. The two new assets ride with the code step and are its other
half: `assets/music.wav`, the loop built so its seam is silence, and
`assets/effect.wav`, the one-shot lesson 067 fires. Their bytes are
binary and so are their diffs — git prints `Binary files … differ` for
them; the run's facts line above is their content in the engine's own
numbers. The mix, the pool, and the allocator are lesson 065's,
untouched. Its end state is tagged `lesson-066`.

```diff
diff --git a/assets/effect.wav b/assets/effect.wav
new file mode 100644
index 0000000..df7300c
Binary files /dev/null and b/assets/effect.wav differ
diff --git a/assets/music.wav b/assets/music.wav
new file mode 100644
index 0000000..bede2fe
Binary files /dev/null and b/assets/music.wav differ
diff --git a/src/audio.cpp b/src/audio.cpp
index abe239b..0849b9d 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -193,6 +193,7 @@ void ChannelPlay(Channel &channel, const Sample &sample, int volume)
     channel.cursor = 0;
     channel.volume = volume;
     channel.active = true;
+    channel.loop = false; /* lesson 066: unlooped unless a route says otherwise */
 }
 
 namespace {
@@ -202,8 +203,26 @@ namespace {
    silence is not a value here, it is the absence of a contribution. */
 int ChannelFrame(Channel &channel)
 {
-    if (channel.active && channel.sample &&
-        channel.cursor < channel.sample->frame_count) {
+    if (channel.active && channel.sample) {
+        /* Lesson 066: the cursor's arithmetic at the sample's end. A
+           looping channel wraps — the cursor returns to the sample's
+           first frame and the pull below takes it again from there — so
+           the sample plays again from its start instead of ending. The
+           wrap is this one line; the mix above never knows it happened.
+           A sample with no frames has nothing to wrap to, and ends here
+           like any other. */
+        if (channel.cursor >= channel.sample->frame_count) {
+            if (channel.loop && channel.sample->frame_count > 0)
+                channel.cursor = 0;
+            else {
+                /* The sample's end — the fact frame_count carries. A
+                   channel that does not loop ends here, exactly as
+                   lesson 065 had it. */
+                channel.active = false;
+                return 0;
+            }
+        }
+
         /* One sample frame, scaled to the channel's volume. A frame is
            sample.channels values wide; the engine's stream is one
            channel wide, so it takes the frame's first value. */
@@ -213,7 +232,7 @@ int ChannelFrame(Channel &channel)
         return (frame * channel.volume) / AUDIO_VOLUME_FULL;
     }
 
-    /* The sample's end — the fact frame_count carries. */
+    /* Silence: the absence of a contribution. */
     channel.active = false;
     return 0;
 }
@@ -231,6 +250,7 @@ void MixerInit(Mixer &mixer)
         mixer.channels[c].cursor = 0;
         mixer.channels[c].volume = 0;
         mixer.channels[c].active = false;
+        mixer.channels[c].loop = false;
         mixer.channels[c].started = 0;
     }
     mixer.order = 0;
@@ -262,6 +282,25 @@ int MixerPlay(Mixer &mixer, const Sample &sample, int volume)
     return oldest;
 }
 
+void MixerPlayMusic(Mixer &mixer, const Sample &sample, int volume)
+{
+    /* The music channel — the reservation of lesson 065, spent here —
+       and the loop flag set: the run stops this sound with MixerStop,
+       the sample never does. */
+    ChannelPlay(mixer.channels[AUDIO_MUSIC_CHANNEL], sample, volume);
+    mixer.channels[AUDIO_MUSIC_CHANNEL].loop = true;
+    mixer.channels[AUDIO_MUSIC_CHANNEL].started = ++mixer.order;
+}
+
+void MixerStop(Mixer &mixer, int channel)
+{
+    /* The channel goes inactive and the mix stops pulling from it. Its
+       cursor keeps the place it stopped at; whatever plays on the channel
+       next starts from the sample's first frame. */
+    if (channel >= 0 && channel < AUDIO_MIXER_CHANNELS)
+        mixer.channels[channel].active = false;
+}
+
 void MixBuffer(Mixer &mixer, short *out, int frame_count)
 {
     for (int i = 0; i < frame_count; ++i) {
diff --git a/src/audio.h b/src/audio.h
index af36e32..e20f6d6 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -74,17 +74,25 @@ constexpr int AUDIO_VOLUME_FULL = 256;
 
 /* Lesson 063: one channel — the unit of playback. What it is playing,
    where it is in the sample, and how loud: three plain values, not a
-   device. A channel plays its sample to its end and then is free again. */
+   device. A channel plays its sample to its end and then is free again.
+   Lesson 066: or it loops — its cursor returns to the sample's first
+   frame at the sample's end and the channel plays on until the run
+   stops it. */
 struct Channel {
     const Sample *sample; /* what it is playing, or 0 */
     int cursor;           /* the next sample frame to read */
     int volume;           /* 0..AUDIO_VOLUME_FULL, fixed point */
     bool active;          /* playing now */
+    bool loop;            /* lesson 066: wrap to the sample's first frame
+                             at its end instead of ending */
     long started;         /* when this channel began, in the mixer's order */
 };
 
 /* Starts `sample` playing on this channel at `volume`, from its first
-   frame. Playing on one channel leaves every other channel alone. */
+   frame. Playing on one channel leaves every other channel alone. The
+   channel starts unlooped: looping is a decision made where a sound is
+   routed — `MixerPlayMusic` makes it — never something carried over from
+   whatever played on the channel before. */
 void ChannelPlay(Channel &channel, const Sample &sample, int volume);
 
 /* The mixer's fixed set of channels. */
@@ -112,6 +120,19 @@ void MixerInit(Mixer &mixer);
    music channel is never stolen. */
 int MixerPlay(Mixer &mixer, const Sample &sample, int volume);
 
+/* Starts `sample` playing as the run's music: on the music channel,
+   looping, at `volume`. That is what the reserved channel was reserved
+   for — a looping sound under everything else, one that no effect ever
+   takes or steals. The music runs until the run stops it with
+   `MixerStop`; its sample never ends it. */
+void MixerPlayMusic(Mixer &mixer, const Sample &sample, int volume);
+
+/* Stops the sound on `channel` now — the run's decision, because a
+   looping channel does not end on its own. The channel keeps the place
+   it stopped at, goes inactive, and whatever plays on it next starts
+   from the sample's first frame. */
+void MixerStop(Mixer &mixer, int channel);
+
 /* The mix: `frame_count` frames of stream, each one the sum of every
    active channel's next frame at its volume, clamped to the format's
    range. Clamped, never wrapped — a sum past the range lands on the
diff --git a/src/main.cpp b/src/main.cpp
index 1e86986..10cdc13 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -57,6 +57,53 @@ static void DrawScene(Framebuffer &fb, const TileMap &map,
     BlitSprite(fb, sprite, sprite_x - x, sprite_y - y);
 }
 
+/* Lesson 066: a loaded sample's facts, printed — the run's byte-level
+   check on its two sounds. The peak is the largest frame the sample
+   holds, and it is what says how much room the format still has above
+   the sound. */
+static void PrintSample(const char *name, const Sample &sample)
+{
+    int peak = 0;
+    for (int i = 0; i < sample.frame_count; ++i) {
+        int v = sample.frames[i * sample.channels];
+        if (v < 0)
+            v = -v;
+        if (v > peak)
+            peak = v;
+    }
+    std::printf("engine: %s: %d frames at %d Hz, %d channel%s, peak %d, first frames:",
+                name, sample.frame_count, sample.rate, sample.channels,
+                sample.channels == 1 ? "" : "s", peak);
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+        std::printf(" %d", (int)sample.frames[i]);
+    std::printf(", last frame %d\n",
+                sample.frame_count ? (int)sample.frames[sample.frame_count - 1]
+                                   : 0);
+}
+
+/* Lesson 066: one asset load's whole failure path — a failed load is
+   named typed and ends the run by name, exactly like the loads above it. */
+static bool LoadRunSample(Arena &arena, const char *path, Sample &into)
+{
+    SampleResult loaded = LoadSample(arena, path);
+    if (loaded.error == SAMPLE_OK) {
+        into = loaded.sample;
+        return true;
+    }
+    switch (loaded.error) {
+    case SAMPLE_MISSING:
+        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
+        break;
+    case SAMPLE_MALFORMED:
+        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
+        break;
+    default:
+        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
+        break;
+    }
+    return false;
+}
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -126,58 +173,48 @@ int Run(void)
     }
     TileSheet &sheet = tiles_loaded.sheet;
 
-    /* Lesson 061: the run's sound as a file's bytes. A sample is frames
-       of amplitude in a container, and the load either yields the
-       complete sample or names what went wrong — like every asset above.
-       A failure ends the run by name, like every asset above. */
-    SampleResult sample_loaded = LoadSample(arena, "assets/tone.wav");
-    if (sample_loaded.error != SAMPLE_OK) {
-        switch (sample_loaded.error) {
-        case SAMPLE_MISSING:
-            std::fprintf(stderr,
-                         "engine: assets/tone.wav: could not load (missing)\n");
-            break;
-        case SAMPLE_MALFORMED:
-            std::fprintf(stderr,
-                         "engine: assets/tone.wav: could not load (malformed)\n");
-            break;
-        default:
-            std::fprintf(stderr,
-                         "engine: assets/tone.wav: could not load (no room)\n");
-            break;
-        }
+    /* Lesson 066: the run's two sounds as files' bytes — the music that
+       loops and the effect that plays once. Lesson 061's tone leaves the
+       run here (it stays on disk: the file lessons 059-065 were built
+       on); the game's own sounds are these two. Each load either yields
+       the complete sample or names what went wrong, and a failure ends
+       the run by name — like every asset above. */
+    Sample music = {}, effect = {};
+    if (!LoadRunSample(arena, "assets/music.wav", music) ||
+        !LoadRunSample(arena, "assets/effect.wav", effect)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
-    Sample &sample = sample_loaded.sample;
-
-    /* The byte-level check, before anything is played: the sample's facts
-       and its first frames — the same bytes lesson 059 computed, now read
-       from a file instead. */
-    std::printf("engine: sample: %d frames at %d Hz, %d channel%s, first frames:",
-                sample.frame_count, sample.rate, sample.channels,
-                sample.channels == 1 ? "" : "s");
-    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
-        std::printf(" %d", (int)sample.frames[i]);
-    std::printf("\n");
 
-    /* Lesson 065: a scripted burst of effects — one more than the pool's
-       effect channels — so the allocation policy runs in front of the
-       reader: the first free channel for each sound, and then the oldest
-       effect channel stolen. Volume is low so sixteen of them still sum
-       inside the format. */
+    /* The byte-level check, before anything is played: each sound's facts,
+       its peak, and its first frames — the same check lesson 061 made on
+       its one file, now on both. */
+    PrintSample("music", music);
+    PrintSample("effect", effect);
+
+    /* Lesson 066: the music as a loop and one effect as a one-shot in the
+       same run — the difference this lesson is about. The music takes the
+       music channel and runs until the run stops it; the effect takes a
+       pool channel and runs to its end. Both are the engine's format and
+       sum through the same mix. */
     Mixer mixer;
     MixerInit(mixer);
-    const int EFFECT_CHANNELS =
-        AUDIO_MIXER_CHANNELS - AUDIO_MUSIC_CHANNEL - 1;
-    for (int i = 0; i <= EFFECT_CHANNELS; ++i) {
-        int ch = MixerPlay(mixer, sample, AUDIO_VOLUME_FULL / 16);
-        std::printf("engine: mix: effect %2d -> channel %2d%s\n", i + 1, ch,
-                    i < EFFECT_CHANNELS ? "" : " (the oldest was stolen)");
-    }
-    std::printf("engine: mix: music channel %d is reserved and was never stolen\n",
-                AUDIO_MUSIC_CHANNEL);
+    MixerPlayMusic(mixer, music, AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: music  -> channel %2d (looping, volume %d of %d)\n",
+                AUDIO_MUSIC_CHANNEL, mixer.channels[AUDIO_MUSIC_CHANNEL].volume,
+                AUDIO_VOLUME_FULL);
+    int effect_channel = MixerPlay(mixer, effect, AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: effect -> channel %2d (one-shot, volume %d of %d)\n",
+                effect_channel, mixer.channels[effect_channel].volume,
+                AUDIO_VOLUME_FULL);
+    std::printf("engine: mix: the effect plays to its end; the music plays until the run stops it\n");
+
+    /* The run's script: the music stops after this many buffers. 600
+       buffers of 735 frames are 441000 frames of music — three and a
+       third times around its 132300-frame loop. The stop is the run's
+       decision; the loop itself would go on. */
+    constexpr int MUSIC_STOP_FEEDS = 600;
 
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
@@ -219,7 +256,7 @@ int Run(void)
            feed — the horizon the paced wait keeps queued. The buffer's
            length in time is the sample's own rate answering. */
         std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
-                    CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / sample.rate);
+                    CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / music.rate);
     }
 
     /* The frame step: read news, update from polled state, feed the
@@ -235,12 +272,15 @@ int Run(void)
        last one — and the loop knows that without asking the platform. */
     double next_feed = platform::Now();
 
-    /* Lesson 062: the channel's place in the sample — how far playback
-       has reached, what has been fed, and whether the end has been
-       named. The sample's frame_count is the fact that says when the
-       sample ends; nothing here assumes how long it is. */
-    int sample_feeds = 0;       /* buffers that carried sound */
-    bool sample_end_named = false;
+    /* Lesson 066: the loop's own bookkeeping, in the run's numbers — how
+       many buffers have been fed and how many carried sound, how far the
+       music has played (its wraps and its cursor say), and whether the
+       silence after the stop has been named. Nothing here assumes how
+       long the loop is. */
+    int feeds = 0;         /* buffers handed to the device */
+    int sound_feeds = 0;   /* buffers that carried sound */
+    int music_wraps = 0;   /* times the looping cursor returned to frame 0 */
+    bool silence_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -344,32 +384,68 @@ int Run(void)
 
            Lesson 064: the stream is the mix. One buffer is every active
            channel's next frames summed and clamped — silence where no
-           channel has anything to say. frame_count is the fact that says
-           when a sample ends; no channel ever runs past it. */
+           channel has anything to say. Lesson 066: frame_count is still
+           the fact that says where a sample ends; a channel that loops
+           wraps there instead of ending, and the mix does not know the
+           difference. */
         double t_audio = platform::Now();
         if (audio.output && t_audio >= next_feed) {
-            /* Did this buffer carry sound? Answered from the cursors: the
-               buffer carried sample frames exactly when some channel's
-               cursor moved during the mix. */
-            long before = 0;
+            /* The run's script, one decision in it: at the 600th buffer
+               the run stops the music. A looping channel does not end on
+               its own, so ending it is the run's call — and this is the
+               call, made in front of the reader. */
+            if (feeds == MUSIC_STOP_FEEDS) {
+                MixerStop(mixer, AUDIO_MUSIC_CHANNEL);
+                std::printf("engine: mix: MixerStop ended the music on channel %d — %d frames in %d buffers, %d wraps, cursor %d of %d\n",
+                            AUDIO_MUSIC_CHANNEL, feeds * CHUNK_FRAMES, feeds,
+                            music_wraps,
+                            mixer.channels[AUDIO_MUSIC_CHANNEL].cursor,
+                            music.frame_count);
+            }
+
+            /* Does this buffer carry sound, and did the music wrap? Both
+               answered from the channels' own state: a channel active when
+               the mix starts speaks in this buffer, and a looping cursor
+               going backwards is the wrap. */
+            bool any = false;
             for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
-                before += mixer.channels[c].cursor;
+                any = any || mixer.channels[c].active;
+            int music_before = mixer.channels[AUDIO_MUSIC_CHANNEL].cursor;
+            bool effect_before = mixer.channels[effect_channel].active;
+
             MixBuffer(mixer, stream, CHUNK_FRAMES);
-            long after = 0;
-            for (int c = 0; c < AUDIO_MIXER_CHANNELS; ++c)
-                after += mixer.channels[c].cursor;
-            bool any = after > before;
+
+            if (effect_before && !mixer.channels[effect_channel].active) {
+                /* The one-shot's end, named in the sample's own numbers:
+                   it played once, to its end, and its channel is free. */
+                std::printf("engine: loop: the effect ended on channel %d — %d frames in %d buffers, the one-shot played to its end\n",
+                            effect_channel, effect.frame_count,
+                            (effect.frame_count + CHUNK_FRAMES - 1) /
+                                CHUNK_FRAMES);
+            }
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
+            }
 
             if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
                 if (any)
-                    sample_feeds += 1;
-                if (!sample_end_named && !any) {
-                    /* The end, named in the mix's own numbers: how many
-                       buffers carried sound before it ran out. */
-                    sample_end_named = true;
+                    sound_feeds += 1;
+                if (!silence_named && !any) {
+                    /* The silence after the stop, named in the mix's own
+                       numbers: how many buffers carried sound before the
+                       run stopped the last sound. */
+                    silence_named = true;
                     std::printf("engine: mix: %d buffers of sound, then silence\n",
-                                sample_feeds);
+                                sound_feeds);
                 }
             } else {
                 /* A device that will not take the samples is named once,
@@ -380,10 +456,11 @@ int Run(void)
                 platform::CloseAudioOutput(audio.output);
                 audio.output = 0;
             }
+            feeds += 1;
             /* The schedule restarts from now, not from the missed slot: a
                long frame is caught up by one buffer, never by a backlog. */
             next_feed = platform::Now() +
-                        (double)CHUNK_FRAMES / (double)sample.rate;
+                        (double)CHUNK_FRAMES / (double)music.rate;
         }
         frame.audio = platform::Now() - t_audio;
 
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The seam, predicted *(predict-the-output)*

A loop's seam is where its last frame meets its first, and this
lesson's buffers make it sit exactly on a buffer boundary. Before
running anything, predict the numbers around it. The run's buffers are
735 frames and the loop is 132300: say after which buffer the cursor
first sits on the sample's end, which buffer's pulls wrap it and where
that buffer leaves the cursor, and what the mixed buffer holds across
the seam — the frames at the end of the buffer before the wrap and at
the start of the buffer after it, for the music playing alone at full
volume. Finish the prediction with the wrap line's own arithmetic: how
`frames played` is made from the wrap count and the cursor. Then give
the run a probe — a scratch mixer fed buffer by buffer for just past
one loop, printing the cursor, the wrap count, and the frames around
the seam — and reconcile every number with the cursor's arithmetic.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-066/ex1.md)

### Exercise 2 — Pause and resume *(extend-the-code)*

`MixerStop` ends a channel's sound and leaves its cursor where it
stopped. A game's pause menu wants something else: `MixerPause` halts
the sound where it is and `MixerResume` continues from exactly there,
without the sound being over. Make both real beside `MixerStop`, with
the sample, the cursor, the volume, and the loop flag all surviving a
pause untouched. Drive the run's music through one on a scratch mixer —
pause it at a known cursor, let buffers run while it is paused, resume
it — and report the cursor at the pause, during it, and after the
resume, plus what the mix held in the paused buffers. Then answer one
question in prose: what does the first-free walk of `MixerPlay` do to a
paused **effect** channel, and is that what a game wants from its pause
button?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-066/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 065 — channel allocation](lesson-065-allocation.md) ·
**Next:** [Lesson 067 — effects as one-shots](lesson-067-effects.md) ·
**Code tag:** [`lesson-066`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-066)
