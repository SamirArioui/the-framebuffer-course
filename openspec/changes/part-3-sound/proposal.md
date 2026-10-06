# Proposal

## Why

Part 2 delivered the software renderer, but the MVD's frozen checklist
still has an unchecked line — *audio: music + sfx through our own mixer*
— and the backward-propagation backlog names its home: **O7, per-channel
audio, Part 3**. Part 5's L14 is an *integration* lesson ("music and sfx
routed through the mixer's per-channel playback"), so the mixer itself
must be finished here; nothing in Part 4-5 can sound like anything until
Part 3 gives the engine samples, a mixer, and a way to reach the
speakers.

## What Changes

- **Author Part 3's lesson arc** per the foundation design (sound built
  bottom-up on the Part 2 engine: sound as bytes, the seam's audio
  output, the sample format defined by hand, one sample played, the
  mixer, music and sfx on channels, and the measured close) — growing
  `src/` linearly from lesson-058's closing state (`lesson-059` onward).
- **Deliver the MVD's Part 3 obligation (O7)**: a mixer with
  **per-channel playback** — fixed channels with cursors, volumes, and
  loop flags, summed per frame into the output buffer — with **music and
  sfx routed as channels** (a looping music channel and one-shot sfx
  channels), so Part 5's L14 is wiring, not invention.
- **Load sound through the seam's whole-file I/O** — sample files read
  with the contract lesson 037 fixed, parsed by hand, and kept in the
  engine's arena (lessons 040-041): the same asset habit as sprites,
  fonts, and maps, extended to sound.
- **Grow the platform seam with audio output** — the OS's sound device
  lives behind the seam like its window and clock do; engine code keeps
  naming no OS (`tools/check-boundary.sh` keeps it checked), and a
  second OS replaces the implementation file and nothing else.
- **Keep the toolchain curriculum going** (curriculum: Toolchain is
  curriculum): the build flags sound work introduces are explained as
  they appear, the mixer's cost is measured in the frame record rather
  than guessed, and the loop's pacing teaches why sound changes when a
  frame may end.
- **Turn the continuity machinery onto the sound**: `src/` continues its
  one linear history tagged `lesson-NNN` (course-wide numbers continue
  from `lesson-059`), prose co-committed with every code step, and
  exercises at the **Parts 3-4 density** — 1-2 mixed exercises per
  lesson (conventions §4).
- **Close with a part-boundary review** in `plan/`: velocity against the
  10-20 h/week review budget, exercise counts against the conventions
  table, the measured claims re-verified, and the frozen-prefix
  recommendation updated per `plan/part2-review.md`.

## Capabilities

### New Capabilities

- `audio-samples`: sound as loadable data — the sample asset format
  (PCM in a RIFF/WAVE container) defined by hand, loaded whole through
  the seam into the arena, and either complete or a typed failure — so
  sound can be authored, loaded, and tested without an output device.
- `audio-mixer`: per-channel playback and mixing — channels carrying
  samples with playback position, volume, and looping, summed every
  frame into the output buffer without silent failures on overflow, and
  music and sfx routed as channels (the MVD's O7).

### Modified Capabilities

- `platform-layer`: the seam grows **audio output** — opening the
  device for a fixed sample format, accepting mixed sample buffers, and
  pacing the run so the loop feeds it — as a new requirement beside the
  window, input, clock, memory, and file I/O it already owns. Engine
  code still sees only the interface.

## Impact

- **`src/`**: grows from lesson-058's closing state with the sample
  loader, the mixer (channels and the mix), and the seam's audio output
  (interface plus the Linux implementation); the frame record grows its
  audio phase beside the render's named subsystems (the frame-budget
  table gains a row). All engine code stays inside the language law and
  behind the platform seam.
- **`assets/`** (grows): course-owned sample files — at least one music
  loop and one effect — in the format the lessons define by hand.
- **`book/`**: Part 3's lesson pages (one per lesson, 1-2 mixed
  exercises each with diff solutions) and the Part 3 section in
  `SUMMARY.md`; the stability-horizon banner keeps naming the frozen
  prefix and the volatile tail.
- **Tags**: `lesson-059` onward, course-wide sequence, one per lesson,
  co-committed with prose; `git diff` between tags equals the code steps
  between them.
- **`plan/`**: Part 3's boundary-review notes (velocity, density, the
  measured claims, the frozen-prefix recommendation).
- **Scope size**: a whole part (~12-15 lessons) implemented in
  birth-and-growth batches, with the boundary review closing the change.
