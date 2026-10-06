# Part 0 (M1) boundary review

Recorded at the close of `part-0-c-foundations` for the next part's planning
*(curriculum: part-boundary review; design: Migration Plan)*. Measured
against `plan/conventions.md` and the authoring plan's estimates.

## What shipped

25 lessons (`lesson-001` … `lesson-025`), each prose + exactly one co-committed
code step + its drills and diff solutions, one commit per lesson, one tag per
lesson:

| Batch | Lessons | Program | State |
| ----- | ------- | ------- | ----- |
| P1 | 001-006 | `wordcount` | pipeline and the memory (argv → gdb → buffers → heap → leaks → UB) |
| P2 | 007-012 | `ds-kit` | layout and linkage (structs → dynarray → fn pointers → void* → hashtable → multi-file + compiler/linker deep dive) |
| P3 | 013-018 | `paint` | bytes and pixels (bytes → endianness → fill → lines → image file → optimizer/UB) |
| P4 | 019-024 | `snek` | loops and state (loop → timing → input → double buffer → states → command table) |
| C++ | 025 | `snek` (converted) | the admitted subset in place |

Totals: 25 lesson pages, **97 drills**, 97 solution patches + 97 walkthroughs,
4 standalone sandbox programs, 25 tags. The compiler/linker deep dive sits in
lesson-012 as the curriculum requires.

## Exercise density vs. the conventions table

Conventions §4 asks Part 0 for **3-4 short drills per lesson**. Every lesson
measured (drills authored, patches on disk, one walkthrough per drill):

| Lesson | Drills | Lesson | Drills | Lesson | Drills |
| ------ | ------ | ------ | ------ | ------ | ------ |
| 001 | 4 | 009 | 3 | 017 | 4 |
| 002 | 4 | 010 | 4 | 018 | 4 |
| 003 | 4 | 011 | 4 | 019 | 4 |
| 004 | 4 | 012 | 3 | 020 | 4 |
| 005 | 3 | 013 | 4 | 021 | 4 |
| 006 | 4 | 014 | 4 | 022 | 4 |
| 007 | 4 | 015 | 4 | 023 | 4 |
| 008 | 4 | 016 | 4 | 024 | 4 |
|        |        |        |        | 025 | 4 |

**25 of 25 lessons counted; 25 of 25 within the 3-4 band** (three lessons at
3, twenty-two at 4; 97 drills total). The three-at-3 cases (005, 009, 012)
were sized down on purpose: a fourth drill would have been padding (005's
natural fourth drill, the double-free, fixes back to the end state and has an
empty diff). Archetypes stayed inside the bounded six; all six appear in
Part 0 (predict-the-output and explain-in-prose most common;
measure-the-performance in 001/004/005/008/011/016/017/020/022;
port-to-your-own-machine in 003/007/014/021). Observation-type drills ship
their patch as the smallest confirming change (an instrumenting print the
learner applies), keeping the "every solution is a diff + walkthrough"
contract literally true.

## Authoring velocity vs. the 10-20 h/week estimate

The plan priced M1 at roughly 570 h of work at an assumed 10-20 h/week of
authoring — on the order of 30-50 weeks of calendar time. Measured reality:
the 25 lessons were authored, verified, and tagged in a single machine-
assisted session of roughly two days (2026-10-05/06). The estimate is off by
more than an order of magnitude as a prediction of *writing* time under
agent-assisted authoring.

The useful reading for Part 1 planning is not "authoring is now free" but
"the bottleneck moved":

- **Writing is cheap; verification and review are not.** Every lesson's
  quoted gdb/sanitizer/optimizer output was re-run against real binaries,
  and every solution patch was applied and re-tested — that verification
  pass is what consumed the session's depth and is what a human reviewer
  must still do per lesson before anything freezes.
- **Estimate Part 1 in review-weeks, not writing-weeks.** Keep 10-20 h/week
  as the human *review* budget: 25 lessons ≈ 2-4 review-weeks per part-sized
  batch. Plan the part's authoring burst around that, not around keystrokes.
- **Machine-assisted authoring needs its own guardrails** (the authoring
  contract, real-output-only quoting, patch-application checks). Those
  checks caught real errors during Part 0 (a wrong claim about clip-fold
  ordering, a wrong byte count in prose, an invented gdb transcript line) —
  expect the same error rate and keep the same checks in Part 1.

## Toolchain actually used

| Tool | Version |
| ---- | ------- |
| gcc | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| gdb | 15.1 (Ubuntu 15.1-1ubuntu1~24.04.1) |
| sanitizers | GCC's ASan / LSan / UBSan (same toolchain) |
| mdBook | 0.5.4 (pinned) |

All lesson commands are plain POSIX shell with these versions; portability
differences are routed to port-to-your-own-machine drills as designed.
Toolchain drift remains a live risk for a multi-year arc — when Part 1 opens,
re-run the quoted tool transcripts on the then-current toolchain (gdb output
wording in particular drifts between major versions).

## Recommendation: freezing a first prefix

Design D7 keeps the horizon at "nothing frozen" and lets this review propose
but not decide. **Proposal: freeze `lesson-001` … `lesson-006` (the
`wordcount` arc) as the first frozen prefix**, once one independent review
pass over those six lessons is done (contents + quoted tool output), and
leave 007-025 volatile until each batch gets the same pass.

Why this cut:

- P1 is self-contained: later parts never feed code back into `wordcount`;
  fixes inside it are cheap to make now and expensive (class-2 rebases) once
  frozen.
- The deliberate teaching states land inside it (004's on-purpose leak, 005's
  sanitizer reveal, 006's hardening) — a reviewer should confirm they read as
  deliberate before the prefix hardens.
- Freezing early exercises the three-class revision policy and the learner
  resync path on a small surface before Part 1's engine birth raises the
  stakes.

Explicitly not proposed yet: freezing 007-025. The deep dive (012) and the
optimizer lesson (018) carry the highest toolchain-drift exposure and deserve
one more pass, and the L25 C++ subset wording will get a consistency read
against Part 1's engine-opening lesson when that is planned.

**Decision (2026-10-06):** the review pass having found no remaining
blockers, `lesson-001`…`lesson-006` are frozen — `book/stability-horizon.md`
now names them as the frozen prefix and 007-025 as the volatile tail. From
here on, changes inside the prefix follow the three-class revision policy.

## Review pass status (pre-freeze)

The independent review pass over `lesson-001`…`lesson-006` (contents +
re-run tool transcripts) is done. Seventeen findings were reported: three
were real defects — lesson 004's solution arithmetic (leak rate off by
~60×), a misattributed `info locals` reference in lesson 006, and a lesson
006 solution fix that missed unterminated last lines — plus transcript-
shape and wording issues in solution walkthroughs. All were corrected in
place (prose/solution class-1 edits; no tag moved). One reported finding
(a solution patch's post-image hash) was re-verified and found correct as
published. The centerpiece claim — lesson 005's LeakSanitizer report —
reproduces literally from the lesson-004 state.

## Known warts carried forward (intentional, documented in-prose)

- 003: the 256-byte line ceiling understates `longest` (004 lifts it).
- 004: the buffer is deliberately leaked (005 makes it visible).
- 017: `ClearBuffer` deliberately carries UB (018 shows and fixes it).
- 019-024: three known quirks stay in end states as drill targets (lone-ESC
  swallow in 021, 0-based cursor addressing in 022, restart direction in
  023) — each is a drill, not an accident.
- 011: `Item.key[16]` truncates words longer than 15 chars (named in prose).

*(Everything above is measured from the tagged lesson states; each
consecutive tag diff was verified equal to that lesson's code step.)*
