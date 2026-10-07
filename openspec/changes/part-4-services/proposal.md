# Proposal

## Why

Part 3 delivered the mixer (O7), so every obligation Parts 1-3 own is met.
Part 5's assembly lessons, though, still assume three services that do not
exist. There is no game-time scale, so pause and hitstop have nothing to
ride on (L11 needs the hook, L1 the pause). There is no data-driven place
to hold enemy types, the boss, projectiles, or particle bursts, so L7's
archetype tables have nowhere to live and L12's bursts have no source. And
nothing has yet shown the engine's services *compose*: every part so far
proved its own subsystem, and none of them together. Part 4 closes those
three gaps and ends on the capstone that turns "Part 4 done" into a
testable moment.

## What Changes

- **Author Part 4's lesson arc** per the foundation design (services built
  on the Part 3 engine: data-driven entity definitions, live entity
  storage, game-time, and the vertical slice) — growing `src/` linearly
  from lesson-070's closing state (`lesson-071` onward).
- **Deliver the MVD's Part 4 obligation (O5)**: a **game-time scale** the
  game can set — pause sets it to 0, hitstop sets a fraction — so the
  simulation's `dt` is game-time and the loop's wall clock stays the
  measurer it already is. Part 5's juice toolkit drives it; Part 4
  defines it.
- **Deliver O6 — archetype storage**: **data-driven entity/asset tables**
  that serve 3 enemy types, the boss, projectiles, and particle bursts.
  The tables are a course-owned asset in a format the lessons define by
  hand (the same habit as the tilemap's `map.txt` and the WAV loader),
  loaded whole with typed failures; live entities are rows created from
  them, iterated every frame, and retired.
- **Deliver O8 — the L0\* services gate**: Part 4's closing lesson draws a
  **hero walking a tilemap with the camera following, using only finished
  services**. It introduces no new behavior; if it is not trivial, a
  service is missing and Part 4 is not done.
- **Load the tables through the seam's whole-file I/O** — the contract
  lesson 037 fixed, parsed by hand, kept in the engine's arena (lessons
  040-041): the same asset habit as sprites, fonts, maps, and samples,
  extended to data tables.
- **Keep the toolchain curriculum going** (Toolchain is curriculum): the
  table format's parsing is taught byte by byte like every other format in
  the course, and the vertical slice's cost lands in the frame record
  rather than being guessed.
- **Turn the continuity machinery onto the services**: `src/` continues its
  one linear history tagged `lesson-NNN` (course-wide numbers continue
  from `lesson-071`), prose co-committed with every code step, and
  exercises at the **Parts 3-4 density** — 1-2 mixed exercises per lesson
  (conventions §4).
- **Close with a part-boundary review** in `plan/`: velocity against the
  10-20 h/week review budget, exercise counts against the conventions
  table, the measured claims re-verified, and the frozen-prefix
  recommendation updated per `plan/part3-review.md`.

## Capabilities

### New Capabilities

- `entity-tables`: data-driven entity definitions as loadable data — the
  archetype table asset format defined by hand, loaded whole through the
  seam into the arena, and either complete or a typed failure — so enemy
  types, weapons, and particle bursts can be authored as data, changed
  without recompiling, and tested without running the game.
- `entities`: live entity storage and lifetime — entities created from
  the tables, carrying the facts the game acts on (position, facing,
  sprite, health), iterated every frame, moved against the tilemap, and
  retired — so the game holds many of them without inventing storage per
  feature.
- `game-time`: the game-time scale — a single scale factor the game sets
  that turns wall-clock time into the `dt` the simulation advances by, so
  pause and hitstop are one knob rather than special cases threaded
  through the update.

### Modified Capabilities

None. Every existing capability's requirements stand as they are: the
seam's contract is unchanged (the tables are read through the whole-file
I/O it already has), the frame record still measures wall-clock phases,
and the tilemap's collision queries are used, not altered.

## Impact

- **`src/`**: grows from lesson-070's closing state with the table loader,
  entity storage (rows, iteration, lifetime), and the game-time scale; the
  demo grows into the vertical slice. All engine code stays inside the
  language law and behind the platform seam.
- **`assets/`** (grows): course-owned table files — at least the hero's
  definition and one enemy type — in the format the lessons define by
  hand.
- **`book/`**: Part 4's lesson pages (one per lesson, 1-2 mixed exercises
  each with diff solutions) and the Part 4 section in `SUMMARY.md`; the
  stability-horizon banner keeps naming the frozen prefix and the
  volatile tail.
- **Tags**: `lesson-071` onward, course-wide sequence, one per lesson,
  co-committed with prose; `git diff` between tags equals the code steps
  between them.
- **`plan/`**: Part 4's boundary-review notes (velocity, density, the
  measured claims, the frozen-prefix recommendation).
- **Scope size**: a whole part (~10-12 lessons) implemented in
  birth-and-growth batches, with the boundary review and the L0\*
  capstone closing the change.
