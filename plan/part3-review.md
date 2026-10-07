# Part 3 boundary review — sound (our own mixer)

Part 3 closed with lessons `lesson-059`…`lesson-070` — 12 lessons,
prose + one code step + 2 mixed exercises with diff solutions each, one
commit and one tag per lesson, co-committed per the authoring contract.
This review is the part-boundary record: velocity against the review
budget, exercise density against the conventions table, the toolchain
actually used, the measured claims re-verified, and the frozen-prefix
recommendation updated from `plan/part2-review.md`.

## The arc, as it landed

| Batch | Lessons | Arc | Status |
| ----- | ------- | --- | ------ |
| Sound as bytes | 059-060 | samples as frames of amplitude (16-bit PCM, the rate, a tone computed by code); the seam's audio output and the loop that feeds it — the paced wait | tagged |
| The sample asset | 061-062 | the WAV container defined by hand (the RIFF chunk walk); samples loaded whole into the arena with typed failures; the playback facts | tagged |
| The mixer | 063-065 | one channel (cursor, volume, end); the mix (sum, clamp, silence); channels allocated and reused | tagged |
| Music and effects | 066-068 | music as a looping channel; effects as one-shots; music + sfx together — **O7 delivered** | tagged |
| Close | 069-070 | the closing demo (world + sound in one measured loop); the mix's cost in the frame budget — the audio row | tagged |

The MVD's Part 3 obligation (**O7, per-channel audio**) is delivered and
demonstrable: a mixer with fixed channels carrying cursors, volumes and
loop flags, summed every frame into the output buffer, with music and sfx
routed as channels. Part 5's L14 is wiring, not invention.

## Authoring velocity vs. the 10-20 h/week estimate

`plan/part0-review.md` priced the curriculum at roughly 570 h at an
assumed 10-20 h/week of authoring, and recorded that the estimate is off
by more than an order of magnitude as a prediction of *writing* time
under agent-assisted authoring. Part 3 confirms it: the 12 lessons were
authored, verified, and tagged in a single machine-assisted session
across 2026-10-06/07.

Part 0's reading carries forward unchanged, and Part 3 sharpens it:

- **Writing is cheap; verification is not.** Every quoted number in
  these twelve pages came from a run that was re-run against the exact
  committed state, and every solution patch was applied at its own tag.
  Three numbers were wrong on first draft and were caught that way: the
  paced-wait input figures (measured before a gate change, re-measured
  after), the mix's total audio work (claimed to *drop* when the horizon
  doubled; measured, it is flat), and a buffer count that fired one
  buffer late. The check is what caught them.
- **Estimate Part 3's cost in review-weeks.** Keep 10-20 h/week as the
  human *review* budget: 12 lessons ≈ **1-2 review-weeks**, on Part 0's
  calibration (25 lessons ≈ 2-4 review-weeks). Plan the review pass
  around that, not around keystrokes.
- **Machine-assisted authoring needs its own guardrails**, still: the
  authoring contract, real-output-only quoting, and patch-application
  checks. Expect the same error rate and keep the same checks.

## Exercise density against the conventions table

Conventions §4 sets Parts 3-4 at **1-2 mixed exercises** per lesson. Every
lesson counted:

| Lesson | Exercises | Distinct archetypes |
| ------ | --------- | ------------------- |
| 059 sound as samples | 2 | 2 |
| 060 the stream's shape | 2 | 2 |
| 061 the WAV container | 2 | 2 |
| 062 the sample's playback facts | 2 | 2 |
| 063 one channel | 2 | 2 |
| 064 the mix | 2 | 2 |
| 065 channel allocation | 2 | 2 |
| 066 music as a loop | 2 | 2 |
| 067 effects as one-shots | 2 | 2 |
| 068 music and effects together | 2 | 2 |
| 069 the closing demo | 2 | 2 |
| 070 the mix's cost in the frame budget | 2 | 2 |
| **Total** | **24** | — |

**Every Part 3 density expectation is met**: 12 of 12 lessons have 2
exercises, inside the 1-2 band, and no lesson repeats an archetype, so
each lesson's pair is genuinely *mixed* (not the "make it yours" pairing
Parts 1-2 leaned on).

Archetype spread across the part: predict-the-output 7, extend-the-code
7, port-to-your-own-machine 4, measure-the-performance 4, fix-the-crash
1, explain-in-prose 1 — **all six archetypes used**, matching design
D9's plan for what sound earns (measure the mixer's cost, predict the
clamp and the stealing, port to real speakers, fix a truncated WAV).

## Toolchain actually used

| Tool | Version | Used for |
| ---- | ------- | -------- |
| Linux | 6.6.87.2-microsoft-standard-WSL2 x86_64 (Ubuntu 24.04.4 LTS) | the authoring machine |
| gcc / g++ | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) | every build |
| ALSA library / headers | 1.2.11 (`libasound2-dev` 1.2.11-1ubuntu0.3) | the seam's sound implementation; the `null` device every audio check ran against |
| Xvfb | 2:21.1.12-1ubuntu1.8 | the headless display every window check ran on |
| xdotool | 1:3.20160805.1 | scripted input checks (060, 069) |
| mdBook | 0.5.4 (pinned) | every page render check |
| openspec | 1.14.0 | every planning validation |
| git | 2.43.0 | the history, tags, and every patch application |

No sound hardware: `/dev/snd` holds only `timer`, and ALSA's `default`
device does not open. Every audio check ran against the software `null`
device.

One toolchain finding the part introduced, recorded at task 1.1 and
taught in lesson 059: ALSA's header does **not** compile under strict
`-std=c11` (it redefines `struct timespec`); the engine is C++ and takes
it unchanged, so the only build change the part needed was `-lasound` on
the link line.

## Measured claims, re-verified

Each claim below was re-run at its own lesson tag on the committed state.

| Claim | Lesson | Result |
| ----- | ------ | ------ |
| Tone bytes: first frames `0 513 1024 1531`, peak `8191` at frame 25 | 059 | exact |
| Paced wait, idle: ticks at the buffer horizon | 060 | 176 then 175 frames / 3 s (~59/s against the 16.7 ms horizon) |
| Paced wait, under input: feeds stay at the horizon rate | 060 | 218 frames, 175 feeds, 43 news-woken at `audio 0.000 ms` |
| No output: the bound disappears | 060 | 1 frame in the same 3 s |
| A sample loads completely; its frames match the file | 061 | 22050 frames, first eight `0 513 1024 1531 2032 2525 3009 3480` |
| Missing and malformed files fail typed | 061 | `(missing)` and `(malformed)`, run ends by name; five corruptions each refused |
| A sample plays exactly its frame count | 062 | 22050 frames in 30 buffers of 735 |
| Per-channel volume is checkable byte for byte | 063 | at 128 of 256: `0 256 512 765 1016 1262 1504 1740` |
| The mix sums the active channels | 064 | `0 384 768 1147 1524 1893 2256 2610` |
| Overflow clamps instead of wrapping | 064 | 40955 → 32767, −40955 → −32768; 4 channels land on 32764 |
| Idle channels contribute nothing | 064 | 15 idle channels change the mix byte-identically not at all |
| First free channel; the oldest effect stolen when busy | 065 | effects 1-15 → channels 1-15; effect 16 steals channel 1; music channel untouched |
| A looping channel continues and wraps | 066 | wraps at 133035 / 265335 / 397635 frames played; `MixerStop` ends it |
| An effect plays as a one-shot and its channel returns | 067 | 8820 frames in 12 buffers; the next effect reuses channel 1 |
| Music and effects mix together | 068 | `0 584 1163 1732 2286 2820 3330 3813`; `audio` phase avg 0.038 ms |
| The closing demo runs with every capability at once | 069 | 656 frames, 582 buffers fed, 25 effects, 3 music wraps |
| The audio row is a real measurement | 070 | table row-for-row identical to the average of the run's `frame N:` lines |

The frame's arithmetic closes with the new row: at the lesson-070
numbers `0.001 + 0.045 + 1.523 + 0.732 = 2.302 = total`, and without the
row the rows fell 0.045 ms short of the total — the gap *was* the phase.

## Integration checks (task 7.2)

From a clean checkout of `lesson-070`, following only `README.md`:

| Check | Result |
| ----- | ------ |
| `./build.sh` | `build: OK (14 source(s) compiled -> build/game)` — **warning-free** |
| `mdbook build` | `HTML book written to site` |
| `openspec validate --all` | `Totals: 11 passed, 0 failed (11 items)` |
| the closing demo headlessly | runs, feeds, prints its account, closes cleanly (`engine: closed`) |

Tags: `lesson-059`…`lesson-070` are consecutive, and for all twelve the
`src/` diff between consecutive tags is byte-identical to that lesson's
own commit's code step. Only lesson commits touch `src/` in the range.

## Recommendation: extending the frozen prefix

Following `plan/part2-review.md`'s discipline (no extension on authoring
evidence alone):

- **Do not extend the prefix on this review.** Part 3's 12 lessons have
  the author's verification behind them and no reviewer's pass.
- **When the review passes land, freeze in blocks:** `lesson-026`…`lesson-043`
  first (Part 1's dependency block), then `lesson-044`…`lesson-058`
  (Part 2), then `lesson-059`…`lesson-070` as the third block. Freezing
  Part 3 first would freeze sound built on unfrozen seams.
- **Part 3's block has one new tail risk: the quoted timing numbers.**
  The `audio` phase figures (0.038 ms and friends) move with the machine
  and the moment, unlike Part 2's instruction counts. The block-freeze
  review should re-run the lesson-070 table-vs-log reconciliation rather
  than re-read it. The *byte-level* claims (tone frames, mix sums, clamp
  values, channel assignments) are deterministic and will reproduce
  exactly.
- **The resync path is unchanged** and covers this part: `git checkout
  lesson-NNN -- src/`.

## Review pass status (pre-freeze)

The independent review pass over Part 3 has **not** run; this review
records the author's verification, which is its input. What a reviewer
should re-run, in order of what breaks first:

1. `./tools/check-boundary.sh`, `openspec validate --all`, `mdbook build`,
   `./build.sh` (seconds; the build must be warning-free).
2. The closing demo headlessly (lesson-069's transcript), with the
   frame-budget table reconciled against the run's `frame N:` lines the
   way lesson 070's review section shows.
3. The byte-level check tables: 063's volume scaling, 064's sums and its
   clamp, 065's channel assignments, 067's pool return — each is
   printable and exact.
4. The loader's typed failures (061): the five corruptions — truncated
   `data`, `data` size not a whole number of frames, a format claiming
   another rate, a `RIFF` size that disagrees with the file, trailing
   bytes.
5. Every solution patch: `git apply`, build, run, reverse — 24 patches.
6. The asset facts: `assets/tone.wav` must hold lesson 059's tone exactly;
   `assets/music.wav` must wrap with no seam.

## Known warts carried forward (intentional, documented in-prose)

- **An idle run with sound ticks at the audio cadence** (060) — ~59 Hz
  instead of sleeping between news. The design recorded this trade-off
  and the lesson teaches it: sound is the first thing in this engine that
  needs the loop to wake on a schedule. A run *without* an output keeps
  the old behaviour exactly.
- **`null` never pushes back.** The paced wait's real purpose — keeping
  the device from starving — cannot be demonstrated on this machine,
  because ALSA's `null` device accepts samples instantly at every buffer
  size tried. What was verified is the engine's half: the wait is bounded
  by the buffer horizon and the run ticks at the audio cadence. Real
  back-pressure is routed to lesson 060's port exercise. The lessons say
  this out loud rather than implying more than was checked.
- **The `audio` row mixes and submits together.** On `null` the submit
  returns immediately so the row reads as the mix's cost; on real
  hardware a submit can wait for room in the device's buffer, the way
  `present` includes the copy's sync. Lesson 070 names this.
- **The sample loader refuses trailing bytes.** A WAV whose `data` chunk
  is not the file's last bytes fails typed, even though such a file is
  legal WAV. This is the course's format being defined by hand, and it
  is taught — but a WAV exported from a real editor (which often adds
  `LIST`/`INFO` chunks) will be refused. Worth a decision before the
  part is published: accept trailing chunks, or keep the strictness and
  say so in the loader's own documentation.
- **The engine's mix is mono and the platform duplicates it** into the
  device's stereo layout. No panning model (the design's non-goal); the
  channel's volume is the contract.
- **The sample assets are course-owned and their generators are not in
  the repository.** `assets/tone.wav` holds lesson 059's tone exactly;
  `music.wav` and `effect.wav` were produced by throwaway authoring
  scripts. A learner who wants their own samples has lesson 061's
  "write the file back" exercise and the format the lessons define.
- **Bookkeeping commits sit between some lesson tags** ("Record task
  progress", planning artifacts only). A raw `git diff lesson-N-1
  lesson-N` therefore includes `openspec/` changes; the `src/` diff is
  exactly that lesson's code step, verified for all twelve. Part 1's
  close did the same.
- **Two authoring decisions were taken where the artifacts were
  ambiguous**, recorded in the change's `notes.md`: the `audio` phase is
  named at lesson 060 (the frame has audio work from the lesson where the
  loop feeds the device) while its budget-table row first appears at
  lesson 070 as the task requires; and the seam carries the engine's mono
  samples while the platform maps them into the device's own layout.

*(Everything above is measured from the tagged lesson states. The closing
demo's run, the table-vs-log reconciliation, the byte-level check tables,
the loader's typed failures, and every solution patch were run on the
authoring machine before this review was written.)*
