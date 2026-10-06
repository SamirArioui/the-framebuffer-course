# Lesson 059 — sound as samples

{{#include ../../stability-horizon.md}}

## Prose

Part 3's subject is sound, and this lesson carries exactly one idea:
**sound is samples — frames of amplitude at a fixed rate**. It is the
move Part 0 made for pictures (lesson 013 turned a pixel into a number),
made again one sense later: before sound is a pitch, a volume, or a
file, it is a run of plain integers. Everything Part 3 grows is
bookkeeping over those integers — lesson 061's loader reads them, the
mixer of lessons 063-065 sums them — and their shape never changes.

### The frame, and the rate

The engine's sample format has exactly two halves, and `audio.h`
declares both.

**One sample frame is one `short`** — a 16-bit signed number, −32768 to
32767 — saying where the speaker sits at that instant. Zero is rest;
32767 is as far out as the format reaches; −32768 is as far in. A frame
carries nothing else: no timestamp, no channel, no compression and no
interpretation. This is **16-bit PCM** — plain amplitude values, the
audio equivalent of lesson 030's packed pixel: a format so plain the
numbers can be checked before anything is played.

**The rate is the format's other half.** `AUDIO_RATE` is 44100, and
44100 frames make one second of sound. The rate is what turns a frame's
*index* into a *time*: frame `i` is heard `i / 44100` seconds in. Frame
0 is `0 / 44100` = 0 s; frame 4410 is 0.1 s; frame 44100 is one second.
Like the framebuffer's width — the number that makes a pixel's offset
mean a position — the rate is not playback policy; it is the *meaning*
of the numbers.

The engine's mix is one channel wide (`AUDIO_OUTPUT_CHANNELS`): one
frame, one number. What a particular machine's device wants instead is
the seam's business, and the second half of this lesson is that seam.

### A tone computed by code

Where do frames come from before lesson 061 reads them out of files?
This lesson computes them. `GenerateTone` writes `frame_count` frames of
a sine wave at a given frequency and amplitude — a worked example of
what a sample *is*, not a synthesis feature. Its loop body is three
lines of arithmetic, one idea each:

- `time = i / AUDIO_RATE` — the frame index as a time, the rate doing
  its one job;
- `wave = sin(TURN * frequency * time)` — the time as a position on one
  turn of the sine (`TURN` is two pi). `wave` runs −1.0 to 1.0 and makes
  one full turn every `1 / frequency` seconds — 440 turns a second at
  440 Hz;
- `frames[i] = (short)(wave * amplitude * SAMPLE_PEAK)` — the wave as a
  sample: the scaling maps the sine's ±1.0 onto the format at the given
  amplitude, and the cast to `short` stores the count (truncating toward
  zero).

`SAMPLE_PEAK` is 32767.0, the format's most positive sample — 32767
and not 32768, because +32767 is the largest count the format holds;
scaling by 32768 would overflow at the wave's positive crest, and
scaling by 32767 lands inside the format at every amplitude up to 1.0.
At amplitude 0.25 the crest is `0.25 * 32767` = 8191.75 counts — and
this is why the peak is 8191 at amplitude 0.25: the true crest falls
between two samples, so the loudest frame the tone ever stores is 8191.

The numbers are checkable with no sound device at all, and the run
checks them. The demo prints the tone's first frames as it makes them:

```
engine: tone: 22050 frames at 44100 Hz, first frames: 0 513 1024 1531
engine: tone played
```

The buffer is `TONE_FRAMES` — `AUDIO_RATE / 2` = 22050 frames, half a
second of sound. Frame 0 is exactly 0 because the sine starts at rest,
and each later frame is one more step of phase: 440/44100 of a turn
(about 3.6°) per frame. The steps are visible in the four printed
frames — 513, then 511, then 507 — and they keep shrinking as the sine
bends toward its crest. Where that crest lands, and what the frames
around it are, is predictable before any run; exercise 1 asks you to
predict it.

### The wrap with no seam

One fact worth carrying into lesson 060's feeding loop: one second of a
440 Hz tone is **exactly 440 cycles** at 44,100 samples per second —
about 100.227 frames per cycle, but a whole 440 cycles per second. The
run's half-second buffer holds a whole 220 cycles for the same reason.
So the buffer's end meets its beginning with no seam: the frame just
past its end would be exactly frame 0 again. Play the buffer back to
back and nothing clicks at the join — the numbers wrap. This is a
property of *this* tone's frequency against *this* rate, not of buffers
in general, and lesson 060's loop leans on it when it hands the same
run of frames to the device again and again.

### Sound at the seam

Reaching a device is the platform seam's business, exactly as reaching
a window is. `platform.h` grows the audio output beside the window:
`OpenAudioOutput`, `SubmitSamples`, `CloseAudioOutput`. The shape is the
one the seam has had since lesson 027:

- `OpenAudioOutput` either hands back an `AudioOutput` or names the step
  that failed — the typed failure `AUDIO_NO_DEVICE` sits beside
  `OPEN_NO_DISPLAY`, `AudioResult` beside `WindowResult`. A machine with
  no usable output is a *named* condition: not a crash, not a mystery.
- `SubmitSamples` hands the device the next `frames` sample frames in
  the engine's format. The contract is plain: the samples are consumed
  before the call returns, so the buffer is the caller's again, and a
  device that would not take them answers `false`.
- `CloseAudioOutput` releases what the open took, and drains first — the
  samples handed over are the samples played.

What a device wants *instead* of the engine's format is the
implementation's business, and `src/platform_alsa.cpp` is the file where
that business lives — with `src/platform_x11.cpp`, one of the two a
second OS replaces. The engine's one channel becomes the device's
interleaved two in that file's staging buffer, each sample landing in
every channel of its frame — the mapping `Present` performs for pixels,
performed for samples. Which device gets opened is that file's business
too; the engine never sees the name.

Lesson 042's boundary check keeps all of this honest, and its patterns
grow with the seam: `<alsa/` joins the OS headers, `snd_` joins the OS
calls. The check still prints its account unchanged:

```
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp src/platform_alsa.cpp
boundary: 17 single-line declarations there (multi-line ones are in the header)
```

### The toolchain flag

The rule since Part 0 is that **the toolchain is curriculum**: a build
flag is taught when it lands. This lesson's flag is `-lasound`, and it
lands on `build.sh`'s link line beside `-lX11` — the window's library
and the sound device's library, one per side of the seam.

The compile lines are unchanged, and that is itself the lesson's one
toolchain finding, worth recording because it cost a detour to find:
ALSA's header does **not** compile under strict `-std=c11` — it redefines
`struct timespec`, which strict C reserves for the implementation — but
the engine is C++ (`-std=c++17`) and takes the header unchanged. The
only build change this lesson needs is the link flag. The prerequisites
grow `libasound2-dev` to match (README), because the header and the
library arrive in their own package.

### What this run verified, and what it did not

This authoring machine has **no sound hardware**. Saying so plainly is
cheaper than a fiction:

- **The run was verified against ALSA's software `null` device** — the
  device that accepts and discards samples, through the same
  `OpenAudioOutput` / `SubmitSamples` / `CloseAudioOutput` calls a real
  device gets. `engine: tone played` means the device took the frames;
  on `null`, that is *all* it means.
- **The missing-device path was verified as a typed failure** — point
  the run at a device name that does not exist and stderr says, above
  our account:

  ```
  ALSA lib pcm.c:2721:(snd_pcm_open_noupdate) Unknown PCM no-such-device-xyz
  engine: no audio output on this machine
  engine: continuing without sound
  ```

  The first line is ALSA's own library talking — the OS reporting for
  itself. Ours is the typed failure: `AUDIO_NO_DEVICE`, named and
  printed, and the run continues to a clean close (`engine: closed`).
  Failure is a value, not an ending.
- **No speaker has made a sound.** Whether the tone is audible, at what
  volume, at what pitch on real hardware — none of that was checked
  here, and nothing in this course pretends otherwise. "Does it
  actually sound right?" is a question about *your* machine, and
  exercise 2 is where it gets answered.

## Code step

One change for this lesson, from format to device: `src/audio.h` /
`src/audio.cpp` grow the sample format and `GenerateTone` — the tone
computed by code, before any file holds one; `src/platform.h` grows the
audio output with its typed failure `AUDIO_NO_DEVICE`, and
`src/platform_alsa.cpp` is the new OS file a second OS replaces;
`src/main.cpp` opens the output, generates the run's first sound, and
submits it; `build.sh` links `-lasound`; and `tools/check-boundary.sh`
grows its OS patterns so the boundary keeps being checked. The demo
loop, the world, and the drawing path are untouched. Its end state is
tagged `lesson-059`.

```diff
diff --git a/build.sh b/build.sh
index f852f8c..0b62656 100755
--- a/build.sh
+++ b/build.sh
@@ -13,8 +13,9 @@
 # Environment overrides:
 #   CC, CFLAGS       compiler and flags for C sources
 #   CXX, CXXFLAGS    compiler and flags for C++ sources
-#   LDFLAGS          extra link flags (default: the OS library the platform
-#                    layer wraps — -lX11 on the Linux/X11 main line)
+#   LDFLAGS          extra link flags (default: the OS libraries the platform
+#                    layer wraps — -lX11 for the window on the Linux/X11 main
+#                    line, -lasound for the sound device)
 #   BUILD_DIR        output directory (default: build)
 
 set -euo pipefail
@@ -25,7 +26,7 @@ CC="${CC:-gcc}"
 CXX="${CXX:-g++}"
 CFLAGS="${CFLAGS:--std=c11 -O0 -g -Wall -Wextra}"
 CXXFLAGS="${CXXFLAGS:--std=c++17 -O0 -g -Wall -Wextra}"
-LDFLAGS="${LDFLAGS:--lX11}"
+LDFLAGS="${LDFLAGS:--lX11 -lasound}"
 BUILD_DIR="${BUILD_DIR:-build}"
 OBJ_DIR="$BUILD_DIR/obj"
 BIN="$BUILD_DIR/game"
diff --git a/src/audio.cpp b/src/audio.cpp
new file mode 100644
index 0000000..e942411
--- /dev/null
+++ b/src/audio.cpp
@@ -0,0 +1,38 @@
+// audio.cpp — the tone computed by code: arithmetic that becomes sound.
+//
+// Lesson 059: every sample in the engine is a sequence of numbers, and
+// this file makes one out of a sine wave so the numbers can be read,
+// checked, and played before any file format is involved. The same bytes
+// come back later as an asset (lesson 061) and go into the mixer (lesson
+// 063); here they are simply written.
+
+#include "audio.h"
+
+#include <cmath>
+
+namespace engine {
+namespace {
+
+/* One turn of the sine, in radians: two pi. */
+constexpr double TURN = 6.283185307179586;
+
+/* The format's most positive sample. The sine runs -1.0 to 1.0; scaling
+   by amplitude * 32767 lands inside the format at any amplitude <= 1.0. */
+constexpr double SAMPLE_PEAK = 32767.0;
+
+} /* namespace */
+
+void GenerateTone(short *frames, int frame_count, double frequency,
+                  double amplitude)
+{
+    for (int i = 0; i < frame_count; ++i) {
+        /* Frame i is heard i / AUDIO_RATE seconds in: the rate turns a
+           frame number into a time, and the sine turns a time into an
+           amplitude. */
+        double time = (double)i / (double)AUDIO_RATE;
+        double wave = std::sin(TURN * frequency * time);
+        frames[i] = (short)(wave * amplitude * SAMPLE_PEAK);
+    }
+}
+
+} /* namespace engine */
diff --git a/src/audio.h b/src/audio.h
new file mode 100644
index 0000000..11264b0
--- /dev/null
+++ b/src/audio.h
@@ -0,0 +1,37 @@
+// audio.h — sound as samples: frames of amplitude, the engine's format.
+//
+// Lesson 059: sound is data before it is sound. A sample is a frame of
+// amplitude — one number saying where the speaker sits at that instant —
+// and a sound is a run of those frames at a fixed rate. Nothing here knows
+// about devices, files, or mixing: this is the format the engine's output
+// speaks, the format lesson 061's loader accepts and refuses everything
+// else, and the format the mixer of lessons 063-065 sums.
+#ifndef AUDIO_H
+#define AUDIO_H
+
+namespace engine {
+
+/* The engine's sample format: 16-bit signed frames at this rate. One
+   sample frame is one `short`, from -32768 to 32767. The rate is the
+   format's other half: AUDIO_RATE frames make one second of sound, so a
+   frame's number in the run says exactly when it is heard. */
+constexpr int AUDIO_RATE = 44100; /* sample frames per second */
+
+/* The engine's mix is one channel wide. What the device itself wants is
+   the platform layer's business — it maps these samples into the device's
+   own layout, exactly as Present maps the framebuffer's pixels into the
+   window's. */
+constexpr int AUDIO_OUTPUT_CHANNELS = 1;
+
+/* A tone computed by code: `frame_count` sample frames of a sine wave at
+   `frequency` hertz, at `amplitude` (0.0 to 1.0), in the engine's format.
+   This is what a sample looks like before any file holds one — the bytes
+   a sound is made of, produced by arithmetic instead of read from disk.
+   It is a worked example of what a sample *is*, not a synthesis feature:
+   the engine plays samples, it does not design sounds. */
+void GenerateTone(short *frames, int frame_count, double frequency,
+                  double amplitude);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index 95f09af..a848cad 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -10,6 +10,7 @@
 #include <cstdio>
 
 #include "arena.h"
+#include "audio.h"
 #include "blit.h"
 #include "camera.h"
 #include "font.h"
@@ -28,6 +29,9 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
+/* Lesson 059: the run's first sound is half a second of tone. */
+constexpr int TONE_FRAMES = AUDIO_RATE / 2;
+
 /* Lesson 054: the scene, drawn through the camera. The camera's summed
    offset is applied once, at each draw's origin — the map's and the
    sprite's. The HUD is not scene and does not pass through here. */
@@ -125,6 +129,48 @@ int Run(void)
     std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
     std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);
 
+    /* Lesson 059: the run's first sound. A sample is a frame of amplitude
+       at the engine's rate — here a tone computed by code instead of read
+       from a file — and the seam's audio output is what puts those frames
+       in front of a device. */
+    platform::AudioResult audio =
+        platform::OpenAudioOutput(AUDIO_RATE, AUDIO_OUTPUT_CHANNELS);
+    if (!audio.output) {
+        switch (audio.error) {
+        case platform::AUDIO_NO_DEVICE:
+            std::fprintf(stderr, "engine: no audio output on this machine\n");
+            break;
+        default:
+            std::fprintf(stderr, "engine: audio error %d\n", audio.error);
+            break;
+        }
+        /* The failure is a value, not an ending: a machine with no output
+           still runs — this one continues without sound. */
+        std::fprintf(stderr, "engine: continuing without sound\n");
+    } else {
+        short *tone = (short *)ArenaAlloc(arena, TONE_FRAMES * sizeof(short),
+                                          sizeof(short));
+        if (!tone) {
+            std::fprintf(stderr, "engine: no room for the tone\n");
+        } else {
+            GenerateTone(tone, TONE_FRAMES, 440.0, 0.25);
+
+            /* The bytes are checkable before they are audible: the first
+               frames of the tone, in the engine's own format. */
+            std::printf("engine: tone: %d frames at %d Hz, first frames:",
+                        TONE_FRAMES, AUDIO_RATE);
+            for (int i = 0; i < 4; ++i)
+                std::printf(" %d", (int)tone[i]);
+            std::printf("\n");
+
+            if (platform::SubmitSamples(audio.output, tone, TONE_FRAMES))
+                std::printf("engine: tone played\n");
+            else
+                std::fprintf(stderr,
+                             "engine: the output would not take the samples\n");
+        }
+    }
+
     /* The frame step: read news, update from polled state, draw, present —
        every phase measured, one record per frame. */
     int exit_code = 0;
@@ -284,6 +330,7 @@ int Run(void)
 
     if (platform::CloseRequested(opened.window))
         std::printf("engine: close reported\n");
+    platform::CloseAudioOutput(audio.output);
     platform::CloseWindow(opened.window);
     ArenaRelease(arena);
     std::printf("engine: closed\n");
diff --git a/src/platform.h b/src/platform.h
index 29e8dd9..8e0f1ba 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -137,6 +137,38 @@ bool Present(Window *window, const unsigned char *pixels, int width,
 /* Releases everything OpenWindow took from the OS. */
 void CloseWindow(Window *window);
 
+/* The audio output: the OS's sound device behind the seam, beside the
+   window. The sample format is this interface's contract, not any OS's —
+   `rate` mono sample frames per second, each frame one signed 16-bit
+   value. What the device itself wants is the implementation's business:
+   it maps these samples into the device's own layout, exactly as Present
+   maps the framebuffer's pixels into the window's. */
+enum AudioError {
+    AUDIO_OK = 0,
+    AUDIO_NO_DEVICE, /* the OS has no usable output */
+};
+
+struct AudioOutput;
+
+struct AudioResult {
+    AudioOutput *output; /* the open output, or 0 on failure */
+    AudioError error;    /* AUDIO_OK exactly when output is non-0 */
+};
+
+/* Opens the OS's audio output at the engine's fixed sample format, or
+   names the step that failed. A machine with no usable output is a typed
+   failure — and a run that hears it may continue without sound. */
+AudioResult OpenAudioOutput(int rate, int channels);
+
+/* Hands the device the next `frames` sample frames of mixed samples —
+   `frames` * `channels` values, one frame after another. The samples are
+   consumed before the call returns, so the buffer is the caller's again.
+   Returns false when the device could not take them. */
+bool SubmitSamples(AudioOutput *output, const short *samples, int frames);
+
+/* Releases everything OpenAudioOutput took from the OS. */
+void CloseAudioOutput(AudioOutput *output);
+
 } /* namespace platform */
 
 #endif
diff --git a/src/platform_alsa.cpp b/src/platform_alsa.cpp
new file mode 100644
index 0000000..59e1fe6
--- /dev/null
+++ b/src/platform_alsa.cpp
@@ -0,0 +1,144 @@
+// platform_alsa.cpp — the ALSA implementation of the audio output.
+//
+// Lesson 059: the sound device is OS business, like the window is. This
+// file and platform_x11.cpp are the two a second OS replaces (the boundary
+// check's implementation list names them), and this is the only place an
+// ALSA header or an ALSA call appears. The engine sees platform.h and
+// nothing else.
+//
+// Which device this run opens is this file's business too: a machine with
+// no usable output reports AUDIO_NO_DEVICE, and a machine that has none
+// still runs.
+#define _POSIX_C_SOURCE 200809L
+
+#include "platform.h"
+
+#include <alsa/asoundlib.h>
+
+#include <stdlib.h> /* getenv */
+
+namespace platform {
+
+/* What an audio output is made of on this OS. The definition lives here,
+   where ALSA is visible; the engine holds the pointer and never looks
+   inside. */
+struct AudioOutput {
+    snd_pcm_t *device;
+    int engine_channels; /* the seam's frame width, and the device's own */
+    int device_channels; /* layout, which this file maps between */
+};
+
+/* The one output, in static storage: no new, no delete — the language law
+   of lesson 026 keeps allocation out of the engine and this layer alike. */
+static AudioOutput audio_state;
+
+/* The device to open. ALSA's `default` is what a machine with sound
+   answers with; the software device that accepts and discards samples
+   (the headless check's `null`) is named here instead. The engine never
+   sees the name — choosing a device is OS business. */
+static const char *DeviceName(void)
+{
+    const char *named = getenv("ALSA_DEVICE");
+    return named && named[0] ? named : "default";
+}
+
+/* The device's own shape. The engine's samples are one channel wide; this
+   machine's outputs take interleaved frames across two, so the samples are
+   duplicated here — the mix across both channels, which is the device's
+   business and not the engine's. */
+constexpr int DEVICE_CHANNELS = 2;
+
+/* The conversion buffer, in chunks: the engine's mono frames become the
+   device's interleaved frames. Static, like the rest of this file. */
+constexpr int STAGE_FRAMES = 1024;
+static short stage[STAGE_FRAMES * DEVICE_CHANNELS];
+
+AudioResult OpenAudioOutput(int rate, int channels)
+{
+    AudioResult result = { 0, AUDIO_NO_DEVICE };
+
+    snd_pcm_t *device = 0;
+    if (snd_pcm_open(&device, DeviceName(), SND_PCM_STREAM_PLAYBACK, 0) < 0)
+        return result; /* no usable output: named, not hidden */
+
+    /* The engine's format, field by field — every one is a promise the
+       engine's samples rely on. A device that cannot keep the rate is not
+       the engine's output: there is no resampling here, so the open fails
+       typed rather than quietly playing at the wrong speed. */
+    snd_pcm_hw_params_t *hw = 0;
+    snd_pcm_hw_params_malloc(&hw);
+    snd_pcm_hw_params_any(device, hw);
+    unsigned device_rate = (unsigned)rate;
+    bool formatted =
+        snd_pcm_hw_params_set_access(device, hw,
+                                     SND_PCM_ACCESS_RW_INTERLEAVED) >= 0 &&
+        snd_pcm_hw_params_set_format(device, hw, SND_PCM_FORMAT_S16_LE) >= 0 &&
+        snd_pcm_hw_params_set_channels(device, hw,
+                                       (unsigned)DEVICE_CHANNELS) >= 0 &&
+        snd_pcm_hw_params_set_rate_near(device, hw, &device_rate, 0) >= 0 &&
+        (unsigned)rate == device_rate &&
+        snd_pcm_hw_params(device, hw) >= 0;
+    snd_pcm_hw_params_free(hw);
+
+    if (!formatted) {
+        snd_pcm_close(device);
+        return result;
+    }
+
+    audio_state.engine_channels = channels;
+    audio_state.device = device;
+    audio_state.device_channels = DEVICE_CHANNELS;
+    result.output = &audio_state;
+    result.error = AUDIO_OK;
+    return result;
+}
+
+bool SubmitSamples(AudioOutput *output, const short *samples, int frames)
+{
+    if (!output || !output->device)
+        return false;
+
+    const short *at = samples;
+    int left = frames;
+    while (left > 0) {
+        int chunk = left < STAGE_FRAMES ? left : STAGE_FRAMES;
+
+        /* The engine's frames become the device's. The device has its own
+           channel count and the engine has `engine_channels` values per
+           frame; each device channel takes one of the engine's, wrapping
+           back to the first. With the engine's one channel that is one
+           sample in every channel of its frame — the mix duplicated
+           across the device's channels. */
+        for (int f = 0; f < chunk; ++f) {
+            for (int c = 0; c < output->device_channels; ++c) {
+                int from = f * output->engine_channels +
+                           c % output->engine_channels;
+                stage[f * output->device_channels + c] = at[from];
+            }
+        }
+
+        /* The device takes what it has room for; the rest comes back
+           around. When it takes nothing at all, the caller hears false. */
+        snd_pcm_sframes_t took = snd_pcm_writei(
+            output->device, stage, (snd_pcm_uframes_t)chunk);
+        if (took < 0)
+            return false;
+        at += (int)took * output->engine_channels;
+        left -= (int)took;
+    }
+    return true;
+}
+
+void CloseAudioOutput(AudioOutput *output)
+{
+    if (!output || !output->device)
+        return;
+
+    /* Drain first: the device finishes what it already has before the
+       handle goes away — the samples handed over are the samples played. */
+    snd_pcm_drain(output->device);
+    snd_pcm_close(output->device);
+    output->device = 0;
+}
+
+} /* namespace platform */
diff --git a/tools/check-boundary.sh b/tools/check-boundary.sh
index 5b55848..7caad33 100755
--- a/tools/check-boundary.sh
+++ b/tools/check-boundary.sh
@@ -25,10 +25,10 @@ IMPL="src/platform_x11.cpp src/platform_alsa.cpp"
 
 # Headers that only an OS has. The language's own headers (<cstdio>,
 # <cstring>, <stddef.h>, ...) are fine anywhere — they are not an OS.
-OS_HEADERS='<X11/|<sys/|<unistd\.h>|<fcntl\.h>|<poll\.h>|<signal\.h>|<errno\.h>|<time\.h>'
+OS_HEADERS='<X11/|<alsa/|<sys/|<unistd\.h>|<fcntl\.h>|<poll\.h>|<signal\.h>|<errno\.h>|<time\.h>'
 
 # Calls only an OS answers. The list grows with the seam.
-OS_CALLS='(^|[^A-Za-z0-9_:])(X[A-Z][A-Za-z]+|mmap|munmap|mprotect|clock_gettime|nanosleep|sysconf|open|close|read|write|fstat|poll|signal)\s*\('
+OS_CALLS='(^|[^A-Za-z0-9_:])(X[A-Z][A-Za-z]+|snd_[a-z_]+|mmap|munmap|mprotect|clock_gettime|nanosleep|sysconf|open|close|read|write|fstat|poll|signal)\s*\('
 
 status=0
 
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The first eight frames *(predict-the-output)*

The run prints the tone's first four frames, and those four are on this
page — enough to check your arithmetic against. Before you run
anything, predict the first **eight** sample frames `GenerateTone`
writes for the run's tone (440 Hz, amplitude 0.25) as exact integers,
and the frame number where the buffer's loudest sample lands. Then
extend the run's tone probe so it prints eight frames and the loudest
sample with its frame number, run it — no sound device is needed — and
reconcile every value against the scaling in `GenerateTone`.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-059/ex1.md)

### Exercise 2 — Hear it on real speakers *(port-to-your-own-machine)*

Every check for this lesson ran against ALSA's `null` device; no speaker
has made a sound. Take the demo to a machine with real sound hardware
and answer the question `null` cannot: is the tone audible, and is it
what half a second of 440 Hz at amplitude 0.25 should be? Have the run
report which device it opened, so the log on your machine records what
was chosen; then run, listen, and report what you heard — the device,
the tone's duration, pitch, and volume — and anything that differs from
what this lesson predicts.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-059/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 058 — the frame-budget table](../part-2/lesson-058-budget.md) ·
**Next:** [Lesson 060 — the stream's shape](lesson-060-stream.md) ·
**Code tag:** [`lesson-059`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-059)
