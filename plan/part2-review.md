# Part 2 (M3) boundary review

Recorded at the close of `part-2-software-rendering` for the next part's
planning *(curriculum: part-boundary review; design: Migration Plan)*.
Measured against `plan/conventions.md` and the authoring plan's estimates.

## What shipped

15 lessons (`lesson-044` … `lesson-058`), each prose + exactly one
co-committed code step + its extensions and diff solutions, one commit
per lesson, one tag per lesson:

| Batch | Lessons | Arc | State |
| ----- | ------- | --- | ----- |
| Blitter | 044-046 | a sprite as loaded bytes (PPM by hand); the clipped, transparent blit — one copy loop; the sprite moves by polled input, the marker retires, the record names its first subsystem | tagged |
| Caches | 047 | the **caches deep dive**: cache lines and locality measured on the blit's own copy walk — concept-first, x86-64 as the worked example | tagged |
| SIMD reading | 048-049 | the **SIMD/assembly-reading deep dive**: the blit's `-O0` listing read instruction by instruction; the SIMD lens — the vectorized copy read, never written | tagged |
| Text | 050-051 | the bitmap font as an asset (the sheet cut into glyph sprites); text on screen — the layout, the missing-glyph rule, the HUD (O3) | tagged |
| Tilemap | 052-054 | the tilemap asset format (kinds carry solidity); tilemap drawing through the blit; the camera — base and additive (O2) | tagged |
| Collision | 055-056 | the queries over solid cells (O4); the mover that stops at walls — the hero's precursor | tagged |
| Close | 057-058 | the closing demo (every capability at once); the frame-budget table (O1's payoff) | tagged |

Totals: 15 lesson pages, **30 extensions**, 30 solution patches + 30
walkthroughs, 15 tags, one new tool (`tools/disasm.sh`), four authored
assets (`assets/sprite.ppm`, `font.ppm`, `map.txt`, `tiles.ppm`), and
three new capabilities (`software-rendering`, `tilemap`,
`frame-accounting`) delivered against their delta specs. The MVD's four
Part 2 obligations — O1 instrumentation, O2 camera, O3 text, O4 map +
collision — are implemented and demonstrable in lesson-057's closing
demo.

One class-2 correctness fix landed mid-part and is documented in the
change notes: the lesson-053 tile sheet was authored cell-by-cell while
PPM is raster order, scrambling the art. It was regenerated, amended
into the lesson-053 commit, and `lesson-053`…`lesson-056` re-tagged
(the volatile tail permits this; `backup-before-fix` holds the old
history). The fix changed no page and no quoted number.

## Exercise density vs. the conventions table

Conventions §4 asks Parts 1-2 for **1-2 "make it yours" extensions per
lesson**. Every lesson measured (extensions authored, patches on disk,
one walkthrough per extension):

| Lesson | Ext. | Lesson | Ext. | Lesson | Ext. |
| ------ | ---- | ------ | ---- | ------ | ---- |
| 044 | 2 | 049 | 2 | 054 | 2 |
| 045 | 2 | 050 | 2 | 055 | 2 |
| 046 | 2 | 051 | 2 | 056 | 2 |
| 047 | 2 | 052 | 2 | 057 | 2 |
| 048 | 2 | 053 | 2 | 058 | 2 |

**15 of 15 lessons counted; 15 of 15 within the 1-2 band** (all at 2; 30
extensions total). As in Part 1, the band's top is a default, not a
verdict — if a review finds a lesson padded, its second extension is the
first cut.

Archetypes stayed inside the bounded six:
predict-the-output 12, extend-the-code 12, port-to-your-own-machine 3,
measure-the-performance 2, explain-in-prose 1. **fix-the-crash does not
appear in Part 2** — the typed-failure material (lessons 044, 052) was
taught through prediction and extension instead; if Part 3 has a lesson
whose natural exercise is a deliberate breakage, the archetype is free
to return. The deep dives' exercises are almost all predict-the-output
by design: the dives read, and the exercises train the prediction.

## Authoring velocity vs. the 10-20 h/week estimate

Part 2 was authored, verified, and tagged in a **single machine-assisted
session (2026-10-06)** — 15 lessons including two deep dives with real
machine data and one mid-part asset fix. The Part 1 reading stands and
is better evidenced:

- **Verification is still the work.** Every quoted number in Part 2 is
  real: pixel readbacks at reported positions (045, 050, 053, 057), the
  check tables (055's twelve collision answers, 054's three camera
  scenarios), scripted-input runs (046, 054, 056, 057), the deep dives'
  sweeps and listings (047-049), and the frame-budget cross-check (058:
  every table row reproduced by averaging the session's frame log). The
  asset fix above was found *by* a verification run, not by reading —
  the argument for re-running, not re-reading, writes itself.
- **Estimate Part 2 in review-weeks.** 15 lessons ≈ **2-3 review-weeks**
  at the 10-20 h/week human budget (contents + re-run transcripts +
  patch application each), of which the deep dives are the slowest
  reading and the mover/collision lessons the fastest.
- **Machine-assisted guardrails kept paying.** Real-output-only quoting
  caught: the tile sheet's raster-order scramble (053), a mover report
  that flickered on sub-pixel frames (056, fixed before commit), a
  signed-`char` index discussion that the bounds check made moot-but-
  instructive (050), and several arithmetic slips in check expectations
  (045's clip counts, 052's solidity storage) — all corrected before
  the affected tag.

## Toolchain actually used

| Tool | Version | Used for |
| ---- | ------- | -------- |
| g++ / gcc | 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1) | every build (`build.sh`, `-O2`/`-O3`/`-march=native` deep-dive builds) |
| GNU objdump (binutils) | 2.42 | `tools/disasm.sh` — the listings and the vector census (lessons 048-049) |
| taskset (util-linux) | 2.39.3 | pinning the cache measurements to one core (lesson 047) |
| libx11-dev (Xlib) | 2:1.8.7-1build1 | the seam's implementation; the window readback probe |
| Xvfb | 2:21.1.12-1ubuntu1.8 | the headless display every window check ran on |
| xdotool | 1:3.20160805.1 | scripted keyboard checks (046, 054, 056, 057) |
| mdBook | 0.5.4 (pinned) | every page render check |
| CPU | Intel Core Ultra 5 225F — L1d 48K, L2 3M, L3 20M, 64 B lines | the deep dives' worked example and every quoted measurement |

`perf` and `valgrind` are **not installed** on the authoring machine;
lesson 047's cache method is clock-based and pinned on purpose, and says
so. The authoring machine is headless by design; real-desktop variability
is routed to the port exercises (043's pattern continued: 048, 049,
057).

## Deep dives: quoted measurements, re-verified

Both dives quote real machine data; at the close of the part every
quoted measurement was **re-run at its own lesson tag** (a clean
worktree per tag, the lessons' own commands):

| Claim | Lesson | Re-verification | Result |
| ----- | ------ | --------------- | ------ |
| `-O0` copy walk: flat ~1.5 GB/s, stride 64 declining 0.5→0.2 | 047 | re-run at `lesson-047` | shape reproduces; digits within the documented noise |
| `-O3` copy walk: 102→17 GB/s sequential with knees at 48K/3M/20M; stride 64 at 3.0→0.3 | 047 | re-run at `lesson-047` | knees identical (115/58/52/15/18/13); the memory row moves most (16.9→13.2) — as the page warns |
| BlitSprite `-O0`: 18 + 20 + 34 + 4 = 76 instructions per opaque pixel, ~43 per key pixel | 048 | re-counted at `lesson-048` | exact: 18/20/34/4 |
| `movdqu` count: 0 at `-O0`, 6 at `-O3` | 049 | re-counted at `lesson-049` | exact: 6 |
| Vector census at `-O3`: Run 188, AccountFrame 14, Now 6, LoadSprite 5 | 049 | re-run at `lesson-049` | exact |
| `-march=native`: CopySequential widens to 32-byte registers | 049 | re-run at `lesson-049` | 4 vector instructions, ymm |

Timing claims move with the machine and the moment; instruction counts
and listings are deterministic and reproduced exactly. The rule the
dives' prose already carries — quote the shape, name the machine,
re-run rather than re-read — held up under the re-verification.

## Recommendation: extending the frozen prefix

The horizon still freezes `lesson-001`…`lesson-006`. Following
`plan/part1-review.md`'s discipline (no extension on authoring evidence
alone):

- **Do not extend the prefix on this review.** Part 2's 15 lessons have
  the author's verification behind them and no reviewer's pass.
- **When the review passes land, freeze in blocks, not lessons:**
  `lesson-026`…`lesson-043` first (Part 1's dependency block — Part 2
  exercises exactly those seams and they held), then
  `lesson-044`…`lesson-058` as the second block. Freezing Part 2 without
  Part 1 would freeze the things built on the unfrozen seams.
- **The deep dives are the block's tail risk.** Lessons 047-049 quote
  machine data and compiler output; a toolchain upgrade (gcc 13.3.0 →
  newer) can move every listing in 048-049 without any code changing.
  The block-freeze review should either re-run those two lessons'
  listings with the *current* toolchain or pin the toolchain in the
  README. This is the one place where "tags do not move" and "the
  numbers must be real" can collide.
- **Lesson-053's asset fix is the model for future class-2 fixes** —
  amend at the introducing lesson, rebase forward, re-tag the tail, and
  record it in the change notes. The procedure cost about ten minutes
  and left no page stale.

## Review pass status (pre-freeze)

The independent review pass over Part 2 has **not** run; this review
records the author's verification, which is its input. What a reviewer
should re-run, in order of what breaks first:

1. `./tools/check-boundary.sh`, `openspec validate`, `mdbook build`
   (seconds).
2. The closing demo headlessly (lesson-057's transcript) with pixel
   readback at a reported position — the whole part in one run.
3. The check tables: 045's blit check, 052-053's map checks, 054's
   camera scenarios, 055's twelve collision answers (each prints its
   verdict).
4. The deep dives' measurements (the re-verification table above is the
   checklist — lesson-047's two sweeps, 048's counts, 049's census).
5. Every solution patch: `git apply`, build, run, reverse — 30 patches.
6. The frame-budget cross-check: average the run's `frame N:` lines and
   compare against the printed table (the exercise in 058's review
   section shows the awk).

## Known warts carried forward (intentional, documented in-prose)

- **Frames happen when news happens** (034's pacing note) — now visible
  as game behavior: the mover's steps are all-or-nothing, and a long
  frame refuses its whole step rather than sliding to the wall
  (lesson-056, its exercise 2 names the three fixes; Part 4's hero takes
  substepping).
- **The tilemap walk redraws the world every frame** — `tilemap` owns
  the render row of the budget (lesson-058). Clipping makes off-screen
  cells cheap but not free (053's exercise 2 measured the gradient);
  culling is a named Part 5 lever.
- **The clear is the budget's unnamed 22%** (058's exercise 1's `(rest)`
  row). The final report should name it; the row's *text* must say it is
  a drawing strategy, not a subsystem.
- **The engine is byte-oriented** — a UTF-8 `Ö` is two characters to the
  layout and occupies two slots (051's check). The missing-glyph rule
  keeps the text honest; code-point awareness is out of scope for the
  whole course and the HUD uses ASCII only.
- **`ReadFile` trusts the reported size** (039's exercise 2's wart) —
  unchanged; no Part 2 asset is a virtual file, but the wart stands
  where Part 1 left it.
- **Tags `lesson-053`…`lesson-056` moved** for the tile-sheet fix. The
  volatile tail permits it and the resync command (`git checkout
  lesson-NNN -- src/`) covers learners; if the prefix freezes through
  058, this is the last such move before the freeze.
- **The closing demo's world reuses the map asset** (grown at lesson
  053 from 052's room) — the design's open question settled as *reuse*,
  not a second file. No contract changed either way.

*(Everything above is measured from the tagged lesson states; the
closing demo's run, the check tables, the deep dives' re-verification,
and every solution patch were run on the authoring machine before this
review was written.)*
