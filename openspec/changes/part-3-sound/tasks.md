# Tasks

## 1. Part 3 authoring setup

- [x] 1.1 Verify the authoring/verification toolchain for the sound batch: the ALSA development headers (`libasound2-dev`, added to the README's prerequisites), the submission path against ALSA's `null` device, the typed-failure path on a device that does not exist, and the boundary check's implementation list ready for the audio file; verify the exact commands run as written and record the tool versions used.
- [x] 1.2 Audit the seam and the frame record against the `platform-layer` delta and `frame-accounting` ("at least" clause): record in the change notes what Part 3 must grow (audio output on the seam; the record's `audio` phase) and verify the audit is recorded before the first sound lesson is authored.

## 2. Sound as bytes (lesson-059…060)

- [x] 2.1 Author lesson-059 (sound as samples: frames of amplitude — 16-bit PCM bytes, the sample rate, a tone computed by code and played) — prose + one code step + 1-2 mixed exercises written before the prose with diff solutions linked after each prompt; verify the tone's bytes are what the lesson claims and the run plays through the `null` device as documented, the page renders, and the co-committed commit is tagged `lesson-059`.
- [x] 2.2 Author lesson-060 (the stream's shape: the output's format and rate, and the loop that feeds it — the paced wait that keeps the device from starving) with its exercises and diff solutions; verify the paced wait runs as documented under scripted input and idle, the page renders, and the commit is tagged `lesson-060`.

## 3. The sample asset (lesson-061…062)

- [x] 3.1 Author lesson-061 (the WAV container defined by hand: the RIFF chunk walk, a sample loaded whole into the arena, typed failures) with its exercises and diff solutions; verify a sample loads completely, malformed and missing files fail typed as documented, the page renders, and the commit is tagged `lesson-061`.
- [x] 3.2 Author lesson-062 (the sample's playback facts — length and format carried with the data — and a loaded sample played to its end) with its exercises and diff solutions; verify a sample plays exactly its frame count as documented, the page renders, and the commit is tagged `lesson-062`.

## 4. The mixer (lesson-063…065)

- [x] 4.1 Author lesson-063 (one channel: the playback cursor, per-channel volume, and the end of a sample) with its exercises and diff solutions; verify a channel plays its sample to its end as documented, the page renders, and the commit is tagged `lesson-063`.
- [x] 4.2 Author lesson-064 (the mix: active channels summed per buffer, clamped instead of wrapped, idle channels silent) with its exercises and diff solutions; verify the mix's sums and its clamp behave as documented with byte-level readback, the page renders, and the commit is tagged `lesson-064`.
- [x] 4.3 Author lesson-065 (channel allocation: the first free channel, and the busy case's documented reuse policy — the oldest effect channel stolen) with its exercises and diff solutions; verify the busy case behaves as documented under scripted playback, the page renders, and the commit is tagged `lesson-065`.

## 5. Music and effects (lesson-066…068)

- [x] 5.1 Author lesson-066 (music as a looping channel: the loop flag, playback wrapping at the sample's end, stopping explicitly) with its exercises and diff solutions; verify the loop continues and wraps as documented, the page renders, and the commit is tagged `lesson-066`.
- [x] 5.2 Author lesson-067 (effects as one-shots: play once to the end, the channel returns to the pool, per-sound volume) with its exercises and diff solutions; verify one-shot playback and channel return as documented, the page renders, and the commit is tagged `lesson-067`.
- [x] 5.3 Author lesson-068 (music and effects together through the one mixer — O7 delivered) with its exercises and diff solutions; verify music and sfx mix together as documented with the frame record's audio phase measured, the page renders, and the commit is tagged `lesson-068`.

## 6. Close (lesson-069…070)

- [ ] 6.1 Author lesson-069 (the closing demo: one measured frame loop with the world and its sound — every Part 3 capability at once) with its exercises and diff solutions; verify the demo runs as documented under the headless check, the page renders, and the commit is tagged `lesson-069`.
- [ ] 6.2 Author lesson-070 (the mix's cost in the frame budget: the audio row's first appearance — measured, not guessed) with its exercises and diff solutions; verify the reported numbers are real measurements of the demo's frames, the page renders, and the commit is tagged `lesson-070`.

## 7. Part 3 boundary review and integration checks

- [ ] 7.1 Write `plan/part3-review.md` recording authoring velocity against the 10-20 h/week review budget from `plan/part0-review.md`, exercise counts per lesson against the conventions density table (Parts 3-4: 1-2 mixed exercises), the toolchain versions used, the measured claims re-verified, and the frozen-prefix recommendation updated per `plan/part2-review.md`; verify every Part 3 lesson is counted and every Part 3 density expectation is checked.
- [ ] 7.2 From a clean checkout following only `README.md`: run `./build.sh`, run `mdbook build`, run `openspec validate`, and run the closing demo headlessly; verify all succeed and `git tag` shows consecutive `lesson-059`…`lesson-070` where each consecutive tag diff equals that lesson's code step.
