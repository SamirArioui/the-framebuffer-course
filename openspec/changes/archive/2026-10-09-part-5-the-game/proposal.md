# Proposal

## Why

Part 4 closed the services gap and its gate passed: L0\* runs a hero
across the tilemap with the camera following, on finished services only,
and the slice invented nothing. What is left is the game itself. Every
line of the MVD's frozen checklist is still open — the hero's feel, the
combat, the enemies and the boss, the states, the juice toolkit, the
HUD, and the performance line — and the contract says the game is
declared complete only when every line is implemented and demonstrable,
60 fps holds on modest hardware, and the final frame-budget report
accounts for the frame. Part 5 is the course's last part: assemble the
game on the services Parts 1-4 built, pay the architectural debt the
assembly accumulates, run the fixed three-pass optimization, and close.

## What Changes

- **Author Part 5's lesson arc** per the frozen skeleton
  (`plan/part5-skeleton.md`: L1-L22, hard cap 22 lessons) — growing
  `src/` linearly from lesson-081's closing state (`lesson-082` onward,
  course-wide sequence).
- **Game assembly (skeleton L1-L15, lessons 082-096)**: the game on
  finished services — the update/render loop and the game-state machine
  (title, play, pause, death, victory); the tilemap and camera
  reassembled as the game's; tile collision resolved for hero and
  entities; the hero's eight-direction movement with accel/decel feel;
  sprite animation and the feedback hooks; projectiles and the two
  weapon types; the enemy archetype tables; the three AI behaviors
  (chase, keep-distance, flee); the boss; waves; the juice toolkit
  (hitstop and screenshake first, then particle bursts and easing); the
  HUD (score, health, timers); audio integration through the mixer's
  per-channel playback; and the screens in final form.
- **Pay the debt (L16, lesson 097)**: one fix-forward refactor lesson —
  "refactor is curriculum" — for whatever the assembly accumulated.
- **The fixed three-pass optimization (L17-L20, lessons 098-101)**:
  measure and name the top-2 hotspots; fix exactly those two with the
  deep dives' techniques (cache layout, SIMD, allocation); produce the
  **final frame-budget report** — the course finale — attributing
  per-frame time to each major subsystem.
- **Closing (L21-L22, lessons 102-103)**: the retrospective (our engine
  vs. real ones; the epilogue map) and "now make YOUR game".
- **Grow the table format additively** where the game's data needs it —
  damage, projectile life, burst count, wave timing — as named columns
  on the definition struct and the loader. A header names the columns a
  file uses, so every earlier table keeps loading unchanged and the one
  refusal edge this relaxes ("a column not named at all") is taught as a
  deliberate fix-forward step.
- **Complete the MVD checklist**: all eight frozen lines implemented and
  demonstrable — the ones Parts 2-3 already delivered (world, audio)
  re-verified as the game assembles them, the rest built here.
- **Spec-level**: six new capabilities carry the game's behavior
  contracts. **No existing requirement changes** — `target-game`'s frozen
  contract is satisfied, not edited; `frame-accounting`'s record and
  report contracts are what the finale measures against; `entities`,
  `entity-tables`, `game-time`, `tilemap`, and `audio-mixer` are used,
  not modified.

## Capabilities

### New Capabilities
- `hero-movement`: the hero's eight-direction movement with accel/decel
  feel — how intent becomes motion over time, and what the player
  experiences as it starts, turns, and stops.
- `combat`: projectiles and the two weapon types — what each weapon
  fires, how projectiles live and end, and how combat reads on screen.
- `enemies`: the three enemy types, the boss, and the waves — the
  chase / keep-distance / flee behaviors over the archetype tables, and
  how waves bring the types together.
- `game-states`: the game-state machine — title, play, pause, death, and
  victory: what each state shows, what it accepts, and how transitions
  happen.
- `game-feel`: the juice toolkit's four effects — hitstop, screenshake,
  particle bursts, easing — each immediate and readable, and nothing
  outside the four.
- `hud`: the play screen's readouts — score, health, and timers — kept
  readable while the game runs.

### Modified Capabilities

None. Every existing requirement stands as it is: the target game's
contract is frozen (`target-game`), the frame record's meaning is fixed
(`frame-accounting`), and the services Parts 1-4 delivered are consumed
as they are.

## Impact

- **`src/`**: grows from lesson-081's closing state with the game layer —
  the state machine, hero movement and its feel, weapons and
  projectiles, enemy behavior and waves, the four feel effects, and the
  HUD — then L16's refactor and the three-pass optimization's measured
  fixes. All engine code stays inside the language law and behind the
  platform seam; the optimization passes are the only place the deep
  dives' techniques (cache layout, SIMD, allocation) touch the engine.
- **`assets/`** (grows): the game's tables — enemy types, the boss, the
  two weapons, projectile and burst definitions — in the format lessons
  071-072 defined, grown additively for the game's data, plus any art
  the animation and screens need.
- **`book/`**: Part 5's lesson pages (one per lesson, 1-2 mixed
  exercises each with diff solutions) and the Part 5 section in
  `SUMMARY.md`; the stability-horizon banner keeps naming the frozen
  prefix and the volatile tail.
- **Tags**: `lesson-082`…`lesson-103` — 22 lessons, course-wide
  sequence, one per lesson, co-committed with prose.
- **`plan/`**: Part 5's boundary-review notes (velocity, density, the
  measured claims, the definition-of-done checklist).
- **Scope size**: the course's largest part — 22 lessons, the hard cap —
  implemented in birth-and-growth batches, with the L16 debt lesson and
  the three-pass menu keeping the scope closed: new ideas are recorded
  as extras, never added.
