# Lesson 102 — the retrospective: our engine against real ones

{{#include ../../stability-horizon.md}}

## Prose

The game is done. The MVD's frozen checklist is complete line by line,
and the finale's report attributes every subsystem of every frame the
finished game spent — measured, never modeled, on a named machine
(design D12). Before the last lesson hands the engine over, this one
looks back honestly and then forward at the horizon: **what this engine
is, against the engines games actually ship on** — and **the epilogue
map**, the two pieces of work that would come next if the course
continued. Both of them are post-course material by decision, not
scope. The MVD's rule from the first day of Part 5: an idea outside
the frozen contract is recorded as extras and never enters the part's
scope. This lesson is where that rule is kept — in writing.

### Our engine against real ones

Compare like with like, and with named numbers. Everything below is
this engine at `lesson-101`'s state, measured on this machine (WSL2,
Xvfb `:99`, no sound hardware, the course's `-O0` build) — the
finale's table, `2,583` real frames of the finished game played hard.

**The pixels.** Ours is a software framebuffer: the CPU writes every
one of the 640×480 pixels of a frame. `ClearBuffer` fills all 307,200
of them, `DrawTileMap` walks the camera's cells and blits each tile
through `BlitSprite`, sprites and `FontGlyph` text land on top, and
`platform::Present` copies the whole buffer to the window once per
frame — the engine's one copy loop. The finale measured that frame:
`render 0.799 ms` (of which `clear 0.244` and `tilemap 0.538`),
`present 0.438 ms` of wall for `0.004 ms` of CPU — the seam's wait on
the X server — `1.313 ms` a play frame in total. A shipping 2D game
draws through a GPU: the CPU builds a command list, textures live in
VRAM, and the frame's pixels are never the CPU's; the display's copy
is a buffer swap. What the difference buys them is scale — resolution,
effects, particle counts, all on the GPU's budget — and what it costs
them is a driver stack, shader toolchains, GPU-specific debugging, and
frames that are harder to attribute. What ours buys instead is
legibility: every byte is ours to read, and the frame account names
every phase's cost to three decimals.

**The entities.** Ours is one fixed store — `ENTITY_CAP` rows, created
from table definitions, refused typed when full, never stealing — with
one walk that expresses per-entity work once and behaviors
(`chase`/`keep`/`flee`, and the boss's pattern) that write a movement
request through the mover like the player's input writes the hero's.
Production engines run entity-component systems, scene graphs, and job
schedulers over thousands of entities. What the difference buys them
is composition and scale; what ours buys instead is a memory story
that never changes while the game runs — no allocation after start-up,
a refusal instead of a surprise, and the policy the store taught in
lesson 074: cosmetic work may be dropped, gameplay work may not.

**The data.** Ours is hand-authored text: the tables under `assets/`
name their columns in a header, load whole into the arena, and fail
typed and complete-or-named; the game changes when the file changes,
with no rebuild. Production pipelines cook binary assets through
importers, validators, and content tools. What the difference buys
them is pipeline safety at team scale; what ours buys instead is a
format you can read end to end — and one loader, in one file pair, that
you have already read.

**The sound.** Ours is our own mixer: sixteen channels
(`AUDIO_MIXER_CHANNELS`), WAVs loaded by our own loader, one mono
16-bit feed at 44.1 kHz through the seam, and — on this machine — the
whole mix rendered in silence because there is no sound device. Games
ship on middleware (FMOD, Wwise): busses, effects, occlusion,
streaming, and authoring tools. What the difference buys them is
production tooling; what ours buys instead is a signal path where
every sample is findable.

**The loop.** Ours is one thread, event-driven, under the language
law: no allocation while the game runs, no exceptions, nothing behind
the seam's back. There is no fixed timestep and no interpolation —
named non-goals from the earlier designs, not accidents. Engines that
ship run job systems across cores and interpolate their motion. What
the difference buys them is core scale and steady motion at any frame
rate; what ours buys instead is that concurrency bugs cannot exist
here, and that the frame record (lesson 079's contract: wall time at
any scale) stays honest.

**The seam.** Ours is `platform.h` — the contract a second OS
implements, one OS's answer in two files (`platform_x11.cpp`,
`platform_alsa.cpp`), and `tools/check-boundary.sh` enforcing that no
engine file names an OS. SDL and GLFW cover dozens of platforms and
devices; theirs buys reach, ours buys a seam small enough to hold in
one head — and a second OS is a bounded piece of work, not a port.

**The discipline.** Ours is the frame account plus `gprof` plus the
census of what the compiler actually emitted — and the frozen
three-pass menu: measure, fix exactly what measurement named, report
what you did not. The two passes spent their fixes on the map's draw
and the clear (`1.437 → 0.782 ms` together) and touched nothing else;
everything else measured and named sits on the future-work ledger.
Shipped engines have render-docs, platform profilers, and budgets
enforced in CI. What the difference buys them is precision across the
whole stack; what ours buys instead is the same discipline with no
tooling to learn first — and the habit survives every tool.

Read the whole comparison in one sentence: **this engine trades scale
for legibility**, and the trade is deliberate. One caution before you
quote any number from it: ours were measured at `-O0` on a headless
rig — the *shapes* are the claims (the rows, the shares, the ledger),
not the milliseconds, which move with every machine (D12).

### The epilogue map — post-course, not scope

If the course continued, two pieces of work would come next. They are
**not** this course's scope and they are **not** your game's
obligations. They are recorded here so the record is honest about
where the horizon is.

**A GPU port.** What it would change: `Framebuffer`'s bytes stop being
the pixels — the clear, the map's draw, the sprites, and the text
become work for a GPU-owned surface; `platform::Present` becomes a
swap; the frame account's rows change shape (the render rows stop
being CPU cost); and the tiles' pixel-format item on the ledger
resolves itself, because a texture is the GPU's format at load. What
would survive untouched: the game layer whole — states, waves, combat,
AI, the toolkit — the tables and their format, the store's policies,
game time, the frame account's discipline, and the seam (`platform.h`
grows a rendering half, or the port lives behind `Present`). Its
first milestone, if it ever happens: keep the engine's pixel contract
(BGRA bytes) and replace only the presentation path, measuring at each
step — and it happens only when measurement names a row the CPU cannot
shed. "GPUs are faster" is not a measurement.

**A Windows platform module.** `platform.h`'s contract in one OS's
terms: a window (open, close, the title the run reports against), the
key state (`KeyDown`/`KeyPressed`, the nine keys mapped), focus,
`Now`, `PageSize`, the memory reservation pair, the file read/write
pair, the event pump, the close request, `Present` — the same BGRA
bytes, the same "the copy has happened" promise — and the audio
output's feed contract mapped to WASAPI or DirectSound. Two files, one
per OS responsibility, exactly the way `platform_x11.cpp` and
`platform_alsa.cpp` are two files today; `tools/check-boundary.sh`'s
list grows to name them; and **no engine file changes** — the promise
lesson 042 audited. What the port must keep: the typed failures (each
error name is the contract's), every run report, and the
demonstrations re-running unchanged. What may legitimately differ:
the loop's pacing (a real sound device paces the feed; our rig was
jiggle-paced at ~25 fps), the `present` row's CPU/wait split, and the
physical key names. This one is not speculative architecture — it is
the existing seam, implemented a second time.

**Everything else is extras — recorded, never added.** The ledger, as
it stands at the course's close:

- the present's copy (a double-buffered or MIT-SHM present — a
  platform-layer change the menu did not make; the GPU port or the
  Windows module would resolve it);
- the tiles' pixel format (copy wider's layout change — measurement
  must ask for it);
- the audio mix at full load (all sixteen channels firing — unmeasured
  here);
- the update at a full 64-slot store (unmeasured here);
- the reports' own printing inside the measured phases (a quieter run
  measures cheaper phases);
- a fifth feel effect (out of scope by definition — the toolkit is
  four, and this is the rule lesson 092 kept and this lesson keeps
  again);
- a fixed timestep, interpolation, an entity-component system,
  threads, an editor, multiplayer, a second game mode (design
  non-goals, recorded so nobody rediscovers them as ideas).

Every line above is a decision to *not* build — kept the way the MVD
said to keep it: recorded as extras, never added. Scope is a promise;
the checklist decides. Your game will need the same ledger — that is
the last lesson's job to hand over.

### What this page verified, and what it did not

- **The retrospective's numbers are not new measurements.** Every
  number above is one the earlier lessons measured and quoted with
  their machine (D12): the finale's report (lesson 101's ten-leg run),
  the seam's CPU/wait split (lesson 098's exercise 1), the two passes'
  before/after rows (lessons 099-100). This page measured nothing.
- **The page itself was checked where a page can be checked**: it
  renders (`mdbook build`), and its claims about code name symbols and
  structures that exist at this lesson's end state — whose diff
  against `lesson-101` is empty (the code step below).

What this page did **not** verify is any of the epilogue map: the GPU
port and the Windows module are scoped work that has not been
attempted, and nothing here should read as a promise that they work.
That is what "post-course material rather than scope" means, said out
loud.

## Code step

**This lesson's code step is empty, on purpose.** The retrospective
adds no behavior: nothing under `src/` or `assets/` changes, and
`git diff lesson-101 lesson-102 -- src/` prints nothing at all. The
template's diff block would be empty — here is the honest statement
instead of an empty fence. The tag `lesson-102` marks the same code as
`lesson-101`, with this page beside it; the co-commit rule holds
either way (code and text as of lesson N, and here the code step's
content is: none). If you expected a token edit to justify the tag —
there is none to make, and inventing one would be the dishonesty.

## Exercises

Two, and both are writing and judgment rather than code — this lesson
teaches no new mechanism. Each ends with its solution after the
prompt.

### Exercise 1 — our engine, honestly compared *(explain-in-prose)*

Pick one subsystem of this engine — the pixels, the entities, the
data, the sound, the loop, or the seam — and write its honest
one-pager: at most a page, three claims about what ours does, each
carrying a number the course measured (name the lesson it was measured
in and the machine it was measured on), three facts about what
production engines do instead, and a closing judgment in your own
words: what the gap buys them and costs them, and what ours buys
instead. The deliverable is the page; there is no code to write in
this exercise.

> **Solution:** [ex1 — walkthrough](../../solutions/lesson-102/ex1.md)

### Exercise 2 — the epilogue map on your machine *(port-to-your-own-machine)*

The epilogue map above was scoped on a rig with no GPU, no sound
device, and one OS. Run the finished engine on your machine — a real
window, a real sound device if you have one — and re-order the map for
your desk: which of the two pieces does your machine demand first, and
which report rows decide it; then scope that piece's first milestone
in your machine's terms (for the module: the contract's functions in
your OS's APIs; for the port: which rows of the frame-budget report
die, which survive, and what replaces the copy loop). Say what your
machine changed in the picture. The deliverable is the map; there is
no code to write in this exercise.

> **Solution:** [ex2 — walkthrough](../../solutions/lesson-102/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 101 — pass 3: the frame-budget report](lesson-101-frame-budget.md) ·
**Next:** [Lesson 103 — now make YOUR game](lesson-103-your-game.md) ·
**Code tag:** [`lesson-102`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-102)
