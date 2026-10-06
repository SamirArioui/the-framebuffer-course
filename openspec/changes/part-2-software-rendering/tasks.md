# Tasks

## 1. Part 2 authoring setup

- [x] 1.1 Verify the authoring/verification toolchain for the rendering batch: the pixel-level readback check against the framebuffer (sprite, clip, and camera claims must be checkable headlessly), asset round-trip checks through the seam's whole-file I/O, and the disassembly/measurement tooling the deep dives quote (compiler output of the blitter; a repeatable cache-measurement method); verify the exact commands run as written and record the tool versions used.
- [x] 1.2 Audit lesson-036's frame record and log against the `frame-accounting` spec; record in the change notes what Part 2 must grow (per-subsystem attribution for the frame-budget table) and verify the audit is recorded before the blitter batch is authored.

## 2. Blitter (lesson-044…046)

- [x] 2.1 Author lesson-044 (a sprite as loaded bytes: the PPM asset format defined by hand, the sprite loaded through the seam's whole-file read into the arena, and the bytes inspected) — prose + one code step + 1-2 "make it yours" extensions written before the prose with diff solutions linked after each prompt; verify the asset loads as documented and the byte-level checks run as written, the page renders, and the co-committed commit is tagged `lesson-044`.
- [x] 2.2 Author lesson-045 (the clipped, transparent blit: the one copy loop that draws a sprite into the framebuffer with color-key transparency and the fold's clipping) with its extensions and diff solutions; verify the drawn pixels match the sprite byte-for-byte and the clip and transparency cases behave as documented, the page renders, and the commit is tagged `lesson-045`.
- [x] 2.3 Author lesson-046 (the frame loop draws a sprite moved by polled input — the marker retires — and the frame record's render phase is named) with its extensions and diff solutions; verify the interactive behavior with scripted input and a pixel readback at the reported position, the page renders, and the commit is tagged `lesson-046`.

## 3. Caches (lesson-047)

- [x] 3.1 Author lesson-047 (the caches deep dive: cache lines, locality, and what the blitter's copy actually costs — concept-first with x86-64 as the worked example) with its extensions and diff solutions; verify every quoted cache measurement is real and reproducible on this machine, the page renders, and the commit is tagged `lesson-047`.

## 4. SIMD and assembly reading (lesson-048…049)

- [x] 4.1 Author lesson-048 (the blitter's compiled assembly, read instruction by instruction — what the compiler made of the copy loop) with its extensions and diff solutions; verify every quoted disassembly line is real output of the current build, the page renders, and the commit is tagged `lesson-048`.
- [x] 4.2 Author lesson-049 (the SIMD lens: vector registers and the vectorized assembly the compiler reached for, read rather than written) with its extensions and diff solutions; verify the quoted vector instructions and their measurements are real, the page renders, and the commit is tagged `lesson-049`.

## 5. Bitmap text (lesson-050…051)

- [x] 5.1 Author lesson-050 (the bitmap font as an asset: the glyph sheet's format and glyphs drawn through the blit path) with its extensions and diff solutions; verify glyphs draw pixel-exact through the blitter as documented, the page renders, and the commit is tagged `lesson-050`.
- [x] 5.2 Author lesson-051 (text on screen: strings laid out, the missing-glyph rule, and the HUD the demo will use — O3 delivered) with its extensions and diff solutions; verify the text behavior runs as written including the missing-character case, the page renders, and the commit is tagged `lesson-051`.

## 6. Tilemap and camera (lesson-052…054)

- [x] 6.1 Author lesson-052 (the tilemap asset format: dimensions, tile kinds with solidity, one character per cell — defined by hand and loaded through whole-file I/O) with its extensions and diff solutions; verify a map loads completely and a malformed file fails typed as documented, the page renders, and the commit is tagged `lesson-052`.
- [x] 6.2 Author lesson-053 (tilemap drawing: the map's cells drawn through the blit path at world positions) with its extensions and diff solutions; verify a map larger than the framebuffer draws correctly at moved offsets with a pixel readback, the page renders, and the commit is tagged `lesson-053`.
- [x] 6.3 Author lesson-054 (the camera: the base offset that scrolls the world and the additive offset the juice toolkit will drive — O2 delivered) with its extensions and diff solutions; verify base, additive, and combined offsets behave as the spec's scenarios document, the page renders, and the commit is tagged `lesson-054`.

## 7. Collision (lesson-055…056)

- [x] 7.1 Author lesson-055 (tile kinds carry solidity and the map answers queries: point and rectangle collision against solid cells) with its extensions and diff solutions; verify the collision queries' answers run as written including out-of-bounds positions, the page renders, and the commit is tagged `lesson-055`.
- [x] 7.2 Author lesson-056 (the mover that stops at walls: collision queries driving a controllable sprite over the map — the hero's precursor) with its extensions and diff solutions; verify the movement and blocking behavior with scripted input, the page renders, and the commit is tagged `lesson-056`.

## 8. The closing demo and the measured report (lesson-057…058)

- [x] 8.1 Author lesson-057 (the closing demo: one measured frame loop drawing a scrolling map with a movable sprite and text — every Part 2 capability at once) with its extensions and diff solutions; verify the demo runs as documented under the headless check with pixel readback, the page renders, and the commit is tagged `lesson-057`.
- [x] 8.2 Author lesson-058 (the frame-budget table's first draft: the frame account attributed per subsystem — O1's payoff and the format Part 5's finale grows) with its extensions and diff solutions; verify the reported attribution numbers are real measurements of the demo's frames, the page renders, and the commit is tagged `lesson-058`.

## 9. Part 2 boundary review and integration checks

- [ ] 9.1 Write `plan/part2-review.md` recording authoring velocity against the 10-20 h/week review budget from `plan/part0-review.md`, exercise counts per lesson against the conventions density table (Parts 1-2: 1-2 "make it yours" extensions), the toolchain versions used, the deep dives' quoted measurements re-verified, and the frozen-prefix recommendation updated per `plan/part1-review.md`; verify every Part 2 lesson is counted and every Part 2 density expectation is checked.
- [ ] 9.2 From a clean checkout following only `README.md`: run `./build.sh`, run `mdbook build`, run `openspec validate`, and run the closing demo headlessly; verify all succeed and `git tag` shows consecutive `lesson-044`…`lesson-058` where each consecutive tag diff equals that lesson's code step.
