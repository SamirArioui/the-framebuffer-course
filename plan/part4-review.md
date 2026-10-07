# Part 4 boundary review — services (tables, entities, game time)

Part 4 closed with lessons `lesson-071`…`lesson-081` — 11 lessons, prose
+ one code step + 2 mixed exercises with diff solutions each, one commit
and one tag per lesson, co-committed per the authoring contract. This
review is the part-boundary record: velocity against the review budget,
exercise density against the conventions table, the toolchain actually
used, the measured claims re-verified, and the frozen-prefix
recommendation updated from `plan/part3-review.md`.

## The arc, as it landed

| Batch | Lessons | Arc | Status |
| ----- | ------- | --- | ------ |
| The table as data | 071-072 | the archetype table defined by hand (a header naming its columns, one row per definition, parsed byte by byte); the load whole into the arena, typed failures, nothing partial kept | tagged |
| Entity storage | 073-075 | entities as rows created from definitions; one fixed store (capacity decided, refusal typed, never stealing); the walk, retirement, and slot reuse | tagged |
| The hero | 076-077 | the hero as the first entity — position, sprite, a speed, moved by polled input and drawn through the camera; movement resolved against the tilemap's collision queries | tagged |
| Game-time | 078-079 | the game-time scale (pause 0, hitstop a fraction, play full); the scale reaches the step and nothing else | tagged |
| Close | 080-081 | the L0\* vertical slice (a hero walks the tilemap, the camera follows); the slice's cost in the frame budget | tagged |

The MVD's Part 4 obligations are delivered:

- **O5 (time-scale hook)** — `GameTime`/`GameTimeStep`, one knob from
  stopped to full speed on the update's step. Part 5's L11 drives it;
  lesson 078 defines it and lesson 079 keeps the measurement honest.
- **O6 (archetype storage)** — the table asset (the format defined by
  hand, loaded whole, complete or named) and the entity store (rows from
  the definitions, walked, retired, reused). Part 5's L7 holds its enemy
  types here; the store's policy answers the bursts question the design
  left open.
- **O8 (L0\* services gate)** — lesson 080's slice composes the tilemap,
  the camera, the mover, entities, and game-time and **invents
  nothing**; the code step is mostly deletions of the demo's
  scaffolding. The gate's answer is on the record: Part 4 is done.

## Authoring velocity vs. the 10-20 h/week estimate

`plan/part0-review.md` priced the curriculum at roughly 570 h at an
assumed 10-20 h/week of authoring and recorded that the estimate is off
by more than an order of magnitude as a prediction of *writing* time
under agent-assisted authoring. Part 4 confirms it a fourth time: the 11
lessons were authored, verified, and tagged in a single machine-assisted
session on 2026-10-07.

Part 3's reading carries forward, sharpened where this part differed:

- **Verification is still the cost, and this part's verification was
  arithmetic.** Every quoted number came from a run re-taken against the
  exact committed state — and the numbers that caught drafts were the
  reconciliations: the walk's visit account (`29 = 8 + 5 + 8 + 8`),
  the frame budget's table-vs-log equality, and the camera-base
  reconciliation (0 disagreements over the run's reported pairs).
- **Three authoring defects were caught by checks, not reading**: an
  out-of-bounds row index in lesson 075's reuse script (caught when
  lesson 076's draw walk dereferenced the garbage it produced), a
  lesson page whose embedded diff had drifted from its own tag (caught
  by the tag-vs-page check in task 7.2), and a `\n`-escape mangling
  introduced by the very tool that fixed the first (caught by the same
  check re-run). The check is what caught all three; the review should
  assume the same rate.
- **Estimate Part 4's cost in review-weeks.** Keep 10-20 h/week as the
  human *review* budget: 11 lessons ≈ **1-2 review-weeks**, on Part 0's
  calibration (25 lessons ≈ 2-4). The review pass should weight the
  reconciliation tables below first — they are the load-bearing claims.

## Exercise density against the conventions table

Conventions §4 sets Parts 3-4 at **1-2 mixed exercises** per lesson.
Every lesson counted:

| Lesson | Exercises | Distinct archetypes |
| ------ | --------- | ------------------- |
| 071 the archetype table | 2 | 2 |
| 072 the load, complete or named | 2 | 2 |
| 073 entities as rows | 2 | 2 |
| 074 one fixed store | 2 | 2 |
| 075 the walk and the free slot | 2 | 2 |
| 076 the hero as an entity | 2 | 2 |
| 077 the mover on an entity | 2 | 2 |
| 078 the game-time scale | 2 | 2 |
| 079 measurement is not scaled | 2 | 2 |
| 080 the vertical slice | 2 | 2 |
| 081 the slice's cost in the frame budget | 2 | 2 |
| **Total** | **22** | — |

**Every Part 4 density expectation is met**: 11 of 11 lessons have 2
exercises, inside the 1-2 band, and no lesson repeats an archetype, so
each lesson's pair is genuinely *mixed*.

Archetype spread across the part: predict-the-output 8, extend-the-code
5, explain-in-prose 4, measure-the-performance 3, fix-the-crash 1,
port-to-your-own-machine 1 — **all six archetypes used**. The spread
follows what services earn: predictions about policies (the store's
refusal, the camera's clamp, the mover's axes), measurement of the walk
and the frame, and prose defenses of the designs the specs fix (the one
knob, the row's boundary, the gate).

## Toolchain actually used

| Tool | Version | Used for |
| ---- | ------- | -------- |
| Linux | 6.6.87.2-microsoft-standard-WSL2 x86_64 (Ubuntu 24.04.4 LTS) | the authoring machine |
| gcc / g++ | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) | every build (17 sources at the part's close) |
| Xvfb | 2:21.1.12-1ubuntu1.8 | the headless display every window check ran on |
| xdotool | 1:3.20160805.1 | scripted input checks (076, 077, 078, 079, 080, 081) |
| X11 development headers | `libx11-dev` 2:1.8.7-1build1 | the seam's window implementation |
| ALSA library / headers | 1.2.11 (`libasound2-dev` 1.2.11-1ubuntu0.3) | the seam's sound implementation; the `null` device |
| mdBook | 0.5.4 (pinned) | every page render check |
| openspec | 1.14.0 | every planning validation |
| git | 2.43.0 | the history, tags, and every patch application |

No sound hardware: `/dev/snd` holds only `timer`. Every run continued
without sound, and the lessons' numbers say so where it matters.

Two toolchain findings this part introduced, both recorded in the
lessons or here:

1. **`stdbuf`'s preload breaks AddressSanitizer.** Running the game
   under `stdbuf -o0` (to see a crashed run's output) fails with `ASan
   runtime does not come first in initial library list` — the LD_PRELOAD
   `stdbuf` uses outranks the sanitizer. The fix is to not combine them:
   capture output to a file and read what survives, or run unbuffered by
   other means. Lesson 074's exercise documents the sanitizer command
   that works.
2. **The loop is event-driven without audio.** With no output device the
   loop sleeps between input news, so a scripted press wakes two frames
   (press and release) and only the frame that finds the key down moves
   the hero. Lessons 076 and 079 state this in prose; the numbers that
   depend on it are labeled.

## Measured claims, re-verified

Each claim below was re-run at its own lesson tag on the committed state.

| Claim | Lesson | Result |
| ----- | ------ | ------ |
| The table loads completely; every definition carries its row's values | 071 | `def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm` (and slime) — exact against the file |
| A missing or wrong-shaped file fails typed | 071-072 | `(missing)`, `(malformed)` for non-number / short row / duplicate name / bad facing / unknown column / column twice / header alone; `(too many rows)` at 17 rows in lesson 071's array |
| The rows are the file's count; a refused load keeps nothing | 072 | 17 rows load in 1700 bytes; the probe's arena unmoved across four refusals (`1900 -> 1900`) |
| An entity carries the table's values | 073 | field for field (`entity hero: x 312 y 232 facing 0 speed 240 health 3 sprite 16x16`) |
| A definition the table does not hold is a typed failure | 073 | `table: "dragon" -> unknown` |
| The full store refuses; creation allocates nothing | 074 | `live 64 of 64`; `arena 1253648 -> 1253648` across 64 creations; the probe's 65th request: `error 1, entity (nil)`, `slots changed: 0 of 64` |
| Every live entity walked exactly once per frame | 075 | `29 visits over 4 frames = 8 + 5 + 8 + 8`; the probe's visit lists `0-7`, `0 1 2 4 5`, `0 1 2 3 4 5` |
| A freed slot is reused before any never-used one | 075 | `created in slot 2 / 4 / 6`, slots 8-63 untouched |
| The hero moves at its row's speed | 076 | `312 -> 792` in a 2.004 s step (240 px/s × 2.004 = 481 px); `240 px/s while moving` over a scripted run |
| The camera follows and clamps to the map's bounds | 076, 080 | bases `120,0`/`123,0` following, `128,0` and `128,32` at the limits; every reported base reconciled against the clamp (0 disagreements) |
| The hero stops at solid tiles; the movement along the wall still does | 077 | `hero blocked at 706,480` holding; the slide `658 -> 663 -> 668` at y 477 |
| The step scales at 0, at a fraction, and at full speed | 078 | `3.807 ms of a 15.230 ms wall step` (×0.25 to the digit), `0.000 of 1018.469`, `15.122 of 15.122` |
| The record is wall-clock at any scale | 079 | `frame 50: step 0.000 ms, update 0.020 … total 1.649 ms` — phases real, the log's arithmetic closing |
| The slice composes; the walk's account closes | 080 | `372 visits over 186 frames = 2 × 186`; `world: 2 entities from the table's rows` |
| The entities row is measured, not guessed | 081 | 121 frames: log averages `update 0.012 / entities 0.001 / total 1.830` = the table's rows, digit for digit |

The walk's cost curve (081's exercise, this machine): 2 entities 0.001
ms, 20 → 0.002 ms, 200 → 0.007 ms — a fixed floor plus ~30 ns an
entity, roughly linear.

## Integration checks (task 7.2)

From a clean checkout (`git clone` of the repository) following only
`README.md`:

| Check | Result |
| ----- | ------ |
| `./build.sh` | `build: OK (17 source(s) compiled -> build/game)` — warning-free |
| `mdbook build` | `HTML book written to site` |
| `openspec validate --all` | `Totals: 13 passed, 0 failed (13 items)` |
| the vertical slice headlessly | runs, walks (140 visits over 70 frames), prints its budget, closes cleanly (`engine: closed`) |

Tags: `lesson-071`…`lesson-081` are consecutive (11 of 11), and for all
eleven the `src/`+`assets/` diff between consecutive tags is
byte-identical to that lesson's own code step — checked two ways: exactly
one commit touches `src/`+`assets/` in each range, and the diff block
embedded in each lesson page equals the tag-to-tag diff byte for byte.

## Recommendation: extending the frozen prefix

Following `plan/part3-review.md`'s discipline (no extension on authoring
evidence alone):

- **Do not extend the prefix on this review.** Part 4's 11 lessons have
  the author's verification behind them and no reviewer's pass.
- **When the review passes land, freeze in blocks:** `lesson-026`…`lesson-043`
  first (Part 1's dependency block), then `lesson-044`…`lesson-058`
  (Part 2), then `lesson-059`…`lesson-070` (Part 3), and
  `lesson-071`…`lesson-081` as the fourth block. Freezing Part 4 first
  would freeze services built on unfrozen rendering and sound.
- **Part 4's block carries Part 3's tail risk and adds one.** The timing
  rows (the `audio` phase, and now the `entities` row at 0.001 ms scale)
  move with the machine and the moment; the block-freeze review should
  re-run the table-vs-log reconciliation rather than re-read it. The
  *structural* claims — the visit accounts, the slot reuse, the camera
  clamp arithmetic, the typed failures — are deterministic and will
  reproduce exactly.
- **The resync path is unchanged** and covers this part: `git checkout
  lesson-NNN -- src/`.

## Review pass status (pre-freeze)

The independent review pass over Part 4 has **not** run; this review
records the author's verification, which is its input. What a reviewer
should re-run, in order of what breaks first:

1. `./tools/check-boundary.sh`, `openspec validate --all`, `mdbook
   build`, `./build.sh` (seconds; the build must be warning-free).
2. The clean-checkout integration checks above, including the tag-vs-page
   equality check for all eleven lessons.
3. The reconciliations: 075's visit account, 080's camera clamp, 081's
   table-vs-log. Each is arithmetic over a run's own reports.
4. The policy checks: 074's refusal (the probe's `0 of 64` slots
   changed), 072's rollback (`1900 -> 1900`), 077's mover (the corner
   probe's three answers).
5. Every solution patch: `git apply`, build, run, reverse — 22 patches.
6. The data claim itself: edit `assets/entities.txt` and see the game
   change with no rebuild (073's exercise states the protocol).

## Known warts carried forward (intentional, documented in-prose)

- **Bookkeeping commits sit between lesson tags** ("Record task
  progress", planning artifacts only). A raw `git diff lesson-N-1
  lesson-N` therefore includes `openspec/` changes; the `src/`+`assets/`
  diff is exactly that lesson's code step, verified for all eleven.
- **The table loader's duplicate-name check is quadratic** (each row's
  name compared against the rows before it). At the game's table sizes —
  a dozen rows — it is microseconds; lesson 072's exercise measures the
  curve and says where the line is. If a table ever holds thousands of
  rows, the check changes, not the format.
- **The entity's collider is its art's rectangle** (`MoveEntity` asks
  the map about `sprite->width × sprite->height`). Deliberate and
  documented in lesson 077: what an entity draws is what it collides
  as. An invisible entity or art larger than the body would need a
  different answer, and none exists yet.
- **`assets/entities.txt` names one sprite for both rows.** The course
  ships one sprite asset; the table is where the art would change (the
  lessons say so). Part 5's art is its own business.
- **The game-time demo's scale script runs on wall time** (three, five,
  seven seconds). Lesson 078's exercise asks the game-time/wall-time
  question for a fired effect and settles it: anything that must *end*
  while the game is stopped cannot run on game time.
- **The loop is event-driven without an audio output** (Part 3's wart,
  unchanged): on this machine a scripted press wakes two frames and only
  one moves the hero. Lessons 076, 078, and 079 label the numbers this
  affects.
- **`ENTITY_CAP` is 64**, tuned in lesson 074 and named here as the
  design requires: enough for the hero, the enemy types, and a screenful
  of projectiles. 081's exercise prices 200 entities at 0.007 ms a
  frame, so raising it is cheap; the store's *policy* (refuse typed,
  never steal) is what makes a wrong capacity a tuning problem.
- **Two authoring defects were fixed by amending lesson 075 during the
  batch** (a row index in the reuse script, and that fix's echo in the
  page's diff block) — before any review, in the volatile tail. The tag
  moved twice within the session; the final state is what tasks 7.2's
  checks ran against.
- **The slice's world is two entities** — the table's rows. The demo's
  scaffolding (the fill script, the kill check, the reuse script) lives
  at its own tags (lessons 074-075) and is removed at 080 by design:
  the slice invents nothing, and what it deletes is the demo.

*(Everything above is measured from the tagged lesson states. The
clean-checkout checks, the tag-vs-page equality check, the
reconciliations, the policy probes, and every solution patch were run on
the authoring machine before this review was written.)*
