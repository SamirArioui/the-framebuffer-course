# Design

## Context

Part 3 closed with the mixer working and measured: sound loads as data,
plays on channels, and its cost has a row in the frame budget. What the
engine still has is one object at a time — the demo's sprite, one map,
one font — and a loop whose `dt` is whatever the wall clock says. Part
5's assembly lessons assume neither of those hold.

Constraints that shape the how:

- **No external libraries in student-visible code** — `tools/check-boundary.sh`
  keeps proving it, and the tables are parsed by hand like every other
  format in the course.
- **The language law (lesson 026) still governs `src/`** — no threads, no
  exceptions, no allocation while the game runs. Entity storage is the
  arena habit again: capacity decided up front.
- **Every concept a lesson uses is introduced in that lesson or an earlier
  one** *(curriculum: no forward dependencies)*. Part 4 cannot reach for
  Part 5's ideas — no AI, no animation, no weapon logic.
- **The toolchain is curriculum** — the table format's parsing is taught
  byte by byte, and the vertical slice's cost is measured, not guessed.

## Goals / Non-Goals

**Goals:**
- One store of live entities the game walks every frame, created from
  data — so Part 5's enemies, projectiles, and particles are rows, not
  new subsystems.
- One knob for game-time, so pause and hitstop are the same mechanism.
- A closing lesson that draws a hero walking a tilemap with the camera
  following **and introduces nothing** — the gate that says Part 4 is done.

**Non-Goals:**
- No AI, no animation, no weapon or damage model — those are Part 5's
  assembly lessons (L5-L9), and their presence here would break the
  no-forward-dependencies rule the other way: Part 4 would be teaching
  Part 5's content.
- No entity-component architecture. Entities are a struct of the facts
  the game acts on; component indirection is not needed to reach L0\* and
  would be a redesign rather than a service.
- No fixed-timestep or interpolation — game-time is a scale on the
  existing step. Determinism across machines is not a course goal.
- No pooling policy that steals live entities (see D3).
- No change to the seam, the frame record's meaning, or the asset
  contract the earlier parts fixed.

## Decisions

### D1. The Part 4 arc: 11 lessons, `lesson-071`…`lesson-081`, in five batches

| Batch | Lessons | Arc |
| ----- | ------- | --- |
| The table as data | 071-072 | the archetype table defined by hand (a row per definition, its columns named); tables loaded whole into the arena with typed failures |
| Entity storage | 073-075 | entities as rows created from a definition; one fixed store; iteration over the live entities; lifetime and slot reuse |
| The hero | 076-077 | the hero as the first entity — position, sprite, movement read from input; movement resolved against the tilemap's collision queries |
| Game-time | 078-079 | the game-time scale (pause sets 0, hitstop a fraction); the frame record keeps measuring wall-clock |
| Close | 080-081 | the L0\* vertical slice (a hero walks the tilemap, the camera follows); the slice's cost in the frame record |

Each batch is audible/visible before the next needs it: data before rows,
rows before movement, movement before the slice. Mirrors Part 3's shape
(12 lessons in five batches) at Part 4's lighter weight.

### D2. The table format: one row per definition, columns named by a header

A table file is a header line naming its columns, then one row per
definition, whitespace-separated, parsed byte by byte like `map.txt` was.
The loader refuses anything the format does not describe: a row with the
wrong field count, a value where a number is required, a definition
duplicated. Alternatives: key/value blocks (more ceremony than the
course's data needs, and the column header already names the fields), a
binary format (not inspectable, which is the point of hand-parsed assets
here), and CSV with quoting rules (a second parser discipline the course
does not teach).

The format is fixed in the lesson that defines it (the standing rule);
later lessons read more of it, never reinterpret it.

### D3. Entity storage: one fixed store, refuse when full — never steal

`ENTITY_CAP` slots decided at build time, each slot holding one entity or
nothing. Creation takes the first free slot; retirement frees one and the
slot is reusable. **When the store is full the request is refused as a
typed result** — the game decides what that means. Alternatives: steal the
oldest entity (right for sound effects, wrong here — an entity silently
replacing another is a correctness bug the player experiences as a
disappearing enemy), and allocate on demand (breaks the language law and
the "capacity is a decision" habit every buffer in this engine follows).
Part 5's particle bursts may want a stealing store; that is a second,
narrower store, not a change to this one.

### D4. Game-time is a scale on the existing step, and only there

`dt_game = dt_wall * scale`, where `scale` is one number the game sets (0
for pause, a fraction for hitstop, 1 for play). The platform clock stays
the measurer and the frame record's phases stay wall-clock durations —
the scale reaches the simulation's step and nothing else. Alternatives: a
separate game clock (two clocks to reason about, and lesson 035 already
taught why the monotonic clock is the one to trust), stopping the update
phase entirely on pause (the update still runs — the game reads input,
and drawing continues), and a per-system scale (hitstop is global by
definition).

### D5. The hero is an entity, and L0\* is assembly

The hero is the first row the game creates — position, sprite, a speed —
moved by input and resolved against the tilemap with the same collision
queries lesson 055 defined. L0\* adds no behavior: it composes the
tilemap (052-053), the camera (054), the mover habit (056), entities
(073-077), and game-time (078-079). The lesson's whole content is that
the fit is trivial. If it is not, the gate has done its job and a service
is missing.

**Entity attributes are a fixed struct, named by the table's header.**
An entity carries named fields the game reads directly — no key/value
bag, no lookup by string. The table's header names its columns and the
loader fills the fields it declares. Alternatives: a key/value bag read
by name (a second parsing discipline and a lookup per attribute per
frame — the opposite of what the frame budget wants), and a per-type
struct hierarchy (Part 5's types are rows in one table, not classes).
This is settled here because it decides the data model the tasks build.

### D6. Verification stays headless — and honest

- **The table loader is data→data**: a file in, definitions out or a typed
  failure. Byte-level checks, like every loader before it.
- **Entity storage is checkable without a game**: create, iterate, retire,
  reuse — the store's state is printable.
- **Game-time is checkable by its numbers**: the step at each scale value.
- **L0\* is verified the way Part 2's demo was** — the run under the
  headless display, the hero's position and the camera's base reported
  and reconciled against the map's bounds. What cannot be verified here —
  whether the slice *feels* right — is routed to port exercises, as
  desktop rendering was in Parts 1-2.

## Risks / Trade-offs

- [The fixed capacity is wrong for some game] → it is one constant, tuned
  in the lesson that introduces it and named in the closing review. The
  typed refusal keeps an over-full store loud rather than silent.
- [Refusing on full feels harsh next to the mixer's stealing] → taught
  side by side: the two policies differ because a stolen sound is
  inaudible and a stolen enemy is a bug.
- [The table format churns after learners have files] → fixed in the
  lesson that defines it; later lessons read more of it.
- [Game-time and wall-clock get confused] → the frame record keeps
  measuring wall-clock and the lesson says so on the same page as the
  scale; the scenario "the scale does not reach the seam" is the contract.
- [L0\* is not trivial] → that is the gate, not a failure of the lesson.
  The closing review records how much the slice had to invent; anything
  it invented is a missing service and goes on the record.
- [Entity iteration costs surprise the budget] → the `update` phase is
  already named; lesson 081 puts the slice's numbers in the record rather
  than guessing them (O1's pattern, again).

## Migration Plan

Content-only lessons on the Part 3 engine; each lesson commits and tags as
it lands (`lesson-071` … `lesson-081`), prose co-committed, assets in
`assets/` in the batch that first loads them. The change closes with
`plan/part4-review.md` (velocity, density, measurements re-verified,
frozen-prefix recommendation updated per `plan/part3-review.md`).
Rollback: content-only; lessons are revertable per commit and the revision
policy governs anything published.

## Open Questions

- Whether Part 5's particle bursts get their own stealing store or reuse
  this one with a per-request policy — deferred until Part 5's L12, which
  is where bursts exist. It changes nothing in this change's specs, its
  approach, or its task breakdown.
