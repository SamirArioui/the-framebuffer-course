# Proposal

## Why

Part 1 delivered the platform layer, but the course's central promise —
*every pixel is written by code we own* — has no renderer yet. Part 2 is
where the engine starts drawing: the software renderer (foundation D3),
with the MVD's backward obligations O1-O4 landing here (frame-timing
instrumentation, camera offsets, bitmap text, tilemap + collision) and the
curriculum's caches and SIMD/assembly-reading deep dives living on the
blitter (D9). Nothing in Part 3-5 can be authored until its text, maps,
and camera exist.

## What Changes

- **Author Part 2's lesson arc** per the foundation design (D3 software
  renderer as the main line; D9 deep dives placed, not scheduled): sprite
  drawing through a clipped, transparent blitter; the **caches deep dive**
  on the renderer's memory behavior; the **SIMD/assembly-reading deep dive**
  on the blitter; bitmap text; tilemap drawing; tile-level collision
  queries; and camera offsets — growing `src/` linearly from lesson-043's
  closing-demo state (`lesson-044` onward).
- **Deliver the MVD's Part 2 obligations**: O1 frame-timing instrumentation
  (per-frame timing hooks and a frame-time log, early — on Part 1's clock,
  for Part 5's profiler lesson and frame-budget report), O2 camera offsets
  (viewport offset that scrolls the map plus the additive camera-offset
  hook the juice toolkit will drive), O3 bitmap text (glyph rendering from
  a bitmap font onto our own framebuffer), and O4 tilemap + collision (a
  tilemap asset format, tilemap drawing, and tile-level collision queries
  that Part 5 turns into resolution).
- **Load assets through the seam's whole-file I/O** — sprite, font, and
  tilemap data are files the engine reads with the contract lesson 037
  fixed; asset bytes live in the engine's arena (lessons 040-041).
- **Keep the toolchain curriculum going** (curriculum: Toolchain is
  curriculum): the profiler's role in measurement before optimization is
  set up by the frame-time log here (the profiler lesson itself is Part
  5's), flags explained as they appear, and both deep dives quote real
  disassembly and real measurements.
- **Turn the continuity machinery onto the renderer**: `src/` continues its
  one linear history tagged `lesson-NNN` (course-wide numbers continue from
  `lesson-044`), prose co-committed with every code step, exercise density
  at the Parts 1-2 level (1-2 "make it yours" extensions per lesson).
- **Close with a part-boundary review** in `plan/`: velocity against the
  10-20 h/week review budget, exercise density against the conventions
  table, the deep dives' quoted measurements re-verified, and the
  frozen-prefix recommendation updated.

## Capabilities

### New Capabilities

- `software-rendering`: the engine's scene drawing — sprite blits with
  clipping and transparency, tilemap drawing, bitmap text, and camera
  offsets — all as pixels written into the engine's own framebuffer.
- `tilemap`: the map as data — the tilemap asset format the engine loads
  and the tile-level collision queries game logic runs against it.
- `frame-accounting`: per-frame measurement — the frame record and
  frame-time log that the profiler lesson and the final frame-budget
  report both read.

### Modified Capabilities

(none — Part 2 implements contracts the existing specs already fix: the
curriculum's deep-dive placement and part order, the exercises density
table, and the platform-layer contract Part 1 delivered all govern Part 2
as written.)

## Impact

- **`src/`**: grows from lesson-043's closing-demo state with the renderer
  (blitter, text, tilemap drawing, camera), the tilemap data and collision
  queries, and the frame-accounting growth of lesson-036's record. All
  engine code stays inside the language law and behind the platform seam
  (`tools/check-boundary.sh` keeps it checked).
- **`assets/`** (new): course-owned asset files the lessons load — a bitmap
  font, a tilemap, and sprite data, in formats the lessons define by hand.
- **`book/`**: Part 2's lesson pages (one per lesson, 1-2 extensions each
  with diff solutions) and the Part 2 section in `SUMMARY.md`; the
  stability-horizon banner keeps naming the frozen prefix and the volatile
  tail.
- **Tags**: `lesson-044` onward, course-wide sequence, one per lesson,
  co-committed with prose; `git diff` between tags equals the code steps
  between them.
- **Deep dives**: two embedded lessons' worth of quoted, re-verified
  machine data (cache behavior of the blitter; x86-64 assembly reading of
  the compiler's output) — each revisited in Part 5's optimization passes.
- **`plan/`**: Part 2's boundary-review notes (velocity, density, the deep
  dives' measurements, the frozen-prefix recommendation).
- **Scope size**: a whole part (~15-18 lessons) implemented in
  birth-and-growth batches, with the boundary review closing the change.
