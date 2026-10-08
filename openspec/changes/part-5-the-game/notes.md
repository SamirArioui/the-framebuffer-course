# Change notes — part-5-the-game

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
| xdotool | 3.20160805.1 |
| X11 development headers | `libx11-dev` 2:1.8.7-1build1 |
| ALSA library / headers | 1.2.11 (`libasound2-dev` 1.2.11-1ubuntu0.3) |
| gprof (the measure-pass profiler) | GNU Binutils 2.42 |
| mdBook | v0.5.4 (pinned) |
| openspec | 1.14.0 |
| git | 2.43.0 |

The machine is unchanged from Part 4: no sound hardware (`/dev/snd` holds
only `timer`), headless by design, Xvfb on `:99` with a 800x600x24 screen.

### The commands, run as written

```
$ ./build.sh
build: OK (17 source(s) compiled -> build/game)

$ ./tools/check-boundary.sh
boundary: engine code must not name an OS
boundary: OK — OS headers and OS calls appear only in src/platform_x11.cpp src/platform_alsa.cpp
boundary: the contract a second OS implements is declared in src/platform.h
boundary: 17 single-line declarations there (multi-line ones are in the header)

$ mdbook build
 INFO HTML book written to `/home/bad5am/code_tuto/gamedevtuto/site`

$ openspec validate --all
Totals: 16 passed, 0 failed (16 items)
```

The build is warning-free at the Part 4 closing state (lesson-081), and
the boundary check scans every `src/*.h` and `src/*.cpp` — so the game
layer's file pairs (`game.*`, `hero.*`, `combat.*`, `ai.*`, `feel.*`,
`hud.*`) are covered by it the moment they exist, and each lesson's run
is preceded by the check.

### The headless display and scripted input the game's checks will need

The window check and the scripted-input check run against the same Xvfb
display the earlier parts used, and both were exercised end to end at the
lesson-081 state — this batch's checks are state-by-state (title, play,
pause, death, victory) and drive more keys than Part 4's walk did, but
the machinery is the same and it works:

```
$ pgrep -a Xvfb
602704 Xvfb :99 -screen 0 800x600x24 -nolisten tcp

$ DISPLAY=:99 ./build/game &            # the run, its window titled "the framebuffer engine"
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
2097153
$ DISPLAY=:99 xdotool keydown --window 2097153 Right   # a held key: the hero walks
$ DISPLAY=:99 xdotool keyup --window 2097153 Right
$ DISPLAY=:99 xdotool windowclose 2097153
```

The run's tail, from that exact session:

```
engine: hero at 492,232 (t=3.007)
engine: camera base 128,0 (t=3.007)
engine: frame budget — 3 frames, avg 2.362 ms, worst 2.718 ms (frame 1)
engine:   subsystem   avg ms    share
engine:   update       0.004       0%
engine:     entities   0.001       0%
engine:   audio        0.000       0%
engine:   render       1.688      71%
engine:     sprites    0.003       0%
engine:     text       0.008       0%
engine:     tilemap    1.009      43%
engine:   present      0.671      28%
engine:   total        2.362     100%
engine: close reported
engine: closed
```

So the game batch's checks have everything they need: a headless display
to draw on, scripted keyboard input to drive each state's input with
(held keys via `keydown`/`keyup`, brief presses via `key`), and a close
path that ends the run by name. The hero position and the camera base are
printed by the run already (lesson 054's report, kept through 081) and
Part 5's state-by-state checks lean on the same reports.

The one warning the session prints — `XGetInputFocus returned the focused
window of 1` — is a known benign Xvfb/Xlib quirk on focus, seen in the
earlier parts' sessions too; it is not an engine failure.

### The frame account and the profiler the three-pass optimization sits on

The three-pass menu's **measure** pass has two instruments on this
machine, and both were verified to run as written:

1. **The frame account** — `FrameRecord` / `FrameStats` /
   `PrintFrameBudget` (`src/frame.h`, `src/frame.cpp`). This is the
   built-in instrument and the one the **final frame-budget report**
   (L20) is produced from. It already attributes per-frame wall time to
   each major subsystem (update, its `entities` sub-phase, audio, render
   and its `sprites`/`text`/`tilemap` sub-phases, present) and prints the
   table above. Verified by the run quoted in §1.1.

2. **`gprof` (GNU Binutils 2.42) via `-pg`** — the function-level profiler
   the **measure** pass (L17) names the top-2 hotspots with. Built and run
   as written to confirm it produces a readable profile:

   ```
   $ BUILD_DIR=build-pg CXXFLAGS="-std=c++17 -O0 -g -Wall -Wextra -pg" \
       LDFLAGS="-lX11 -lasound -pg" ./build.sh
   build-pg OK
   $ DISPLAY=:99 ./build-pg/game &   # drive a few frames, then close
   $ gprof build-pg/game gmon.out | head
   Flat profile:
   Each sample counts as 0.01 seconds.
    no time accumulated
     %   cumulative   self              self     total
    time   seconds   seconds    calls  Ts/call  Ts/call  name
     0.00      0.00     0.00        2     0.00     0.00  engine::ArenaAlloc(...)
     ...
   ```

   `gmon.out` is written on close and `gprof` reads it and lists the
   engine's functions. The short verification run is too brief to
   accumulate samples (hence "no time accumulated"), which is expected —
   L17's measure pass runs real frames to fill the profile.

What is **not** on this machine, and is named so the measure pass does
not pretend otherwise: `perf` (and `perf_event_paranoid` is `2`, so it
would be restricted even if installed), `valgrind`/callgrind, `ltrace`,
`strace`. So the measure pass sits on **the frame account plus `gprof`**,
and nothing is guessed — the two hotspots L17 names come from `gprof`'s
flat profile and the frame account's subsystem table, measured from real
frames of the finished game on this machine.

## 1.2 Audit: the engine and the game's data against the six deltas

Recorded before the first game lesson is authored. What Part 5 must grow,
named by the delta that requires it, and grounded in the MVD's frozen
checklist (`plan/target-game-mvd.md`):

| Delta / MVD line | What Part 5 must grow | Where it lands |
| ---------------- | --------------------- | -------------- |
| `game-states` *(MVD: states)* | **the game layer beside the services** — the update/render loop and the five-state machine (title, play, pause, death, victory), each state owning its screen and its input, transitions on named conditions, the simulation standing still outside play (the state sets the game-time scale) | `src/game.*` and the state's own files, beside the services (design D2); the states drive `gametime`'s scale |
| `hero-movement` *(MVD: hero)* | **eight-direction movement with accel/decel** — intent from polled input normalized to unit length (the diagonal no faster than straight), a velocity easing toward `intent × speed` and toward zero, resolved against the map through the mover; the ease is a data constant its row carries | `src/hero.*`, using `entity` + `tilemap` collision queries |
| `combat` *(MVD: combat)* | **projectiles and two weapon types** — weapons as table rows naming the projectile kind they fire; projectiles as entities moving in game time, retiring at walls / range / the entity they hit, a hit reducing health by the row's damage | `src/combat.*`, the store + mover |
| `enemies` *(MVD: enemies)* | **three enemy types + the boss** as table rows, the chase / keep-distance / flee behaviors over the mover, the boss composing the three plus its own pattern, and waves that bring the roster together and advance when cleared | `src/ai.*` + `src/game.*` (waves) |
| `game-feel` *(MVD: juice toolkit)* | **the four effects and nothing else** — hitstop (the scale with a wall-time deadline), screenshake (the camera's additive offset, rested at zero), particle bursts (particles as entities, bounded by the store's policy), easing (values that arrive at their targets) | `src/feel.*`, on `gametime` + `camera` hooks that already exist |
| `hud` *(MVD: — )* | **the play screen's readouts** — score, health, timers — read from the game's own state, drawn over the scene, not moving with the camera | `src/hud.*`, on the text/font path |
| `frame-accounting` *(MVD: perf)* | **the final frame-budget report** — per-frame time attributed to each major subsystem from real frames of the finished game | grows `FrameRecord`/`PrintFrameBudget`; produced in L20 |
| — *(MVD: perf)* | **the three-pass optimization** — measure, fix exactly the two measured hotspots (cache layout, SIMD, allocation), report | L17-L20; the deep dives' techniques touch the engine only here |

### The game layer beside the services (design D2)

The game does not grow inside the services' files and the services never
grow game behavior. The state machine, hero feel, combat, AI, the
toolkit, and the HUD land in their own file pairs beside the services
(`game.*`, `hero.*`, `combat.*`, `ai.*`, `feel.*`, `hud.*` — the lessons
settle the exact splits), each under the language law (lesson 026) and
behind the platform seam. The boundary check already covers every
`src/*.h` / `src/*.cpp`, so the new pairs are scanned the moment they
exist.

### The table format's named columns and the compatibility rule (design D3)

The seven columns the format knows today (`name x y facing speed health
sprite`, `assets/entities.txt`) do not carry the game's new facts —
damage, projectile life, burst count, wave timing, the accel/decel time
constant, the behavior kind, a weapon's rate and the projectile it names.
The format grows **by named columns, additively**:

- `EntityDef` and the loader gain fields; a **header names the columns a
  file uses**, and the loader fills the named fields and **leaves the
  rest at their defaults**.
- **Compatibility rule:** every file the course has shipped keeps loading
  **byte-for-byte** — `assets/entities.txt` (2 rows, 7 columns) is
  verified unchanged. A new file names more columns.
- The one refusal edge this relaxes — 072's "a column not named at all" —
  is taught as a deliberate fix-forward step in the lesson that grows the
  format (L6 / lesson-087): a file may now omit columns the format knows,
  and those fields take their defaults. A column the format does *not*
  know is still a malformed file.

Additive named columns keep one format, one loader, and honest values —
no reinterpreting a column per kind (health-as-damage lies), no
game-owned constants (breaks "per-type attributes are data"), no second
format (two loaders).

### The game's own tables (assets)

`assets/` grows the game's data in the grown format: the three enemy
types and the boss as rows, the two weapons as rows (each naming the
projectile kind it fires), and the projectile and burst definitions —
plus any art the animation and screens need. All per-type attributes are
data in these tables; **no per-type copy of the attributes appears in
code**.

## Authoring decisions where the artifacts were ambiguous

Recorded here as the earlier parts' notes recorded theirs: authoring
decisions, not design changes.

- **The measure-pass profiler is `gprof` via `-pg`.** The design says
  "whatever profiler the measure pass uses" without naming one; on this
  machine only `gprof` is present (`perf` and `valgrind` are absent).
  L17's measure pass therefore names the top-2 hotspots from `gprof`'s
  flat profile cross-checked against the frame account's subsystem table.
  The frame account is the instrument the report is produced from (D11).
- **The frame account is the finale's source of truth.** The final
  frame-budget report (L20) is produced from `frame-accounting`'s account
  — measured, never modeled (D11). The optimization passes do not
  redesign the record; they add rows only where a new subsystem needs
  naming.
- **State-by-state verification is scripted against the same Xvfb
  display** Part 4 used. Part 5's checks drive more keys (the state
  machine's inputs: start, pause, resume, restart) than the walk did, but
  through the same `xdotool` `key`/`keydown`/`keyup` path — no new
  verification machinery is invented.
- **The two weapons are data, not code** (combat spec: "The weapons are
  data"). Their rows live in the grown table alongside the enemy rows;
  the lesson that grows the format (087) is where the columns they need
  first appear.

## Lesson-082 — the game skeleton (L1): authoring record

Authored and verified end to end. The code step stands up the
game-state machine as the game layer's first file pair (`game.h` /
`game.cpp`, design D2) and wires `main.cpp`'s loop to it; the Part 4
play behaviour (the slice) stays where it is and runs under the machine.

### The design decision this lesson settled

- **Where the game layer starts.** `game.h` / `game.cpp` hold the whole
  machine — `GameState`, `Game`, `GameInput`, `GameScale`,
  `GameDrawPanel` — and the loop keeps the platform/asset/audio/frame
  scaffold. The play gameplay (hero intent → walk → camera → score) is
  *not* moved into the machine yet: it stays in `Run`, gated by the
  state's scale (the walk runs every frame but advances zero outside
  play). Extracting the play loop into `game.*` is structural debt that
  L16's refactor can take; the skeleton's job is the state machine, not
  a file shuffle.
- **Two named conditions have no gameplay yet** (combat reduces health,
  lesson 087; waves complete the game, lesson 091). The lesson
  demonstrates both transitions with a documented **stand-in**, the same
  device lesson 078 used for the scale: Space is a hit on the hero (the
  defeat condition is the spec's exact "health reaches zero"), and Enter
  in play is the game being complete. Both are **keyed** — they fire on
  a key press and never on their own during a gameplay test — marked as
  scaffolding in `game.cpp`, and replaced when the real triggers arrive.
  The *transitions* are the game's own and do not change.
- **The demo scaffolding is reclaimed.** The slice's wall-time scale
  script and the Space-shake demo are gone: the scale is the state's now
  (play = full, else 0), and Space is the hit. The camera's additive
  offset is left at exactly zero — the screenshake hook, waiting for
  lesson 092.

### What the runs verified (headless, Xvfb `:99`, scripted input)

Two runs cover the five states and every named transition:

- **Run A** (title → play → pause → play → death → title): arrows move
  the hero in play; Escape pauses and resumes; Space ×3 lowers the
  hero's health to zero → death; Enter returns to the title. The
  transition log names each one with the spec's reason.
- **Run B** (title → play → victory): the completion key (Enter) is
  pressed in play → the game is complete → victory.

Transition log (Run A and B, real output):

```
engine: game: 5 states, starting on title
engine: state title -> play (the player started)
engine: state play -> pause (the player paused)
engine: state pause -> play (the player resumed)
engine: hero takes a hit — health 2 / 1 / 0 (t=…)
engine: state play -> death (the hero's health reached zero)
engine: state death -> title (the player returned to the title)
engine: the waves are complete (t=0.644)
engine: state play -> victory (the game's waves are complete)
```

**Each state's screen, and the simulation standing still outside
play** — the frame rows distinguish the screens and show the freeze
(title panel, a play frame, pause panel):

```
frame 1: step 0.000 ms, update 0.001 ms (entities 0.000), … render 0.859 ms (sprites 0.000, text 0.008, tilemap 0.000), …   <- title panel
frame 3: step 20.211 ms, update 0.002 ms (entities 0.002), … render 1.391 ms (sprites 0.002, text 0.008, tilemap 0.941), …   <- play scene
frame 4: step 0.000 ms, update 0.004 ms (entities 0.002), … render 0.472 ms (sprites 0.000, text 0.006, tilemap 0.000), …   <- pause panel
```

- `step 0.000` outside play (world advances nothing), a real step in
  play.
- `update` costs real time even on paused frames (0.004 ms) — the update
  *runs*; only the step is zero. That is the freeze living in the scale,
  not in a skipped update (lesson 079's contract: the record is not
  scaled).
- `tilemap 0.941` in play (the world is drawn) vs `tilemap 0.000` on the
  panels (only `text` draws — the state's own screen). The clear is
  attributed to the render phase, not to `text`, so the sub-phase rows
  stay honest.

The build is warning-free (18 sources) and the boundary check clean
(`game.*` names no OS). The page renders (`mdbook build`) and
`openspec validate --all` passes. The two exercises (extend-the-code:
the score on the end screens + Escape from the end states; explain-in-
prose: why the scale) each ship a real patch generated against the
lesson's end state and verified to compile, plus a walkthrough quoting
real runs.

## Lesson-083 — the tilemap and camera (L2): authoring record

Authored and verified. The code step reassembles the map and camera
**as the game's** (design D2): `Game` grows a `Camera` (its world-view),
the camera's follow moves into `GameFollow` (base follows the hero,
clamped to the map's bounds), and the world's draw moves into
`GameDrawMap` / `GameDrawSprites` (through the game's camera, split so
the frame record keeps timing the two sub-phases). `main.cpp` drops its
own `Camera`, the inline follow, and the inline world draw.

### What the runs verified (headless, Xvfb `:99`, scripted input)

The map draws (the `tilemap` sub-phase costs real time in play) and the
camera follows and clamps to the map's bounds. The map is `768×512` px,
the frame `640×480`, so the view's origin sits in `x ∈ [0,128]`,
`y ∈ [0,32]`. Real output — the hero walked right, then back left:

```
engine: hero at 389,232 (t=3.792)
engine: camera base 77,0              <- following: 389+8-320 = 77
engine: hero at 290,232 (t=9.224)
engine: camera base 0,0               <- clamped left: 290+8-320 = -22 -> 0
```

and a run that reached the far right:

```
engine: hero at 441,235 (t=3.562)
engine: camera base 128,3             <- clamped right: 441+8-320 = 129 -> 128
```

Follow (the base tracks `hero + half sprite − half frame`) and clamp
(base held at `0` and `128`, the map's bounds). Build warning-free,
boundary clean (the game's world code names no OS), the page renders.

### The completion stand-in: fixed to a keyed trigger

Lesson-082 first shipped the completion stand-in as "surviving unharmed
~3 s is the game being complete." It **auto-fired during any play longer
than ~3 s with the hero untouched** — it pulled lesson-083's camera runs
into the victory state mid-test. Because the fix changes lesson-082's
code, it was applied as a class-2 revision (conventions §3): the fix,
rebase-forward of the downstream lesson, and re-tag. The completion
stand-in is now **`ENTER` in play** — a keyed trigger, like `SPACE` is a
hit — so it never fires on its own during a gameplay test. Lesson-082's
prose and code-step diff were updated to match, and its Run B re-quoted
from a real run of the fixed machine (`the waves are complete
(t=0.644)`). Both `lesson-082` and `lesson-083` tags were moved to the
revised commits; the tag-to-tag `src/` diffs still equal each lesson's
code step. Run A (the defeat path: `SPACE` ×3 → death) is unaffected by
the fix — the completion stand-in is the only changed code, and Run A
never triggers it — so its transcript stands.

## Lesson-083 exercises

- **ex1 (extend-the-code) — the camera that leads.** `GAME_LOOKAHEAD`
  (48 px) shifts the base in the hero's movement direction: the camera
  leads on the move and re-centers at rest. Real run: moving right at
  hero `389`, base reads `125` (`77 + 48`) instead of `77`; at rest it
  falls back to `77`.
- **ex2 (predict-the-output) — the base at the corners.** A probe prints
  the wanted vs clamped base. All four map corners clamp on *both* axes
  (the map barely scrolls — only 128×32 px of camera travel), and only a
  hero in the central 128×32 box is unclamped. Real clamp line: `camera
  base 2,0 (wanted 2,-1)` — the free base went one pixel past the top
  edge and the clamp held it at 0.

## Lesson-084 — tile collision (L3): authoring record

The collision query (`TileRectSolid`) and the mover (`MoveEntity`) were
built in lessons 055 and 077. The code step makes the **movement
resolution the game's**: `GameWalk` (game.cpp) turns every live entity's
movement request into motion through the mover — one axis at a time, the
slide — and `main.cpp` drops its inline walk loop for it. A non-hero
entity is given a walk (down-right) at spawn so an entity is resolved
against the map here too; it is a stand-in for the AI lesson 089 brings.

### What the runs verified (headless, Xvfb `:99`, scripted input)

Real output — the hero driven up-and-left:

```
engine: hero at 312,232
engine: hero at 210,232 (t=3.447)      <- x moved 312->210, y frozen at 232: slid along a wall
engine: hero blocked at 206,228 (t=6.477)   <- both axes refused: stopped at solid tiles
```

Stop (both axes refused → the entity does not enter the wall) and slide
(one axis refused, the other free → motion along the wall) — both the
one-axis rule in `MoveEntity`. Build warning-free (18 sources), boundary
clean, the page renders.

### Note: the frame-rate overshoot

The authoring machine's loop runs at roughly a frame a second under the
headless check (event-driven pacing), so `dt` is large and `speed × dt`
is a big step — the hero can overshoot a wall in one step and get wedged.
`MoveEntity` correctly refuses the step into solid (so it never tunnels
in these runs), but the jerkiness is the big step, not the collision. It
is exactly the failure mode exercise 2 targets (sub-stepping so a fast
hero cannot cross a thin wall in one step) and is worth watching as
later lessons drive more entities. The engine's frame pacing itself is a
separate concern (the MVD's 60 fps line, checked at the finale).

### Lesson-084 exercises

- **ex1 (predict-the-output) — the corner and the wall.** A probe prints
  which axis moved (`x moved/stopped, y moved/stopped`): a wall is one
  axis refused (slide), a corner is both (stop). x resolves first
  (against the original y), so x wins in a squeeze.
- **ex2 (extend-the-code) — no tunneling.** Sub-step the mover (≤4 px
  each, one-axis rule kept per sub-step) so a long frame cannot carry a
  fast entity across a thin wall. `MAX_STEP` must be under a tile.

## Lesson-085 — hero movement (L4): authoring record

New behavior (the `hero-movement` delta). `hero.h`/`hero.cpp` are the
hero's own (design D2): `HeroMove` reads the held direction, normalizes
it (a diagonal scaled by 1/√2 → the straight-line speed), and eases the
hero's velocity toward the intent (accel) or toward rest (decel) —
`move += (intent − move) × dt/HERO_TIME`, frame-rate-independent.

### What the runs verified (headless, Xvfb `:99`, scripted input)

Real output — the hero's velocity report:

```
engine: hero velocity 240,0 (t=3.348)     <- full speed straight, from rest
engine: hero velocity 0,0 (t=5.864)       <- released, eased to rest
engine: hero velocity 231,20 (t=3.362)    <- one step of the ease, turning
engine: hero velocity 169,169 (t=5.377)   <- the diagonal, settled
```

- **Diagonal = straight-line speed:** `169,169` has magnitude
  `√(169²+169²) ≈ 239` ≈ the straight `240` — not `√2 × 240`. The
  normalization works.
- **Accel/decel from/to rest:** the velocity leaves `0` and returns to
  `0` (easing, `231,20` is one step), never a step change.

### Note: the ease is sampled coarsely at the headless ~1 fps

The headless loop is event-driven (~1 fps; see the lesson-084 note), so
`dt ≫ HERO_TIME`, `k` clamps to 1, and the ease completes *within* a
frame — the smooth "rise over several frames" is what appears at 60 fps
(where `dt/HERO_TIME` spreads it over ~7 frames). The ease is `dt`-scaled
and frame-rate-independent either way. Also: X auto-repeat makes a held
key flicker (KeyPress/KeyRelease pairs), which muddies the intent in a
scripted run — turn it off (`xset r off`) for a clean hold. The velocity
report (a diagnostic added in this lesson's step) is what makes the ease
measurable.

### Lesson-085 exercises

- **ex1 (predict-the-output) — the curve, predicted.** The ease is
  geometric (`move` closes `dt/HERO_TIME` of the gap per frame): 14%,
  26%, 36% of full speed after 1, 2, 3 frames at 60 fps with
  `HERO_TIME=0.12`; half that with `0.24`. Not full in three frames.
- **ex2 (extend-the-code) — heavier to stop.** Split `HERO_TIME` into
  `HERO_ACCEL`/`HERO_DECEL`, chosen by whether the intent is faster or
  slower than the current velocity (magnitude test). Diagonal
  normalization and frame-rate independence kept.

## Lesson-086 — feedback and animation (L5): authoring record

New behavior (the `game-feel` toolkit's hooks + sprite animation).

- **Sprite animation:** the hero's art is now a sprite sheet
  (`assets/hero.ppm`, two 16×16 walk frames). The frame advances while
  the hero moves (`frame += 1` per `ANIM_STEP`, wrapping) and rests at 0.
  An entity collides as **one frame** (`ANIM_FRAME_W`), not the sheet —
  `MoveEntity`'s box is now `ANIM_FRAME_W × height`. `BlitSpriteFrame`
  draws one frame of a sheet.
- **The feedback hooks** (`feel.h`/`feel.cpp`): `FeelShake` (the camera's
  additive offset moves and rests at exactly zero) and `FeelHitstop` (the
  game-time scale drops to a fraction and returns to full on its own
  **wall-time** deadline — lesson 078's clock). The scale is the state's
  × the hitstop's factor. **Decision (the 086/092 split):** 086 ships the
  hooks' fire-and-rest *mechanisms*; lesson 092 is the toolkit firing
  them from the game's events ("feedback starts with the event") and 093
  adds the burst + easing. A wall-time demonstration fires both once here
  so the fire-and-rest is visible; 092 replaces the script with events.
- **083's placeholder removed:** `GameFollow` no longer pins the camera's
  additive offset to zero — the feedback (FeelUpdate) drives and rests it
  now. (A tiny change to 083's code in this step; noted as the hooks
  taking over the juice offset.)

### What the runs verified (headless, Xvfb `:99`, scripted input)

```
engine: feel: shake fired (6 px, 0.5s), hitstop fired (0.25x, 0.4s)
engine: feel: hitstop rested — full speed again
engine: feel: shake rested at 0,0
engine: hero frame 1 (t=3.348)
engine: hero frame 0 (t=7.863)
```

- **The hooks fire and rest** — the shake rests at exactly `0,0`; the
  hitstop returns to full speed on its own deadline.
- **The animation advances** — the walk-cycle frame leaves 0 and steps
  while the hero moves, resting at 0 at rest.

### Lesson-086 exercises

- **ex1 (extend-the-code) — feedback starts with the event.** The hit
  handler fires a short hitstop + shake in the hit's own frame
  (`GameInput` gains `Feedback &feel`); the hooks' fire-and-rest is
  unchanged.
- **ex2 (explain-in-prose) — why wall-time.** The hooks' countdowns run
  on the wall clock: on game time a hitstop would be slowed by its own
  factor (and frozen forever under a pause, scale 0). The probe prints
  the wall seconds left beside the scale factor.

## Lesson-087 — projectiles and two weapons (L6): authoring record

New behavior (the `combat` delta) and the format growth (D3). The code
step grows the table format by **eight named columns in one step**
(`accel`, `damage`, `rate`, `fires`, `range`, `behavior`, `wave`,
`count`), stands combat up in its own file pair (`combat.*`, design
D2), and puts weapons on rows and projectiles in the store exactly as
D5 prescribes. `assets/entities.txt` is not touched.

### The design decisions this lesson settled

- **The format grows once, by every column the batch needs.** D3's
  "the lesson that grows the format" is this one: `behavior` is used
  here (`fly`) and by 088-090's enemy rows; `wave`/`count` are carried
  by 088's rows and read by 091's waves. Growing them now keeps one
  growth, one loader, one defaults contract — the later lessons grow
  files, not formats.
- **`rate` is rounds per minute** (an int, like every number the
  format reads): shots-per-second could not express a slow weapon at
  all (the smallest non-zero rate would be one shot a second), and RPM
  is what a weapon's rate means anyway — the blaster's `300` is a shot
  every fifth of a second, the cannon's `60` one a second.
- **A weapon row carries `damage` and `rate` and names `fires`** (D5
  verbatim); a projectile row carries `range` — its life, in world
  pixels (D5's "its range's end"). The fired projectile carries the
  shooter's damage into the hit: "a hit reduces the target's health by
  the row's damage" is the firing row's damage, measured.
- **The hero's feel is data via the default.** `hero.h` promised the
  feel becomes data when the format grows named columns; the hero's row
  predates the column and keeps loading byte-for-byte, so its `accel`
  is the format's default — 120 ms, exactly lesson 085's constant. A
  row that wants another weight names it.
- **The aim is the eight compass points of the hero's motion** (its
  facing at rest) — no square root runs at run time: the diagonal is
  `AIM_DIAG`, the same precomputed 1/√2 lesson 085 uses. The
  consequence is honest and was seen in the runs: a shot flies one of
  eight lanes, and a shot fired point-blank lands — which is why the
  flight's hit check runs at the position the shot is *fired* at, not
  only after its first step.
- **The hit rule: a shot hits any live actor except its owner — and
  flies through other shots.** The owner exclusion is by pointer into
  the store's fixed slots (slots never move, so the identity is safe
  and cheap). The "flies through other shots" clause was added after
  the scatter exercise's run showed sibling pellets destroying each
  other (`hit: bolt hits bolt`): crossing fire must not cancel in
  mid-air, and a projectile has no health to spend.
- **A zero-health entity is retired — the hero excepted.** The hero's
  zero health is the game's defeat condition (the state machine reads
  it); retiring the game's actor would leave a fresh game with no hero
  to move. Verified by hitting the hero after a restart: `health 3 ->
  1` — it is still live (the hit check only sees live entities).
- **`ENTITY_NO_ART` (exercise 2's fix surfaced the hole).** A
  definition with no image became an entity with `sprite = 0` and the
  flight's first read of the sprite's size segfaulted. The exercise
  fixes it typed at `EntityCreate`; it is not folded into the main line
  (it is the exercise's work), and the hole is real and reproducible at
  the lesson's end state.

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **`assets/entities.txt` loads byte-for-byte.** This lesson does not
  touch the file (`git diff lesson-086 lesson-087 -- assets/entities.txt`
  is empty) and the run prints its two rows as lesson 071 wrote them,
  the unnamed fields at the defaults:
  `def hero: x 312 y 232 facing 0 speed 240 health 3 sprite
  assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior
  none wave 0 count 1`.
- **The refusal edge, one notch relaxed** (a scratch copy of the run):
  a header naming `sprit` → `could not load (malformed)`; a header
  naming only `name x y speed health sprite` → loads, `facing 0`,
  `accel 120`, `behavior none` in the printed row; a row one value
  short → `could not load (malformed)`.
- **Each weapon fires its row's projectile kind** (real output):
  `arm: hero arms blaster (damage 1, rate 300, fires bolt)` →
  `fire: hero -> bolt (damage 1, range 160)`; `arm: hero arms cannon
  (damage 2, rate 60, fires shell)` → `fire: hero -> shell (damage 2,
  range 400)`.
- **The three retirements** (real output): `shot bolt retired — wall`
  (a pillar met mid-flight), `shot bolt retired — range` (its 160 px
  spent over open ground), `shot bolt retired — hit slime` and `shot
  shell retired — hit hero`.
- **A hit reduces health by the row's damage** (real output): the
  blaster's `damage 1` took the slime `health 1 -> 0`; the cannon's
  `damage 2` took the hero `health 3 -> 1` and `1 -> 0` → `state play
  -> death (the hero's health reached zero)`, then `state death ->
  title`, `state title -> play`, and `hit: shell hits hero — damage 2,
  health 3 -> 1` again — the fresh game restored the hero and it is
  still live. `slime retired — zero health` shows the retirement rule.

### Note: the verification method on this machine (updated)

`xset` is no longer installed on this machine (the notes' earlier
"turn auto-repeat off" step is unavailable). The runs instead rely on
the platform layer's own detectable auto-repeat (`XkbSetDetectable
AutoRepeat`, lesson 032's code) so a held key stays held, and they
pace frames by sending window-move events (`xdotool windowmove`) at
about 25 a second — the loop wakes on news, and a window move is news.
That gives `dt ≈ 40 ms` per frame in the scripted runs (much closer to
a real frame than the ~1 s steps of the earlier sessions) and it is
what the numbers above were measured at. A small one-off X tool
(`XAutoRepeatOff/On`) and the frame-pacing helper live in the author's
scratch space, not in the repo.

### Lesson-087 exercises

- **ex1 (extend-the-code) — the scatter shot.** A third weapon row
  firing a three-shot spread (`scatter 1 120 bolt 3`) via a new named
  column `burst` (default 1) grown exactly the lesson's way; the spread
  is the aim plus the aim turned ±45° — a turn of a unit direction
  preserves its length, so every pellet flies at the shot's own speed.
  Real run: one pull, three `fire` lines, and three different fates
  (`wall`, `wall`, `range` — the fan is real).
- **ex2 (fix-the-crash) — the projectile with no art.** A projectile
  file naming no `sprite` column loads, and firing its kind segfaults
  (real: `Segmentation fault (core dumped)`) at the flight's first read
  of the sprite's size. The fix refuses it typed at `EntityCreate`
  (`ENTITY_NO_ART`) and the run says `fire refused — the kind has no
  art`; the weapon rows (no art, never entities) are unaffected.

## Lesson-088 — enemy archetype tables (L7): authoring record

New data, and the smallest code step of the batch: `assets/enemies.txt`
(four rows — bat, wisp, spitter, golem — the three types and the boss),
four 16×16 magenta-keyed sprites, `Entity` carrying the row's last two
facts (`wave`, `count`) like all the others, and `main.cpp` loading the
roster's table and spawning one entity per row with a print that
mirrors the definition's. Nothing else moves.

### The design decisions this lesson settled

- **The roster carries the facts the later lessons act on.** The rows
  state `damage`/`rate`/`fires` (their attacks — lesson 090 fires
  them) and `behavior` (lesson 089 acts on it) and `wave`/`count`
  (lesson 091's waves) now, so "every enemy carries its row's values"
  is literally true from this lesson on: the entity carries every value
  its row states, and the run prints the definition and the entity in
  the same words for an eye-checkable comparison.
- **The wave column's semantics (settled here, spent in 091).** A
  kind's `wave` is the wave it *joins* — it spawns in that wave and
  every wave after it — so the final wave is where the three types and
  the boss stand together (bat/wisp join at 1, spitter at 2, golem at
  3). `count` is how many of the kind each of its waves spawns. 087's
  column doc is one sentence short of this and 091's step extends the
  comment.
- **Enemy art is one colour and a pair of eyes per kind** (four small
  PPMs) — enough for the roster to be told apart on screen without
  turning the lesson into an art exercise. The kinds may also share art
  (the exercise's swarmling wears the wisp's colours): art is a row's
  value like any other.

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **Every enemy carries its row's values.** Real output, definition
  beside entity (the sprite prints as dimensions — the entity carries
  the row's art *loaded*):
  `def bat: x 560 y 72 facing 2 speed 160 health 2 sprite
  assets/bat.ppm accel 120 damage 1 rate 60 fires bolt range 0 behavior
  chase wave 1 count 2` / `entity bat: x 560 y 72 facing 2 speed 160
  health 2 sprite 16x16 accel 120 damage 1 rate 60 fires bolt range 0
  behavior chase wave 1 count 2` — and the same for wisp, spitter,
  golem. `roster: 4 enemies from the table's rows, live 6 of 64`.
- **No per-type copy of the attributes in code.** The spawn is one
  loop over the table's rows; a scratch copy with a fifth row
  (`swarmling …`) grows the roster to 5 and spawns it carrying its
  values (`roster: 5 enemies … live 7 of 64`) with `git diff --stat
  src/` empty. The only per-name lookups in the code are the game's
  own asks (the hero's row; the weapons' rows) and the typed-failure
  demonstration (`"dragon" -> unknown`).

### Lesson-088 exercises

- **ex1 (extend-the-code) — the kinds nobody made yet.** Three kinds as
  rows only (a `count 6` swarm, a 240-speed one-health sprinter, a
  12-health tank) with reused art: the run spawns all three carrying
  their values and `git diff --stat src/` is empty. Real run quoted:
  `entity swarm … count 6`, `entity sprinter … speed 240 health 1`,
  `entity tank … health 12 damage 3`, `roster: 7 enemies … live 9 of
  64`.
- **ex2 (explain-in-prose) — why the boss does not deserve code.** A
  probe prints the walk's per-entity visits (one shape, six kinds
  through it: `walk visit: golem (behavior boss)`); the walkthrough
  argues the 075 rule, the wave scale, and the data protection — and
  steelmans the boss: what it needs that no row carries is a
  *schedule*, which lesson 090 gives it without its own movement
  machinery.

## Lesson-089 — enemy AI (L8): authoring record

New behavior (the `enemies` delta's AI, design D6): `ai.h`/`ai.cpp`
with three small functions writing an entity's movement request, the
walk's one switch on the row's behavior, lesson 084's stand-in walk
gone, and the run reporting the world's motion (each entity as it
travels a tile, its distance to the hero beside it) and where it ended.

### The design decisions this lesson settled

- **A behavior writes a compass point, not an arbitrary vector** — D6's
  "the way the player's input writes the hero's" taken literally: the
  hero's intent is one of eight directions and so is every request the
  AI writes. No square root runs in the behaviors (the distances are
  compared squared); `CombatAim`'s compass point is shared by the aim
  and the movement — the world moves in eight lanes.
- **`AI_KEEP` is the behavior's fact, not any kind's** — every keeper
  keeps 160 px, and the band (±8) keeps a keeper at its distance from
  twitching across the line. (Exercise 1 makes it a row's fact; the
  main line keeps the constant so the lesson's step stays about the
  behaviors.)
- **The behaviors write requests and nothing else** — no firing, no
  health, no knowledge of rows or the store. The enemy rows' weapons
  stay carried until lesson 090 fires them.
- **Lesson 084's stand-in walk is removed here** (the comment always
  said it stood in for this lesson's AI). The slime's row says
  `behavior none`, so it stands; the `G` spit stand-in is unaffected.
- **Two honest quirks were measured and recorded, not hidden.** (1) The
  alignment wobble: when a delta component is near zero its sign flips
  between frames and the compass point flips with it — the chaser
  zigzags down its last leg. (2) The wedge: a compass-point chaser can
  trap itself at a wall corner (the run's bat held at a pillar's top,
  its y refused and its x oscillating inside the pillar's column). Both
  are the eight-lane model's honest limits; no pathfinding is in scope.

### What the runs verified (headless, Xvfb `:99`, scripted input)

One scripted run (the hero standing still) moves all three kinds at
once (real output):

- **Chase shrinks the distance:** the bat `177 → 149 → 126 → 101 → 77
  → 55 px of the hero`, closing down its diagonal lane.
- **Flee grows it:** the wisp `401 → 426 → 447 → 469 → 491 px` — and
  its `y` pinned at `479` while its `x` slides right (`659 → 685 → 710
  → 736`): the mover's slide rule, resolved against the bottom wall.
- **Keep holds it:** the spitter settles at `154 px` of the hero —
  inside the band (152-168) its behavior keeps — and stays there. (It
  opened at 111 px, too close, and backed off first: the behavior's
  correction visible in its first two reports.)
- **Through the mover:** every request is resolved by `MoveEntity` —
  the flee's wall slide above, and the chase's bat wedged at
  `311,176` against the pillar below the hero (its y step refused).
- **Per-entity work expressed once:** the walk's one switch on the
  row's behavior; the closing account names the five kinds and none of
  them appear in the code (`slime ends at 400,320` — `none`; `golem
  ends at 384,96` — `boss`, standing until lesson 090).

### Lesson-089 exercises

- **ex1 (extend-the-code) — the keeper's distance is data.** The `keep`
  named column (default `TABLE_KEEP_DEFAULT` = 160), carried like every
  value, `AiKeep` reading the entity's own number. Real run: `def hero:
  … keep 160 …` (entities.txt silent, the default), `spitter … keep
  120`, `warden … keep 240`; the closing account: `spitter ends at
  221,298 — 112 px`, `warden ends at 156,403 — 232 px` — two keepers at
  two distances.
- **ex2 (predict-the-output) — the chaser's staircase.** The prediction
  (one diagonal from (560,72), turning at (400,232) into a straight
  88-px leg) against the measured reality: the probe prints the turns
  and they are a frame-by-frame wobble at the alignment (`ai: bat
  steers -0.707,0.707 / ai: bat steers 0.707,0.707 / …`), the map
  bends the path (the mover resolves x first, the slide follows), and
  the bat ends wedged at the pillar — the eight-lane model's honest
  limit.

## Lesson-090 — the boss (L9): authoring record

New behavior (the `enemies` delta's boss, design D6) and the enemy
attacks. `AiBoss` is a schedule — per-entity state (`phase`,
`phase_t`) and timing — composing `AiChase`/`AiKeep`/`AiFlee`; the
walk's per-entity work gains the `boss` case and the attack line
(`CombatAttack`); lesson 087's enemy-fire stand-in and its `G` key are
gone.

### The design decisions this lesson settled

- **The pattern is state and timing, and nothing else.** The boss's
  own machinery is a phase index and an elapsed-time counter; every
  step it takes is a shared behavior's request through the same
  `MoveEntity`. This is the answer to lesson 088's steelman: the one
  thing a row cannot carry is a schedule, and the schedule costs two
  fields and a timer.
- **The schedule counts elapsed phase time, not a countdown.** The
  first implementation counted down from zero and advanced on
  `<= 0` — which made the pattern churn during the freeze (dt = 0) and
  skip its first phase. Counting *up* to the phase's length makes the
  schedule a function of game time alone: the freeze freezes it, and a
  new pattern starts on its first phase naturally.
- **The attack is one line of the per-entity work** — an armed entity
  (its row names a projectile kind) fires at the hero at its row's
  rate, while the hero is within its shot's reach (the projectile
  kind's `range` — the shot's own fact bounds the threat). The hero is
  exempt (its trigger is the player's); the slime's row names no weapon
  and it never fires.
- **The G stand-in died here, as its comment promised.** `KEY_G` left
  the seam (`platform.h`, `platform_x11.cpp`) and the slime's
  demonstration arming with it. "Combat reduces the hero's health" is
  now the enemy rows' own work.
- **The golem's rate was tuned to the fight this lesson demonstrates**
  (a shell every 3 s instead of 2) so one full pattern cycle is
  observable before the hero falls — an asset edit in the lesson's
  step, the data's job.

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **The boss composes the behaviors plus its pattern** (a scratch
  roster holding only the golem's row — the boss alone on the map,
  stated as such). Real output: `boss: golem's pattern -> keep (2 s)`
  while the distance backs off `48 → 71 → 95 → 119 → 144` (AiKeep to
  its 160), `-> flee (1 s)` while it grows `168 → 193 → 218` (AiFlee),
  `-> chase (3 s)` while it falls `193 → … → 26` (AiChase), then
  `-> keep` again — the cycle exact.
- **No separate movement machinery.** `AiBoss` contains a timer, a
  counter, and three calls to the shared behaviors — no movement math.
  The measured phases are those behaviors' own signatures (the keep's
  band, the flee's growth, the chase's close).
- **The enemy attacks are real** (the stand-in's replacement): `hit:
  shell hits hero — damage 2, health 3 -> 1`, `hit: shell hits hero —
  damage 2, health 1 -> 0`, `state play -> death (the hero's health
  reached zero)` — the golem's row's damage, from the boss's own fire,
  with no key held down.

### Lesson-090 exercises

- **ex1 (extend-the-code) — the pattern owns its attacks.** The walk's
  generic attack line skips bosses; `AiBoss` fires in its `keep` phase
  only. Real run: every `fire` line falls inside a `keep` phase and
  nowhere else (`keep (2 s) → fire → hit … damage 2, health 3 -> 1`,
  `chase`/`flee` silent, `keep (2 s) → fire → hit … health 1 -> 0`).
- **ex2 (predict-the-output) — the schedule's timeline.** The 3/2/1 s
  cycle predicted as a table (keep at t≈3, flee at t≈5, chase at t≈6,
  keep at t≈9 …) against the probe's game-clock stamps: `keep (2 s) at
  t=3.0`, `flee (1 s) at t=5.0`, `chase (3 s) at t=6.1`, `keep (2 s) at
  t=9.1` — the prediction matched, jittered by the frame each change
  lands in. The walkthrough also defends the elapsed-time schedule
  against the countdown bug the first implementation had.

## Lesson-091 — waves (L10): authoring record

New behavior (the `enemies` delta's waves). `GameWaves` (game.cpp) is
the fight's shape: a fresh game's clear, the composition from the
table's rows, the advance on the last retirement, and the completion
that spends `waves_remaining`. Lesson 088's standing roster spawn
gives way to the waves; the last stand-in (ENTER's completion key)
dies; the hit rule grows one clause (a shot flies through its own
kind).

### The design decisions this lesson settled

- **The wave composition is the rows' `wave`/`count`, and a kind joins
  at its wave and every wave after it.** So each stage is thicker than
  the last, and the final wave is the three types and the boss together
  — the phrase the batch's plan used, realized literally. (The
  alternative — a kind spawns in its one wave only — made the final
  wave a solo boss, which is not what "bring together" means.)
- **The wave's state is the store's**: the wave is being fought while
  any fighter (behavior chase/keep/flee/boss) is live. No kill counter
  to keep in step with the retirements; the store is the truth. The
  hero (none), the scenery (none), and the shots (fly) are not
  fighters, so a stray shot or the standing slime never advances a
  wave.
- **The copies stand in a line beside their row's spot** (`x + n ×
  ANIM_FRAME_W`) — the simplest honest spread; a copy landing in a wall
  is the mover's problem to solve when the wave fights, not the
  spawn's.
- **A shot flies through its own kind** (the hit rule's new clause):
  without it the wave's copies shot each other down — the run that
  found it printed `hit: bolt hits bat` from one bat to another. A
  shot's targets are the other kinds (the hero's shots hit the enemies;
  the enemies' shots hit the hero). Mixed-kind cross-fire remains
  possible and is cosmetic; the owner-only exclusion is not enough for
  a game that spawns copies.
- **The fresh fight clears the store of fighters and shots** (behavior
  not `none`): the hero is the game's actor and the scenery is the
  world's; everything the last game brought is retired before wave 1
  spawns again.
- **The wave column's doc was extended** in this step ("the wave this
  kind joins — it spawns in that wave and every wave after"), as
  promised at the design when the column grew.

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **A wave spawns its composition from the table's definitions** (the
  real roster, the hero fighting it): `wave 1 begins — 2 enemies`
  (bat ×2), and on its clear `wave 2 begins — 5 enemies` — bat ×2,
  wisp ×1, spitter ×2, each spawn line naming the kind, its spot, and
  the values its row carries. Every count is the rows' `count`; every
  kind is the rows' `wave` answered.
- **The next wave begins when the last entity of the current one is
  retired** — `bat retired — zero health`, `bat retired — zero health`,
  `wave 1 cleared — the next begins`, in that order and in the same
  breath.
- **The waves bring the three types and the boss together, and they
  complete the game** (a scratch roster — the same waves with the
  enemies slow, fragile, unarmed, and spawned at the hero's feet; the
  wave mechanics without the hunt, stated as such in the lesson):
  `wave 3 begins — 6 enemies` (bat ×2, wisp, spitter ×2, golem), six
  `retired — zero health` lines, `the waves are complete (t=3.487)`,
  `state play -> victory (the game's waves are complete)` — the named
  condition's real trigger, and **no stand-ins remain** in the machine.

### Lesson-091 exercises

- **ex1 (extend-the-code) — the breath between waves.** A game-time
  intermission (`wave_wait`, `WAVE_BREATH_S`, `GameWaves` gaining the
  frame's dt). Real run: `wave 1 cleared — wave 2 incoming (t=0.333)`
  → `wave 2 begins — 5 enemies (t=2.662)` — the two-second breath plus
  the frame it lands in. The walkthrough defends game time against wall
  time (a breath that waits while paused) and locates it beside
  lesson 086's wall-time hooks.
- **ex2 (predict-the-output) — the wave plan.** Predicted from the
  rows' `wave`/`count` alone: wave 1 = 2, wave 2 = 5, wave 3 = 6 (the
  three types + the boss). The probe prints the plan from the table
  (`plan: wave 3 brings … golem x1 / wave 3 total 6`) and the runs
  spawn exactly those; moving one row's `wave` value inverts the
  contest — the composition is data.

### Batch 3 (lessons 087-091) — closing note

All five lessons are authored, verified on real runs, co-committed,
and tagged (`lesson-087`…`lesson-091`); each tag-to-tag `src/` diff
equals that lesson's code step alone. The combat/enemies batch lands
the `combat` and `enemies` deltas of the audit (§1.2) on the grown
table format (D3), with weapons as rows and projectiles as entities
(D5) and the AI over the mover (D6) — and the state machine's five
named conditions are now all driven by real gameplay (no stand-ins).

## Lesson-092 — hitstop and screenshake (L11): authoring record

New wiring on 086's hooks (the `game-feel` delta's first half). The code
step fires `FeelHitstop`/`FeelShake` from the game's own events — a hit
lands, a death falls — at the lines where they happen in the flight
(`CombatFly`), removes the wall-time demonstration script from
`main.cpp`, and moves the toolkit's per-frame run (`FeelUpdate`) to
after the frame's events and before its draw. The hooks' fire-and-rest
mechanisms are untouched; the hooks now report their own firing beside
the event's lines (the demonstration's print died with the
demonstration).

### The design decisions this lesson settled

- **The event fires the hook at the event's own line.** `Feedback
  &feel` is threaded through `GameWalk` into `CombatFly` and the calls
  sit in the hit branch — the alternative (collect events, fire in the
  loop) keeps the causality in bookkeeping; here "feedback starts with
  the event" is literal: the fire is on the hit's line of the flight.
- **The weights are the event's.** A hit: hitstop `0.25x / 0.15s` +
  shake `5 px / 0.25s`. A death: `0.25x / 0.30s` + `10 px / 0.50s`. A
  killing blow answers as both and the death's weights win — firing is
  overwrite semantics (the hooks are re-armed, never stacked), so the
  heaviest event of the frame is what the player feels.
- **The frame order is half the rule.** The toolkit settles after the
  walk (which fired it) and before the render (which shows it), so a
  hit's shake is in the hit's own frame's picture. Honest limit
  recorded in the prose: the frame's *step* is already spent when the
  hit lands — the first slowed step is the next frame's; a step cannot
  shrink retroactively. What the event's frame carries is the fire, the
  shaken draw, and the factor already down.
- **The deadline answers at the frame's granularity, and that is
  measured, not assumed.** `FeelUpdate` settles a frame once, after its
  events, subtracting the frame's whole wall step — so a 0.15 s
  hitstop rested after 124 ms of wall time on this run's ~44 ms paced
  frames (one frame early, never late; ≤16 ms at 60 fps). 086's
  mechanism is not touched for this.

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **Feedback starts with the event** (run A, the real roster — the hero
  holding still while wave 1's bats close in): two bolts land in one
  frame and every `fired` line sits between frame 101's account and
  frame 102's — no `frame` line between a hit and its feedback:
  `hit: bolt hits hero — damage 1, health 3 -> 2` → `feel: hitstop
  fired (0.25x, 0.15s)` → `feel: shake fired (5 px, 0.25s)` → …
- **Hitstop slows to a fraction and returns on its own wall-time
  deadline**: frames 103-105 at `step 10.779 / 11.001 / 10.808 ms`
  against the paced `43 ms` — the fired quarter — with `feel: hitstop
  rested — full speed again` before frame 106 (`step 43.493 ms`).
  Wall-stamped fire→rest: `32.265 → 32.389` = 124 ms for the 0.15 s
  asked (frame granularity, above). Frame 105 is still slow although
  the rest prints before it: its step was computed at its start.
- **The shake's offset rests at exactly zero**: `feel: shake rested at
  0,0` in every run (wall `32.265 → 32.476` = 211 ms for the 0.25 s),
  and a scratch probe printing the offset the draw uses shows it moving
  — `camera add 10,0 / 10,0 / -10,0 …` (the death's 10 px, alternating)
  — then the rest at `0,0`. (Probe, not shipped; stated as such.)
- **The two events, one frame** (run B, scratch roster — one fragile
  standing kind at the hero's feet, stated as such): `hit: bolt hits
  bag — health 1 -> 0` → hit's two firings → `bag retired — zero
  health` → death's two heavier firings, all in one frame's account.
  The hero's own death (run A, `health 1 -> 0`) answers the same way
  and `state play -> death` follows.
- **Honest edge, measured and recorded**: the volley's second bolt hits
  a hero already at zero (`health 0 -> 0`) and fires feedback again —
  the hit rule asks "is it live", the game's actor is never retired, and
  the defeat condition is read next frame. One frame of double
  feedback; not hidden.
- **The pause's product** (exercise 2's probe): `state pause, scale
  0.00 x hitstop 0.25 = 0.00 (hitstop 0.08s left)` with `step 0.000`
  frames, `hitstop rested` between two *paused* frames, the shake
  resting during the pause too, and the resume straight back to the
  paced `43 ms` steps.

### Note: the verification harness (updated)

Two gotchas this lesson hit, recorded so the next batch does not:
`DISPLAY` in the authoring shell is `:0` — every run and every xdotool
call must pin `DISPLAY=:99` or the game opens its window on `:0` while
the harness drives `:99` (the search then finds nothing and the run
hangs un-paced). And xdotool's key name is lowercase `space`
(`keydown Space` is "No such key name"). Frame pacing is still
`hold.sh`'s windowmove jiggle (~25/s); wall stamps come from wrapping
the run's stdout in a line-stamping reader (`stdbuf -oL` keeps printf
line-buffered through the pipe).

### Lesson-092 exercises

- **ex1 (extend-the-code) — the shake that settles.** `shake_total`
  records the shake's whole life and the drive scales the offset's
  magnitude by what is left. Real ramp (the death's 10 px): `-9, -8,
  -7, 6, 5, 4, 3, -3, -2, -1, 0` over the shake's half-second, then
  `shake rested at 0,0` — still one effect (screenshake), still resting
  at exactly zero. The first probe line reads `-9`, not `-10`: the
  fire's frame already settled once — the lesson's frame granularity
  again.
- **ex2 (predict-the-output) — the pause that meets the hitstop.** The
  prediction (the product is zero, the rest lands *during* the pause,
  the resume is clean) against the probe's measured product: `scale
  0.00 x hitstop 0.25 = 0.00`, `hitstop rested` between two `step
  0.000` frames, resume at the paced `43 ms` with nothing hanging over.
  The walkthrough defends the product over an if-else: zero times
  anything settles every edge for free.

## Lesson-093 — particle bursts and easing (L12): authoring record

The toolkit's last two effects (the `game-feel` delta's second half) and
the completion of the bounded four. Particles are entities from a table
kind (`assets/particles.txt`, one row: `spark … accel 400 range 64
settle`, art `assets/spark.ppm`); the burst fires from the hit and the
death in their own frame (like every feel effect); the settle is one
eased value per particle that arrives exactly at its row's range and
retires there. `feel.*` grows the ease set (`EaseInQuad`,
`EaseOutQuad`, `EaseInOutQuad`), `FeelBurst`, and `FeelParticle`; the
format does **not** grow — the spark rides the existing `accel` (the
settle's length in ms) and `range` (the travel budget) columns and one
new `behavior` spelling (`settle`).

### The design decisions this lesson settled

- **The cosmetic share is the policy that makes the task's acceptance
  sentence true.** Task 4.2 asks: "a full store drops particles while
  gameplay spawns are kept." With drop-on-full alone, a store full of
  sparks would refuse gameplay spawns too — the sentence would be
  false. So the store documents a **cosmetic share**
  (`FEEL_COSMETIC_SLOTS = ENTITY_CAP / 2` in `feel.h`): particles take
  those slots and no others, a burst that finds no cosmetic slot drops
  its particle (counted, reported), and the game's spawns are kept —
  they cannot be starved by cosmetics. 074's store is untouched: the
  store still never steals, and a gameplay spawn that meets a literally
  full store (cosmetics + gameplay to 64) is still the loud typed
  refusal — "gameplay work may not be dropped" means that refusal is a
  named bug-level event, never a silent loss. Rejected alternatives:
  the gameplay-yield (the game drops the oldest spark to serve a shot —
  viable, but it makes the yield a hidden eviction policy rather than a
  documented bound) and D8's already-rejected second narrow burst store.
- **The particle's life is the ease's duration.** "Move, settle, and
  retire": the distance out follows the ease and lands exactly on the
  row's `range` at `accel` ms — the life ends where it settles. No
  format growth (the 087 decision — "the later lessons grow files, not
  formats" — held).
- **The position is measured from the burst point, never accumulated.**
  The first implementation advanced `x += dir × Δtraveled` per frame;
  the accumulated rounding landed one landing a pixel short (`279,240`
  where the lane says `280`) beside an `(exact)` value line — measured,
  then fixed: the particle carries `from_x/from_y` and computes
  `from + dir × traveled` every frame, so the arrival is the target's
  own value and not a rounding of it. The eased value (`traveled`) was
  exact in both; the fix makes the position honest too.
- **Sparks are not targets.** The hit rule grows one skip: a shot flies
  through the cosmetic, exactly as it flies through other shots —
  without it, sparks would eat the player's fire.
- **The debris is cleared with the last fight.** A spark settles in
  game time; the burst that fires as the game *ends* would hang frozen
  over the end screens forever (the pause's freeze, pointed at the
  toolkit). The fresh-fight clear sweeps particles with the fighters and
  shots.
- **The eight lanes are data-free and reproducible.** The burst's
  spread is the world's compass points (a diagonal at 1/√2, the same
  lanes the aim and the movement use) — a burst is identical on every
  machine and every run, which is what makes the runs quotable.

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **A burst is bounded by the store's policy and its particles retire**
  (run A, the scratch killing blow): `burst: spark x4 at 320,232 — 4
  made, 0 dropped` (the hit) and `burst: spark x8 at 344,240 — 8 made,
  0 dropped` (the death), then twelve `spark settled at … — 64 px out,
  its row's range 64 (exact)` lines — move, settle, retire. The
  landings map the lanes exactly: the hit's four at `384,232` /
  `320,296` / `256,232` / `320,168` (the cardinals, 64 px out), the
  death's eight around the compass with the diagonals at `±45` px.
- **A full store drops particles while gameplay spawns are kept** (run
  C, a flood probe of 24 sparks a frame, stated as such): the share
  fills `24 + 8 = 32` and the report goes `8 made, 16 dropped` → `0
  made, 24 dropped` — while `fire: hero -> bolt` and `wave 2: spawns
  bag at 336,232` keep landing through the drops. Three gameplay spawns
  and three hero fires succeeded while bursts dropped everything; the
  drops are sparks only, counted per burst.
- **Eased values arrive exactly at their targets** (run B, a scratch
  probe at 17 digits): `ease at 0: 0/0/0`, `ease at 0.5: in 0.25, out
  0.75, inout 0.5`, `ease at 1: in 1, out 1, inout 1` — the set's ends
  clamped to exactness; and one spark's value `63.695704497446457 at t
  0.931` → `64 at t 1.000` (printed as `64`, not `63.99999999999999`),
  the run's own arrival line agreeing: `64 px out, its row's range 64
  (exact)`.
- **One data bug caught by the runs** (recorded because it is the
  method working): the burst lane table's north/south vectors were
  `0.707` instead of `1.0`, and a spark settled at `320,277` where the
  lane says `320,296` — a45-px cardinal in a 64-px lane. Fixed before
  the step shipped; the corrected run lands every cardinal at ±64 and
  every diagonal at ±45.

### Lesson-093 exercises

- **ex1 (extend-the-code) — the sparks inherit the blow.** `FeelBurst`
  gains the blow's direction; the lanes are picked by dot product (the
  spray carries the shot's way) while the death's `(0, 0)` stays
  radial. Real run (the shot flying east): the hit's four settle at
  `384,232` (east), `365,277` (south-east), `365,186` (north-east),
  `320,296` (the north/south tie) — three of four forward of the
  impact; the death's eight stay the full compass.
- **ex2 (predict-the-output) — the curve, predicted.** The prediction
  (0.75 of the travel at half the time → 48 px of 64; exactly 64 at the
  end) against the probe's curve: `48.857177534509155 at t 0.514`
  (the frame past halfway), `63.999110491731322 at t 0.996`, `64 at t
  1.000` — the last frame lands *on* the target. The walkthrough
  separates the two vocabularies the engine now names: eases that
  arrive, and the exponential approach that never does (weight).

## Lesson-094 — the HUD (L13): authoring record

New behavior (the `hud` delta, design D9) and the game layer's next
file pair: `hud.h`/`hud.cpp` — one function, `HudDraw`, reading the
game's own state and drawing the four readouts (`SCORE`/`HEALTH` left,
`WAVE`/`TIME` right) at screen coordinates. The score (the ground the
hero has walked) moves from the loop's local into `Game`, where the HUD
and the states can both read it; `main.cpp`'s play render draws through
the HUD and drops its inline text.

### The design decisions this lesson settled

- **The HUD reports what it read and where it drew it, on change.** A
  headless run cannot see pixels; the verification surface is the draw
  itself: `HudDraw` prints its four values and its anchor beside the
  camera's base whenever anything changes — so a state change and the
  readout showing it are provably in the same frame's account, and the
  anchor is provably constant while the camera moves. (A probe in
  exercise 2 adds the counterfactual coordinates.)
- **The score's meaning is the game's; its home is `Game`.** The
  shipped score is the ground the hero has covered (the value the
  slice's line has counted since lesson 080). The kill score — the
  thing lesson 091's page pointed at ("that is the HUD's lesson") —
  wants a `points` column, and the 087 design decision says the format
  grows ONCE ("the later lessons grow files, not formats"). Resolved:
  the main line keeps the format frozen and the kill score is exercise
  1 — the learner grows the column the lesson-087 way, with the
  compatibility rule re-verified. If the human review prefers the kill
  score in the main line, it is one column and one addition away.
- **The timer is the play clock as it is** — `play_clock` counts the
  wall seconds spent in play (it freezes with the pause and is not
  slowed by a hitstop). Whether a game's readout *should* slow with
  slow-motion is a design question, deliberately left open; the readout
  reflects the game's actual state either way.

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **The readouts reflect the game's actual state in the same frame**
  (run A, the real roster — the bats' volleys): `hit: bolt hits hero —
  damage 1, health 3 -> 2` → `hud: score 000000, health 2/3, …` — both
  before that frame's `frame 102` line; `health 2 -> 1` → `health 1/3`
  before frame 103 (whose `step 10.972 ms` is lesson 092's hitstop,
  answering the same frame's hit). The wave readout moves in the wave's
  own frame (`wave 1 begins — 2 enemies` → `hud: … wave 1/3 …` before
  frame 2). The restore: `health 0/3` in the frame the hero fell,
  `health 3/3` in the frame the fresh game restored him — the spec's
  "falls or is restored" scenario end to end.
- **The HUD stays in place over the world as the camera moves** (run B,
  the map walked end to end): the camera travels `8,0 → 13,0 → 20,0 →
  28,0 → 37,0 → 47,0 → … → 128,0` (its full range) while every line
  reads `at 8,8` — and the score ticks beside it (`000000 → 000328`),
  each tick in the frame the hero moved.
- **The exercises' runs**: ex1's kill score moves in the kill's own
  frame (`bag retired — zero health` → `hud: score 000500 …`) with
  `assets/entities.txt` byte-for-byte unchanged (its rows load `points
  0`, the default); ex2's probe measures the counterfactual — `camera
  128,0 would make them -120,8` — the readout's pixels past the frame's
  left edge if the HUD were scene.

### Note: one harness bug found and fixed (recorded)

A jiggle that moves the window to the *same* coordinates generates no
ConfigureNotify — the loop sleeps through it and the run degrades to a
few frames with giant steps (`step 11971 ms` in the first attempt).
Frame pacing must alternate positions (as `hold.sh` does). The runs
above are paced at ~25 fps with the fixed jiggle.

## Lesson-095 — audio integration (L14): authoring record

The game's sound as the game has it: `sound.h`/`sound.cpp` — the game
layer's use of the finished mixer (audio.* untouched) — the music
looping on the music channel and one effect per event fired on the
pool, from the same event lines the feel toolkit fires from. Three new
sfx assets (`shot.wav`, `hit.wav`, `death.wav` — small synthesized
tones); the demonstration files of lessons 061/066 (`tone.wav`,
`effect.wav`) stay on disk and leave the run, as 066 did the tone.

### The design decisions this lesson settled

- **The mix is the game's work; the device is the seam's.** The old
  loop mixed only inside `if (audio.output && …)` — on this machine the
  mix never ran at all. Now the stream is mixed on the engine's rate
  whether or not a device exists and only `SubmitSamples` is gated on
  one: a machine with no output runs the game's whole sound in silence
  and the reports still say what the channels and the stream carried.
  That is exactly what task 4.4's verification can measure here.
- **The events fire the sounds at their own lines** (shot in
  `CombatFire`, hit and death in `CombatFly`) — the same event sites as
  the feel toolkit, same frame. The demonstration rhythm (a timed
  effect every second) died here, completing the stand-in removals.
- **The volumes are the sounds' own** (shot quarter, hit and death
  half) so the sum stays inside the format while the fight gets loud —
  068's demo rule, now data of the sound module.

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **The effects reach the channels** (run A, the scratch killing blow):
  `fire: hero -> bolt` → `sound: shot -> channel 1 (volume 64 of 256)`;
  `hit: bolt hits bag` → `sound: hit -> channel 2` and `bag retired` →
  `sound: death -> channel 3` — all inside the event's frame, on the
  pool's channels 1–15, channel 0 the music's and never touched.
- **The music reaches the channel and the stream, byte for byte**: the
  music file's first frames (`0 277 554 831 1107 1381 1655 1927`) and
  the first mixed buffer's frames are the *same eight numbers*; the
  wrap report closes it (`music wrapped on channel 0 — wrap 1, 133035
  frames played, cursor 735 of 132300`).
- **The effects reach the stream, accounted for**: the first buffer
  carrying an effect reads `-8810 -8634 -8456 …` and every frame is the
  music's frame at that cursor (`-8810 -8889 -8961 …`) plus the shot's
  (`0 255 505 …`) at its quarter volume — verified arithmetically
  against the files (music cursor 735 = buffer 1).
- **The counts close the account**: `304 frames measured, 302 buffers
  of stream mixed (221970 frames), 12 effects fired, 1 music wraps` —
  all with `no audio output on this machine / continuing without sound`
  printed at startup. The frame record's `audio` phase now carries real
  work (0.03 ms, ~3% of the frame).
- **Honest limit, stated in the prose**: nothing audible is verified —
  this machine has no speakers; the stream's bytes say what the sound
  is, not how it sounds.

### Lesson-095 exercises

- **ex1 (extend-the-code) — the volume follows the distance.** A
  quadratic falloff in the squared distance (no square root per sound),
  the events carrying their offset from the hero. Real run (two armed
  scratch bags at 24 px and 150 px): `shot -> channel 1 (volume 63)` /
  `shot -> channel 2 (volume 42)` / the hero's own `volume 64` — the
  formula's numbers exactly (`64 × (1 − (150/256)²) = 42.0`).
- **ex2 (predict-the-output) — the pool under fire.** Predicted the
  twenty-sound frame: channels 1–15, then the stealing from the oldest
  (1, 2, 3, 4, 5). The run matches exactly; the walkthrough ties the
  oldest-stealing pool to 074's never-stealing store as the deliberate
  contrast (a dropped sound is inaudible; a dropped enemy is a bug).

## Lesson-096 — screen polish (L15): authoring record

The assembly's last lesson: the four screens in final form. The title
teaches the controls; the pause and the end screens carry the run's
final numbers (the game's own score, play clock, and wave — the same
values the HUD reads); every screen names the input it acts on. The
toolkit's easing finds its application at last — the screens' fades
(D8's example): the backdrop eases from black to the screen's own
color (the death reddens at 56,16,16, the victory greens at 16,48,16)
and arrives exactly.

### The design decisions this lesson settled

- **The screen reports what it draws.** A headless run cannot see a
  layout; `GameDrawPanel` prints its content the first frame it draws
  (`screen: death: "GAME OVER" / "SCORE 000424 …" / "ENTER: TITLE"`),
  so "each screen renders" has a measurable meaning and the prompts'
  documented inputs pair with the transition log's actions.
- **The fade's clock is the presentation's (wall time)** — game time is
  zero wherever a screen shows, so a fade on game time would never
  leave its first frame. Lesson 078's split, now on the presentation's
  side: the simulation on game time, the machinery and the screens on
  the wall's. The arrival is reported with its exact color (`fade
  arrived at 56,16,16 (its own color)`) — the eased value's contract
  (093) verified on a new value.
- **The screens' prompts document their inputs, and the inputs are
  exactly what they document** — no input changed in this lesson; the
  final form is the *communication* of the machine that already
  existed (a design choice: screens describe behavior, they do not
  grow it).

### What the runs verified (headless, Xvfb `:99`, scripted input)

- **Each screen renders and its input acts as documented** (run A, the
  real roster — title → play → pause → play → death → title; run B, the
  scratch fight — title → play → victory → title):
  `screen: title … "ENTER: PLAY"` + `state title -> play (the player
  started)`; `screen: pause … "ESCAPE: RESUME"` + `state pause -> play
  (the player resumed)`; `screen: death: "GAME OVER" / "SCORE 000424
  TIME 0:06 WAVE 1/3" / "ENTER: TITLE"` + `state death -> title (the
  player returned to the title)`; `screen: victory: "VICTORY" / "SCORE
  000000 TIME 0:01 WAVE 3/3" / "ENTER: TITLE"` after `the waves are
  complete (t=1.023)`. Four screens, four prompts, four documented
  actions — each pair in the same run.
- **The fades arrive exactly at their targets** — `title fade arrived
  at 24,24,40`, `pause fade arrived at 24,24,40`, `death fade arrived
  at 56,16,16`, `victory fade arrived at 16,48,16 (its own color)` —
  the eased value landing on the screen's own color, not a rounding
  from it.
- **Honest limit, stated in the prose**: the layout's beauty is
  unverified (no screen to look at); the numbers and the content are
  what a headless run can honestly measure.

### Lesson-096 exercises

- **ex1 (extend-the-code) — the pause over the frozen world.** The
  framebuffer grows `ClearRect` (the clear clipped to a rectangle) and
  the pause draws the world behind its panel. The frame record proves
  it: the pause frames carry `tilemap 0.961, sprites 0.004` beside
  `step 0.000` (the world drawn and frozen) where the other panels
  carry `tilemap 0.000` — quoted from the run.
- **ex2 (predict-the-output) — the fade's clock.** The prediction (the
  step frozen at 0.000 while the fade climbs ~0.043 a paced frame to
  its 0.30 s) against the probe's measured pair: `fade 0.043 — step
  0.000 ms` … `fade 0.259` → `fade arrived` → `fade 0.302` (clamped).
  The walkthrough gives the two clocks' rule of thumb: what must
  happen while the world is frozen is wall time; what must wait for
  the player is game time.

### Batch 4 (lessons 092-096) — closing note

All five lessons are authored, verified on real runs, co-committed, and
tagged (`lesson-092`…`lesson-096`); each tag-to-tag `src/` diff equals
that lesson's code step alone. The feel-and-polish batch completes the
MVD's juice toolkit — hitstop, screenshake, particle bursts, easing,
all four and no fifth (092 wires the hooks to the game's events and
teaches "feedback starts with the event"; 093 brings the bursts and
the easing under the store's cosmetic-share policy) — and the game's
presentation (094's HUD reads the game state in the same frame and
never scrolls; 095 routes the music and the events' sfx through the
mixer's channels into one stream, verified byte-for-byte on a machine
with no sound device; 096 puts the screens in final form with the
fades on the presentation's clock). The frozen checklist's "juice
toolkit" and "states" lines are now demonstrable end to end; the
"audio" line is demonstrated to the byte, its audible half routed to
the learner's machine.

## Lesson-097 — pay the debt (L16): authoring record

The planned debt lesson (design D10): no behavior change, only shape.
One code step — the run gets its shape — and the verification is the
transcript's bill.

### What debt it paid (the ledger)

- **`Run()` did five jobs** (open the window, wire the assets, run the
  frame's phases, print the probes, close with the account — ~670
  lines, every lesson's report added mid-loop). Paid by the split:
  `load.*` (the run's asset wiring — `LoadRunSample`/`LoadRunTable`/
  `LoadRunArt` + the byte checks `PrintSample`/`PrintDefs`, moved
  whole), `world.*` (`World` + `WorldStart`: the startup's exact
  sequence as one named composition — the game's machine still starts
  between the hero's creation and the world's fill, so the report order
  is preserved), `report.*` (`RunReport` + `ReportFrame` + `ReportEnd`).
  `main.cpp` keeps the window, the banners, the loop, the close.
- **The probes tangled through the loop** — one per measurement lesson
  (077's mover state, 085's velocity, 086's walk frame, 089's world
  travel) with anonymous `was_*` bookkeeping between the phases. Paid:
  the probes live in `report.*`, their bookkeeping is one named struct
  (`RunReport`: `blocked`, `vx`, `vy`, `walk_frame`, `seen_x/y`), and
  the printfs moved **verbatim** (a report line is a contract with every
  lesson that quoted it).
- **The wiring reached into the mixer** — the loop's audio step read the
  music channel's cursor (the wrap's detection), scheduled the next
  buffer, reported the stream's first bytes, named a refusing device,
  submitted to the seam. Paid: `sound.*` grows `CHUNK_FRAMES`, `Feed`
  and `SoundFeed`; the loop times the audio phase and calls one
  function. `main.cpp` no longer knows the mixer has channels.
- **Names that stopped fitting** — `main.cpp`'s header still said
  "Lesson 069: the Part 3 closing demo"; the loop's `was_*` locals named
  their birth lesson. Paid: the header says what the file is; the
  bookkeeping's names are its job.
- **Looked at and kept (judged contract, not debt):** `GameWalk`'s
  explicit parameter list — design D6's "per-entity work is expressed
  once", the game's modules meeting at the walk made visible at the
  call site. The lesson's prose names the judgment; the bundling
  alternative is left to the reader's own judgment.

### What the runs verified (headless, Xvfb `:99`, scripted input)

The scenario: title → `Return` into play → 4 s walking right + firing →
`Escape` pause → `Escape` resume → 2 s walking left → close; paced
~25 fps by window-move jiggles (`xdotool windowmove`); four runs —
twice at `lesson-096` (worktree build) and twice at `lesson-097`.

- **The transcript's bill is zero.** Each run reduced to its report
  shape (digits stripped, repeats collapsed): **105 templates in every
  run**; the two refactored runs' sets are *byte-identical* to the
  first old run's (`md5 42c2b6b1…` ×3). The noise floor — old run 1 vs
  old run 2 — is **8 templates apart** (4 each way; the second old run
  killed the hero early: `state play -> death`, the death screen, no
  spitter shell). The only before/after sequential difference: one
  `hero unblocked` probe a few lines earlier — same jitter class.
- **The checklist's demonstrations re-run as documented**: `state title
  -> play (the player started)`; `wave 1 begins — 2 enemies`; `hit: bolt
  hits hero — damage 1, health 3 -> 2`; `feel: hitstop fired (0.25x,
  0.15s)` / `shake fired (5 px, 0.25s)`; `burst: spark x4 at 429,218 — 4
  made, 0 dropped`; `spark settled … 64 px out, its row's range 64
  (exact)`; `state play -> pause` + `screen: pause: "PAUSED" / "SCORE
  000424 TIME 0:04 WAVE 2/3" / "ESCAPE: RESUME"` + `pause fade arrived
  at 24,24,40`; `state pause -> play`; the closing account (`sound: 164
  frames measured, 157 buffers of stream mixed (115395 frames), 32
  effects fired, 0 music wraps`; `walk: 1462 visits over 164 frames`).
- **The cost is unmoved**: frame budget old `163 frames, avg 1.936 ms
  (tilemap 0.981, render 1.465, present 0.427)` vs new `avg 1.947 ms
  (tilemap 0.957, render 1.425, present 0.477)` — the same shape within
  the noise floor, and the strictest witness: `arena: 1580894 of
  33554432 bytes used` byte-identical in all four runs.
- Build warning-free (27 sources now) and `tools/check-boundary.sh`
  clean; `mdbook build` renders the page.

### Lesson-097 exercises

- **ex1 (extend-the-code) — the banners come in from the cold.** The
  five identity banners move into `report.*` (`ReportBanners`); a real
  run shows the same five lines, same words, same order (quoted in the
  walkthrough), and `main.cpp` loses its last `font.h`/`tiles.h` uses.
- **ex2 (measure-the-performance) — the transcript's bill.** The
  whole measurement with the noise floor first: `tools/`
  `transcript-normalize.sh` (shipped as the solution's patch) reduces a
  run to its shape; two old runs are 8 templates apart, the refactored
  runs 0 from the old one. The walkthrough's method note: report the
  floor next to the verdict, always.

### Class-1 prose fixes riding along

The stale "Next:" footers — `lesson-082`…`lesson-086`, `lesson-091`,
and `lesson-096` said "the course home" while a next lesson exists —
now link forward (the task's grant named 086 and 091; 082-085 and 096
were the same staleness and were fixed the same way).

## Lesson-098 — pass 1: measure (L17): authoring record

The measure pass on the frozen menu (D11): instrument, profile, name
the top-2 hotspots with numbers from real frames. Instruments: the
frame account + `gprof` via `-pg` (§1.1 — `perf`/`valgrind` are not on
this machine). Machine: WSL2, Xvfb `:99`, no sound hardware (D12).

### The design decisions this lesson settled

- **The instrument's gap first.** The render's clear was the unnamed
  remainder between the `render` row and its sub-phases' sum. The code
  step names it: `FrameRecord`/`FrameStats` grow `clear`, the loop
  times it, the log line and the table carry it. After this the
  table's numbers add up (`clear + sprites + text + tilemap = render`).
- **The play/panel mix is measured, not assumed.** The record's `step`
  field classifies the frames (step > 0 = play): 1,415 play vs 136
  panel frames in the measurement run. The whole-run average is the
  mix (tilemap 0.895 = the weighted 0.981 and 0.000); the hotspots are
  named from the play frames.
- **Wall time vs CPU, named explicitly.** `present` is 0.444 ms/frame
  of wall (23%) but 0.01 s of CPU across 2,583 frames (0.28%) — the
  seam's wait on the X server, not process work. Both instruments are
  quoted for every claim.

### What the runs verified (headless, Xvfb `:99`, scripted input)

Measurement scenario: `Return` into play, then legs of 10 s walking +
firing (alternating directions), each leg followed by two `Return`s
that restart after a death and do nothing in play (the keep-playing
trick — the hero dies and a dead game is a title screen); paced ~25 fps
by window-move jiggles.

- **The instrumented run (1,551 frames, one death):** whole-run table
  `avg 1.857 ms (update 0.013, entities 0.005, audio 0.032, render
  1.370 — clear 0.458, sprites 0.006, text 0.012, tilemap 0.895,
  present 0.442)`. Split by `step`: **play frames (1,415)** total
  1.944 — clear 0.456, sprites 0.006, text 0.012, **tilemap 0.981**,
  present 0.444; **panel frames (136)** total 0.961 — clear 0.473,
  tilemap 0.000, present 0.427.
- **The profiled run (2,583 frames, `-O0 -g -pg`, same scenario):**
  frame account `avg 1.834 ms (tilemap 0.835, clear 0.443, present
  0.433)` — the same run `gmon.out` was written on. `gprof` flat
  profile, 3.53 s CPU in 353 samples: `BlitSprite` **59.77%** (2.11 s,
  3,501,190 calls), `ClearBuffer` **37.68%** (1.33 s, 2,584 calls),
  `DrawTileMap` 1.13% (0.04 s, 2,204 calls), `BlitSpriteFrame` 0.57%,
  `ChannelFrame` 0.28%, `platform::Present` 0.28% (0.01 s),
  `MixBuffer` 0.28%. Call arithmetic: `DrawTileMap`'s 2,204 draws ×
  1,536 cells = 3,385,344 of `BlitSprite`'s calls (**96.7%**); the
  remainder, 115,846, is exactly the `FontGlyph` call count (the
  glyphs ride the same loop).
- **The top-2 hotspots, named** (both instruments agree on names and
  order):
  - **#1 the map's draw** — `DrawTileMap`'s walk, `BlitSprite` per
    cell: `tilemap 0.981 ms` of a play frame's `1.944 ms` (50%);
    2.15 s of 3.53 s profiled CPU (60.9%).
  - **#2 the frame's clear** — `ClearBuffer`: `clear 0.456 ms` (23% of
    a play frame; 49% of a panel frame); 1.33 s (37.7%) of CPU.
- **Named and recorded as future work (D11), not fixed:** the
  presentation (`present` 0.444 ms wall / 0.004 ms CPU — the seam's
  copy; a double-buffered/MIT-SHM present is a platform-layer change
  the menu does not make); the audio mix at full load (audio 0.031 ms,
  few effects firing); the update's per-entity work at a full 64-slot
  store (entities 0.005 ms); the reports'/frame log's own `printf`
  cost (inside update/text and outside the measured phases).
- **Honest limits, stated in the prose:** all numbers are the course's
  build (`-O0`; the profile's build adds `-pg`), not `-O3`'s; 353
  samples put each line within a percent or two; the paced ~25 fps
  headless run is a rig, not modest-hardware 60 fps evidence (that is
  L20's claim to check).

### Lesson-098 exercises

- **ex1 (measure-the-performance) — what the profile cannot see.**
  `ReportSeam` splits the frame's wall into engine work and seam wait.
  Real run: `engine: seam: 777 frames — engine work 1.323 ms/frame,
  seam wait 0.425 ms/frame` (1.323 + 0.425 = 1.748 = the table's
  total). Answer: the profiler is blind to ~24% of the frame's wall;
  the flat profile's shares are shares of the CPU.
- **ex2 (port-to-your-own-machine) — the measure pass on your
  machine.** `tools/profile-card.sh` prints one machine's card (top
  flat lines + profiled CPU: `3.53 s in 353 samples`); the walkthrough
  names what may legitimately differ (the present's CPU/wait split,
  the device's feed, the build's flags) and demands the machine's name
  beside every number (D12).
