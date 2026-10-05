# Design

## Context

Greenfield repo: no code, no specs, no history. The proposal (see `proposal.md`) fixes the what: a written course taking Python/Ruby developers from C foundations to a finished 2D arcade game on a hand-written engine. This document fixes the how: the authoring contracts and technical policies that make a ~570 h, 10-14 month solo authoring effort (at 10-20 h/week) survive contact with reality.

Constraints that shape everything below:

- "No external libraries" governs **student-visible C/C++ code only**. Authoring tooling is pragmatic.
- The audience has general programming fluency but zero C/C++/manual-memory experience; the toolchain is as foreign as the language.
- Deep low-level dives are mandatory curriculum (caches, virtual memory, SIMD/assembly, compiler/linker) — not asides.
- The main line is Linux/X11, x86-64 as the deep-dive reference architecture.

## Goals / Non-Goals

**Goals:**
- A single authoring contract that keeps prose, code, exercises, and website in sync by construction (co-committed steps, symbol references, diff-based solutions).
- Default-harmless policies for the three known risks: revision churn, exercise volume, Part 5 sprawl.
- A curriculum skeleton precise enough that each future part/lesson change can be scoped mechanically.
- Frozen target-game contract early enough to shape Parts 1-4 backward.

**Non-Goals:**
- No lessons or engine code authored in this change (planning artifacts and repo scaffold only).
- No video content, no translation tracks, no LMS/interactive IDE.
- No Windows/macOS implementation and no GPU path in the main line (epilogue scope only).
- No general concurrency curriculum; no automated exercise grading infrastructure (see Open Questions).
- No community-contribution program yet (see Open Questions).

## Decisions

### D1. Language path: C sandbox, then feature-by-feature C++ subset
Part 0 teaches C (compilation model, pointers, memory, structs, UB) in small throwaway programs. The engine (Part 1+) uses C++, where each language feature is admitted only if we can explain what it compiles down to: references, overloading, namespaces, `constexpr`, classes with vtables. Templates only in clearly labeled tooling code. The subset policy is documented in the first engine lesson and enforced by review, not tooling.
- *Alternatives*: strict C-with-classes (too much ergonomic drudgery in Parts 4-5); modern C++ with STL/RAII (abstraction layers hide the machine, contradicting the deep-dive pillar); C only (manual data-structure work would swamp later parts).

### D2. Platform layer first, Linux/X11 first, x86-64 reference
The platform layer API (window, input, timing, file I/O) is designed before its first implementation; Linux/X11 is the only implementation in the main line. Windows (Win32) is the first optional epilogue module and the platform API is kept free of X11 idioms so it can be added without redesign. macOS is out of scope. Deep dives anchor on x86-64 with conceptual-first framing (ARM64 gets margin notes at best).
- *Alternatives*: Win32 first (maximum Handmade Hero fidelity, but untestable authoring environment on this machine); cross-platform from episode 1 (every topic drags N implementations); headless core first (delays the "I see pixels" payoff past the students' patience).

### D3. Software renderer as the main line; GPU is an optional epilogue
All rendering is pixels written into our own framebuffer, handed to the platform layer for presentation. The GPU port (if ever) is a labeled epilogue chapter that ports the renderer; it never rewrites the main line. This keeps "no libraries" pure and makes caches/SIMD deep dives land on code students actually wrote.
- *Alternatives*: GPU-presented software draw (marginal benefit, more platform surface); shaders early (context creation and GL loaders dwarf the lessons they serve).

### D4. Part 0 is a throwaway sandbox; the engine is born at Part 1
Part 0's programs are discarded. Part 1 starts from a literal blank file ("the codebase is born") and `src/` evolves linearly to the end of the course. This preserves the Handmade Hero "one continuous codebase" pillar across the engine arc without forcing lesson 1 to be a window.
- *Alternatives*: engine from lesson 1 (C fundamentals get taught through engine files — too heavy early); sandbox becomes a tool (constrains Part 0 pedagogy to what tools need).

### D5. Lesson/repo mechanics: linear history + tags, prose co-committed
- One linear git history in `src/`; each lesson's end state is tagged `lesson-NNN`; prose and its code step are **co-committed**, so a tag means "code and text as of lesson N".
- Prose references **symbols, never line numbers** ("the clipping branch of `RenderRectangle`"), which eliminates most downstream prose churn by construction.
- `book/` (mdBook) is freely editable at any time; only code tags carry stability semantics.
- *Alternatives*: snapshot directory per lesson (duplication, noisy diffs); branch per lesson (git mechanics too heavy for the audience); generated per-lesson views (tooling forever — retained only as an escape hatch, see Risks).

### D6. Stability horizon and revision policy
The site declares a **frozen prefix** ("Lessons 1-40 stable") and a **volatile tail**. Tags in the frozen prefix are immutable except for correctness fixes. A three-class revision policy:
1. **Prose-only** (typos, clarity): edit `book/` in place, always.
2. **Behavior-changing code fix in the frozen prefix**: surgical — rebase-forward the states, re-tag, fix downstream prose touch-points (cheap because of the symbol-reference rule).
3. **Redesign / wrong architecture**: fix-forward at the tip as an explicit lesson ("refactor is curriculum"), or wait for an edition rewrite of the affected volume.
Student recovery from a moved tag: `git checkout lesson-NNN -- src/`.
- *Alternatives*: publish-per-part (no public feedback for months); all-volatile until edition 1.0 (early students are guinea pigs); errata-only past (published course accumulates known bugs).

### D7. Exercise system: variable density, archetypes, diff solutions
- **Density by part**: Part 0: 3-4 short drills/lesson; Parts 1-2: 1-2 "make it yours" extensions; Parts 3-4: 1-2 mixed; Part 5: fewer, bigger challenges (~1.7 average).
- **Archetypes** (~6 recurring shapes): predict-the-output, fix-the-crash, extend-the-code, measure-the-performance, explain-in-prose, port-to-your-own-machine.
- **Solutions as diffs + short walkthrough** (`solutions/lesson-NNN/exN.patch` + a few sentences), not full listings — halves authoring and survives code revisions via the same rebase-forward workflow.
- **Exercises are written before the prose**: if no good exercise exists, the lesson is mis-sized; the exercise acts as the lesson's acceptance test.
- *Alternatives*: uniform 2-4 with full listings (~150-200 h, unaffordable); automated self-checks (pins behavior into revision churn — deferred, see Open Questions).

### D8. Target game contract: frozen MVD, bounded feel, fixed optimization menu
The **MVD checklist is frozen now** and propagates backward into Parts 1-4 design (Part 5 needs 3 enemy types -> Part 4 entity storage must serve it; needs a 60 fps report -> Part 2 instruments frame timing early; needs juice -> Part 4 leaves camera/feedback hooks; needs tile collision -> Part 2's sprite pipeline covers tilemaps).

```
  [ ] hero: 8-dir movement with accel/decel feel
  [ ] combat: projectiles, 2 weapon types
  [ ] enemies: 3 types + 1 boss (chase / keep-distance / flee AI)
  [ ] world: single scrolling map, tile collision
  [ ] states: title, play, pause, death, victory
  [ ] juice toolkit: hitstop, screenshake, particle burst, easing
  [ ] audio: music + sfx through our own mixer
  [ ] perf: 60 fps on modest hardware + final frame-budget report
```

- **Definition of done** = checklist complete + 60 fps + frame-budget report. "More features" is an extras chapter or post-course.
- **"Feel" is a 4-effect toolkit + one principle** (immediate feedback, readable input) — never an open-ended tour.
- **Optimization is a fixed 3-pass menu**: (1) measure — profiler lesson, find top-2 hotspots; (2) fix top-2 — cache layout / SIMD / allocation, where the deep dives pay off narratively; (3) report — the frame-budget table is the course finale.
- **Part 5 is capped at 22 lessons (L1-L22)**, skeletoned during M0 before lesson 1 exists. The skeleton groups as: L1-L5 game skeleton, tilemap+camera, tile collision, hero movement, feedback+animation; L6-L10 projectiles/2 weapons, enemy archetype tables, chase/keep-distance/flee AI, boss, waves; L11-L15 juice toolkit, HUD, audio integration, screen polish; L16 the debt/refactor lesson; L17-L20 the 3-pass optimization menu; L21-L22 retrospective (our engine vs. real ones, epilogue map) + "now make YOUR game". The full per-lesson list is an M0 deliverable.
- **The Part 4 capstone "vertical slice" (L0*)**: one closing lesson where a hero walks a tilemap with the camera following, using only finished services. It turns "Part 4 done" into a testable moment and derisks Part 5's opening. Backward obligation: Part 4 must end with services complete enough to make this lesson trivial.
- *Alternatives*: emergent scope (sprawl risk is the whole point of this decision); lesson-count cap only (no backward propagation benefit); no capstone ("Part 4 done" stays a feeling and Part 5's first lessons absorb the integration risk).

### D9. Deep dives: placed, not scheduled
Compiler/linker internals live in Part 0 (the compilation model *is* the C lesson); virtual memory in Part 1 (platform layer + arenas); caches in Part 2 (renderer/entity layout) and revisited in Part 5's passes; SIMD/assembly reading in Part 2 (the blitter) and revisited in Part 5. Threads are **not** a deep dive: audio gets exactly one contained lesson — platform callback + hand-rolled ring buffer ("the one place the OS calls us"), taught as mechanics, not a concurrency arc.
- *Alternatives*: standalone deep-dive interludes (breaks the codebase continuity); blocking-write audio on the main thread (simpler, but hides the real-time constraint entirely).

### D10. Toolchain and website
- `build.sh`: one command compiles everything, always; it stays a shell script (Make/CMake are never curriculum). Compiler flags, gdb/lldb, sanitizers, and a profiler are taught explicitly in Part 0-2 — for this audience the toolchain is curriculum.
- Website: **mdBook** (`book/`), run locally via `mdbook serve`, published as a static site. The no-libraries rule does not apply here. The site renders lessons, exercises, and linked solution diffs, and shows the stability-horizon marker.

### D11. Part 0 sandbox arc: four programs, thrown away on purpose
Part 0 (~25 lessons) is four small standalone programs, chosen against four criteria: the problem must be **familiar** to a Python/Ruby dev (the novelty is C, never the domain), each program must make one **machine-visible aha** tangible, be right-sized at **4-6 lessons**, and **foreshadow** an engine concept without being engine code:

1. **P1 `wordcount` (L1-6)** — the pipeline and the memory: argv, files, char buffers, malloc/free, leaks and sanitizers, gdb from lesson 2, stack frames, UB and buffer overflows. Foreshadows file I/O and asset loading.
2. **P2 data-structures kit: `dynarray` + `hashtable` (L7-12)** — layout and linkage: structs, sizeof/alignment/padding surprises, realloc growth, function pointers, genericity via `void*` and its pain, multi-file builds into the **compiler/linker deep dive**. Foreshadows arenas, entity storage, asset tables.
3. **P3 `paint`, a BMP/PPM painter (L13-18)** — bytes and pixels: raw bytes, pixel formats, endianness, fill-rect/line onto a memory buffer, writing a real image file by hand, plus the `-O2`/optimizer UB lesson. Foreshadows Part 2's framebuffer. The artifact opens in an image viewer: "I just wrote a software renderer with no window."
4. **P4 `snek`, a terminal game (L19-24)** — loops and state: game loop, `clock_gettime` timing, raw-terminal input via escape codes (bytes only), double-buffered char grid (= a tiny framebuffer), state machine, function-pointer command table. Foreshadows the update/render split, input state, and game states.

Ordering is **pixels then game** (P3 before P4): buffering/byte intuition lands while it is the only topic, and the part ends on an actual game loop as the emotional bridge to Part 1. **L25 teaches the C++ subset** (classes = structs + functions + a drawable vtable), so Part 1's blank file is born already in C++. Part 0's code is discarded; the pillar "one continuous codebase" governs the engine arc (Parts 1-5), per D4.
- *Alternatives*: three deeper programs (less concept coverage per domain); five wider programs (Part 0 balloons past ~25 lessons); game first, pixels later (motivation earlier, but double-buffering is taught without pixel intuition); transition at Part 1's start (cleaner birth, heavier lesson 1); C++ features threaded through Part 0 as pains appear (organic but the subset goes blurry).

## Risks / Trade-offs

- [Revision churn in the frozen prefix (rare class-2 fixes touch 100+ downstream states)] → symbol-reference discipline keeps prose intact; documented rebase-forward + re-tag recipe in `tools/`; oversized fixes wait for an edition rewrite rather than churning live lessons.
- [Exercise authoring backlog (~260 exercises) inflates the schedule] → variable density and diff-based solutions target ~70-90 h; exercises-first rule prevents silent lesson bloat; density is revisited at each part boundary against actual velocity.
- [Part 5 sprawl despite the cap ("feel" and optimization invite infinity)] → frozen MVD checklist, fixed 3-pass menu, 4-effect juice toolkit, hard lesson cap; anything outside the checklist is explicitly extras/post-course.
- [Solo-author variance (10-20 h/week is an average, not a promise)] → milestone map with part-boundary review points (M0-M4, ~10-14 months); the volatile tail absorbs mid-course learning without breaking promises; scope cuts land on exercises and epilogue first, never on deep dives.
- [Audio threading drags in an unbudgeted concurrency curriculum] → D9's single contained lesson (callback + ring buffer) keeps it mechanics-sized.
- [x86-64-anchored deep dives underserve ARM64 readers] → deep dives lead with concepts (cache lines, pages) and use x86-64 only as the worked example; ARM64 margin notes where cheap.
- [Tag mutations mid-arc confuse live students] → the site's stability-horizon marker names the frozen prefix explicitly; `git checkout lesson-NNN -- src/` resync path is documented in lesson 1.
- [mdBook/tooling drift breaks the site over a multi-year arc] → pin the mdBook version in-repo; `book/` markdown remains the source of truth and is tool-agnostic.

## Migration Plan

Greenfield — deployment is the authoring plan itself:

1. **M0 (~3 weeks, before lesson 1)**: scaffold `book/`, `src/`, `tools/`, `build.sh`; freeze the MVD checklist; skeleton Part 5's L1-L22 (D8) and derive Part 1-4 requirements backward, including the L0* vertical-slice obligation on Part 4.
2. **M1**: Part 0 (~25 lessons) — the four sandbox programs of D11 (`wordcount`, data-structures kit, `paint`, `snek`), toolchain and debugger curriculum, linker deep dive, C++ subset transition (L25). First part boundary review: adjust exercise density and velocity estimates.
3. **M2**: Parts 1-2 — platform layer + software renderer; caches and SIMD/assembly deep dives.
4. **M3**: Parts 3-4 — sound + services; contained audio concurrency lesson; ends on the vertical-slice capstone (L0*).
5. **M4**: Part 5 (L1-L22) + epilogue — game checklist, 3-pass optimization, frame-budget report finale.

Rollback: not applicable (nothing runs in production). The edition model is the long-term rollback story for content: each part boundary can freeze into an edition tag, and redesigns wait for the next edition rather than churning the frozen prefix.

## Open Questions

- **Automated self-checks**: a thin `build.sh check` for output-objective exercises would help self-learners but pins behavior into revisions. Revisit at M1 (Part 0 authoring) as a small separate change; if adopted, shell-script based and narrowly scoped.
- **Community contributions** (exercise PRs, translations): only worth it if review cost < authoring cost. Parked until edition 1.0; needs a contribution policy first.
- **Concrete game theme/art style**: the MVD freezes features and feel, not theme; decide at M3 when Part 4's content pipeline exists. Does not affect any requirement above.
- **Publishing venue specifics** (GitHub + Pages assumed): implementation detail of `course-website`; decide at M0 scaffolding.
