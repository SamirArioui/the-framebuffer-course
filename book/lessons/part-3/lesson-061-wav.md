# Lesson 061 — the WAV container

{{#include ../../stability-horizon.md}}

## Prose

Lesson 059 left a promise in its own comment: the frames it computed
would come back later as an asset. This lesson keeps it. `assets/tone.wav`
is on disk now, and it holds exactly what `GenerateTone` wrote — 22050
frames of a 440 Hz sine at amplitude 0.25, the file's first eight frames
reading `0 513 1024 1531 2032 2525 3009 3480`, just as lesson 059's run
printed the first four of those same numbers from its buffer. The bytes
did not change; the box they travel in did. So the idea of this lesson is
one, and it is about the container: **a WAV file is a box of chunks, and
the loader walks it by hand** — because a file's *claims* and a file's
*bytes* are two different things, and the craft of a loader is the check
that they agree.

### The container, chunk by chunk

A WAV file is a RIFF file, and RIFF is a container format whose whole
grammar fits in a paragraph: the file opens with a four-byte type tag
(`RIFF`), a four-byte size, and a four-byte form tag (`WAVE`) — and then
is a sequence of **chunks**. A chunk is a four-byte id, a four-byte size,
and exactly that many bytes of payload, padded out to an even length.
That is the entire grammar. There is no table of contents and no offset
to trust: a reader finds anything in the file by walking the chunks one
after another and reading each id as it comes to it.

`assets/tone.wav` is 44144 bytes. Here are its first forty-eight:

```
$ xxd assets/tone.wav | head -4
00000000: 5249 4646 68ac 0000 5741 5645 666d 7420  RIFFh...WAVEfmt 
00000010: 1000 0000 0100 0100 44ac 0000 8858 0100  ........D....X..
00000020: 0200 1000 6461 7461 44ac 0000 0000 0102  ....dataD.......
00000030: 0004 fb05 f007 dd09 c10b 980d 620f 1c11  ............b...
```

Read it with the grammar in hand. `5249 4646` is `RIFF`, and `68ac 0000`
is the container's claim about its own length: 0x0000ac68 = 44136,
little-endian — low byte first, lesson 014's move, because the file's
bytes are not the machine's bytes and the loader says which is which.
44136 is exactly the file's 44144 bytes minus the eight the claim does
not cover. `5741 5645` is `WAVE`: the form inside the box.

Then the chunks. `666d 7420` is `fmt ` — space included, four bytes like
every id — and `1000 0000` says the chunk is 16 bytes long. Those
sixteen bytes are the format's facts, each a two- or four-byte
little-endian number: `0100` the format tag (1, linear PCM), `0100` one
channel, `44ac 0000` = 44100 the rate, `8858 0100` = 88200 the byte
rate, `0200` the block align, `1000` sixteen bits. Then `6461 7461` is
`data`, and `44ac 0000` says the chunk is 44100 bytes of frames — 22050
frames of one `short` each — and those frames run to the end of the file.
The first of them are already visible in the dump: `0000 0102 0004 fb05`
is 0, 513, 1024, 1531, little-endian. The run's report prints the same
frames as numbers, which is the whole point of printing them.

### The claims and the bytes

Every field above is a **claim**, and a claim is not bytes. Lesson 044's
loader already had the habit — nothing in a file is assumed to be there
until the bytes say so — and this loader keeps it against numbers that
are allowed to lie:

- the RIFF size must equal the file's length minus eight — a file whose
  claim disagrees with its own length is malformed before its chunks are
  even walked;
- every chunk's size must fit inside the bytes the file actually has. A
  `data` chunk claiming 44100 bytes in a 20000-byte file is not "almost a
  sample"; it is a claim that runs off the end, and following it is how a
  loader reads memory that was never its file's;
- the `fmt ` facts must be the engine's format, typed: linear PCM, one
  channel, sixteen bits, 44100 Hz. Nothing here resamples, converts, or
  reinterprets a frame to make a foreign file fit;
- the `fmt ` chunk's derived numbers must agree with the facts they are
  derived from — the byte rate is 44100 × 1 × 2, the block align is
  1 × 2. A container that contradicts itself is malformed before any
  frame is read;
- the `data` size must be a whole number of frames: one frame is one
  `short`, two bytes, and 44101 bytes of data claim something the format
  cannot hold;
- and the frames must be the file's last bytes — no short read presented
  as a sample, and nothing after the frames the loader would
  half-understand.

Why this strict? Because the failures it refuses are worse than the ones
it makes. A 22050 Hz file read as if it were 44100 is a note played an
octave too high for half as long — *and it plays fine*: the wrong sound
that works is the failure mode no report ever catches. A truncated file
whose missing tail is read out of whatever memory lies past it plays
garbage at the end of a sound and calls it done. A typed failure is the
cheap, honest answer: the run names the file, names the failure, and no
sound is ever played that the loader could not vouch for. "A load yields
a complete sample or a typed failure — never partial data presented as
success" is the same rule lesson 044 gave sprites, kept exactly.

### Why by hand

The parse is by hand because no library reads a file in student-visible
engine code — the same move as the PPM header in lesson 044, one chunk
deeper. There, the header was three lines of ASCII; here it is nested
binary chunks and little-endian integers, and the walk is the new idea
while the byte-by-byte reading is lesson 014's, reused. A RIFF library
would hide exactly this lesson's subject behind a call: the difference
between what a file says and what a file has. What the habit buys is on
the page above — the bytes stay inspectable with `xxd`, every check is a
line a student can point at, and the whole loader is small enough to hold
in one reading. The language law of lesson 026 holds over it unchanged:
no allocation but the arena's, no exceptions, no library the seam does
not own.

### A load is complete or named

`LoadSample`'s contract is `LoadSprite`'s, and `SampleResult` says it:
`SAMPLE_OK` exactly when the frames pointer is non-0, and three named
failures beside it — `SAMPLE_MISSING` when the file is not there or
cannot be read, `SAMPLE_MALFORMED` when the bytes are not a complete
sample in the engine's format, `SAMPLE_NO_ROOM` when the arena has no
room for the frames. The frames themselves are copied into the arena and
the file's bytes go back to the OS: what the engine keeps is its copy.
And the copy is bracketed by a mark — `ArenaMark` before it, the failure
paths rolling back to it — so a load that refuses leaves **no partial
frames behind**. The arena's accounting in the run is where that promise
is visible: after a refused load, its used count has not moved.

The struct the load fills carries the frames *and the facts playback
needs* — the length in sample frames, the rate, the channels. That is not
decoration. The questions playback asks — when does this end, and what
does a frame's index mean in time — must be answered from the sample
alone, never from assumptions about the file that produced it. Lesson 062
is exactly that: the mixer of the next lessons never opens a file, and it
never guesses; it reads these fields.

### What this run verified, and what it did not

All of the numbers below come from real runs of this lesson's end state
on this machine:

- **The sample loads completely.** The run reports
  `engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames:
  0 513 1024 1531 2032 2525 3009 3480` — the facts from the `fmt ` chunk
  and the frames from `data`, the same eight numbers lesson 059 computed.
  A scratch probe (not engine code) compared every one of the 22050
  loaded frames against the file's own bytes: all 22050 match exactly.
- **A missing file fails typed.** Point the run at a directory where
  `assets/tone.wav` is not and it says
  `engine: assets/tone.wav: could not load (missing)` and ends, the file
  named and the failure named.
- **A malformed file fails typed.** Four corruptions were cut from the
  real file, one claim each, and every one reports
  `engine: assets/tone.wav: could not load (malformed)`:
  a file truncated mid-`data` (20000 bytes of 44144); a `data` size of
  44101 — not a whole number of frames; a `fmt ` chunk claiming 22050 Hz
  with its byte rate still agreeing with it; and a RIFF size claiming
  44137 against the file's real length. The scratch probe ran the same
  four through `LoadSample` and watched the arena: after each refusal the
  used count had not moved — the rollback's promise, checked.

What this lesson does **not** do is play anything. The stream still plays
lesson 060's computed tone — the file is loaded and checked, not yet
heard — and no speaker on this machine has made a sound, this lesson or
any other. The bytes are checked; the hearing is yours. Lesson 062 plays
the file.

## Code step

One change for this lesson, from file to frames: `src/audio.h` /
`src/audio.cpp` grow `Sample`, its typed failures, and `LoadSample` — the
RIFF/WAVE container walked chunk by chunk and byte by byte, every claim
checked against the bytes before any frame is copied. `src/main.cpp`
grows the run's startup: `assets/tone.wav` is loaded beside the other
assets, a successful load reports the sample's facts and its first
frames, and a failed one ends the run by name like every other asset. The
new asset is the code step's other half: `assets/tone.wav`, the frames
lesson 059 computed, in the container this lesson defines. The stream,
the loop, and the seam are untouched. Its end state is tagged
`lesson-061`.

The asset's bytes are binary and so is its diff — git prints `Binary
files … differ` for it. The header and the first frames are the hexdump
and the run's line above; `git diff --binary` prints the full patch.

```diff
diff --git a/assets/tone.wav b/assets/tone.wav
new file mode 100644
index 0000000..346da99
Binary files /dev/null and b/assets/tone.wav differ
diff --git a/src/audio.cpp b/src/audio.cpp
index e942411..82223bd 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -1,15 +1,21 @@
-// audio.cpp — the tone computed by code: arithmetic that becomes sound.
+// audio.cpp — the tone computed by code, and the sample read from a file.
 //
 // Lesson 059: every sample in the engine is a sequence of numbers, and
 // this file makes one out of a sine wave so the numbers can be read,
-// checked, and played before any file format is involved. The same bytes
-// come back later as an asset (lesson 061) and go into the mixer (lesson
-// 063); here they are simply written.
+// checked, and played before any file format is involved.
+//
+// Lesson 061: the same bytes now come back out of a file. LoadSample
+// walks a RIFF/WAVE container chunk by chunk — every size checked against
+// the bytes around it, every format fact checked against the engine's —
+// and copies the frames into the arena. The numbers are the point on both
+// sides: what GenerateTone writes, the loader reads back.
 
 #include "audio.h"
 
 #include <cmath>
 
+#include "platform.h"
+
 namespace engine {
 namespace {
 
@@ -20,6 +26,30 @@ constexpr double TURN = 6.283185307179586;
    by amplitude * 32767 lands inside the format at any amplitude <= 1.0. */
 constexpr double SAMPLE_PEAK = 32767.0;
 
+/* The container's numbers are little-endian — the file's bytes are not
+   the machine's bytes, and this is where the difference is resolved
+   (lesson 014): low byte first, assembled by hand. */
+unsigned ReadU32(const unsigned char *p)
+{
+    return (unsigned)p[0] | ((unsigned)p[1] << 8) | ((unsigned)p[2] << 16) |
+           ((unsigned)p[3] << 24);
+}
+
+/* One sample frame: two little-endian bytes whose count carries its sign
+   in bit 15 — 0x8000 and up are the format's negative frames. */
+short ReadFrame(const unsigned char *p)
+{
+    int count = p[0] | (p[1] << 8);
+    return (short)(count < 0x8000 ? count : count - 0x10000);
+}
+
+/* A four-byte chunk id, exactly — `fmt ` keeps its space. */
+bool IdIs(const unsigned char *p, const char *id)
+{
+    return p[0] == (unsigned char)id[0] && p[1] == (unsigned char)id[1] &&
+           p[2] == (unsigned char)id[2] && p[3] == (unsigned char)id[3];
+}
+
 } /* namespace */
 
 void GenerateTone(short *frames, int frame_count, double frequency,
@@ -35,4 +65,126 @@ void GenerateTone(short *frames, int frame_count, double frequency,
     }
 }
 
+SampleResult LoadSample(Arena &arena, const char *path)
+{
+    SampleResult result = { { 0, 0, 0, 0 }, SAMPLE_OK };
+
+    platform::FileData file = platform::ReadFile(path);
+    if (file.error != platform::FILE_OK) {
+        result.error = SAMPLE_MISSING;
+        return result;
+    }
+
+    const unsigned char *data = file.data;
+    size_t size = file.size;
+
+    /* The container's account of itself: RIFF names the form, claims a
+       length, and says the form inside is WAVE. A claim is only a claim —
+       this one is checked against the bytes the file actually has, and a
+       file whose RIFF size disagrees with its own length is malformed
+       before its chunks are even walked. */
+    bool ok = size >= 12 && IdIs(data, "RIFF") && IdIs(data + 8, "WAVE");
+    ok = ok && ReadU32(data + 4) == size - 8;
+
+    /* The walk: chunk after chunk — an id, a size, then exactly that many
+       bytes (padded to an even length). Chunks are never assumed to be
+       where they "should" be: the walk finds `fmt ` and `data` wherever
+       they are, steps over what it does not know, and refuses any chunk
+       whose own size disagrees with the bytes around it. */
+    size_t at = 12;
+    bool have_format = false, have_frames = false;
+    unsigned rate = 0, channels = 0, bits = 0;
+    size_t frames_at = 0, frames_bytes = 0;
+
+    while (ok && at + 8 <= size) {
+        const unsigned char *id = data + at;
+        size_t body = at + 8;
+        size_t chunk = ReadU32(data + at + 4);
+        ok = ok && chunk <= size - body;
+        if (!ok)
+            break;
+
+        if (IdIs(id, "fmt ")) {
+            /* The format chunk: at least the sixteen bytes of PCM facts.
+               Every fact is checked typed — not mono, not 16-bit, not
+               AUDIO_RATE is not the engine's format, and nothing here
+               will resample or reinterpret a frame to make it fit. The
+               chunk's own derived numbers are checked against the facts
+               they are derived from: the claims must agree with each
+               other as well as with the bytes. */
+            ok = ok && !have_format && chunk >= 16;
+            if (ok) {
+                unsigned format =
+                    (unsigned)data[body] | ((unsigned)data[body + 1] << 8);
+                channels =
+                    (unsigned)data[body + 2] | ((unsigned)data[body + 3] << 8);
+                rate = ReadU32(data + body + 4);
+                unsigned byte_rate = ReadU32(data + body + 8);
+                unsigned block_align = (unsigned)data[body + 12] |
+                                       ((unsigned)data[body + 13] << 8);
+                bits = (unsigned)data[body + 14] |
+                       ((unsigned)data[body + 15] << 8);
+                bool claims_agree =
+                    byte_rate == rate * channels * (bits / 8) &&
+                    block_align == channels * (bits / 8);
+                ok = ok && format == 1 /* linear PCM */ && channels == 1 &&
+                     bits == 16 && rate == (unsigned)AUDIO_RATE &&
+                     claims_agree;
+            }
+            have_format = true;
+        } else if (IdIs(id, "data")) {
+            ok = ok && !have_frames;
+            frames_at = body;
+            frames_bytes = chunk;
+            have_frames = true;
+        }
+        at = body + chunk + (chunk & 1); /* chunks pad to an even length */
+    }
+
+    /* The walk's verdict: both halves found, and the frames a whole
+       number of them — one frame is one short, and a data chunk whose
+       size is not a whole number of frames claims something the format
+       cannot hold. */
+    ok = ok && have_format && have_frames;
+    ok = ok && frames_bytes % 2 == 0;
+
+    if (!ok) {
+        result.error = SAMPLE_MALFORMED;
+        platform::ReleaseFile(file);
+        return result;
+    }
+    int frame_count = (int)(frames_bytes / 2);
+
+    /* The frames, into the arena — the engine keeps its own copy, and the
+       file's bytes go back to the OS. The mark is the load's transaction:
+       from here on a refusal rolls the arena back, and a failed load
+       leaves no partial frames behind. */
+    size_t mark = ArenaMark(arena);
+    short *frames = (short *)ArenaAlloc(arena, frames_bytes, sizeof(short));
+    if (!frames) {
+        result.error = SAMPLE_NO_ROOM;
+        platform::ReleaseFile(file);
+        return result;
+    }
+    for (int i = 0; i < frame_count; ++i)
+        frames[i] = ReadFrame(data + frames_at + (size_t)i * 2);
+    platform::ReleaseFile(file);
+
+    /* The last agreement: the frames are the file's last bytes. The file
+       holds exactly the sample — no short read presented as a sample, and
+       nothing after the frames the loader would half-understand. */
+    if (frames_at + frames_bytes != size) {
+        ArenaRollback(arena, mark);
+        result.error = SAMPLE_MALFORMED;
+        return result;
+    }
+
+    result.sample.frames = frames;
+    result.sample.frame_count = frame_count;
+    result.sample.rate = (int)rate;
+    result.sample.channels = (int)channels;
+    result.error = SAMPLE_OK;
+    return result;
+}
+
 } /* namespace engine */
diff --git a/src/audio.h b/src/audio.h
index 11264b0..f05c1b0 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -3,12 +3,19 @@
 // Lesson 059: sound is data before it is sound. A sample is a frame of
 // amplitude — one number saying where the speaker sits at that instant —
 // and a sound is a run of those frames at a fixed rate. Nothing here knows
-// about devices, files, or mixing: this is the format the engine's output
-// speaks, the format lesson 061's loader accepts and refuses everything
-// else, and the format the mixer of lessons 063-065 sums.
+// about devices or mixing: this is the format the engine's output speaks
+// and the format the mixer of lessons 063-065 sums.
+//
+// Lesson 061: the format is now loadable. A sample is authored as a file
+// — the same bytes GenerateTone computes, in a RIFF/WAVE container — and
+// the loader below either hands over the complete sample or names what
+// went wrong. From there on the sample carries its own playback facts, so
+// lesson 062 plays it from the sample alone.
 #ifndef AUDIO_H
 #define AUDIO_H
 
+#include "arena.h"
+
 namespace engine {
 
 /* The engine's sample format: 16-bit signed frames at this rate. One
@@ -32,6 +39,34 @@ constexpr int AUDIO_OUTPUT_CHANNELS = 1;
 void GenerateTone(short *frames, int frame_count, double frequency,
                   double amplitude);
 
+/* A loaded sample: the frames, and the facts playback needs carried with
+   them — its length in sample frames and its format. */
+struct Sample {
+    short *frames;
+    int frame_count;
+    int rate;
+    int channels;
+};
+
+enum SampleError {
+    SAMPLE_OK = 0,
+    SAMPLE_MISSING,   /* the file is not there or cannot be read */
+    SAMPLE_MALFORMED, /* the bytes are not a complete sample in the engine's format */
+    SAMPLE_NO_ROOM,   /* the arena had no room for the frames */
+};
+
+struct SampleResult {
+    Sample sample;
+    SampleError error; /* SAMPLE_OK exactly when sample.frames is non-0 */
+};
+
+/* Loads a sample from a RIFF/WAVE file. The container is walked chunk by
+   chunk and byte by byte — no library reads it — and anything that is not
+   a complete sample in the engine's format is refused typed. The frames
+   are copied into the arena and the file's own bytes go back to the OS:
+   what the engine keeps is its copy, and a refused load keeps nothing. */
+SampleResult LoadSample(Arena &arena, const char *path);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index 5044acc..e46c5bd 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -126,6 +126,42 @@ int Run(void)
     }
     TileSheet &sheet = tiles_loaded.sheet;
 
+    /* Lesson 061: the run's sound as a file's bytes. A sample is frames
+       of amplitude in a container, and the load either yields the
+       complete sample or names what went wrong — like every asset above.
+       A failure ends the run by name, like every asset above. */
+    SampleResult sample_loaded = LoadSample(arena, "assets/tone.wav");
+    if (sample_loaded.error != SAMPLE_OK) {
+        switch (sample_loaded.error) {
+        case SAMPLE_MISSING:
+            std::fprintf(stderr,
+                         "engine: assets/tone.wav: could not load (missing)\n");
+            break;
+        case SAMPLE_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: assets/tone.wav: could not load (malformed)\n");
+            break;
+        default:
+            std::fprintf(stderr,
+                         "engine: assets/tone.wav: could not load (no room)\n");
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Sample &sample = sample_loaded.sample;
+
+    /* The byte-level check, before anything is played: the sample's facts
+       and its first frames — the same bytes lesson 059 computed, now read
+       from a file instead. */
+    std::printf("engine: sample: %d frames at %d Hz, %d channel%s, first frames:",
+                sample.frame_count, sample.rate, sample.channels,
+                sample.channels == 1 ? "" : "s");
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+        std::printf(" %d", (int)sample.frames[i]);
+    std::printf("\n");
+
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
     double last = started;
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Write the file back *(extend-the-code)*

The loader reads this lesson's container; teach the engine to write one.
Add a writer beside the loader — the header the lesson walks, assembled
by hand into bytes, and the frames little-endian after it, through the
seam's whole-file write — in the engine's format and no other. Then make
the run a round trip: write the loaded sample to a file of your own, load
that file back with `LoadSample`, and compare every frame against the
sample that went out — report the first frame that disagrees, or that
every frame agrees. Compare the file you wrote against `assets/tone.wav`
outside the run. What does the round trip prove about the format that a
hexdump of the file cannot?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-061/ex1.md)

### Exercise 2 — The file that ends too soon *(fix-the-crash)*

A claim is not bytes, and this exercise makes that cost visible. In the
chunk walk in `LoadSample`, take out the refusal that stops a chunk whose
size runs past the file. Then cut `assets/tone.wav` to 20000 bytes —
inside its `data` chunk — with its RIFF size following the cut, so the
container's first claim still agrees with the bytes and the `data`
chunk's claim is the lie that remains. Rebuild with lesson 013's
instrument — AddressSanitizer, through `./build.sh`'s `CXXFLAGS` and
`LDFLAGS` overrides — and run. The load no longer fails typed; read what
the sanitizer names. Then fix the loader so the overrun is impossible
even where a check is missed: the copy itself must refuse to take a frame
from bytes the file does not have, the load must fail typed, and no
partial frames may survive in the arena. Prove the same run is clean
under the sanitizer and reports the typed failure.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-061/ex2.md)

---

**Part:** [Part 3 — sound](../../index.md) ·
**Previous:** [Lesson 060 — the stream's shape](lesson-060-stream.md) ·
**Next:** [Lesson 062 — the sample's playback facts](lesson-062-playback.md) ·
**Code tag:** [`lesson-061`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-061)
