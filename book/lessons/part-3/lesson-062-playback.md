# Lesson 062 — the sample's playback facts

{{#include ../../stability-horizon.md}}

## Prose

Lesson 061 loaded the sample and vouched for its bytes: 22050 frames of
440 Hz at amplitude 0.25, in the engine's format, checked claim by
claim. This lesson plays them, and its idea fits in one line: **the
playback facts ride with the data**. A sample answers the questions
playback asks — when does it end, what does a frame's index mean in time,
how wide is a frame — from its own fields, and never from assumptions
about the file that produced it. Everything else here is that line
working.

### Played to its end

The stream of lesson 060 had a cursor walking a tone forever. The stream
now is the sample, and it stops. Each feed fills one buffer — the
`stream` buffer, `CHUNK_FRAMES` frames of it — from `sample.frames`
where the sample has frames left, and with **silence** beyond its end.
The fill is bounded by one subtraction and one minimum: what is left is
`sample.frame_count - sample_cursor`, and the feed takes that much or one
buffer's worth, whichever is smaller. When the sample runs out mid-buffer
the rest of the buffer is zeros; when it is long gone the whole buffer
is. Silence is a stream too — the device still gets its buffers, at the
horizon's pace, until the run ends.

The run's own numbers say the rest. When the cursor reaches
`frame_count`, the end is named in the sample's terms:

```
engine: sample: 22050 frames fed in 30 buffers — the sample's end; the stream is silence from here
```

Thirty buffers of 735 frames are 22050 — the sample's frame count, to
the frame. Nothing was read past the end (the fill never leaves the
sample's frames), and nothing was dropped (the count is not 22049 or
22051). The line lands in the run's log with the thirtieth feed, between
the frames the run was on either side of it — thirty horizons of 16.7 ms
after the stream began, which is the half second 22050 / 44100 predicts.
And the run does not stop when the sound does: the log keeps its
`frame N:` lines going to the end of the run with the `audio` phase still
at work in them — the silence buffers keep flowing (this run: 178 frames
in three seconds, every one after the end still feeding).

One arithmetic fact worth naming before an exercise leans on it:
22050 / 735 = 30 exactly is *this asset's* length against *this* buffer
size. It is not a promise the engine makes. A sample of 1000 frames does
not divide into buffers at all; its last feed carries 265 sample frames
and 470 frames of silence. The code does not care — `frame_count` says
when, the fill spends exactly that many frames, and the buffer is always
one horizon deep.

### The facts ride with the data

Look at where the sample's fields are spent in the feed, because that is
the whole argument of this lesson:

- `sample.frame_count` bounds the fill — the sample's *length* is the
  sample's fact, not a constant in the run and not the file's name;
- `sample.rate` turns the buffer into time — the next feed is due one
  `CHUNK_FRAMES / sample.rate` from the last, and the run asks the
  sample how long a buffer is, not the format;
- `sample.channels` is the stride between frames — each frame is that
  many values wide.

The mixer of the next lessons never opens a file. It cannot: a sound is
already in memory when mixing starts, and everything mixing and playback
need came with it. That is what `Sample` was shaped for in lesson 061,
and it is why the struct carries those three fields beside the frames
instead of leaving them implicit in "the engine's format."

The other half of the promise is lesson 061's typed refusal: **a sample
in another format fails typed rather than being played misread.** What a
misread sounds like is worth spelling out, because it is silent in the
logs: that `fmt ` chunk claiming 22050 Hz, played as if it were 44100,
is a note an octave too high for half as long — and it *plays fine*. No
crash, no wrong number in any report; just the wrong sound, everywhere
and forever. The loader refuses such a file at the door, so playback
never even gets the chance to be wrong. The engine's format is not a
preference; it is the condition under which these three fields mean what
the play loop takes them to mean.

And since stopping is now the default: *unless looping* is a policy, not
a fact. Lesson 060's cursor looped the tone forever and the wrap was
seamless — one second of 440 Hz at 44100 is exactly 440 cycles, the wrap
fact of lesson 059. A channel that loops a sample chooses to, and the
seam it gets is its own question.

### The end is not the wrap

Which leaves one small, real number at the boundary. The loop wrapped at
0: the frame just past the tone's end *was* frame 0 again, so nothing
clicked. A stop has no such luck — the sample's last frames are −2032,
−1531, −1024, −513, and silence is 0. The speaker steps from −513 to
rest: about 1.6% of the format's range, a small step, and at most a faint
tick. Whether that tick is audible on real hardware is a question only
hardware can answer; the run's account below is honest about not having
asked it.

### What this run verified, and what it did not

All numbers from real runs of this lesson's end state on this machine,
against ALSA's `null` device:

- **The channel plays exactly the sample's frame count and stops at its
  end.** The run's report: `22050 frames fed in 30 buffers` — 30 × 735 =
  22050 = `frame_count` — and the end named with the feed that reached
  it. The feed before silence is exactly the sample; the feeds after it
  are pure silence.
- **Silence keeps the stream alive.** The three-second run ran 178
  frames and every frame after the sample's end still shows the `audio`
  phase at work — the device kept getting its buffers, one per horizon,
  right up to the close.
- **The typed refusal is unchanged** — a missing or malformed file still
  ends the run by name, lesson 061's runs re-ran against this state
  unmodified (the loader is untouched by this lesson).

What this run cannot verify is the part that needs ears: **no speaker has
made a sound** — `null` takes the samples and discards them. "It played
for half a second and stopped clean" is a claim about your machine and
your ears, and exercise 2 is where it gets made. This lesson claims the
numbers, and the numbers say the sample's frames were spent exactly.

## Code step

One change for this lesson, and one file: `src/main.cpp` replaces lesson
060's tone cursor with the sample. The feed fills one buffer of stream
from `sample.frames` — bounded by `sample.frame_count`, silencing
whatever the sample does not cover — and names the sample's end in its
own numbers when it gets there. The gate, the `audio` phase, the paced
wait, and the horizon are exactly as lesson 060 left them; what changes
is what the stream *is*. The computed tone leaves the run —
`GenerateTone` stays in `src/audio.cpp` as the worked example that
authored `assets/tone.wav` in the first place. Its end state is tagged
`lesson-062`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index e46c5bd..fbfd37d 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -29,21 +29,21 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
-/* Lesson 059: the run's sound is half a second of tone — 22050 frames.
-   One second of a 440 Hz tone is exactly 440 cycles at AUDIO_RATE, so
-   this buffer holds a whole 220 cycles and its end meets its beginning
-   with no click: the wrap lesson 059 found, which lesson 060's feeding
-   cursor leans on when it hands the same run of frames to the device
-   again and again. */
-constexpr int TONE_FRAMES = AUDIO_RATE / 2; /* 22050 */
-
 /* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
-   the horizon the loop keeps queued. The cursor relies on the tone's
-   length dividing evenly into buffers: 22050 / 735 = 30 exactly, so it
-   wraps at a buffer boundary and no feed ever has to copy across the
-   tone's end. */
+   the horizon the loop keeps queued. Lesson 062: a feed is always
+   exactly this much stream — the sample's frames where the sample has
+   them, silence beyond its end — so the horizon arithmetic is untouched
+   whatever the sample's length is. The sample's own length is the file's
+   fact: playback stops where its frame_count says it stops, not where a
+   constant here would. */
 constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */
 
+/* Lesson 062: the buffer of stream one feed hands the device, filled
+   from the sample (or with silence) as the feed is due. Static, like the
+   platform layer's own staging buffers — the language law of lesson 026
+   keeps allocation out of the run. */
+static short stream[CHUNK_FRAMES];
+
 /* Lesson 054: the scene, drawn through the camera. The camera's summed
    offset is applied once, at each draw's origin — the map's and the
    sprite's. The HUD is not scene and does not pass through here. */
@@ -177,16 +177,14 @@ int Run(void)
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
-    /* Lesson 059: the run's sound. A sample is a frame of amplitude at
-       the engine's rate — here a tone computed by code instead of read
-       from a file — and the seam's audio output is what puts those frames
-       in front of a device. Lesson 060: those frames are a *stream*, not
-       one submission done at startup — the frame loop feeds the device
-       buffer by buffer, for as long as the run lasts. */
+    /* Lesson 059: the run's sound is a run of amplitude at the engine's
+       rate, and the seam's audio output is what puts those frames in
+       front of a device. Lesson 062: the frames are the sample loaded at
+       startup — a file's bytes, played to their end — and the loop feeds
+       them as the stream lesson 060 shaped: buffer by buffer, at the
+       horizon's pace, silence once the sample is done. */
     platform::AudioResult audio =
         platform::OpenAudioOutput(AUDIO_RATE, AUDIO_OUTPUT_CHANNELS);
-    short *tone = 0;
-    int tone_cursor = 0; /* where the stream's next buffer starts */
     if (!audio.output) {
         switch (audio.error) {
         case platform::AUDIO_NO_DEVICE:
@@ -200,26 +198,11 @@ int Run(void)
            still runs — this one continues without sound. */
         std::fprintf(stderr, "engine: continuing without sound\n");
     } else {
-        tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
-                                   sizeof(short));
-        if (!tone) {
-            std::fprintf(stderr, "engine: no room for the tone\n");
-        } else {
-            GenerateTone(tone, TONE_FRAMES, 440.0, 0.25);
-
-            /* The bytes are checkable before they are audible: the first
-               frames of the tone, in the engine's own format. */
-            std::printf("engine: tone: %d frames at %d Hz, first frames:",
-                        TONE_FRAMES, AUDIO_RATE);
-            for (int i = 0; i < 4; ++i)
-                std::printf(" %d", (int)tone[i]);
-            std::printf("\n");
-
-            /* And what the loop does with them: one buffer of stream per
-               feed — the horizon the paced wait keeps queued. */
-            std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
-                        CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / AUDIO_RATE);
-        }
+        /* What the loop does with the sample: one buffer of stream per
+           feed — the horizon the paced wait keeps queued. The buffer's
+           length in time is the sample's own rate answering. */
+        std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
+                    CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / sample.rate);
     }
 
     /* The frame step: read news, update from polled state, feed the
@@ -234,6 +217,15 @@ int Run(void)
        the engine's rate, so the next buffer is due one horizon from the
        last one — and the loop knows that without asking the platform. */
     double next_feed = platform::Now();
+
+    /* Lesson 062: the channel's place in the sample — how far playback
+       has reached, what has been fed, and whether the end has been
+       named. The sample's frame_count is the fact that says when the
+       sample ends; nothing here assumes how long it is. */
+    int sample_cursor = 0;      /* the next frame the feed takes */
+    int sample_fed = 0;         /* frames of sample handed to the device */
+    int sample_feeds = 0;       /* feeds that carried sample frames */
+    bool sample_end_named = false;
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -333,12 +325,40 @@ int Run(void)
            extra audio, or the run would bury the device in buffers instead
            of pacing them. The step is measured on every frame — it is ~0
            where no buffer was due — so the phase accounts for all of the
-           frame's audio work. */
+           frame's audio work.
+
+           Lesson 062: the stream is the sample. One buffer is filled from
+           the sample's frames where the sample has them and with silence
+           beyond its end — silence is a stream too, and the device keeps
+           getting its buffers. frame_count is the fact that says when the
+           sample ends; the fill never runs past it. */
         double t_audio = platform::Now();
-        if (audio.output && tone && t_audio >= next_feed) {
-            if (platform::SubmitSamples(audio.output, tone + tone_cursor,
+        if (audio.output && sample.frames && t_audio >= next_feed) {
+            /* Each frame is sample.channels values wide — one here, the
+               loader refuses anything else — and the engine's stream is
+               one channel wide, so a frame is its first (only) channel. */
+            int left = sample.frame_count - sample_cursor;
+            int take = left < CHUNK_FRAMES ? left : CHUNK_FRAMES;
+            for (int i = 0; i < take; ++i)
+                stream[i] =
+                    sample.frames[(sample_cursor + i) * sample.channels];
+            for (int i = take; i < CHUNK_FRAMES; ++i)
+                stream[i] = 0;
+
+            if (platform::SubmitSamples(audio.output, stream,
                                         CHUNK_FRAMES)) {
-                tone_cursor = (tone_cursor + CHUNK_FRAMES) % TONE_FRAMES;
+                sample_cursor += take;
+                sample_fed += take;
+                if (take > 0)
+                    sample_feeds += 1;
+                if (!sample_end_named &&
+                    sample_cursor == sample.frame_count) {
+                    /* The end, named in the sample's own numbers: what was
+                       fed before silence, and how many buffers carried it. */
+                    sample_end_named = true;
+                    std::printf("engine: sample: %d frames fed in %d buffers — the sample's end; the stream is silence from here\n",
+                                sample_fed, sample_feeds);
+                }
             } else {
                 /* A device that will not take the samples is named once,
                    not once per frame: the run closes the output and carries
@@ -351,7 +371,7 @@ int Run(void)
             /* The schedule restarts from now, not from the missed slot: a
                long frame is caught up by one buffer, never by a backlog. */
             next_feed = platform::Now() +
-                        (double)CHUNK_FRAMES / (double)AUDIO_RATE;
+                        (double)CHUNK_FRAMES / (double)sample.rate;
         }
         frame.audio = platform::Now() - t_audio;
 
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The sample that is not thirty buffers *(predict-the-output)*

The run's numbers are suspiciously round: 22050 frames, 30 buffers — the
sample's length divides evenly into the horizon, and nothing in playback
may rely on that. Prove it. Cut `assets/tone.wav` down to 1000 frames —
shorten its `data` chunk and fix both the `data` size and the RIFF size,
so the container is well-formed but short (lesson 061's byte edits).
Before you run anything, predict the whole feed sequence: how many feeds
carry sample frames and how many frames each carries, what the second
feed's buffer holds frame by frame, exactly what the end report will
say, and how many sample frames the device gets in total. Then give the
feed a probe — one line per buffer, naming how many frames came from the
sample and how many from silence — run against your short file, and
reconcile every number with `frame_count`, the fill, and `CHUNK_FRAMES`.
Finish with the question the tail buffer answers: what would a one-line
feed of `CHUNK_FRAMES` frames from `sample.frames + cursor` have done to
this file instead?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-062/ex1.md)

### Exercise 2 — Hear it stop *(port-to-your-own-machine)*

Every check in this lesson ran against ALSA's `null` device — the device
that takes samples and discards them — so the one thing the numbers
cannot say is what the *stop* sounds like. Take the demo to a machine
with real sound hardware and answer it: the tone plays once, half a
second of 440 Hz, and stops while the run keeps ticking and feeding
silence. Have the run account for the stream at its close — the sample's
frames and buffers, and what was fed after the end — so your log carries
the evidence beside your ears; then report the device, what you heard
(the pitch, the duration, the stop — clean or a tick), and how the half
second compares with thirty buffers at 16.7 ms.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-062/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 061 — the WAV container](lesson-061-wav.md) ·
**Next:** [Lesson 063 — one channel](lesson-063-channel.md) ·
**Code tag:** [`lesson-062`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-062)
