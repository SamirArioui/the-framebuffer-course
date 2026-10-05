# Proposal

## Why

There is no structured path for a developer who knows Python/Ruby to reach "I built a game on an engine I wrote myself." Existing material is either language tutorials that stop at syntax, or engine frameworks that hide the machine. This project closes that gap with a free, open, English-language written course: ~130 medium lessons taking the learner from C foundations to a finished 2D arcade game running on a fully hand-written engine (no external libraries in any student-visible C/C++ code), built against the OS behind a platform layer.

This change establishes the foundation: the curriculum arc, the authoring contracts (lesson format, code history, exercises), the definition of done for the finished game, and the policies that keep a multi-month authoring effort from sprawling. Every later part/lesson change builds on these decisions.

## What Changes

- **Establish the course's identity and scope**: written lessons + code repo (English, free and open), for Python/Ruby developers with zero C/C++/manual-memory experience; endpoint is a complete 2D top-down arcade game plus custom engine, Parts 0-5 plus optional epilogue.
- **Define the curriculum arc**: Part 0 (C foundations, throwaway sandbox) -> Part 1 (platform layer, Linux/X11 first, blank file origin) -> Part 2 (software rendering) -> Part 3 (sound, own mixer) -> Part 4 (services: arenas, entities, asset formats) -> Part 5 (the game, feel, 3-pass optimization), with mandatory deep dives (caches, virtual memory, SIMD/assembly, compiler/linker) placed at their natural homes. GPU port and Windows module are optional epilogue scope only.
- **Define the lesson contract**: medium lessons (30-60 min read+code), prose and code step co-committed, prose references symbols never line numbers, ~100-150 lessons across the arc.
- **Define the codebase continuity model**: one linear git history with `lesson-NNN` tags, a published stability horizon (frozen prefix + volatile tail), a three-class revision policy (prose fixes in place, behavior-changing bugs surgical via rebase+re-tag, redesigns as fix-forward lessons or edition rewrites).
- **Define the exercise system**: variable density by part, a bounded set of recurring archetypes, solutions shipped as diffs + short walkthroughs, exercises written before the prose they accompany.
- **Define the website behavior**: a locally runnable site (mdBook) that presents lessons, exercises, and solutions, and displays the stability horizon marker.
- **Freeze the target game contract**: an MVD feature checklist (fixed now) propagated backward into Parts 1-4 design; "done" = checklist complete + 60 fps + final frame-budget report; "feel" bounded to a 4-effect toolkit; optimization bounded to a fixed 3-pass menu.
- **Scaffold the course repo** (`book/`, `src/`, `tools/`, `build.sh`) and authoring tooling conventions (one-command build, debugger as curriculum).

## Capabilities

### New Capabilities
- `curriculum`: The ordered learning arc - parts, their goals, prerequisite ordering, mandatory deep-dive placement, and the backward-propagation rule that Part 5's needs shape earlier parts.
- `lesson-format`: Anatomy and authoring rules of a lesson - prose, co-committed code step, exercises, symbol-reference discipline, and expected student time budget.
- `codebase-continuity`: The single evolving engine repo's history model - linear commits, lesson tags, the stability horizon, the revision policy, and student resync paths.
- `exercises`: The exercise and solutions system - archetypes, per-part density, diff-based solutions, and their maintenance relationship to code revisions.
- `course-website`: The locally runnable course site - lesson browsing, exercises/solutions presentation, stability-horizon display, and static publication.
- `target-game`: The frozen MVD contract for the finished game - feature checklist, bounded "feel" toolkit, the 3-pass optimization menu, and the definition of done.

### Modified Capabilities

(none - greenfield project, no existing specs)

## Impact

- **Repo structure**: creates `book/` (mdBook prose, exercises, solutions), `src/` (engine, linear history), `tools/` (build/check/asset tooling), `build.sh`; git tag convention `lesson-NNN`.
- **Dependencies**: mdBook as authoring/publishing tooling (explicitly outside the "no external libraries" rule, which governs student-visible C/C++ only); gcc/clang + gdb/lldb as course material.
- **Platform surface**: Linux/X11 is the first (and only, for now) supported platform; x86-64 is the deep-dive reference architecture.
- **Nothing breaks**: greenfield project, no existing code, specs, or consumers.
- **Authoring economics**: ~570 h estimated over 10-14 months at 10-20 h/week; risks (revision churn, exercise volume, Part 5 sprawl) are governed by the policies above and reviewed at each part boundary.
