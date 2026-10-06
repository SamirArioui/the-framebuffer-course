# Design

## Context

Part 1 ended with a measured closing demo: a window kept alive, polled
input, an arena-backed framebuffer presented through a 15-function seam
(audited in lesson-042), and per-frame phase timings on the platform's
monotonic clock (the record seeded in lesson-036). The framebuffer is
ours: 640x480 of blue-green-red-x bytes written by `ClearBuffer` and
`PutPixel`. Part 2 replaces the single marker with a scene.

Constraints that shape the how:

- The renderer writes our framebuffer only — foundation D3 keeps the GPU
  an optional epilogue, so the caches and SIMD/assembly deep dives land on
  code students wrote (D9).
- The language law (lesson-026) still governs `src/`: no templates in
  engine code, no `new`/`delete`, features explainable as code generation.
  Memory is the arena; assets are whole-file reads.
- This authoring machine stays headless; drawing is verified by reading
  pixels back (lesson-031's check), and every quoted cache/assembly
  measurement must be real machine data.
- The MVD's frozen checklist needs: tile collision and a scrolling map
  (O4/O2), bitmap text for screens (O3), and a frame-budget report fed by
  early instrumentation (O1).

## Goals / Non-Goals

**Goals:**
- One drawing path — the blitter — that text and tiles both ride, so the
  deep dives examine one honest unit of work.
- Asset formats small enough to define by hand and read by eye (Part 0's
  byte-level habit), loaded through the seam's whole-file I/O into the
  arena.
- Two deep dives (caches; SIMD/assembly reading) with measured, quoted
  machine data, each leaving hooks Part 5's optimization passes revisit.

**Non-Goals:**
- No GPU, shaders, or OS drawing calls (D3).
- No rotation, scaling, or alpha blending — color-key transparency and
  axis-aligned integer blits cover the MVD's needs; the sprite pipeline is
  one copy loop the deep dives can read.
- No entity system, audio, or game states (Parts 3-4).
- No asset editor or build-time asset pipeline — assets are files the
  lessons author by hand.
- No optimization work in Part 2 beyond what the deep dives measure and
  explain; the fixing is Part 5's three-pass menu.

## Decisions

### D1. The Part 2 arc: 15 lessons, blitter -> caches -> SIMD reading -> text -> tilemap -> collision -> close

`lesson-044`…`lesson-058` in seven batches:

| Batch | Lessons | Arc |
| ----- | ------- | --- |
| Blitter | 044-046 | a sprite as loaded bytes; the clipped, transparent blit; the frame loop draws a sprite moved by polled input (the marker retires) |
| Caches | 047 | the **caches deep dive**: cache lines and locality measured on the blitter — concept-first, x86-64 as the worked example |
| SIMD reading | 048-049 | the **SIMD/assembly-reading deep dive**: the blitter's compiled assembly read instruction by instruction; vector registers as what the compiler reached for |
| Text | 050-051 | the bitmap font as an asset; glyph drawing on the blit path; text on screen |
| Tilemap | 052-054 | the tilemap asset format; tilemap drawing at camera offsets; the base camera and the additive offset (O2) |
| Collision | 055-056 | tile kinds and solidity in the format; point and rectangle collision queries (O4) |
| Close | 057-058 | the closing demo (a scrolling map, a moving sprite, text, measured frames) and the frame-budget table's first draft (O1's per-subsystem attribution, the format Part 5's finale grows) |

Ordering makes each batch observable before the next needs it: a sprite you
can move, then the machine's answer to why copying is slow, then the
compiler's answer read aloud, then the map that makes the camera mean
something, then the queries game logic will run. O1's instrumentation is
folded through the arc: the record grows one named phase per subsystem as
batches land (the frame-budget report's per-subsystem attribution starts
here).

### D2. One blitter; text and tiles are sprites with bookkeeping
All scene drawing funnels through one clipped copy: source pixels (with a
transparent color) into the framebuffer at an origin. Glyphs are one-pixel-
wide sprites from a font sheet; tiles are fixed-size sprites from a tile
sheet; the tilemap walk is a loop of blits at computed origins. The
alternatives — specialized loops per draw type (three copies of the clip
and transparency logic to keep honest) or `PutPixel` loops (simple, but the
caches and SIMD dives need a real copy loop to measure and read) — cost
more than they buy. One blitter is also the unit Part 5's optimizer
measures.

### D3. Asset formats are defined by hand and read by eye
Sprites and the font ship as PPM (P6) files — the format Part 0's lesson
017 wrote and lesson 038's screenshot already round-trips — so asset bytes
are inspectable with any image tool and the loader is a header plus a
copy. The tilemap is a small text format: dimensions, a tile-kind table
carrying solidity, then rows of one character per cell. Alternatives: a
binary map (smaller, opaque at exactly the level where learners should see
bytes); BMP (its headers are lesson 014's material but they are noise
here); PNG or any compressed format (violates the no-libraries rule).

### D4. Assets load through the seam into the arena
Every asset is a whole-file read (lesson-037's contract) whose bytes are
copied into an arena allocation (lesson-041) at startup; a missing or
malformed asset is a typed failure and the run reports it like every other
typed failure. Alternative: arrays embedded in the code — it would make
the file I/O delivered in Part 1 dead weight and hide the loading cost the
frame-budget report should eventually see.

### D5. The camera is one struct, summed at draw time
`Camera { base, additive }`; every scene draw adds them once to its origin.
The base scrolls the map (Part 5's capstone camera moves it); the additive
offset is the hook the juice toolkit will drive for screenshake (O2),
zero at rest. Alternatives: a global draw offset (hidden state that HUD
code would trip over); applying the additive offset inside the blitter
(one more parameter on the hottest loop for a once-per-frame value).

### D6. Deep dives: measured on the blitter, placed after it works
Caches follow the blitter batch (there must be a copy loop to measure),
SIMD/assembly reading follows caches (same loop, different lens). Both are
concept-first with x86-64 as the worked example (foundation's ARM64 risk
mitigation), both quote real measurements and real disassembly, and both
name their Part 5 revisit point. The dives read and measure; they do not
optimize — the frame-accounting record is the evidence they extend.

### D7. Verification stays headless and pixel-exact
Drawing is verified by reading the framebuffer and the window back
(lesson-031's checker pattern): sprites by pixel equality at sampled
points, clipping by edge cases, the camera by the same scene at two
offsets, collision by query results (no screen needed). The deep dives'
numbers come from real runs on this machine and say so. Exercise patches
keep the Part 1 standard: applied against the lesson's end state, built,
run.

## Risks / Trade-offs

- [The deep dives drift into optimization] → both dives end at "and here is
  what Part 5 will do about it"; no optimization lands in Part 2 code, and
  the boundary review checks the deep-dive lessons' quoted measurements
  instead of their speedups.
- [One blitter becomes a bottleneck the course can't explain] → lesson
  036's honest-cost pattern applies: the render phase is measured from the
  first blit, and the caches dive exists precisely to explain the numbers.
- [Asset formats churn after learners have files] → each format is fixed
  in the lesson that defines it (like every other contract here); later
  lessons extend by reading more, never by reinterpreting.
- [15 lessons of drawing without game logic feels abstract] → the batches
  move something the learner controls from the first one (the sprite is
  input-driven) and the closing demo is a world, not a test pattern.
- [Headless authoring misses desktop rendering quirks] → the renderer
  writes our own framebuffer; the only OS surface is the already-audited
  presentation path. Port-shaped exercises carry any machine variance.

## Migration Plan

Batch order follows D1; every lesson commits and tags as it lands
(`lesson-044` … `lesson-058`), prose co-committed with the code step.
Assets land in `assets/` in the batch that first reads them. The change
closes with `plan/part2-review.md`: velocity against the 10-20 h/week
review budget, exercise density against the conventions table (Parts 1-2
band), the deep dives' quoted measurements re-verified, and the
frozen-prefix recommendation updated per part-1's review. Rollback:
content-only; lessons are revertable per commit and the revision policy
governs anything published.

## Open Questions

- Whether the closing demo's world reuses the tilemap asset or authors a
  second one for variety — safe to settle at authoring time; it changes no
  contract.
