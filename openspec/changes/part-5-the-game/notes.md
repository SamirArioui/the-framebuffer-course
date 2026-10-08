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
