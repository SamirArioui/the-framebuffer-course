# Design

## Context

Part 2 ended with a measured closing demo: a world drawn entirely by the
engine's own renderer — tilemap through the camera, a mover stopped by
collision queries, text over it — every phase in the frame record. What
it has never done is make a sound. The MVD's audio line needs a mixer
with per-channel playback (O7), and Part 5's L14 is integration only.

Constraints that shape the how:

- **No external libraries in student-visible code** — the OS stays
  behind the seam (lesson 042's contract, `tools/check-boundary.sh`'s
  rule). The sound *device* is OS business like the window is.
- **The language law (lesson 026) still governs `src/`** — and it has
  never admitted threads. Whatever feeds the device does it the way this
  engine does everything: one loop, synchronous calls, memory from the
  arena.
- **This authoring machine has no sound hardware** (`/dev/snd` holds
  only `timer`), no ALSA development headers at planning time, and no
  passwordless sudo. Verification must therefore be honest without a
  speaker: the mixer is data→data and checkable byte-for-byte, and the
  submission path must run against a software device.
- **The toolchain is curriculum** (curriculum spec): whatever sound work
  introduces gets taught — and the loop's pacing is exactly the kind of
  thing this course refuses to leave implicit.

## Goals / Non-Goals

**Goals:**
- One mixer, per-channel, that the game's music and effects both route
  through — Part 5's L14 wires it, never redesigns it.
- Sound as data first: samples load like every other asset (whole-file,
  typed failures, arena memory) and mix without a device attached.
- The device path kept as thin as the window path: an interface, one
  implementation file, typed failures.

**Non-Goals:**
- No synthesis or effects: no oscillators-as-features, envelopes,
  filters, reverb. Sample playback and mixing only.
- No compressed formats (OGG/MP3) — no libraries means PCM in a
  container, parsed by hand.
- No threads and no callbacks crossing the seam in the device's
  direction.
- No resampling: one sample rate for the engine and its assets.
- No 3D audio or panning model beyond per-channel volume (a panning
  exercise may exist; the contract stays volume).
- No Part 4's time-scale hook (O5 lives there) and no Part 4 services.

## Decisions

### D1. The Part 3 arc: 12 lessons, `lesson-059`…`lesson-070`, in five batches

| Batch | Lessons | Arc |
| ----- | ------- | --- |
| Sound as bytes | 059-060 | samples as frames of amplitude (PCM bytes, rate, the tone computed by code); the seam's audio output, and the loop that feeds it |
| The sample asset | 061-062 | the WAV container defined by hand; samples loaded whole into the arena with typed failures |
| The mixer | 063-065 | one channel (cursor, volume, end); the mix (sum, clamp, silence); channels allocated and reused |
| Music and effects | 066-068 | music as a looping channel; effects as one-shots; music + sfx together — **O7 delivered** |
| Close | 069-070 | the closing demo (world + sound in one measured loop); the mix's cost in the frame budget — the audio row |

Ordering makes each batch audible before the next needs it: bytes before
files, one sound before many, the pool before the routing. No mandatory
deep dive lands here (all four are placed); the concept-first material
(sound as samples; what the device does) lives inside 059-060's prose.

### D2. The engine's sound format: 16-bit signed PCM, 44,100 Hz, mono

Samples are mono 16-bit frames at 44.1 kHz; the output is the same rate,
stereo, with the mix duplicated across both channels. The WAV loader
accepts exactly this and refuses everything else typed. Alternatives:
stereo samples (double the asset bytes for no lesson value; panning
belongs to an exercise), floating-point mixing (floats are fine in this
engine but integer arithmetic is the teachable one for clamp/overflow),
multiple rates (resampling is a field of its own — out of scope).

### D3. No threads: the loop feeds the device, and the wait is paced

The seam grows a *synchronous* audio output: open at a fixed format,
submit a buffer of mixed samples, close. Because the device consumes at
its own rate, the run's wait for news may no longer block indefinitely —
`PumpEvents` wakes when the output needs the next buffer (a wait
bounded by the buffer horizon), the loop mixes and submits, and waiting
continues. The pacing is taught explicitly: sound is the first thing in
this engine that needs the loop to wake up *on a schedule*, and the
lesson says why. Alternatives: a mixer thread (threads are outside the
admitted subset and never taught), device-side callbacks (the OS calling
engine code across the seam — the boundary inverts), huge pre-filled
buffers (latency with none of the honesty). Trade-off recorded in the
risks: an idle run with music open now ticks at the audio's cadence.

### D4. The seam's audio interface mirrors the window's

`OpenAudioOutput(rate, channels)` returns a typed result naming the step
that failed (no device → `AUDIO_NO_DEVICE`, exactly like
`OPEN_NO_DISPLAY`); `SubmitSamples(const short *samples, int frames)`
hands the device a mixed buffer; `CloseAudioOutput` releases it. Three
functions beside the seam's fifteen — the interface names no OS type,
and `tools/check-boundary.sh`'s implementation list grows the file the
device lives in. Alternatives: one `Present`-style call per frame with
the mix inside the platform (the mix is engine work, not OS work — the
seam carries bytes, the engine makes them), or a query-style API the
engine polls (stateful and larger for no gain).

### D5. The mixer: fixed channels, integer volumes, clamped sums

`AUDIO_CHANNELS = 16`; each channel is a small struct — sample, frame
cursor, volume (0-256 fixed-point, 256 = full), loop flag, active flag.
The mix walks the active channels into a 32-bit accumulator per output
frame and **clamps** to 16-bit range — the spec's "clamp, never wrap".
Volumes are fixed-point so the mix is integer arithmetic a learner can
follow byte for byte; a `double` volume is the rejected alternative
(same behavior, one more format question). The music channel is
reserved (channel 0, never stolen); effects take channels 1-15, and a
full mixer **steals the oldest effect channel** — the documented reuse
policy the spec's busy scenario requires. Alternatives: refuse the new
sound (legal but silent-feeling), or steal by quietest (needs a notion
of loudness the course has not taught).

### D6. Samples are data before they are sound

`LoadSample` follows `LoadSprite`'s shape exactly: whole-file read
through the seam, a hand parser for the RIFF/WAVE header, the PCM data
copied into the arena, the file's bytes released — and typed failures
(`SAMPLE_MISSING`, `SAMPLE_MALFORMED`, `SAMPLE_NO_ROOM`) where the
format's claims and the file's bytes disagree. The sample struct carries
its length in frames beside its data (the spec's playback-facts
requirement) so the mixer never reasons about files. The WAV header's
chunk walk (RIFF/`fmt `/`data`) is the lesson's byte-level material —
the same habit as the PPM header, one chunk deeper.

### D7. Verification stays headless — and honest

Three layers, matching the machine's reality:

1. **The mix is data→data.** Mixing into a buffer and reading the bytes
   back checks every spec scenario that matters — summed channels,
   clamped overflow, silence from idle ones — with no device at all. The
   pixel-readback habit, for sound.
2. **The submission path runs against ALSA's `null` device** — the
   software device that accepts and discards samples with the same
   open/submit/close calls as real hardware (the Xvfb analog: the path
   runs; the pixels, here the speakers, are not the point).
3. **The typed failure is verified on a missing device** — request a
   device that does not exist and the run reports the named failure.

Measured claims (the mix's cost) come from the frame record's new
`audio` phase — real numbers in the frame-budget table's new row. What
this machine cannot verify — the sound a real speaker makes — is routed
to port exercises, exactly as desktop rendering was in Parts 1-2.

*Toolchain note, recorded at planning time:* this authoring machine has
no PCM device and no ALSA headers; task 1.1 installs `libasound2-dev`
(the `libx11-dev` analog, added to the README's prerequisites) and
verifies the `null` device before any lesson quotes a run.

### D8. The frame record grows one named phase: `audio`

The mix is measured like every other subsystem — one `audio` phase
inside the frame, summed in the account, carried in the log line, and
appearing as a row in lesson-058's frame-budget table (the frame-
accounting spec's "at least" clause already allows it — no spec
change). The named-phase discipline from Part 2 (inside the phase, never
instead of it) carries unchanged.

### D9. Exercises move to the Parts 3-4 band

1-2 **mixed** exercises per lesson (conventions §4) — any of the six
archetypes, not the "make it yours" pairing Parts 1-2 leaned on. Sound
earns the mix: measure-the-performance on the mixer's cost,
predict-the-output on clamping and channel stealing, port-to-your-own-
machine on real speakers, fix-the-crash on a truncated WAV.

## Risks / Trade-offs

- [The paced wait changes the loop's cadence] → an idle run with music
  open ticks at the audio's buffer horizon rather than sleeping between
  news. Mitigation: taught explicitly in the pacing lesson; the frame
  record's phases keep the timing honest; the behavior is named in the
  closing review's warts.
- [Learners' machines have no ALSA] → the typed-failure path is a
  contract, not an afterthought (a run without sound still runs); the
  README names the prerequisite; port exercises cover other machines.
  Same risk routing as Part 1's X11.
- [The sample format churns after learners have files] → the format is
  fixed in the lesson that defines it (the standing rule); later lessons
  read more of it, never reinterpret it.
- [The mixer grows into an effects suite] → the non-goal is binding:
  volumes, loops, and clamping only; anything else is extras.
- [Mixing cost surprises the budget] → the `audio` phase is named from
  the first mixer lesson, and the closing lesson puts its row in the
  budget table (O1's pattern, third use).
- [Integer overflow in the mix] → the clamp is a spec scenario with its
  own check — saturation tested at deliberate overload.

## Migration Plan

Content-only lessons on the Part 2 engine; each lesson commits and tags
as it lands (`lesson-059` … `lesson-070`), prose co-committed, assets in
`assets/` in the batch that first plays them. The change closes with
`plan/part3-review.md` (velocity, density, measurements re-verified,
frozen-prefix recommendation updated per `plan/part2-review.md`).
Rollback: content-only; lessons are revertable per commit and the
revision policy governs anything published.

## Open Questions

- Whether panning (two-sided placement of a channel's volume) is a
  lesson or an exercise — safe to settle at authoring time; it changes
  no contract (the channel's volume is the contract).
- Whether the music asset is one long loop or a short cue set — asset
  content, no contract; settled when the music batch is authored.
