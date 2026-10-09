# Design

## Context

Lesson 081 closed Part 4 with a hero walking the tilemap on finished
services (table, store, walk, mover, game-time, frame record) and the
slice inventing nothing. Part 5 stands 22 lessons on that state — the
largest part, at the skeleton's hard cap — and its acceptance is
external and frozen: `target-game`'s checklist, its bounded toolkit, its
three-pass menu, and its definition of done. Constraints that shape the
how:

- **The language law (lesson 026) governs every line of `src/`** — the
  game layer is engine code under it: no allocation while the game runs,
  no exceptions, nothing behind the seam's back.
- **No forward dependencies** — a lesson uses only what it or earlier
  lessons introduced; the skeleton's order is the dependency order.
- **The format rule**: assets are defined by hand in the lesson that
  needs them and never reinterpreted. Part 5's growth of the table's
  columns is additive and named (decision D3) so learner files from
  Part 4 keep loading.
- **The toolchain is curriculum** — the optimization passes measure on
  real runs; nothing is guessed.

See proposal.md — Why for the motivation and scope.

## Goals / Non-Goals

**Goals:**
- The finished game: every checklist line implemented and demonstrable
  on the engine Parts 1-4 built, with the game layer kept separable from
  the services it stands on.
- A game-data model that stays data: the game's per-kind facts live in
  tables, and the tables keep loading every file the course has shipped.
- The optimization discipline: measure, fix exactly what was measured,
  report — ending in the final frame-budget report.
- The course's close: a retrospective and a hand-off lesson.

**Non-Goals:**
- No fifth feel effect, no feature outside the frozen checklist —
  ideas are recorded as extras, never added.
- No engine rewrite and no service redesign: Part 5 consumes `entities`,
  `entity-tables`, `game-time`, `tilemap`, `audio-mixer`, and
  `frame-accounting` as they are.
- No speculative optimization beyond the two measured hotspots; no
  fixed timestep, no interpolation, no ECS — those are out of scope by
  the earlier designs' non-goals.
- No multiplayer, no editor, no second game mode.

## Decisions

### D1. The arc: 22 lessons in four batches, `lesson-082`…`lesson-103`

| Batch | Skeleton | Lessons | Arc |
| ----- | -------- | ------- | --- |
| The game's shape | L1-L5 | 082-086 | the state machine on finished services; the map and camera as the game's; tile collision; hero movement with accel/decel; animation and the feedback hooks |
| Combat and enemies | L6-L10 | 087-091 | projectiles and two weapons (the format grows here); enemy archetype tables; the three behaviors; the boss; waves |
| Feel and polish | L11-L15 | 092-096 | hitstop and screenshake; particle bursts and easing; the HUD; audio integration; the screens in final form |
| Debt, optimization, close | L16-L22 | 097-103 | pay the debt; measure; fix the two hotspots; the report; the retrospective; now make YOUR game |

The batches are audible/visible before the next needs them: the game's
shape before its fights, its fights before its feel, its feel before its
cost. Mirrors Parts 3-4's batching at the largest size.

### D2. The game layer is engine code in its own files

The game does not grow inside the services' files and the services never
grow game behavior. The state machine, hero feel, combat, AI, the
toolkit, and the HUD land in their own file pairs beside the services
(`game.*`, `hero.*`, `combat.*`, `ai.*`, `feel.*`, `hud.*` — the lessons
settle the exact splits), each under the language law and behind the
seam. Alternative: assembling everything in `main.cpp` (the run is the
game) — rejected because 22 lessons of assembly in one file is where
the debt lesson would drown; and a `game/` tree outside `src/` —
rejected because the build, the boundary check, and the language law all
speak about `src/`, and the game is bound by them too. This boundary is
also what L16's refactor and L17's profiler can point at: the game
layer's costs are attributable because it has edges.

### D3. The table format grows by named columns, additively

Part 5's data needs what the seven columns do not carry (damage,
projectile life, burst count, wave timing). The format grows **by named
columns**: `EntityDef` and the loader gain fields, and a header names the
columns a file uses — the loader fills the named fields and leaves the
rest at their defaults. Every file the course has shipped keeps loading
byte-for-byte; a new file names more columns. The one refusal edge this
relaxes — 072's "a column not named at all" — is taught as a deliberate
fix-forward step in the lesson that grows the format, with the
compatibility rule verified against `assets/entities.txt` unchanged.
Alternatives: reinterpreting columns per kind (health as damage — the
standing rule forbids it and the data lies); game-owned constants for
the extra facts (breaks "per-type attributes are data"); a second asset
format (two loaders, two places to look per kind). Additive named
columns keep one format, one loader, and honest values.

### D4. Hero movement: eased velocity toward the intent

The player's intent is a direction from polled input state, normalized
to unit length — so **the diagonal is no faster than the straight
run** (resolving lesson 076's exercise question in the direction feel
demands). The hero carries a velocity that eases toward
`intent × speed` (accel) and toward zero (decel); the mover resolves
each frame's step against the map exactly as lesson 077 defined. The
easing is a time constant the row can carry (the format grows), so the
feel is data like everything else. Alternative: instant velocity with
animation-only feel — rejected: the MVD names accel/decel as the hero's
behavior.

### D5. Combat: weapons are rows, projectiles are entities

A weapon is a table row that names the projectile definition it fires
and carries its rate and damage; firing creates projectile entities
from that definition through the store, moving through the mover in
game time. A projectile retires at walls, at its range's end, and at
the entity it hit; a hit reduces the target's health by the row's
damage and retires the projectile. Hits are the toolkit's triggers.
Alternative: projectiles as a separate particle-like system — rejected:
the store's iteration, lifetime, and movement already are what
projectiles need.

### D6. AI: three behaviors over the mover

Chase, keep-distance, and flee are small functions — the entity, the
hero's position, the map — that write the entity's movement request the
way the player's input writes the hero's. The walk's per-entity work
gains one branch on the entity's behavior (a fact its row can carry),
and every behavior moves through `MoveEntity`. The boss composes the
three with a pattern of its own (its own schedule, not its own
movement machinery). Alternative: per-type update functions — rejected
by the iteration requirement ("per-entity work is expressed once rather
than per type") and by the no-forward-dependencies rule (no
entity-component machinery this late).

### D7. The state machine drives the game-time scale

Five states — title, play, pause, death, victory — each owning its
screen and its input. The state sets the game-time scale: play is full
speed, every other state is 0. The simulation freezes outside play
while the presentation keeps drawing the state's screen — which is
exactly O5's hook ("pause sets 0") extended to the other screens, and
it keeps the frame record honest (lesson 079's contract). Transitions
are named conditions (the play state's triggers for death and victory),
and the input latch (lesson 033) keeps a pause key from acting twice.

### D8. The toolkit: four effects on the hooks that exist

- **Hitstop** — a fireable scale change with a wall-time deadline
  (lesson 078's exercise settled the clock: anything that must *end*
  while the game is stopped cannot run on game time).
- **Screenshake** — the camera's additive offset (lesson 054's hook),
  shaken for a moment and rested at exactly zero.
- **Particle bursts** — particles are entities from a table kind,
  bounded by the store's policy: **the main entity store serves them,
  and a burst that finds no slot drops the particle** — cosmetic work
  may be dropped, gameplay work may not (the policy contrast lesson 074
  taught: a dropped particle is invisible, a dropped enemy is a bug).
  This resolves Part 4's open question; the fallback — a second, narrow
  burst store with the mixer's stealing policy — is named for the day
  measurement shows pressure (L17 will).
- **Easing** — a small set of ease functions applied to the values the
  game animates (screens' fades, the HUD's counters), arriving exactly
  at their targets.

Nothing else: `target-game`'s "Feel toolkit bounded" is the acceptance
and the five lessons of feel/polish implement it exactly.

### D9. The HUD reads the game state

Score, health, and timers come from the game's own state (the same
values the states act on), drawn over the scene and never scrolling
(lesson 054's HUD rule). The play screen owns it; the other states draw
their own screens.

### D10. L16 is a planned lesson, not an apology

The assembly may accumulate structural debt (files that grew together,
names that stopped fitting). L16 is reserved to fix it forward —
"refactor is curriculum" — and the review records what it paid. Debt is
allowed to exist during L1-L15 and is not allowed to exist after L16.

### D11. The optimization is the frozen menu, and nothing else

Measure (instrument, profile, name the top-2 hotspots), fix exactly
those two (deep dives' techniques: cache layout, SIMD, allocation),
report (the final frame-budget report, attributed per subsystem). Every
optimization change names the hotspot it addresses; anything the passes
do not name is recorded as future work. The report is produced from
`frame-accounting`'s account — measured, never modeled.

### D12. Performance claims carry their machine

"60 fps on modest hardware" cannot be proven on the authoring machine;
the discipline Parts 1-4 established applies: numbers are measured on a
named machine (this one: WSL2, Xvfb, no sound hardware), port exercises
route the real-hardware check to the learner, and the report names its
machine. The definition of done is checked as far as honest measurement
goes, and the rest is said out loud.

## Risks / Trade-offs

- [22 lessons at the cap; scope creep from game ideas] → the frozen
  checklist decides; ideas are recorded as extras (the MVD's own rule);
  the task list mirrors the skeleton's batches so scope is visible.
- [The additive format growth revises a published refusal edge] → the
  compatibility rule keeps every shipped file loading; the growth is one
  lesson with the old files' load verified; the resync path covers
  learners whose files moved.
- [The game layer accretes debt during the assembly] → L16 exists in
  the skeleton for exactly this; the review records what it paid.
- [The two fixed hotspots may not be the two the game deserves] → the
  menu is the MVD's, not a judgment: measure names them, fixes address
  them, and anything else is recorded as future work.
- [Feel and AI are subjective] → every behavior has observable scenarios
  in the specs; what a headless run cannot judge routes to port
  exercises, as Part 4's design routed them.
- [The store's refusal during bursts] → cosmetic work drops, gameplay
  work does not (D8); the refusal count is reported so a full store is
  visible in testing.

## Migration Plan

Content-only lessons on the Part 4 engine; each lesson commits and tags
(`lesson-082` … `lesson-103`), prose co-committed, assets in the batch
that first loads them. The format growth ships with its compatibility
rule (D3) so existing files load unchanged. The change closes with
`plan/part5-review.md` (velocity, density, the measured claims, the
checklist completed) and the clean-checkout checks. Rollback:
content-only; lessons are revertable per commit and the revision policy
governs anything published.

## Open Questions

- The retrospective's exact comparisons (L21: our engine vs. real ones,
  the epilogue map) — content for that lesson, deferrable without
  changing the specs, the approach, or the task breakdown.
- The reference machine for "modest hardware" — named in the closing
  review when the 60 fps line is checked, not before.
