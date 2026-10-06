# Tasks

## 1. Part 1 authoring setup

- [x] 1.1 Do the promised consistency read of lesson-025's C++ subset wording against `plan/conventions.md` §5 and the design of lesson-026's language-law section; record any wording that 026 must reconcile; verify the read is recorded in the change notes before lesson-026 is authored.
- [x] 1.2 Verify the authoring/verification toolchain for the platform batch: Xlib development headers, `./build.sh` compiling `src/`, and Xvfb-driven headless checks (a test window opens and reads back pixels); verify the exact commands run as written and record the tool versions used.

## 2. Birth and window (lesson-026…029)

- [x] 2.1 Author lesson-026 (the codebase is born: a blank `main` in `src/`, `build.sh` goes live, `sandbox/` deleted in the same code step, and the admitted C++ subset documented as the engine's language law) — prose + one code step + 1-2 "make it yours" extensions written before the prose with diff solutions linked after each prompt; verify `./build.sh` builds the born codebase, the page renders, and the co-committed commit is tagged `lesson-026`.
- [x] 2.2 Author lesson-027 (the platform seam and the first X11 window: display, window, the OS-idiom-free interface) with its extensions and diff solutions; verify the window opens under the headless check exactly as the lesson documents, the page renders, and the commit is tagged `lesson-027`.
- [x] 2.3 Author lesson-028 (the event pump: keeping the window alive and reporting close) with its extensions and diff solutions; verify the documented event behavior runs as written, the page renders, and the commit is tagged `lesson-028`.
- [x] 2.4 Author lesson-029 (clean close and error paths: OS resources released on every exit) with its extensions and diff solutions; verify the clean-shutdown and error paths behave as documented, the page renders, and the commit is tagged `lesson-029`.

## 3. Pixels (lesson-030…031)

- [x] 3.1 Author lesson-030 (the framebuffer as our own bytes — Part 0's `paint` intuition reused — and the XImage that can carry it) with its extensions and diff solutions; verify the program builds and its byte-level behavior runs as written, the page renders, and the commit is tagged `lesson-030`.
- [x] 3.2 Author lesson-031 (presentation through the platform layer: the pixels the engine wrote are the pixels the window shows) with its extensions and diff solutions; verify the presentation scenario with a headless pixel readback matching the framebuffer, the page renders, and the commit is tagged `lesson-031`.

## 4. Input state (lesson-032…034)

- [x] 4.1 Author lesson-032 (OS key events folded into polled input state) with its extensions and diff solutions; verify the polled-state behavior runs as written, the page renders, and the commit is tagged `lesson-032`.
- [x] 4.2 Author lesson-033 (latching brief presses and tracking focus: a key pressed and released within one frame is not lost) with its extensions and diff solutions; verify the latching behavior with scripted input, the page renders, and the commit is tagged `lesson-033`.
- [x] 4.3 Author lesson-034 (the first interactive frame: an arrow-key marker moved by polled input) with its extensions and diff solutions; verify the interactive behavior with scripted input, the page renders, and the commit is tagged `lesson-034`.

## 5. Clock (lesson-035…036)

- [x] 5.1 Author lesson-035 (the platform clock: monotonic time and frame delta — `snek`'s timing revisited behind the seam) with its extensions and diff solutions; verify the monotonic-clock behavior runs as written, the page renders, and the commit is tagged `lesson-035`.
- [x] 5.2 Author lesson-036 (frame time as measured data: the per-frame record Part 2 and Part 5 will read, and the honest cost of the presentation copy) with its extensions and diff solutions; verify the measured frame-time numbers quoted in the page are real, the page renders, and the commit is tagged `lesson-036`.

## 6. File I/O (lesson-037…038)

- [x] 6.1 Author lesson-037 (whole-file reads against the OS with typed failures) with its extensions and diff solutions; verify the read and failure behaviors run as written, the page renders, and the commit is tagged `lesson-037`.
- [x] 6.2 Author lesson-038 (whole-file writes and round-trips) with its extensions and diff solutions; verify the write round-trip behavior runs as written, the page renders, and the commit is tagged `lesson-038`.

## 7. Virtual memory and arenas (lesson-039…041)

- [x] 7.1 Author lesson-039 (the virtual-memory deep dive: pages, mappings, protection — concept-first with x86-64 as the worked example) with its extensions and diff solutions; verify every quoted mapping/measurement is real, the page renders, and the commit is tagged `lesson-039`.
- [x] 7.2 Author lesson-040 (reservation-backed buffers: the framebuffer's memory comes from a sized reservation) with its extensions and diff solutions; verify the reservation behavior runs as written, the page renders, and the commit is tagged `lesson-040`.
- [x] 7.3 Author lesson-041 (arenas: bump allocation over a reservation, align and rollback marks, and the sanitizer limitation taught honestly) with its extensions and diff solutions; verify the arena behavior runs as written, the page renders, and the commit is tagged `lesson-041`.

## 8. The seam and the demo (lesson-042…043)

- [x] 8.1 Author lesson-042 (the interface as a contract: what a second OS must implement, audited against the platform spec's single-boundary scenarios) with its extensions and diff solutions; verify the audit claims hold against the code as it stands, the page renders, and the commit is tagged `lesson-042`.
- [x] 8.2 Author lesson-043 (the closing demo: one measured frame loop — window + polled input + arena-backed framebuffer — as "platform layer done") with its extensions and diff solutions; verify the demo runs as documented under the headless check, the page renders, and the commit is tagged `lesson-043`.

## 9. Part 1 boundary review and integration checks

- [x] 9.1 Write `plan/part1-review.md` recording authoring velocity against the 10-20 h/week review budget from `plan/part0-review.md`, exercise counts per lesson against the conventions density table (Parts 1-2: 1-2 "make it yours" extensions), the gcc/gdb/X11 versions used, and a recommendation on extending the frozen prefix; verify every Part 1 lesson is counted and every Part 1 density expectation is checked.
- [ ] 9.2 From a clean checkout following only `README.md`: run `./build.sh`, run `mdbook build`, run `openspec validate`, and run the closing demo headlessly; verify all succeed and `git tag` shows consecutive `lesson-026`…`lesson-043` where each consecutive tag diff equals that lesson's code step.
