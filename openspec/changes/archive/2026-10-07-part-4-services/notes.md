# Change notes — part-4-services

Authoring-time record for this change: what was verified before the
lessons were written, and what the audit found. These notes are authoring
material (like `plan/`); they are not lesson prose and never reach the
site.

## 1.1 Authoring/verification toolchain

Verified on the authoring machine before any lesson quotes a run. The
commands below were run exactly as written.

### Tool versions used

| Tool | Version |
| ---- | ------- |
| Linux | 6.6.87.2-microsoft-standard-WSL2 x86_64 (Ubuntu 24.04.4 LTS) |
| gcc / g++ | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) |
| Xvfb | 2:21.1.12-1ubuntu1.8 (display `:99`, 800x600x24) |
| xdotool | 1:3.20160805.1 (`1:3.20160805.1-5build1`) |
| X11 development headers | `libx11-dev` 2:1.8.7-1build1 |
| ALSA library / headers | 1.2.11 (`libasound2-dev` 1.2.11-1ubuntu0.3) |
| mdBook | v0.5.4 (pinned) |
| openspec | 1.14.0 |
| git | 2.43.0 |

The machine is unchanged from Part 3: no sound hardware (`/dev/snd` holds
only `timer`), headless by design, Xvfb on `:99` with a 800x600x24 screen.

### The commands, run as written

```
$ ./build.sh
build: OK (14 source(s) compiled -> build/game)

$ ./tools/check-boundary.sh
boundary: engine code must not name an OS
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp src/platform_alsa.cpp
boundary: the contract a second OS implements is declared in src/platform.h
boundary: 17 single-line declarations there (multi-line ones are in the header)

$ mdbook build
 INFO HTML book written to `/home/bad5am/code_tuto/gamedevtuto/site`

$ openspec validate --all
Totals: 13 passed, 0 failed (13 items)
```

The build is warning-free at the Part 3 closing state, and the boundary
check scans every `src/*.h` and `src/*.cpp` — so the six files this part
grows (`table.h/.cpp`, `entity.h/.cpp`, `gametime.h/.cpp`) are covered by
it the moment they exist, and each lesson's run is preceded by the check.

### The headless display and scripted input the vertical slice's check will need

The window check and the scripted-input check run against the same Xvfb
display the earlier parts used, and both were exercised end to end at the
lesson-070 state:

```
$ pgrep -a Xvfb
401085 Xvfb :99 -screen 0 800x600x24 -nolisten tcp

$ DISPLAY=:99 ./build/game &            # the run, its window titled "the framebuffer engine"
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
2097153
$ DISPLAY=:99 xdotool key --window 2097153 --delay 0 Right
$ DISPLAY=:99 xdotool windowclose 2097153
```

The run's tail, from that exact session:

```
engine: arena: 1534080 of 33554432 bytes used
engine: close reported
engine: closed
```

So the vertical slice's check has everything it needs: a headless display
to draw on, scripted keyboard input to walk the hero with (held keys via
`keydown`/`keyup`, brief presses via `key`), and a close path that ends
the run by name. Hero position and camera base are printed by the run
already (lesson 054's report) and Part 4 keeps that report.

### The whole-file read the table loader will sit on

`platform::ReadFile` / `platform::ReleaseFile` (src/platform.h) is the
seam's whole-file I/O — the contract lesson 037 fixed. It is what every
asset loader in `src/` sits on (`LoadSprite`, `LoadFont`, `LoadTileMap`,
`LoadTileSheet`, `LoadSample`), and the headless run above loaded all six
assets through it (sprite, font, map, tile sheet, music, effect) with no
partial reads: `FileData` is the file's complete bytes or a typed failure
(`FILE_NOT_FOUND` / `FILE_UNREADABLE`), and the bytes go back to the OS
with `ReleaseFile`. The table loader takes this path unchanged.

## 1.2 Audit: the engine and the frame record against the deltas

Recorded before the first services lesson is authored. What Part 4 must
grow, named by the delta that requires it:

| Delta | What Part 4 must grow | Where it lands |
| ----- | --------------------- | -------------- |
| `entity-tables` | **the table loader beside the other asset loaders** — a file read whole through the seam, parsed byte by hand into a complete table or a typed failure, the rows kept in the engine's arena, a refused load keeping nothing | `src/table.h` / `src/table.cpp`, beside `sprite.h`, `tilemap.h`, `audio.h`; `assets/entities.txt` beside the other course-owned assets |
| `entities` | **the fixed entity store** — entities created from the table's definitions, capacity decided up front (`ENTITY_CAP` slots, the arena habit again), creation taking the first free slot and refusing typed when full, iteration over the live entities, retirement and slot reuse, movement resolved with the tilemap's collision queries | `src/entity.h` / `src/entity.cpp` |
| `game-time` | **the game-time scale on the update's step** — `dt_game = dt_wall * scale`, the scale one number the game sets (0 = pause, a fraction = hitstop, 1 = play); the platform clock stays the measurer and the frame record's phases stay wall-clock | `src/gametime.h` / `src/gametime.cpp`, applied where `Run` computes the update's step |

The engine's existing shape fits all three without redesign:

- **The loader habit** (read whole → parse by hand → copy into the arena
  behind an `ArenaMark` → typed result struct) is five files old and the
  table loader keeps it exactly: `TableResult` is `SpriteResult`'s shape,
  the parse is `map.txt`'s (whitespace-separated, one row per line,
  numbers by hand), and the language law of lesson 026 holds over it.
- **The fixed-store habit** is the mixer's: `AUDIO_MIXER_CHANNELS` is a
  pool decided at build time, nothing allocated while sound plays. The
  entity store is the same decision for entities — with one deliberate
  difference, taught side by side: the mixer *steals* the oldest effect
  when the pool is busy, the entity store *refuses* (`ENTITY_FULL`), and
  the difference is that a stolen sound is inaudible while a stolen enemy
  is a bug the player experiences.
- **The update's step** is one line of arithmetic today (`dt = now -
  last`, feeding `SPRITE_SPEED * dt` in `Run`); the scale multiplies that
  step and nothing else. Everything the frame record measures is already
  taken from `platform::Now()` around each phase, so keeping the record
  wall-clock is the *default* — the audit's job is to keep the scale out
  of those calls.

**The frame record audit.** `FrameRecord` holds durations measured on the
platform clock (`update`, `audio`, `render`, `present`, `total`, and the
named sub-phases inside `render`: `sprites`, `text`, `tilemap`);
`FrameStats` sums them and `PrintFrameBudget` attributes them. Part 4
grows the record twice, both times without changing what a *phase* means:

1. lesson 079 records the **game-time step** beside the phases — it is
   the simulation's advance (seconds of game time), not a duration, and
   it is what makes "the scale does not reach the record" checkable in
   the same line as the wall-clock totals;
2. lesson 081 gives the **update phase its named entity sub-phase**
   (`entities`), the same attribution move lesson 046 made for `render`
   — named times live *inside* a phase and never instead of it.

Nothing in the deltas modifies an existing capability, so no earlier
spec's requirement changes: the seam's contract is untouched (the tables
are read through the whole-file I/O it already has), and the tilemap's
collision queries are used, not altered (`TileRectSolid` is called by the
entity mover; `tilemap.h` does not change).

## Authoring decisions where the artifacts were ambiguous

Recorded here as `plan/part3-review.md` records Part 3's, and like them
these are authoring decisions, not design changes.

- **The table's columns** are `name x y facing speed health sprite`: the
  proposal's "the facts the game acts on (position, facing, sprite,
  health)" plus the speed task 4.1 names. `name` and `sprite` are text
  columns (the definition's identity and the art file it draws); the rest
  are numbers, parsed digit by digit like `map.txt`'s header. The header
  names the columns and the loader fills the fields it declares — an
  unknown or duplicated column name is a malformed file, like a row with
  the wrong field count or a value where a number is required.
- **The sprite column names the art file** and the run loads each
  definition's art at startup, handing the definition its image — so an
  entity created from a definition is answered from the definition alone
  ("entities" requirement: *Definitions carry their facts*). The course
  ships one sprite asset; both rows in `assets/entities.txt` name it, and
  the lesson says so out loud. The table is where the art would change.
- **Lesson 071 / 072's split.** 071 is the format and the parse — the
  rows' destination is a fixed array in the table struct (the shape
  `TileMap::kinds` has), so the parse is the lesson. 072 moves the rows
  into the arena, where the row count is the file's fact and not a
  capacity the array imposed, and completes the load's transaction (an
  `ArenaMark` before the copy, every refusal path rolling back) — "the
  load is complete or named, and nothing partial is kept".
- **`ENTITY_CAP` is 64**, tuned in lesson 074 and named in the closing
  review: enough for the hero, the enemy types, and a screenful of
  projectiles; Part 5's particle bursts may want their own, narrower
  store (design D3's open question, deferred to Part 5's L12).
- **Entities carry a movement request** (`move_x`, `move_y`) beside the
  facts the table states. The table does not declare these columns: they
  are frame state, set by the game (the player's input for the hero,
  Part 5's AI for enemies) and turned into motion by the walk — so
  per-entity work is expressed once rather than per type, which is what
  the `entities` iteration requirement asks for. The hero's `facing`
  follows its movement; the row's value is where it starts facing.
- **The slice keeps the run's sound.** L0\* is the world side (tilemap,
  camera, mover, entities, game-time, per design D5's list); the music
  and effects the Part 3 demo plays are finished services and stay on.
  Nothing is invented for the slice, and nothing finished is dropped.
