# Lesson 060 — the stream's shape

{{#include ../../stability-horizon.md}}

## Prose

Lesson 059 made sound out of numbers and handed half a second of them to
a device, once. This lesson has exactly two ideas, and together they are
the shape of every sound this engine will ever play: **the output's
format and rate**, and **the loop that feeds it — the paced wait that
keeps the device from starving**. The first says what a sound *is* on its
way out of the engine; the second says what a run owes a device while it
does everything else a run does.

### A stream, not a submission

The output is a **stream**: a fixed format and a fixed rate, agreed by
the seam. Not "whatever the device likes" — the contract is the engine's
own samples, lesson 059's format through and through: `AUDIO_RATE` is
44100 frames a second, the engine's mix is one channel wide
(`AUDIO_OUTPUT_CHANNELS`), one frame one `short`. What a particular
machine's device wants instead is the platform's business, and lesson
059's mapping is where it happens — each mono frame duplicated across
the device's stereo interleaved layout inside the ALSA file, exactly as
`Present` maps the framebuffer's pixels into the window's. The device's
own shape never crosses the seam; the stream's shape is the *engine's*.

The rate is the half of the contract that matters here. 44100 frames
make one second — frame `i` is heard `i / 44100` seconds in — and a
device consumes at exactly that rate, frame after frame, as the clock
turns. Hand it half a second of samples and it is busy for half a
second; after that it has nothing. Which is what lesson 059's demo
actually did: one submission at startup (`engine: tone played`), then
silence. A buffer left behind is not a stream. A stream is the same run
of frames handed over *again and again* — a cursor walking the tone
buffer, each feed taking the next `CHUNK_FRAMES` frames.

Two divisibility facts make that walk seamless, and both are recorded
beside the constants the cursor relies on:

- `TONE_FRAMES / CHUNK_FRAMES` is 22050 / 735 = 30 exactly, so the
  cursor wraps at a buffer boundary and no feed ever has to copy across
  the tone's end — the walk is one pointer add and one modulus.
- one second of a 440 Hz tone is exactly 440 cycles — lesson 059's wrap
  fact — so where the cursor wraps, the tone's end meets its beginning
  with no click.

Neither fact is luck: 44100 divides by 60 and by 2, so the buffer
boundary is a real arithmetic fact and not an approximation; and 440 Hz
is 440 whole cycles every whole second, so half a second is 220 — which
is why the wrap closes. Change the tone's frequency or the buffer size
carelessly and the first fact is what breaks — a cursor that wraps
mid-buffer needs a split copy, and a tone whose cycles do not close
clicks at the join.

### Starvation

A device consumes at its own rate. The run does not: it updates the
world, draws, presents, waits for news. If the run goes away and does
other work for longer than the queued samples last, the device runs out
of samples and the sound stutters or stops. That is **starvation**, and
it is not a defect of the device — it is arithmetic. Samples are
consumed at 44100 a second, so a queue's length is a *time*: one buffer
of 735 frames is 735 / 44100 = one sixtieth of a second, and no more.

So the loop must keep buffers queued: feed the device on a schedule,
before the queue runs dry. And "on a schedule" is where sound changes
this engine.

### The paced wait

The fix has two halves, and only one of them is visible in `main.cpp`.
The loop **feeds** the device buffers on a schedule — and the run's wait
for news **may no longer block indefinitely**. `PumpEvents` has slept
since lesson 028 inside a wait that ends on news or on the interrupt;
with an output open it must also end when the queued samples are about to
run out. The wait is bounded by how long the queued samples will last —
this lesson's **buffer horizon** — and it is sound that forces the
change. Sound is the first thing in this engine that needs the loop to
wake up *on a schedule*; everything before it could sleep between news
forever.

The engine does not know any of this, and that is deliberate. The bound
is the platform layer's own state. When `SubmitSamples` hands the device
frames, the ALSA file records when the device will have consumed what it
was given — a deadline on the platform clock, accumulated so overlapping
submits can only move it later:

```
deadline = (deadline > now ? deadline : now) + frames_taken / rate
```

`deadline > now` keeps the end of whatever backlog is still playing; `now`
is where consumption starts when the device has already caught up. And
`AudioWaitSeconds` reports the time left until that deadline. It lives in
`src/platform_internal.h`, a header that is explicitly **not the seam's
contract** — `platform.h` stays the engine's whole view of the OS, a
second OS owes exactly what that header declares and nothing more, and
nothing in the internal header names an OS. `PumpEvents` in the X11 file
turns the answer into the wait's timeout: negative keeps the old
unbounded wait exactly as it was, anything else bounds `poll`. The wait
ends early because the output needs its next buffer, not because
anything happened. The interrupt fold, the event drain, all of it is
unchanged.

The other half of the fix is the frame loop's new **audio step**, and it
is gated: the loop hands the device the next buffer of the stream only
when the buffer is *due* — the run keeps its own schedule, one horizon
per buffer, and a frame that arrives early finds nothing to do. Input
news can wake a frame early — a frame woken early must not queue extra
audio, or the run would bury the device in buffers instead of pacing
them. This is exactly what a scripted-input run shows: driving the
window with key events for three seconds, 218 frames ran but only 175 of
them fed — the other 43 woke on news and logged `audio 0.000 ms`, the
gate closed. Feeds stayed at the horizon's rate (58 per second) no
matter how busy the input made the loop. The queue stays one buffer
deep; that is the whole point of pacing.

### The audio phase

The frame pays for all of this, so the frame record grows a field:
`FrameRecord.audio`, beside `update`, `render` and `present` — the run's
audio step, mixing and submitting the stream. `FrameStats` sums it like
every other phase, the step is timed on every frame (it reads ~0 where no
buffer was due), and the `frame N:` log line carries it in the record's
own order: `update`, `audio`, `render (sprites, text, tilemap)`,
`present`, `total`. From this lesson on, lesson 058's closing arithmetic
has four terms and the record carries all four.

What deliberately does *not* change is the frame-budget table. Its audio
row is **lesson 070's**, with the sound rows that lesson grows; between
here and there the table's rows cover `update + render + present` and not
the whole frame, and 070 is where that gap closes. The phase is measured
from now on precisely so that the row arrives as a measured sum when it
comes — the same early-measure discipline the render sub-phases got.

Lesson 042's boundary check keeps all of this honest, and it still
prints its account unchanged — `platform_internal.h` includes no OS
header and calls no OS function:

```
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp src/platform_alsa.cpp
boundary: 17 single-line declarations there (multi-line ones are in the header)
```

### What an idle run now costs

State the trade-off plainly, because the design records it as a risk: an
idle run with an output open **ticks at the buffer horizon's cadence
rather than sleeping between news**. Measured here: an idle three-second
run against the `null` device ran 176 frames — frames 1 through 176,
about one every 17 ms against the horizon's 16.7 ms, roughly sixty
frames a second where the same run before sound would have run one frame
and then waited (a repeat run ticks at the same cadence with 175 — the
cadence is the claim, not the count). The wake lands just past the
deadline because the wait rounds up to whole milliseconds; the next feed
is due immediately and the cycle repeats.

That is the price of feeding the device without threads: the run wakes
on the output's schedule even when nothing else happens, and pays a
frame's worth of update/render/present per buffer. On this machine the
frame is cheap (~2 ms of work against a 16.7 ms horizon); a heavier
engine would want bigger buffers and a longer horizon, which is the knob
exercise 1 turns. With no output open, nothing changes at all — the wait
is the old unbounded one and the run sleeps between news exactly as it
did before sound.

### No threads

The language law of lesson 026 has never admitted threads, and this
lesson does not smuggle one in. This is one loop, synchronous calls, and
memory from the arena — the `AudioWaitSeconds` bound is what makes that
economically possible at all. The obvious alternatives were rejected for
reasons older than this lesson: a mixer thread would need shared
buffers, a lock, and a second clock domain — three new failure modes to
buy one sleep — and a device callback would be worse than expensive, it
would **invert the boundary**. The seam is a line the engine calls
across; engine code calls down, the OS answers. A callback would have
the OS call engine code — the platform reaching up into the engine,
holding a pointer to engine state, on a thread the engine did not
create. The paced wait keeps the direction of call exactly where lesson
027 put it, and the cost is the one named above.

### What this run verified, and what it did not

This machine has no sound hardware, and its ALSA `null` device accepts
samples instantly and discards them — it never pushes back. What that
leaves verifiable, verified here:

- **The wait is bounded by the buffer horizon.** An idle run with the
  output open ticks at the audio cadence instead of blocking forever
  between news: 176 frames in three seconds, a frame about every 17 ms
  against the 16.7 ms horizon (the frame numbers and their arrival times
  are in the run's log — frames 1–176, 0.000 s to 2.985 s; a repeat run
  ticks 175 at the same cadence).
- **The gate paces the feeds.** Under scripted input the run keeps
  ticking and keeps feeding — 218 frames, 175 of them feeds at the
  horizon's rate (58 per second), 43 of them news-woken frames that
  queued nothing. No burst of queued audio.
- **Without an output, the bound disappears.** Point the run at a device
  name that does not exist and it reports the typed failure and
  continues — and its wait is unbounded again: the same three-second run
  ran **one** frame (the window's own map news at startup) and then
  waited for news until the interrupt, against the `null` run's 176.
  The bound applies only when there is something to feed.

What this machine cannot verify is the claim the paced wait exists for:
**"the device did not starve"** needs hardware that pushes back —
samples consumed in real time, a queue that empties, ears that hear a
stutter when the loop is late. `null` accepts everything instantly and
proves only the engine's side of the schedule. The device's side is
real-hardware territory, and exercise 2 is where it gets its witness.

## Code step

One change for this lesson, from stream to schedule: `src/frame.h` /
`src/frame.cpp` grow the `audio` phase in the record, the sums, and the
log line (and deliberately not the budget table); `src/platform_alsa.cpp`
grows the deadline bookkeeping and `AudioWaitSeconds`;
`src/platform_internal.h` is the new internal header that shares it;
`src/platform_x11.cpp`'s wait becomes bounded by it; `src/platform.h`'s
`PumpEvents` contract names the bound; and `src/main.cpp` retires the
one-shot submission for a loop that feeds the stream — cursor, gate,
phase, startup report. The demo's world and the drawing path are
untouched. Its end state is tagged `lesson-060`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index 3f9f2dd..d15549b 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -12,6 +12,7 @@ void AccountFrame(FrameStats &stats, const FrameRecord &frame)
 {
     stats.frames += 1;
     stats.update_sum += frame.update;
+    stats.audio_sum += frame.audio;
     stats.render_sum += frame.render;
     stats.present_sum += frame.present;
     stats.total_sum += frame.total;
@@ -39,7 +40,9 @@ void PrintFrameBudget(const FrameStats &stats)
 
     /* The attribution: every row a measured sum, every share of the
        average frame. The named phases live inside render — they say
-       where it went, they do not replace it. */
+       where it went, they do not replace it. The audio phase (lesson
+       060) is summed like the rest but gets no row here: the table's
+       sound rows are lesson 070's, and that is deliberate. */
     double update = stats.update_sum / n * 1e3;
     double render = stats.render_sum / n * 1e3;
     double sprites = stats.sprites_sum / n * 1e3;
diff --git a/src/frame.h b/src/frame.h
index 8a998b4..f4b45e3 100644
--- a/src/frame.h
+++ b/src/frame.h
@@ -14,6 +14,8 @@ namespace engine {
 struct FrameRecord {
     long number;   /* the frame's count since the run started */
     double update; /* reading state, moving the world */
+    double audio;  /* lesson 060: the run's audio step — mixing and
+                      submitting the stream */
     double render; /* drawing the scene into the framebuffer */
     double present;/* the copy to the window, sync included */
     double total;  /* the whole frame step */
@@ -31,6 +33,7 @@ struct FrameRecord {
 struct FrameStats {
     long frames;
     double update_sum;
+    double audio_sum; /* lesson 060's phase, summed like the rest */
     double render_sum;
     double present_sum;
     double total_sum;
diff --git a/src/main.cpp b/src/main.cpp
index a848cad..5044acc 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -29,8 +29,20 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
-/* Lesson 059: the run's first sound is half a second of tone. */
-constexpr int TONE_FRAMES = AUDIO_RATE / 2;
+/* Lesson 059: the run's sound is half a second of tone — 22050 frames.
+   One second of a 440 Hz tone is exactly 440 cycles at AUDIO_RATE, so
+   this buffer holds a whole 220 cycles and its end meets its beginning
+   with no click: the wrap lesson 059 found, which lesson 060's feeding
+   cursor leans on when it hands the same run of frames to the device
+   again and again. */
+constexpr int TONE_FRAMES = AUDIO_RATE / 2; /* 22050 */
+
+/* Lesson 060: one buffer of stream per feed — one sixtieth of a second,
+   the horizon the loop keeps queued. The cursor relies on the tone's
+   length dividing evenly into buffers: 22050 / 735 = 30 exactly, so it
+   wraps at a buffer boundary and no feed ever has to copy across the
+   tone's end. */
+constexpr int CHUNK_FRAMES = AUDIO_RATE / 60;   /* 735 */
 
 /* Lesson 054: the scene, drawn through the camera. The camera's summed
    offset is applied once, at each draw's origin — the map's and the
@@ -129,12 +141,16 @@ int Run(void)
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
-    /* Lesson 059: the run's first sound. A sample is a frame of amplitude
-       at the engine's rate — here a tone computed by code instead of read
+    /* Lesson 059: the run's sound. A sample is a frame of amplitude at
+       the engine's rate — here a tone computed by code instead of read
        from a file — and the seam's audio output is what puts those frames
-       in front of a device. */
+       in front of a device. Lesson 060: those frames are a *stream*, not
+       one submission done at startup — the frame loop feeds the device
+       buffer by buffer, for as long as the run lasts. */
     platform::AudioResult audio =
         platform::OpenAudioOutput(AUDIO_RATE, AUDIO_OUTPUT_CHANNELS);
+    short *tone = 0;
+    int tone_cursor = 0; /* where the stream's next buffer starts */
     if (!audio.output) {
         switch (audio.error) {
         case platform::AUDIO_NO_DEVICE:
@@ -148,8 +164,8 @@ int Run(void)
            still runs — this one continues without sound. */
         std::fprintf(stderr, "engine: continuing without sound\n");
     } else {
-        short *tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
-                                          sizeof(short));
+        tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
+                                   sizeof(short));
         if (!tone) {
             std::fprintf(stderr, "engine: no room for the tone\n");
         } else {
@@ -163,20 +179,25 @@ int Run(void)
                 std::printf(" %d", (int)tone[i]);
             std::printf("\n");
 
-            if (platform::SubmitSamples(audio.output, tone, TONE_FRAMES))
-                std::printf("engine: tone played\n");
-            else
-                std::fprintf(stderr,
-                             "engine: the output would not take the samples\n");
+            /* And what the loop does with them: one buffer of stream per
+               feed — the horizon the paced wait keeps queued. */
+            std::printf("engine: stream: %d-frame buffers, horizon %.1f ms; the loop feeds one when it is due\n",
+                        CHUNK_FRAMES, 1e3 * CHUNK_FRAMES / AUDIO_RATE);
         }
     }
 
-    /* The frame step: read news, update from polled state, draw, present —
-       every phase measured, one record per frame. */
+    /* The frame step: read news, update from polled state, feed the
+       stream, draw, present — every phase measured, one record per
+       frame. */
     int exit_code = 0;
     long frame_number = 0;
     FrameStats stats = {};
     Camera camera = { 0, 0, 0, 0 };
+
+    /* Lesson 060: the run's own feeding schedule. The device consumes at
+       the engine's rate, so the next buffer is due one horizon from the
+       last one — and the loop knows that without asking the platform. */
+    double next_feed = platform::Now();
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -269,6 +290,35 @@ int Run(void)
         }
 
         frame.update = platform::Now() - t0;
+
+        /* Lesson 060: the audio step — the loop feeds the device the next
+           buffer of the stream, and only when the buffer is due. Input
+           news can wake a frame early; a frame woken early must not queue
+           extra audio, or the run would bury the device in buffers instead
+           of pacing them. The step is measured on every frame — it is ~0
+           where no buffer was due — so the phase accounts for all of the
+           frame's audio work. */
+        double t_audio = platform::Now();
+        if (audio.output && tone && t_audio >= next_feed) {
+            if (platform::SubmitSamples(audio.output, tone + tone_cursor,
+                                        CHUNK_FRAMES)) {
+                tone_cursor = (tone_cursor + CHUNK_FRAMES) % TONE_FRAMES;
+            } else {
+                /* A device that will not take the samples is named once,
+                   not once per frame: the run closes the output and carries
+                   on in silence — its wait unbounded again. */
+                std::fprintf(stderr,
+                             "engine: the output would not take the samples\n");
+                platform::CloseAudioOutput(audio.output);
+                audio.output = 0;
+            }
+            /* The schedule restarts from now, not from the missed slot: a
+               long frame is caught up by one buffer, never by a backlog. */
+            next_feed = platform::Now() +
+                        (double)CHUNK_FRAMES / (double)AUDIO_RATE;
+        }
+        frame.audio = platform::Now() - t_audio;
+
         double t1 = platform::Now();
 
         /* Render: every frame draws the whole scene — clear, the world
@@ -313,9 +363,11 @@ int Run(void)
         AccountFrame(stats, frame);
 
         /* The frame log: one line per record — the format grows its named
-           fields, one per subsystem, as the parts name them. */
-        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
-                    frame.number, frame.update * 1e3, frame.render * 1e3,
+           fields, one per subsystem, as the parts name them. The audio
+           phase (lesson 060) joins in the record's own order. */
+        std::printf("frame %ld: update %.3f ms, audio %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
+                    frame.number, frame.update * 1e3, frame.audio * 1e3,
+                    frame.render * 1e3,
                     frame.sprites * 1e3, frame.text * 1e3,
                     frame.tilemap * 1e3, frame.present * 1e3,
                     frame.total * 1e3);
diff --git a/src/platform.h b/src/platform.h
index 8e0f1ba..4473e68 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -117,7 +117,9 @@ FileError WriteFile(const char *path, const unsigned char *data, size_t size);
 
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
-   state afterwards. Blocks until there is news or the run is interrupted. */
+   state afterwards. Blocks until there is news or the run is interrupted —
+   or until an open audio output needs its next buffer, whichever comes
+   first. */
 void PumpEvents(Window *window);
 
 /* True once the user has asked for this window to close. An interrupted run
diff --git a/src/platform_alsa.cpp b/src/platform_alsa.cpp
index 59e1fe6..fdb7fb6 100644
--- a/src/platform_alsa.cpp
+++ b/src/platform_alsa.cpp
@@ -9,9 +9,16 @@
 // Which device this run opens is this file's business too: a machine with
 // no usable output reports AUDIO_NO_DEVICE, and a machine that has none
 // still runs.
+//
+// Lesson 060: the deadline bookkeeping. A device consumes samples at its
+// own rate, so this file also knows when the device will have consumed
+// what it was given — the bound the run's paced wait obeys. That number is
+// platform state, shared with the event pump through platform_internal.h
+// and not part of the seam's contract.
 #define _POSIX_C_SOURCE 200809L
 
 #include "platform.h"
+#include "platform_internal.h"
 
 #include <alsa/asoundlib.h>
 
@@ -26,6 +33,10 @@ struct AudioOutput {
     snd_pcm_t *device;
     int engine_channels; /* the seam's frame width, and the device's own */
     int device_channels; /* layout, which this file maps between */
+    int rate;            /* the sample rate it was opened at */
+    double deadline;     /* lesson 060: when the device will have consumed
+                            everything submitted so far — the boundary
+                            AudioWaitSeconds reports */
 };
 
 /* The one output, in static storage: no new, no delete — the language law
@@ -88,6 +99,9 @@ AudioResult OpenAudioOutput(int rate, int channels)
     audio_state.engine_channels = channels;
     audio_state.device = device;
     audio_state.device_channels = DEVICE_CHANNELS;
+    audio_state.rate = rate;
+    /* Nothing is queued yet, so the first buffer is due at once. */
+    audio_state.deadline = 0.0;
     result.output = &audio_state;
     result.error = AUDIO_OK;
     return result;
@@ -126,9 +140,35 @@ bool SubmitSamples(AudioOutput *output, const short *samples, int frames)
         at += (int)took * output->engine_channels;
         left -= (int)took;
     }
+
+    /* Lesson 060: when will the device have consumed what this call gave
+       it? On the platform clock, the answer is the device's own deadline —
+       and it accumulates: a submit made while an earlier one is still
+       playing lands after that backlog, never before it, so overlapping
+       submits can only move the deadline later. `deadline > now` keeps the
+       backlog's end when there is one; `now` is where consumption starts
+       when the device has already caught up (a call that waited for room
+       finds its deadline in the past). */
+    double now = Now();
+    output->deadline = (output->deadline > now ? output->deadline : now) +
+                       (double)frames / (double)output->rate;
     return true;
 }
 
+double AudioWaitSeconds(void)
+{
+    if (!audio_state.device)
+        return -1.0; /* no output to feed: the wait is unbounded, as before
+                        sound — and a negative answer means exactly that */
+
+    /* The time left until the device needs what comes next — zero once the
+       buffer is already due. Zero, and never a negative: zero bounds the
+       wait at once, a negative removes the bound, and confusing the two
+       would make a due buffer sleep instead of feed. */
+    double left = audio_state.deadline - Now();
+    return left > 0.0 ? left : 0.0;
+}
+
 void CloseAudioOutput(AudioOutput *output)
 {
     if (!output || !output->device)
@@ -139,6 +179,7 @@ void CloseAudioOutput(AudioOutput *output)
     snd_pcm_drain(output->device);
     snd_pcm_close(output->device);
     output->device = 0;
+    output->deadline = 0.0; /* a closed output stops bounding the wait */
 }
 
 } /* namespace platform */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 17dae4e..a251f11 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -15,9 +15,15 @@
 // Lesson 037: whole-file reads. File I/O is OS surface too — POSIX here,
 // a second OS's own calls there. Everything in this file is one
 // implementation behind the seam.
+//
+// Lesson 060: the paced wait. The wait for news is no longer unbounded —
+// with an audio output open it is bounded by how long the queued samples
+// will last, so the run wakes to feed the device on schedule. The bound is
+// platform state, asked for through platform_internal.h.
 #define _POSIX_C_SOURCE 200809L
 
 #include "platform.h"
+#include "platform_internal.h"
 
 #include <X11/XKBlib.h>
 #include <X11/Xlib.h>
@@ -357,9 +363,18 @@ void PumpEvents(Window *window)
     /* Wait for news where a signal can wake us. Lesson 028 slept inside
        XNextEvent, where Ctrl+C could not reach it; poll on the OS
        connection returns when there is news *or* when a signal interrupts
-       it — then the flag below is folded in like any other news. */
+       it — then the flag below is folded in like any other news.
+
+       Lesson 060: the wait is bounded. An open audio output needs its next
+       buffer before long, so the wait may not outlast it — and when it
+       ends early it ends because the output needs feeding, not because
+       anything happened. The ceiling to whole milliseconds keeps the wait
+       from ending before the buffer is due; AudioWaitSeconds is negative
+       with no output to feed, and the wait is then the old unbounded one. */
+    double wait = AudioWaitSeconds();
+    int timeout_ms = wait < 0.0 ? -1 : (int)(wait * 1000.0 + 0.999);
     struct pollfd pfd = { ConnectionNumber(window->display), POLLIN, 0 };
-    poll(&pfd, 1, -1);
+    poll(&pfd, 1, timeout_ms);
 
     if (interrupted)
         window->close_requested = true;
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The horizon, doubled *(measure-the-performance)*

The run keeps exactly one buffer of stream queued — `CHUNK_FRAMES`, one
sixtieth of a second — and an idle run with the output open ticks at that
horizon's cadence. Widen the horizon: make each feed one **thirtieth** of
a second of stream instead of one sixtieth. From runs of the same length,
measure before and after: the idle run's frame cadence (frames and gaps),
and the `audio` phase's own cost on a feeding frame against a frame that
fed nothing. Reconcile the cadence against the horizon arithmetic, and
check the cursor's wrap still needs no split copy — what does
`TONE_FRAMES / CHUNK_FRAMES` come to now? Report the numbers from your
runs.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-060/ex1.md)

### Exercise 2 — Starvation on real speakers *(port-to-your-own-machine)*

Every check in this lesson ran against ALSA's `null` device — the device
that accepts samples instantly and never pushes back. "The device did not
starve" is therefore exactly the claim this machine cannot check, and it
is the claim the paced wait exists for. Take the demo to a machine with
real sound hardware and answer the question `null` cannot: when the run
gets busy — the sprite driven into a wall, the camera shaking, the frame
at its heaviest — does the stream hold, or do you hear it stutter? Have
the run record how hard the device pushed back while it played, so the
log carries the evidence beside your ears; then report the device, the
busy run against an idle one, what you heard, and where in the deadline
bookkeeping a stutter would have shown up first.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-060/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 059 — sound as samples](lesson-059-samples.md) ·
**Next:** [Lesson 061 — the WAV container](lesson-061-wav.md) ·
**Code tag:** [`lesson-060`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-060)
