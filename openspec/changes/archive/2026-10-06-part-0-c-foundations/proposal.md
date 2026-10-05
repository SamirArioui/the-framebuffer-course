# Proposal

## Why

Part 0 is the M1 milestone and the course's first real content: it must carry a
Python/Ruby developer with zero C experience through C foundations in four
throwaway sandbox programs and land on the C++ subset so Part 1's engine can be
born. Until these ~25 lessons exist — prose, code steps, exercises — there is
no learner path at all, and the authoring contracts (co-commit, symbol
references, diff solutions) have never met real content. Closing M1 also earns
the first part-boundary review: exercise density and velocity estimates checked
against reality.

## What Changes

- **Author Part 0's lesson arc** per the foundation design (D11): P1
  `wordcount` (L1-6), P2 `dynarray` + `hashtable` kit (L7-12), P3 `paint`
  (L13-18), P4 `snek` (L19-24), and L25 the C++ subset transition — ~25 medium
  lessons.
- **Ship every lesson under the authoring contract**: prose + exactly one
  co-committed code step, symbol-referenced prose, 3-4 short drill exercises
  written before the prose, solutions as diffs + walkthroughs linked at each
  prompt's end.
- **Put the toolchain in the lessons**: compiler flags and gdb from the early
  lessons, sanitizers where memory bugs and UB are taught, the optimizer/UB
  lesson in P3 — and record this contract in the `curriculum` spec, where the
  foundation design constraint (D10) has no requirement yet.
- **Give the compiler/linker deep dive its home** in P2's multi-file builds
  (curriculum already requires it in Part 0).
- **Establish Part 0's code home and discard path**: the sandbox programs build
  independently of the engine and are discarded when Part 1's engine is born;
  the layout decision lands in design.md.
- **Turn on the continuity machinery**: `lesson-NNN` tags begin at L1; the
  site's stability horizon starts naming the volatile tail.
- **Run the M1 part-boundary review**: record exercise density and velocity
  findings in `plan/` for the next part's planning.

## Capabilities

### New Capabilities

(none — Part 0 implements contracts the foundation change already fixed)

### Modified Capabilities

- `curriculum`: adds the **Toolchain is curriculum** requirement — compiler
  flags and the debugger are taught in Part 0, sanitizers where memory bugs are
  taught, and the profiler before optimization work. This is foundation design
  decision D10 ("for this audience the toolchain is curriculum"), which no
  requirement records yet; every later part is accountable to it too.

## Impact

- **`book/`**: ~25 lesson pages plus their exercises and diff-based solutions;
  `SUMMARY.md` gains the Part 0 section in curriculum order.
- **Sandbox code**: four standalone C programs (layout decided in design.md),
  built independently of `build.sh`/`src/`; thrown away when Part 1 starts.
  `src/` stays empty — the engine is born at Part 1 from a blank file.
- **Tags and horizon**: `lesson-NNN` tagging goes live (L1-L25),
  `book/stability-horizon.md` names the volatile tail, and the documented
  resync path starts being real.
- **`plan/`**: the M1 boundary-review notes (velocity, exercise density).
- **Scope size**: this is the whole M1 milestone (~570 h plan: roughly a
  quarter of it). It is implemented in program-by-program batches, with the
  boundary review closing the change.
