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
  defeat condition is the spec's exact "health reaches zero"), and
  surviving unharmed a moment is the game being complete. Both are
  marked as scaffolding in `game.cpp` and are replaced when the real
  triggers arrive. The *transitions* are the game's own and do not
  change.
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
- **Run B** (title → play → victory): the hero is left unharmed and the
  completion stand-in fires at ~3 s of play → victory.

Transition log (Run A and B, real output):

```
engine: game: 5 states, starting on title
engine: state title -> play (the player started)
engine: state play -> pause (the player paused)
engine: state pause -> play (the player resumed)
engine: hero takes a hit — health 2 / 1 / 0 (t=…)
engine: state play -> death (the hero's health reached zero)
engine: state death -> title (the player returned to the title)
engine: the waves are complete (t=3.106)
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
