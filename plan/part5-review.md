# Part 5 boundary review — the game (the cap, and the close)

Part 5 closed with lessons `lesson-082`…`lesson-103` — 22 lessons, the
skeleton's hard cap, prose + code step + exercises with diff solutions
each, one commit and one tag per lesson, co-committed per the authoring
contract. This review is the part-boundary record: velocity against the
review budget, exercise density against the conventions table, the
toolchain actually used, the measured claims re-verified, and the MVD's
frozen checklist completed line by line with its definition of done.

## The arc, as it landed

| Batch | Lessons | Arc | Status |
| ----- | ------- | --- | ------ |
| The game's shape | 082-086 | the state machine on finished services; the map and camera as the game's; tile collision; hero movement with accel/decel; animation and the feedback hooks | tagged |
| Combat and enemies | 087-091 | projectiles and two weapons (the format grows by named columns); enemy archetype tables; the three behaviors; the boss; waves | tagged |
| Feel and polish | 092-096 | hitstop and screenshake; particle bursts and easing; the HUD; audio integration; the screens in final form | tagged |
| Debt, optimization | 097-101 | the debt lesson (four debts paid, the bill measured to zero); the frozen three-pass menu — measure, fix the map's draw (−43%), fix the clear (−47%), the final frame-budget report | tagged |
| Closing | 102-103 | the retrospective (our engine against real ones; the epilogue map recorded as post-course, not scope); the hand-over (now make YOUR game) | tagged |

The cap discipline held: 22 lessons, no more — every idea the assembly
provoked went to the extras ledger (lesson 102 prints it) and none
entered scope. One class-2 revision rode the arc (the lesson-082
completion-stand-in fix, before any review, in the volatile tail) and
one lesson's code step is **empty** (102's, a retrospective adds no
behavior) and one is **comment-only** (103's hand-over map) — both
stated in their pages and recorded in the notes; the tag discipline
holds for both (`git diff lesson-101 lesson-102 -- src/` is empty
exactly; `lesson-102..lesson-103 -- src/` is the 26-line header
exactly).

## The MVD's frozen checklist, completed line by line

`plan/target-game-mvd.md`'s eight lines, each implemented and
demonstrable, each pointing at the lessons that deliver it:

| Checklist line | Delivered by | Demonstrable as |
| -------------- | ------------ | --------------- |
| `hero: 8-dir movement with accel/decel feel` | 085 (movement) on 083-084 (map, collision) | the eased velocity reports climbing `67,0 → 81,0 → 87,0 …` toward the row's 240 px/s and easing back to rest; the diagonal covering ground at the straight-line speed; eight directions under scripted input |
| `combat: projectiles, 2 weapon types` | 087 | `arm: hero arms blaster (damage 1, rate 300, fires bolt)`, the number keys re-arming from the weapons rows; `hit: bolt hits … — damage 1, health 2 -> 1`; projectiles retiring at walls, at range, at what they hit |
| `enemies: 3 types + 1 boss (chase / keep-distance / flee AI)` | 088 (types as rows), 089 (the behaviors), 090 (the boss's pattern) | the wave runs: `wave 1: spawns bat … chase`, `spitter … keep`, `wisp … flee`, `golem … boss`; each behavior moving through the mover as documented |
| `world: single scrolling map, tile collision` | 083 (map + camera), 084 (collision) | the camera base following and clamping to the map's bounds (`camera base 8,0 → 10,0 → 11,0 …`); `hero blocked at …` holding at a wall with the slide along it |
| `states: title, play, pause, death, victory` | 082 (the machine), 096 (the screens in final form) | title → play → pause → resume re-run here (`screen: pause: "PAUSED" … "ESCAPE: RESUME"`); death and restart re-run here (`screen: death: "GAME OVER" … "ENTER: TITLE"`, then title); victory demonstrated at 091/096's runs (trusted here, see below) |
| `juice toolkit: hitstop, screenshake, particle burst, easing` | 086 (the hooks), 092 (hitstop + shake), 093 (bursts + easing) | `feel: hitstop fired (0.25x, 0.15s)` … `feel: hitstop rested — full speed again` on its wall-time deadline; `feel: shake fired (5 px, 0.25s)` resting at exactly zero; the `burst:` reports; eased values arriving at their targets |
| `audio: music + sfx through our own mixer` | 095 (integration over Part 3's mixer) | the music looping on the music channel (`loop: music wrapped on channel 0 — wrap 1 …`), the effects firing the pool (`hit`/`shot`/`death`); on this machine the mix is rendered in silence and the run reports that and continues |
| `perf: 60 fps on modest hardware + final frame-budget report` | 098-101 (the report: 101) | the report itself (re-run below); the 60 fps half is checked as far as D12 allows — see the definition of done |

**Definition of done** (all three):

1. **Every line implemented and demonstrable — yes.** Each line above
   has its demonstration on record (each lesson's task verification ran
   it at its tag), and the closing re-runs sampled six of the eight
   from the committed state.
2. **60 fps on modest hardware — checked as far as honest measurement
   goes (D12), and no further.** On this machine: **0 of 514 frames**
   over the 16.667 ms budget in the closing re-run (worst 2.664 ms =
   16% of it), at the course's `-O0` build — and `0 of 2583` in
   lesson 101's ten-leg run. What this machine is: WSL2, Xvfb `:99`,
   no sound hardware, an event-driven loop paced at ~25 fps — it
   **cannot demonstrate 60 fps** (it never ran at 60) and no number
   here pretends otherwise. "On modest hardware" is a claim about real
   hardware, checked on none: the line is routed to the learner's
   machine exactly as D12 prescribes (lesson 101's exercise 1, the
   percentiles card). **This is the one line of the definition of done
   that is not verifiable on the authoring rig** — checked this far,
   said out loud, hand-off complete.
3. **The final frame-budget report accounts for each major subsystem —
   yes.** `update` (+ its `entities` sub-phase), `audio`, `render`
   (+ `clear`, `sprites`, `text`, `tilemap` — every sub-phase named,
   the rows summing with no unnamed remainder), `present` — all
   measured sums from real frames of the finished game, with the
   by-state split, the budget line, and the machine line (D12).

## Authoring velocity vs. the 10-20 h/week estimate

`plan/part0-review.md` priced the curriculum at roughly 570 h at an
assumed 10-20 h/week and re-read that as a *review* budget ever since.
Part 5 confirms the reading a fifth time, and this part is the largest
single test of it: 22 lessons at the cap, a debt lesson, a three-pass
optimization with real measurements, and a closing.

**The number (a reconstruction from git commit timestamps — not a
stopwatch):** roughly **17 h of active authoring wall-clock** through
`lesson-103`'s tag, on the order of **19-20 h** including this review
and the integration checks. Elapsed span 2026-10-07 11:12 → 2026-10-09
(late) — about 36 h wall with the overnight gap excluded. Where it
went (bursts between timestamps, gaps > 30 min excluded):

| Burst | Window | Contents | ≈ time |
| ----- | ------ | -------- | ------ |
| 1 | 10-07 11:12–18:20 | the change's setup/audit (tasks 1.1-1.2), 082, 083 | 7h10 |
| 2 | 10-07 22:29–10-08 00:54 | the class-2 completion-stand-in fix, 084, 085, 086 | 2h25 |
| 3 | 10-08 14:25–16:19 | 087-091 (combat and enemies) | 1h55 |
| 4 | 10-08 19:05–21:11 | 092-096 (feel and polish) | 2h05 |
| 5 | 10-08 22:11–23:49 | 097-101 (the debt lesson + the three-pass menu) | 1h40 |
| 6 | 10-09 00:46–~02:10 | 102, 103, this review, the integration checks | ~1h30 |

How the arc went, honestly: the game's shape batch cost the most per
lesson (the state machine, the format growth's compatibility rule, and
the verification harness all landed there), and the class-2 revision
mid-arc took a real bite; from 087 on the batches got cheaper as the
table/store/wave patterns were reused and the scripted-run harness
needed no re-invention; the optimization batch was the cheapest per
lesson because one measurement scenario served all four. Nothing in
the arc surprised the design — the risks register's scope-creep risk
was answered by the cap and the ledger, and the debt risk by L16
existing on schedule.

Against the budget: the whole part's authoring is **under one week of
the 10-20 h/week budget**, while part 0's calibration (25 lessons ≈
2-4 review-weeks) puts 22 lessons at **2-3 review-weeks of human
review** — still the dominant cost, unchanged. Estimate the next
edition in review-weeks.

## Exercise density against the conventions table

Conventions §4 sets Part 5 at **fewer, larger — at most 2** exercises
per lesson. Every lesson counted:

| Lesson | Exercises | Lesson | Exercises | Lesson | Exercises |
| ------ | --------- | ------ | --------- | ------ | --------- |
| 082 the game skeleton | 2 | 091 waves | 2 | 100 pass 2b: fix the clear | 2 |
| 083 the tilemap and camera | 2 | 092 hitstop and screenshake | 2 | 101 pass 3: the frame-budget report | 2 |
| 084 tile collision | 2 | 093 bursts and easing | 2 | 102 the retrospective | 2 |
| 085 hero movement | 2 | 094 the HUD | 2 | 103 now make YOUR game | 2 |
| 086 feedback and animation | 2 | 095 audio integration | 2 | | |
| 087 projectiles and weapons | 2 | 096 screen polish | 2 | | |
| 088 enemy archetype tables | 2 | 097 pay the debt | 2 | | |
| 089 enemy AI | 2 | 098 pass 1: measure | 2 | | |
| 090 the boss | 2 | 099 pass 2a: fix the map's draw | 2 | | |
| | | | | **Total** | **44** |

**22 of 22 lessons counted; 22 of 22 at 2 exercises — at most 2, as
the table demands** (44 exercises; not one lesson at 3, not one at 1 —
"fewer, larger" is met at exactly the cap, and the closing lessons'
pairs are the largest of the part). Archetype spread: extend-the-code
18, predict-the-output 11, explain-in-prose 6,
measure-the-performance 4, port-to-your-own-machine 4, fix-the-crash 1
— all six archetypes used, no lesson repeating an archetype in its
pair. The closing pair leans explain-in-prose / port-to-your-own-machine
as planned; the four closing exercises include the part's two
**no-code** solutions (102's pair — prose deliverables whose patches
are empty on purpose and say so), which the contract explicitly
permits for exercises that write no code.

## Toolchain actually used

Re-recorded at the close (run now, against the finished state):

| Tool | Version | Used for |
| ---- | ------- | -------- |
| Linux | 6.6.87.2-microsoft-standard-WSL2 x86_64 (Ubuntu 24.04.4 LTS) | the authoring machine |
| gcc / g++ | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) | every build (27 sources at the part's close) |
| Xvfb | 2:21.1.12-1ubuntu1.8 | the headless display every window check ran on (`:99`, 800x600x24) |
| xdotool | 3.20160805.1 | scripted input and the window-move jiggle pacing (~25 fps) |
| X11 development headers | `libx11-dev` 2:1.8.7-1build1 | the seam's window implementation |
| ALSA library / headers | 1.2.11 (`libasound2-dev` 1.2.11-1ubuntu0.3) | the seam's sound implementation; mixed in silence (no device) |
| gprof | GNU Binutils 2.42 | the measure pass's function-level profiler (`-pg` runs) |
| mdBook | 0.5.4 (pinned) | every page render check |
| openspec | 1.14.0 | every planning validation |
| git | 2.43.0 | the history, tags, and every patch application |

No drift from the versions recorded at the change's opening (notes
§1.1) — every tool that quoted numbers in a page is the same version
today. No sound hardware (`/dev/snd` holds only `timer`); every run
continued without sound and the lessons' numbers say so where it
matters. `perf` and `valgrind` are absent (and `perf_event_paranoid`
is `2`) — the measure pass sat on the frame account plus `gprof`, as
recorded.

## Measured claims, re-verified

Spot-checks against the committed states at the close. **Re-run this
session** (fresh runs at the tagged states, same scripted scenario at
both ends of each comparison):

| Claim | Lesson | Result |
| ----- | ------ | ------ |
| The final report attributes each subsystem from real frames | 101 | **re-run at `lesson-103`**: 514 frames, `avg 1.269 ms (update 0.024, entities 0.006, audio 0.032, render 0.798 — clear 0.228, sprites 0.007, text 0.011, tilemap 0.551, present 0.416)`; `by state 501 play frames at 1.283 ms, 13 screen frames at 0.750 ms`; `budget 0 of 514 frames over it, worst 2.664 ms (16%)`; the machine line printed. Reproduces within the run-noise band the lessons name (101's ten-leg run: play `1.313`, tilemap `0.538`, clear `0.244`, present `0.438`) |
| Pass 2a: the map's draw fell 43% | 099 | **re-run** (pure-play window, `lesson-098` vs `lesson-103`, same scenario): `tilemap 0.885 → 0.501` = **−43%** — the claim reproduces to the point |
| Pass 2b: the clear fell 47% | 100 | **re-run** (same two states): `clear 0.434 → 0.235` = **−46%** (claim −47%); `total 1.875 → 1.236` = **−34%** (claim −35%) |
| 0 frames over the 60 fps budget | 101 | **re-run**: `0 of 514`, worst 2.664 ms (16% of 16.667) — reproduces *on this machine*, as bounded above |
| The format's compatibility: every shipped file loads, unnamed fields default | 087 | **re-run**: `table: unnamed fields at their defaults …` and the `def …` lines for all five tables, `assets/entities.txt` unchanged |
| A definition the table does not hold is a typed failure | 073 | **re-run**: `table: "dragon" -> unknown` at start-up, every run |
| The toolkit fires and rests on its own deadlines | 092-093 | **re-run**: `feel: hitstop fired (0.25x, 0.15s)` … `feel: hitstop rested — full speed again`; `shake fired (5 px, 0.25s)`; the death's heavier weights; `burst:` reports |
| The hero eases toward the intent | 085 | **re-run**: `hero velocity 67,0 → 81,0 → 87,0 …` climbing toward the row's 240 px/s |
| The state machine acts one state at a time | 082, 096 | **re-run** scripted: title → play (Enter) → pause (Escape) → resume (Escape); death screen and restart in the scenario run (`screen: death: "GAME OVER" …`, then title) |
| Waves spawn their row's composition and chain | 091 | **re-run**: `wave 1: spawns …`, `wave 2: spawns …`, `wave 1 cleared — the next begins` |

**Trusted** (recorded at their lessons; not re-run this session — each
would need a rebuild-and-rerun of a heavier instrument than the claims
above, and their records carry their reconciliations):

- the `gprof` flat profile's shares (098: `BlitSprite` 59.77% /
  `ClearBuffer` 37.68% with the call arithmetic closing at 96.7%) —
  the `-pg` run was not re-taken;
- the word fill's byte-level equivalence (100: 0 differing bytes over
  400 randomized clears) and the per-pixel bench's 2.4× (100's
  exercise 1);
- the percentiles card (101's exercise 1: `p50 1.250 / p95 1.750 /
  p99 2.000`) — one instrument, run-shape dependent;
- the victory transition's run (`the waves are complete`) —
  demonstrated at 091/096's sessions and recorded there; this session
  did not play a full game to victory;
- Part 4's reconciliations (075's visit account, 080's camera clamp
  arithmetic, 081's table-vs-log digit equality) — re-verified at part
  4's review.

What a reviewer should re-run first if any of the trusted rows is
doubted: the `-pg` build and the byte-equivalence harness (both are
documented in their lessons' records and reproduce mechanically).

## Integration checks (task 8.2)

From a clean checkout (`git clone` of the repository to a throwaway
directory) following only `README.md`:

| Check | Result |
| ----- | ------ |
| `./build.sh` | `build: OK (27 source(s) compiled -> build/game)` — warning-free |
| `mdbook build` | `HTML book written to site` (the pre-existing search-index WARN, harmless) |
| `openspec validate --all` | `Totals: 16 passed, 0 failed (16 items)` |
| the finished game, headlessly | runs the scripted scenario under Xvfb `:99` + xdotool (window-move jiggle pacing), reports the missing sound device (`engine: no audio output on this machine` / `engine: continuing without sound`) and continues, prints the final frame-budget report (513 frames, `avg 1.254 ms`, `0 of 513` frames over budget, the machine line), closes cleanly |

Tags: `git tag` shows **consecutive `lesson-082`…`lesson-103`** (22 of
22, nothing past 103), and **every consecutive pair's diff equals
that lesson's code step** — checked two ways for all 22 pairs:
exactly one commit per range touches `src/`+`assets/` (zero for
lesson-102's empty step), and each lesson page's embedded `diff` block
equals the tag-to-tag `src/`+`assets/` diff byte for byte **minus
binary-file entries and index/hunk metadata** (the six pairs whose
metadata drifted — 083 and the asset-bearing 086-088/093/095 — match
exactly once `index`/`@@` lines are excluded; the un-embeddable
content is only the lessons' binary assets, each named in its prose).
The two closing pairs read exactly as their pages say:
`lesson-101..lesson-102 -- src/` empty; `lesson-102..lesson-103 --
src/` the 26-line hand-over header.

The throwaway checkout and the `lesson-098` measurement worktree were
deleted after the checks; the main tree is clean.

## Recommendation: extending the frozen prefix

Following `plan/part3-review.md` and `plan/part4-review.md`'s
discipline (no extension on authoring evidence alone):

- **Do not extend the prefix on this review.** Part 5's 22 lessons
  have the author's verification behind them and no reviewer's pass.
- **When the review passes land, freeze in the same block order:**
  `lesson-026`…`lesson-043`, then `lesson-044`…`lesson-058`, then
  `lesson-059`…`lesson-070`, then `lesson-071`…`lesson-081`, and
  Part 5 last: `lesson-082`…`lesson-103`. Part 5 is the whole game on
  top of all four earlier parts — freezing it first would freeze a
  game built on unfrozen services.
- **Part 5's block carries the timing-row risk of all earlier blocks
  and adds two edges.** The closing lessons' empty/comment-only code
  steps (102, 103) and the two no-code exercise solutions (102's pair)
  are the first of their kinds in the course — a block-freeze review
  should confirm the tags' diffs read as the pages say (empty;
  26-line header; empty patches). The measured rows (the frame
  account's timings) move with the machine and the run; the structural
  claims (the typed failures, the compatibility rule, the toolkit's
  fire-and-rest, the state machine) reproduce deterministically.
- **The resync path is unchanged** and covers this part: `git checkout
  lesson-NNN -- src/`.

## Review pass status (pre-freeze)

The independent review pass over Part 5 has **not** run; this review
records the author's verification, which is its input. What a reviewer
should re-run, in order of what breaks first:

1. `./tools/check-boundary.sh`, `openspec validate --all`, `mdbook
   build`, `./build.sh` (seconds; the build must be warning-free).
2. The clean-checkout integration checks (task 8.2), including every
   consecutive tag pair's `src/`+`assets/` diff against that lesson's
   embedded code step.
3. The optimization's before/after at the tags (098 → 099 → 100): the
   pure-play rows above, taken with one scenario at each state.
4. The MVD checklist's demonstrations, line by line (the table above) —
   the perf line's honest boundary included.
5. Every solution patch: `git apply`, build, run, reverse — 44
   patches, of which 42 apply code and two are empty by design (102's
   pair).
6. The two closing lessons' honesty claims: 102's tag diff is empty;
   103's tag diff is the hand-over comment and nothing else.

## Known warts carried forward (intentional, documented in-prose)

- **Bookkeeping commits sit between lesson tags** ("Record task
  progress"), as in Part 4: a raw `git diff lesson-N-1 lesson-N`
  includes `openspec/` changes; the `src/`+`assets/` diff is exactly
  that lesson's code step — verified for all 22 pairs at task 8.2.
- **102's code step is empty and 103's is comment-only.** Deliberate,
  stated in both pages and in the notes: a retrospective adds no
  behavior and a hand-over is a map. Both tags carry their discipline;
  if a future edition wants a token diff at 102, it must first say
  what behavior the retrospective would add.
- **The 60 fps line is not proven on hardware** (the definition of
  done's second condition) — checked to "0 frames over budget at `-O0`
  on the authoring rig" and routed to the learner (D12). Any claim
  stronger than that is a lie until a real machine's card exists.
- **`present` is the finished game's largest frame row** (0.416-0.438
  ms wall) and belongs to the seam — measured, named, on the
  future-work ledger, untouched by the menu (D11).
- **The score's travel rule and the hound's reused art** (103's
  exercise 1's worked example) are demonstration choices, not design:
  the walkthrough names the per-kind bounty as the table seam's
  sanctioned growth and says the art is a placeholder.
- **`RUN_MACHINE` still names the authoring machine** at the tip — by
  design (D12: the numbers carry their machine). Learners edit it in
  101's and 103's exercises; a change to that string outside those
  exercises invalidates every quoted number.
- **mdBook prints `WARN search index is very large`** at every build —
  pre-existing at 100+ lessons, harmless (the site builds and
  deploys), and outside this change's scope to fix.

*(Everything above is measured from the tagged lesson states. The
clean-checkout checks, the tag-pair checks, the re-run spot-checks
above, and every solution patch were run on the authoring machine
before this review was written.)*
