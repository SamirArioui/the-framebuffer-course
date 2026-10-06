# Part 1 (M2) boundary review

Recorded at the close of `part-1-platform-layer` for the next part's
planning *(curriculum: part-boundary review; design: Migration Plan)*.
Measured against `plan/conventions.md` and the authoring plan's estimates.

## What shipped

18 lessons (`lesson-026` … `lesson-043`), each prose + exactly one
co-committed code step + its extensions and diff solutions, one commit per
lesson, one tag per lesson:

| Batch | Lessons | Arc | State |
| ----- | ------- | --- | ----- |
| Birth | 026 | the codebase born from a blank `main`; `build.sh` live; `sandbox/` deleted; the C++ subset as the engine's language law | tagged |
| Window | 027-029 | the seam and the first X11 window; the event pump; clean close and error paths | tagged |
| Pixels | 030-031 | the framebuffer as our own bytes; presentation through the seam | tagged |
| Input | 032-034 | polled input state; latching and focus; the first interactive frame | tagged |
| Clock | 035-036 | the monotonic platform clock; frame time as measured data | tagged |
| Files | 037-038 | whole-file reads; writes and round-trips | tagged |
| Memory | 039-041 | the virtual-memory deep dive; reservation-backed buffers; arenas | tagged |
| Seam | 042-043 | the interface audited as a contract; the closing demo | tagged |

Totals: 18 lesson pages, **36 extensions**, 36 solution patches + 36
walkthroughs, 18 tags, one new tool (`tools/check-boundary.sh`), and one
platform seam of 15 functions that the audit (lesson 042) checked against
the spec's single-boundary scenarios. The virtual-memory deep dive sits in
lesson-039 where the framebuffer's size makes it necessary (foundation
D9), and `sandbox/` died in the lesson-026 code step as designed — every
Part 0 state still retrievable from its tags.

## Exercise density vs. the conventions table

Conventions §4 asks Parts 1-2 for **1-2 "make it yours" extensions per
lesson**. Every lesson measured (extensions authored, patches on disk, one
walkthrough per extension):

| Lesson | Ext. | Lesson | Ext. | Lesson | Ext. |
| ------ | ---- | ------ | ---- | ------ | ---- |
| 026 | 2 | 032 | 2 | 038 | 2 |
| 027 | 2 | 033 | 2 | 039 | 2 |
| 028 | 2 | 034 | 2 | 040 | 2 |
| 029 | 2 | 035 | 2 | 041 | 2 |
| 030 | 2 | 036 | 2 | 042 | 2 |
| 031 | 2 | 037 | 2 | 043 | 2 |

**18 of 18 lessons counted; 18 of 18 within the 1-2 band** (all eighteen
at 2; 36 extensions total). All at the band's top is a deliberate default,
not a verdict: if the review finds any lesson padded, its second extension
is the first thing to cut — the band's floor exists for that.

Archetypes stayed inside the bounded six; all six appear in Part 1
(extend-the-code 14, explain-in-prose 8, predict-the-output 5,
port-to-your-own-machine 5, fix-the-crash 3, measure-the-performance 1).
Two of Part 1's extensions are **deliberate teaching states** — lesson
040's exercise 2 (the crash that is protection working) and lesson 041's
exercise 2 (the use-after-rollback ASan cannot see) — each flagged in its
prompt and walkthrough as designed, per the conventions' rule. Every patch
was applied against the lesson's end state and its build/run verified
before publication, the Part 0 review's standard kept.

## Authoring velocity vs. the 10-20 h/week estimate

Part 0's review found writing cheap and verification deep. Part 1 moved
the same way, faster: the 18 lessons were authored, verified headlessly,
and tagged in a **single machine-assisted session** (2026-10-06) —
including the batch of new verification machinery this part needed (Xvfb
window checks, pixel readback, scripted keyboard input, signal and race
checks).

The reading for Part 2 planning is unchanged and now better evidenced:

- **Verification is still the work.** Every quoted output in Part 1 is
  real: window geometry under Xvfb, pixel readbacks matching the
  framebuffer byte-for-byte, frame-time measurements, `/proc` maps, RSS
  deltas, ASan runs, SIGSEGV transcripts. Building the harness for those
  checks consumed a meaningful share of the session — and it is the part a
  reviewer must re-run, not read.
- **Estimate Part 2 in review-weeks.** 18 lessons ≈ **2-3 review-weeks**
  at the 10-20 h/week human budget (contents + re-run transcripts + patch
  application each). The writing burst should stay scheduled around that.
- **Machine-assisted authoring keeps needing its guardrails** and they
  kept paying: real-output-only quoting caught several near-misses this
  part (a reordered `nm` listing in lesson 042, a claimed but unrun
  transcript in lesson 037's exercise 1, an off-by-one claim about the map
  in lesson 039's exercise 1 — all corrected before commit).

## Toolchain actually used

| Tool | Version | Used for |
| ---- | ------- | -------- |
| g++ / gcc | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) | every build (`build.sh`, exercise builds) |
| gdb | 15.1 (Ubuntu 15.1-1ubuntu1~24.04.1) | available; Part 1 quoted no gdb transcripts (Part 0's tool) |
| libx11-dev (Xlib) | 2:1.8.7-1build1 | the window, input, and presentation implementation |
| Xvfb | 2:21.1.12-1ubuntu1.8 | the headless display every window check ran on |
| xdotool | 1:3.20160805.1 | scripted window and keyboard checks |
| sanitizers | GCC's ASan (same toolchain) | lesson 041's limitation demonstration |
| mdBook | 0.5.4 (pinned) | every page render check |

The authoring machine is headless by design; the design's risk table
routed real-desktop variability to port-to-your-own-machine exercises
(lessons 028, 031, 033, 034, 043 carry them). Xvfb numbers are honest for
Xvfb — lesson 036 and lesson 043 say so in prose.

## Recommendation: extending the frozen prefix

The horizon currently freezes `lesson-001`…`lesson-006`. This review does
**not** propose extending it yet — the precondition from Part 0's review
(independent review pass per batch) has not been satisfied for anything
after 006. The proposal, for the decision after those passes land:

- **Freeze `lesson-026`…`lesson-043` as one block, once one review pass
  covers the part.** Part 1 is one dependency: Part 2's renderer stands on
  the framebuffer and presentation contract (030-031), its loop on the
  input and clock contracts (032-036), its asset loading on the file and
  memory contracts (037-041). Freezing part of it would freeze exactly the
  seams Part 2 is most likely to exercise. A single pass over the part is
  the cheaper unit.
- **Finish the deferred passes for `lesson-007`…`lesson-025` in parallel.**
  They were left volatile in Part 0's review for exactly this moment (the
  deep dive in 012 and the optimizer lesson in 018 being the named
  exposures). With Part 1's tail now also in the review queue, the oldest
  volatile lessons are the cheapest to stabilize first.
- **Do not extend the prefix on authoring evidence alone.** The 36 patches
  were applied and the checks re-run, but that is the author's check — the
  prefix hardens on the reviewer's.

## Review pass status (pre-freeze)

The independent review pass over Part 1 has **not** run yet — this review
records the author's verification, which is the input to that pass, not a
substitute for it. What a reviewer should re-run, in order of what breaks
first if anything drifted:

1. `./tools/check-boundary.sh` and `openspec validate` (seconds).
2. The closing demo headlessly (lesson 043's transcript) and the pixel
   readback of lesson 031.
3. The failure-path runs (lessons 029, 037, 038) — the typed failures.
4. The measured claims (lessons 035, 036, 039, 040) — numbers move with
   the machine; the *shape* is what the prose claims.
5. Every solution patch: `git apply`, build, run, reverse.

## Known warts carried forward (intentional, documented in-prose)

- **Frames happen when news happens** (034's pacing note). The clock makes
  the marker's *speed* honest; the *rate* is still the input's. A frame cap
  is policy and Part 2/5's business — named in 035 and 043.
- **`ReadFile` trusts the reported size in the tagged state.** Virtual
  files (like `/proc/self/maps`) report 0 bytes and hold kilobytes — the
  reader returns 0 bytes presented as success for exactly those files.
  Lesson 039's exercise 2 teaches and fixes it; the fix is deliberately
  exercise state, not end state. If Part 4 reads `/proc`-like files, fold
  the fix forward first.
- **One platform implementation per build** (042's audit finding). Two
  implementation files in `src/` collide at link; the build's file
  selection is the one line that would know.
- **The presentation copy is the frame's biggest cost** (036's honest
  measurement: ~54% under Xvfb). MIT-SHM is the named later optimization;
  Part 5's optimization passes will meet this number.
- **`Present` reports carrier failure only.** X protocol errors are values
  (031), but an error unrelated to our window still ends through the
  recorded-error path rather than a typed failure per cause.

*(Everything above is measured from the tagged lesson states; the closing
demo's account, the boundary check, and every solution patch were run on
the authoring machine before this review was written.)*
