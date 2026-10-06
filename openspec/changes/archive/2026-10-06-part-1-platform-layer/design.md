# Design

## Context

Part 0 is authored and its `wordcount` arc frozen; `sandbox/` holds the four
throwaway programs and `src/` is empty. This change authors Part 1 against
the binding contracts in `plan/conventions.md`, the foundation design
(D2 platform layer first / Linux-X11 / x86-64 reference; D4 the codebase is
born at Part 1; D9 virtual memory in Part 1), and the `platform-layer` spec
delta. Constraints that shape the how:

- The learner arrives fluent in the taught C and the admitted C++ subset
  (lesson-025), not in OS APIs or X11. Novelty stays machine-level, never
  "what is a window".
- Exactly one code step per lesson, co-committed and tagged `lesson-NNN`
  (course-wide numbers continue from `lesson-026`).
- Parts 1-2 carry 1-2 "make it yours" extensions per lesson (exercises
  spec), written before the prose as always.
- This authoring machine has no display: observable window behavior must be
  verifiable headlessly (Xvfb) while the prose targets a real desktop.

## Goals / Non-Goals

**Goals:**
- A platform API fixed before its first implementation, narrow enough to
  name in one page and stable through Parts 2-5.
- A Part 1 lesson arc precise enough that each lesson can be authored and
  tagged mechanically (part-0 design carried its arc in D11; this design
  plays that role for Part 1).
- A verifiable home for the virtual-memory deep dive and for the O1 clock
  Part 2's frame-time instrumentation builds on.

**Non-Goals:**
- No rendering work (Part 2) beyond presenting a test framebuffer; no
  audio, arenas-as-services, or asset formats (Parts 3-4).
- No Win32 or macOS implementation — only the API seam that keeps a Win32
  epilogue possible (foundation D2).
- No build-system change: `build.sh` stays the one-command build.
- No edits to the frozen prefix (`lesson-001`…`lesson-006`).

## Decisions

### D1. The Part 1 arc: 18 lessons, birth → window → pixels → input → clock → files → memory → seam
Part 1 is `lesson-026`…`lesson-043` in eight batches:

| Batch | Lessons | Arc |
| ----- | ------- | --- |
| Birth | 026 | The codebase is born from a blank `main`, `build.sh` goes live, `sandbox/` is deleted in the same code step, and the admitted C++ subset is documented as the engine's language law (conventions §5) |
| Window | 027-029 | X11 display + window; the event pump that keeps it alive; clean close and error paths |
| Pixels | 030-031 | The framebuffer as our own bytes (Part 0's `paint` intuition reused), presentation through the platform layer |
| Input | 032-034 | OS key events folded into polled state; latching brief presses and focus; an arrow-key marker as the first interactive frame |
| Clock | 035-036 | The platform clock (monotonic, frame delta — `snek`'s timing revisited); frame time as measured data (O1's seed) |
| Files | 037-038 | Whole-file reads with typed failures; writes and round-trips |
| Memory | 039-041 | The virtual-memory deep dive (pages, mappings, protection); reservation-backed buffers; arenas as bump allocators over reservations |
| Seam | 042-043 | The interface as a contract (what a second OS must implement); the closing demo: one measured frame loop — window + polled input + arena-backed framebuffer — as "platform layer done" |

Ordering is window before pixels before input before clock: each batch makes
the previous batch observable (a window you can look at, pixels you can see,
a marker you can move, a frame time you can measure), and the deep dive sits
where large buffers make it necessary, not as an interlude (foundation D9).

### D2. One narrow platform header; X11 confined behind it
The platform layer is one small interface — window open/close/present, input
poll, clock, file read/write, typed failures — with the X11 implementation
in its own files. Engine code includes the interface only. This is what the
spec's single-boundary requirement buys: the Win32 epilogue is "one more
implementation", and lesson-042 audits the seam against exactly that test.
- *Alternatives*: Xlib types in engine code (leaks X idioms; breaks the
  spec); a fat platform API (design-before-use becomes fiction); virtual
  interfaces per subsystem (vtables where a function table would do —
  lesson-025 taught the seam already, don't re-teach it here).

### D3. Xlib is the OS surface; `XPutImage` is the first presentation path
Use Xlib (`-lX11`), the OS's own C API — no toolkit, no SDL/GLFW (the
no-external-libraries rule), no raw X protocol (noise). Presentation copies
the framebuffer through `XImage`/`XPutImage`; the copy's cost is real, is
measured in lesson-036's frame-time work, and is honest curriculum — shared
memory (MIT-SHM) is a named later optimization, not a Part 1 topic.
- *Alternatives*: xcb (poorer pedagogy, little gain here); MIT-SHM from the
  start (hides the copy cost that Part 5's frame budget will want visible);
  framebuffer rendered via core X drawing calls (reverses Part 0's paint
  intuition).

### D4. Memory: reservation-backed buffers and bump arenas
Large buffers (the framebuffer first) come from OS-level reservations
(`mmap`-style anonymous mappings) sized in pages, and the arena allocator is
a bump pointer over one reservation with align and rollback marks. The
virtual-memory deep dive explains pages, mapping, and protection
conceptually-first with x86-64 as the worked example (foundation D2).
- *Alternatives*: `malloc` for everything (deep dive loses its home);
  full-fledged heap service now (Part 4's job); `mmap` file mappings as the
  first example (bytes on disk blur the address-space story — file I/O is
  taught separately in 037-038).

### D5. The clock is the platform layer's; the engine measures frames on it
Timing wraps the OS monotonic clock behind the same interface and exposes
delta time and a frame-time record per frame (backlog O1: Part 2 instruments
frame timing "on Part 1's clock"; Part 5's frame-budget report reads it).
Lesson-036 seeds the log format Part 2 will grow.
- *Alternatives*: `std::chrono` (an abstraction layer over exactly the thing
  being taught); timing in engine code (breaks D2's seam).

### D6. Conventions carry over unchanged; two mechanical switches
Co-commit, symbol-reference discipline, exercises-first, diff solutions all
hold. The two switches: exercises are now 1-2 "make it yours" extensions
(exercises spec, Parts 1-2), and the resync path becomes
`git checkout lesson-NNN -- src/` (the horizon banner already names it).
Deliberate teaching states (the part-0 leak/UB pattern) remain available
when a lesson needs one, always flagged in prose as deliberate.

## Risks / Trade-offs

- [No display on the authoring machine; "the window shows our pixels" must
  still be verified] → verify under Xvfb with a pixel readback (the spec's
  presentation scenario is checkable headlessly); real-desktop variability
  goes to port-to-your-own-machine exercises.
- [X11/OS API surface overwhelms the audience] → every OS call is wrapped
  and taught through the seam (D2); the arc spends three lessons on the
  window alone; X11 specifics never appear in engine code.
- [Virtual memory reads as an interlude] → it is placed where the
  framebuffer's size forces it (D4) and its payoff is the arena the demo
  allocates from.
- [Sanitizers and arenas confuse each other] → the memory lessons teach the
  limitation explicitly (ASan does not see inside arena reservations) —
  toolchain-is-curriculum applied honestly.
- [The lesson-025 ↔ first-engine-lesson handoff drifts] → a dedicated task
  reads both and reconciles the subset wording before 026 is tagged.
- [Density shift (3-4 drills → 1-2 extensions) inflates or starves
  lessons] → the part-boundary review measures density against the
  conventions table as Part 0's did.

## Migration Plan

Batch order follows D1: birth → window → pixels → input → clock → files →
memory → seam; each lesson commits and tags as it lands (`lesson-026` …
`lesson-043`). The first code step deletes `sandbox/` wholesale (the
designed discard; tags keep it retrievable). The change closes with the
part-boundary review in `plan/`: velocity against the 10-20 h/week review
budget (per `plan/part0-review.md`), density against the Parts 1-2 table,
and a freeze-prefix recommendation. Rollback: content-only; lessons are
revertable per commit and the revision policy governs anything published.

## Open Questions

- Whether MIT-SHM presentation is taught as a Part 2 optimization lesson or
  waits for Part 5's passes — deferred safely: it changes neither the spec,
  this approach, nor the task breakdown (lesson-036 will have made the copy
  cost measurable either way).
