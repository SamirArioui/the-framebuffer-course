# Design

## Context

The scaffolding change is archived: specs fix the contracts (lesson format,
exercises, continuity, curriculum) and `plan/conventions.md` fixes the authoring
rules. This change authors the first real content against them. See
`proposal.md` for why. Constraints that shape the how:

- `src/` must stay empty until Part 1: the engine is born from a blank file
  (curriculum: Engine born at Part 1). Part 0's code is throwaway.
- `build.sh` compiles everything under `src/` into one binary; multiple
  standalone programs must not live there.
- Every lesson ships prose + one co-committed code step, exercises written
  first, diff-based solutions — `plan/conventions.md` is binding.
- Tags are `lesson-NNN`, zero-padded (codebase-continuity: Linear history with
  lesson tags).
- The audience is a Python/Ruby developer with zero C experience; the novelty is
  C, never the domain (foundation design D11).

## Goals / Non-Goals

**Goals:**
- A concrete home and discard path for Part 0's sandbox code.
- Lesson, numbering, and tag mechanics precise enough that each of the ~25
  lessons can be authored and tagged mechanically.
- The toolchain curriculum's placement per part of Part 0, so the new spec
  requirement is implementable lesson by lesson.

**Non-Goals:**
- No engine code and no Part 1 planning (separate change).
- No site redesign; the existing template, banner, and solution layout are
  used as-is.
- No automated exercise grading (foundation open question, stays parked).
- No changes to `build.sh`, `book.toml`, or the published-deployment story.

## Decisions

### D1. Sandbox code lives in `sandbox/<program>/`, discarded when the engine is born
Part 0's four programs live in `sandbox/wordcount/`, `sandbox/ds-kit/`,
`sandbox/paint/`, `sandbox/snek/` — each builds standalone with no engine
sources (curriculum: Sandbox independence). When Part 1's first lesson is
authored, `sandbox/` is deleted in that lesson's code step: "the codebase is
born" from an empty tree, while `lesson-001`-`lesson-025` tags keep every
throwaway state retrievable.
- *Alternatives*: `src/` (breaks the one-binary `build.sh` and the blank-file
  birth); a separate repository (defeats "diffing two lesson tags equals the
  code steps between them"); keeping `sandbox/` forever (contradicts "thrown
  away on purpose", D4/D11).

### D2. One build command per program, taught as content
Each sandbox program builds with a literal `gcc` command that its lessons grow
flag by flag (the toolchain is curriculum). No Makefile, no `sandbox/build.sh`:
a build abstraction would hide exactly what Part 0 exists to teach. The
engine's `build.sh` remains the one-command build from Part 1 on.
- *Alternatives*: one `sandbox/build.sh` (hides the compilation model); teach
  Make here (D10: Make/CMake are never curriculum).

### D3. Lesson numbering and tags are course-wide; part labels are shorthand
Tags use one course-wide sequence: Part 0 is `lesson-001` … `lesson-025`
(part-local L1-L25 from D11). Part 5's "L1-L22" in `plan/part5-skeleton.md`
stays part-local shorthand; its tags will be course-wide numbers when that part
is authored. Lesson pages live at `book/lessons/part-0/lesson-NNN-<slug>.md`;
solutions keep `book/solutions/lesson-NNN/exN.patch` + `exN.md`.
- *Alternatives*: part-scoped tags like `p0-lesson-NNN` (contradicts the single
  `lesson-NNN` format in codebase-continuity).

### D4. Each lesson is one commit, tagged at its end
A lesson's prose page, its sandbox code step, and its exercises/solutions land
in one commit (lesson-format: Prose and code co-committed) and the commit is
tagged `lesson-NNN`. Lesson order in `SUMMARY.md` follows the arc; the
lesson-template footer carries part, prev/next, and the code-tag link.

### D5. L25 converts `snek` in place rather than adding a fifth program
The C++ subset transition's code step reworks part of `snek` — its command
table becomes a class and its draw path a small drawable interface with a
vtable — so "four standalone sandbox programs" stays true and the subset lands
on code the learner already wrote. The admitted subset is exactly the one in
`plan/conventions.md` §5.
- *Alternatives*: a fifth demo program (reads as a fifth sandbox program,
  against curriculum: Throwaway Part 0 sandbox); rewriting `paint` (its bytes
  focus is doing other work); pure prose with no code step (breaks lesson
  anatomy).

### D6. Toolchain placement inside Part 0
Per the new requirement and D10/D11: `wordcount` teaches argv, the compiler
invocation, flags as they appear, and gdb from its early lessons; the
leaks/UB lessons bring sanitizers; `paint` carries the optimizer/`-O2` UB
lesson; the compiler/linker deep dive anchors in `ds-kit`'s multi-file build.
gcc + gdb are the documented main line (commands must be concrete);
clang/lldb appear as margin notes.

### D7. The horizon stays "nothing frozen" through Part 0
All Part 0 lessons are the volatile tail while the part is being authored;
`book/stability-horizon.md` is only edited when a prefix actually freezes,
which the M1 boundary review may propose but does not decide.

## Risks / Trade-offs

- [~25 lessons in one change is months of authoring] → tasks are grouped per
  program and lesson batch; the change stays open until the boundary review
  closes it; scope cuts land on exercises first (foundation risk policy).
- [Exercise volume: 3-4 drills per lesson ≈ 75-100 exercises plus solutions] →
  exercises-first rule sizes each lesson; diff solutions bound the cost; the
  boundary review measures actual density against velocity.
- [Toolchain drift across machines] → record the gcc/gdb versions used in the
  M1 review; commands stay plain POSIX shell; portability differences go to
  port-to-your-own-machine exercises.
- [Terminal specifics in `snek`] → the main line is a Linux terminal; escape
  codes are bytes (portable in principle), and other terminals are margin
  notes plus a port exercise.

## Migration Plan

Batch order: P1 `wordcount` → P2 `ds-kit` (compiler/linker dive) → P3 `paint`
→ P4 `snek` → L25 transition. Each lesson commits and tags as it lands. The
change closes with the M1 boundary review (`plan/part0-review.md`): velocity
vs. the 10-20 h/week estimate, exercise density vs. the conventions table, and
a recommendation on freezing a first prefix. Rollback: content-only, nothing
runs in production; lessons are revertable per commit and the revision policy
governs anything published.

## Open Questions

- Whether the first frozen prefix is proposed at the M1 boundary or later —
  deferred safely: it changes neither specs, approach, nor task breakdown, and
  the horizon file already handles "nothing frozen".
