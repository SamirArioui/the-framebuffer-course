# Tasks

## 1. Part 5 authoring setup

- [x] 1.1 Verify the authoring/verification toolchain for the game's batch: the headless display and scripted input the game's checks will need, the build and the boundary check with the sources this part grows, and the profiling/measurement tooling the three-pass optimization will sit on (the frame account plus whatever profiler the measure pass uses); verify the exact commands run as written and record the tool versions used.
- [x] 1.2 Audit the engine and the game's data against the six deltas and the MVD checklist: record in the change notes what Part 5 must grow (the game layer beside the services; the table format's named columns and the compatibility rule that keeps every shipped file loading; the game's own tables) and verify the audit is recorded before the first game lesson is authored.

## 2. The game's shape (lesson-082…086)

- [x] 2.1 Author lesson-082 (L1, the game skeleton: the game standing on finished services — the update/render loop and the game-state machine of title, play, pause, death, victory) with its exercises and diff solutions written before the prose; verify one state at a time with each state's screen and input, the transitions named as documented, and the simulation standing still outside play while the presentation keeps drawing, the page renders, and the co-committed commit is tagged `lesson-082`.
- [x] 2.2 Author lesson-083 (L2, the tilemap and camera: the single scrolling map loaded and drawn, the camera's offsets following the hero) with its exercises and diff solutions; verify the map draws and the camera follows and clamps to the map's bounds under scripted input as documented, the page renders, and the commit is tagged `lesson-083`.
- [x] 2.3 Author lesson-084 (L3, tile collision: hero and entity movement resolved against the tilemap) with its exercises and diff solutions; verify the hero and an entity stop at solid tiles and slide along walls under scripted input as documented, the page renders, and the commit is tagged `lesson-084`.
- [x] 2.4 Author lesson-085 (L4, hero movement: eight directions with accel/decel feel) with its exercises and diff solutions; verify the hero accelerates from rest and decelerates to rest under scripted input as documented and that the diagonal covers ground at the straight-line speed, the page renders, and the commit is tagged `lesson-085`.
- [x] 2.5 Author lesson-086 (L5, feedback and animation: sprite animation and the feedback hooks the toolkit will drive) with its exercises and diff solutions; verify the animation advances and the hooks fire and rest as documented, the page renders, and the commit is tagged `lesson-086`.

## 3. Combat and enemies (lesson-087…091)

- [x] 3.1 Author lesson-087 (L6, projectiles and two weapons: the table format grown by named columns, weapons as rows, projectiles as entities) with its exercises and diff solutions; verify every shipped table file still loads unchanged (`assets/entities.txt` byte-for-byte), each weapon fires the projectile kind its row states, projectiles retire at walls and at the entities they hit, and a hit reduces health by the row's damage as documented, the page renders, and the commit is tagged `lesson-087`.
- [x] 3.2 Author lesson-088 (L7, enemy archetype tables: the three types and the boss as rows of the table) with its exercises and diff solutions; verify every enemy carries its row's values as documented and that no per-type copy of the attributes appears in the code, the page renders, and the commit is tagged `lesson-088`.
- [x] 3.3 Author lesson-089 (L8, enemy AI: chase, keep-distance, and flee over the entity services) with its exercises and diff solutions; verify each behavior moves its entity through the mover as documented under scripted runs, the page renders, and the commit is tagged `lesson-089`.
- [x] 3.4 Author lesson-090 (L9, the boss: assembled from the shared behaviors plus a pattern of its own) with its exercises and diff solutions; verify the boss composes the three behaviors and its own pattern as documented with no separate movement machinery, the page renders, and the commit is tagged `lesson-090`.
- [x] 3.5 Author lesson-091 (L10, waves: waves that bring the three types and the boss together) with its exercises and diff solutions; verify a wave spawns its composition from the table's definitions and the next wave begins when the last entity of the current one is retired, as documented, the page renders, and the commit is tagged `lesson-091`.

## 4. Feel and polish (lesson-092…096)

- [ ] 4.1 Author lesson-092 (L11, hitstop and screenshake: through the game-time scale and the camera's additive offset) with its exercises and diff solutions; verify hitstop slows the simulation to a fraction and returns to full speed on its own wall-time deadline, and the shake's offset rests at exactly zero, as documented, the page renders, and the commit is tagged `lesson-092`.
- [ ] 4.2 Author lesson-093 (L12, particle bursts and easing: particles as entities and eased arrivals) with its exercises and diff solutions; verify a burst is bounded by the store's policy and its particles retire, a full store drops particles while gameplay spawns are kept, and eased values arrive exactly at their targets, as documented, the page renders, and the commit is tagged `lesson-093`.
- [ ] 4.3 Author lesson-094 (L13, the HUD: score, health, and timers) with its exercises and diff solutions; verify the readouts reflect the game's actual state in the same frame and the HUD stays in place over the world as the camera moves, as documented, the page renders, and the commit is tagged `lesson-094`.
- [ ] 4.4 Author lesson-095 (L14, audio integration: music and sfx routed through the mixer's channels) with its exercises and diff solutions; verify the music and effects reach the channels and the stream as documented, the page renders, and the commit is tagged `lesson-095`.
- [ ] 4.5 Author lesson-096 (L15, screen polish: the title, pause, death, and victory screens in final form) with its exercises and diff solutions; verify each screen renders and its input acts as documented, the page renders, and the commit is tagged `lesson-096`.

## 5. Pay the debt (lesson-097)

- [ ] 5.1 Author lesson-097 (L16, the debt lesson: fix-forward the structural debt the assembly accumulated — "refactor is curriculum") with its exercises and diff solutions; verify the refactor keeps the game's behavior unchanged (the run's reports and the checklist's demonstrations re-run as documented), the build stays warning-free and the boundary check clean, the page renders, and the commit is tagged `lesson-097`.

## 6. The three-pass optimization (lesson-098…101)

- [ ] 6.1 Author lesson-098 (L17, pass 1 — measure: instrument, profile the game, and name the top-2 hotspots) with its exercises and diff solutions; verify the profile names the top-2 hotspots with numbers measured from real frames, the page renders, and the commit is tagged `lesson-098`.
- [ ] 6.2 Author lesson-099 (L18, pass 2a — fix hotspot #1 with the deep dives' techniques) with its exercises and diff solutions; verify the change addresses the named hotspot and the measured cost falls as documented, with the game's behavior unchanged, the page renders, and the commit is tagged `lesson-099`.
- [ ] 6.3 Author lesson-100 (L19, pass 2b — fix hotspot #2 the same way) with its exercises and diff solutions; verify the change addresses the named hotspot and the measured cost falls as documented, with the game's behavior unchanged, the page renders, and the commit is tagged `lesson-100`.
- [ ] 6.4 Author lesson-101 (L20, pass 3 — the final frame-budget report: the course finale) with its exercises and diff solutions; verify the report attributes per-frame time to each major subsystem from real frames of the finished game and that the 60 fps line is checked as far as this machine honestly measures, the page renders, and the commit is tagged `lesson-101`.

## 7. Closing (lesson-102…103)

- [ ] 7.1 Author lesson-102 (L21, the retrospective: our engine against real ones, and the epilogue map) with its exercises and diff solutions; verify the page renders and records what is post-course material rather than scope, and the commit is tagged `lesson-102`.
- [ ] 7.2 Author lesson-103 (L22, now make YOUR game: the finished engine and toolkit turned toward a game of the learner's own) with its exercises and diff solutions; verify the lesson hands the engine over as documented and the page renders, and the commit is tagged `lesson-103`.

## 8. Part 5 boundary review and integration checks

- [ ] 8.1 Write `plan/part5-review.md` recording authoring velocity against the 10-20 h/week review budget from `plan/part0-review.md`, exercise counts per lesson against the conventions density table (Part 5: fewer, larger — at most 2), the toolchain versions used, the measured claims re-verified, and the MVD's frozen checklist completed line by line with its definition of done; verify every Part 5 lesson is counted and every checklist line is checked.
- [ ] 8.2 From a clean checkout following only `README.md`: run `./build.sh`, run `mdbook build`, run `openspec validate`, and run the finished game headlessly; verify all succeed and `git tag` shows consecutive `lesson-082`…`lesson-103` where each consecutive tag diff equals that lesson's code step.
