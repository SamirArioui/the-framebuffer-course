# Part 5 pre-freeze sweep — mechanical verification report

**Scope:** the "Review pass status (pre-freeze)" re-run list in
`plan/part5-review.md` (items 1–6).
**Date:** 2026-10-09
**Rig:** WSL2 kernel 6.6.87.2, gcc/g++ 13.3.0, mdbook 0.5.4, openspec 1.14.0,
Xvfb + xdotool 3.20160805.1, no sound device (`/dev/snd` holds only `timer`).
**Method:** clean `git clone --local` of the repo at `9197759` under `/tmp`;
every run on a fresh Xvfb display with scripted xdotool input, closed via
`WM_DELETE_WINDOW` so the frame-budget report prints. The main working tree was
not touched (it stayed clean). The sweep's raw artifacts (scripts, run logs,
per-patch results) were machine-local and are not committed; this page is
their record.

**Verdict: all six items pass. No failures found.**

## Item 1 — quick checks

| Check | Result |
| ----- | ------ |
| `tools/check-boundary.sh` | OK — OS headers/calls only in the two platform files; 17 single-line declarations in `platform.h` |
| `./build.sh` | OK — 27 sources, **0 warnings** |
| `mdbook build` | OK — only the documented `WARN search index is very large` |
| `openspec validate --all` | 21 passed, 0 failed (16 at the review's writing; specs have grown since — all still pass) |

## Item 2 — clean-checkout integration + tag-pair equivalence

Clean clone, README-only steps:

- `./build.sh` OK, 0 warnings; `mdbook build` OK; `openspec validate --all` 21/21.
- Headless scripted run: `exit=0`; reported `no audio output on this machine`
  and `continuing without sound`; closed cleanly with the report
  (`294 frames, 0 of 294 over budget, worst 2.544 ms`). *Frame counts are
  run-dependent — the review's 513 vs this sweep's 294; shape, not count, is
  the check.*

Tag pairs, `lesson-081..lesson-103`, all 22 pairs:

- **22/22 equivalent** to their lesson page's embedded diff block.
- 21 pairs match byte-for-byte even with `index`/`@@` lines kept; only
  `lesson-083` needs the documented metadata exclusion.
- Binary entries (11 total, in lessons 086/087/088/093/095) are the only
  un-embeddable content — each named in its lesson's prose, as recorded.
- Exactly **1 commit per range** touches `src/`+`assets/` (**0** for
  lesson-102's empty step).

## Item 3 — optimization before/after (same scenario at each state)

Scenario: `Return` into play, then 3 legs of held walking + firing (~8 s each)
with two `Return`s between legs (restart-if-dead). Play-frame rows (frames with
`step > 0`) averaged from the per-frame log:

| Play row | lesson-098 | lesson-103 | delta | review's re-run |
| -------- | ---------- | ---------- | ----- | --------------- |
| `tilemap` | 0.981 ms | 0.575 ms | **−41%** | −43% (0.885 → 0.501) |
| `clear` | 0.453 ms | 0.230 ms | **−49%** | −47% (0.434 → 0.235) |
| `present` | 0.464 ms | 0.459 ms | −1% (unchanged) | — (the seam's cost, not on the menu) |
| `total` | 1.957 ms | 1.320 ms | **−33%** | −34% (1.875 → 1.236) |

Frames: 098 = 291 (245 play / 46 panel); 103 = 294 (248 play / 46 panel).
Both runs: `exit=0`, clean close, report printed; lesson-103: 0 of 294 over
budget.

Per-pass attribution (same scenario at `lesson-099` and `lesson-100`):

| Step | `tilemap` | `clear` | `total` |
| ---- | --------- | ------- | ------- |
| 098 → 099 (the map's draw) | 0.981 → 0.545 ms (**−44%**) | 0.453 → 0.444 ms (−2%, flat) | 1.957 → 1.466 ms (−25%) |
| 099 → 100 (the clear) | 0.545 → 0.551 ms (+1%, flat) | 0.444 → 0.231 ms (**−48%**) | 1.466 → 1.257 ms (−14%) |

Each pass moved exactly its named row and left the other flat — the menu's
discipline, reproduced. Frames per state: 098 = 291 (245 play), 099 = 294
(248 play), 100 = 294 (248 play), 103 = 294 (248 play); all four runs
`exit=0`, report printed, clean close.

## Item 4 — MVD demonstration lines (one ~55 s scripted journey at lesson-103)

Captured (with 1 death and 2 title→play transitions):

- **States:** `title -> play`, `play -> pause`, `pause -> play`,
  `play -> death` + the death screen + restart.
- **Enemies/waves:** wave 1 (2 enemies) → `wave 1 cleared — the next begins`
  → wave 2 (5 enemies).
- **Hero/world:** eased velocity; `camera base`; `hero blocked` (tile
  collision).
- **Juice:** `hitstop fired`, `shake fired`, `burst:` report.
- **Combat:** `arm: hero arms blaster`; hits (`hit: bolt hits bat — damage 1,
  health 2 -> 1`).
- **Audio:** missing device reported and the game continues; music loop wraps;
  sfx pool lines.
- **Perf:** report + budget line (0 of 590 over) + machine line; clean close.

Not re-run: **victory** — the review's own trusted row ("demonstrated at
091/096's runs"); a scripted journey does not reliably clear three waves, and
the review already names it as trusted.

## Item 5 — the 44 solution patches

**44/44 pass**, each at its lesson's tagged state, in one throwaway clone:

| Stage | Result |
| ----- | ------ |
| apply | 42 ok + 2 empty-by-design (lesson-102's pair) |
| build | 44/44 ok — **0 warnings in all 44 builds** |
| run | 44/44 — every patched state started, printed its report, closed cleanly |
| reverse | 44/44 clean (`checkout -f` + `clean` back to the lesson state) |

Per-patch logs (`patch-logs/lesson-NNN-exN.*`) and the summary table
(`patch-results.tsv`) were machine-local to the sweep.

## Item 6 — closing lessons' honesty claims

- `git diff lesson-101 lesson-102 -- src/`: **empty** (0 lines).
- `git diff lesson-102 lesson-103 -- src/`: the **26-line hand-over comment**
  in `src/game.h` and nothing else (37 raw diff lines = 26 added + headers and
  context).

## Caveats

- Run-dependent numbers (frame counts, milliseconds) differ from the review's
  session; the structural and magnitude claims reproduce (tilemap ≈ −40%,
  clear ≈ −50%, total ≈ −33%).
- This sweep is the mechanical re-run of the review's checklist — **not** the
  independent review pass. The prose/design judgments, the trusted rows, the
  victory transition, and the freeze decision remain the reviewer's to own.

*Raw artifacts were machine-local to the sweep (`tagpairs.py`,
`patch_sweep.sh`, `opt_runs*.sh`, `mvd_check.sh`, `analyze_opt.py`, per-patch
logs); this page is their record.*
