# Proposal

## Why

Part 0 is authored, reviewed, and its `wordcount` arc is frozen — but the
course's central promise, *an engine you wrote yourself*, does not exist
yet. Part 1 is where the codebase is born from a blank file and grows the
platform layer every later part stands on: Part 2's renderer presents
pixels through it, Part 5's frame-budget work measures on its clock, and
the MVD's backward obligations (O1 frame timing, input state for 8-dir
movement, file I/O for asset loading) all build on it. Nothing downstream
can be authored until it exists.

## What Changes

- **Author Part 1's lesson arc** per the foundation design (D2/D4): the
  engine is born from a blank file in the admitted C++ subset and grows a
  platform layer — window, input state, timing, file I/O against the OS —
  on the Linux/X11 main line, ending with our own framebuffer presented
  through the platform layer.
- **Delete `sandbox/` in Part 1's first lesson's code step** — "the codebase
  is born" from an empty tree, while `lesson-001`…`lesson-025` keep every
  Part 0 state retrievable (curriculum: Throwaway Part 0 sandbox).
- **Document the C++ subset in the first engine lesson** (conventions §5),
  including the consistency read of lesson-025's wording promised at the
  Part 0 boundary review.
- **Give the virtual-memory deep dive its lesson home** inside Part 1
  (curriculum: Deep dives embedded) — anchored where the platform layer
  meets arenas and large mappings, per D9.
- **Deliver the platform baseline Part 2 depends on**: a monotonic clock
  precise enough to instrument frame time (backlog O1: "Part 2, early — on
  Part 1's clock"), polled input state, and whole-file I/O.
- **Keep the toolchain curriculum going** (curriculum: Toolchain is
  curriculum): flags explained as they appear, sanitizers where memory bugs
  and UB are taught (mappings, arenas), the profiler still precedes
  optimization work.
- **Turn the continuity machinery onto the engine**: `src/` becomes the one
  linear history tagged `lesson-NNN` (course-wide numbers continue from
  `lesson-026`); the resync path switches to `git checkout lesson-NNN --
  src/`; the stability horizon keeps the frozen prefix and names the
  growing volatile tail.
- **Close with a part-boundary review** in `plan/`: velocity and exercise
  density measured against the Parts 1-2 expectations (1-2 "make it yours"
  extensions per lesson).

## Capabilities

### New Capabilities

- `platform-layer`: the observable contract of the engine's platform layer
  — opening a window and presenting our framebuffer, polled input state,
  monotonic timing, and whole-file I/O against the OS — built with no
  external libraries behind an OS-agnostic API (Linux/X11 implementation
  first; a Win32 epilogue must slot in without redesigning the API).

### Modified Capabilities

(none — Part 1 implements contracts the foundation and Part 0 changes
already fixed: the engine-birth scenario of curriculum's sandbox
requirement, the deep-dive placement, the C++ subset policy, and the
lesson/exercise/continuity formats all govern Part 1 as written.)

## Impact

- **`src/`**: born empty at Part 1's first lesson and grown linearly to the
  end of the course; every engine file is authored in the admitted C++
  subset. `sandbox/` is deleted in that same code step.
- **`book/`**: Part 1's lesson pages (one per lesson, 1-2 exercises each
  with diff solutions) and the Part 1 section in `SUMMARY.md`; the
  stability-horizon banner keeps naming the frozen prefix (`lesson-001`
  …`lesson-006`) and the volatile tail.
- **Tags**: `lesson-026` onward, course-wide sequence, one per lesson,
  co-committed with prose; `git diff` between tags equals the code steps
  between them.
- **`build.sh`**: becomes the live one-command build of a real codebase
  (no structural change; Make/CMake remain never-curriculum).
- **`plan/`**: Part 1's boundary-review notes (velocity vs the 10-20 h/week
  review budget from `plan/part0-review.md`, density vs the Parts 1-2
  table).
- **Scope size**: a whole part (~20 lessons) implemented in
  birth-and-growth batches, with the boundary review closing the change.
