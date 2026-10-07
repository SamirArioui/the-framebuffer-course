# Tasks

## 1. Part 4 authoring setup

- [x] 1.1 Verify the authoring/verification toolchain for the services batch: the headless display and scripted input the vertical slice's check will need, the build and the boundary check with the sources this part grows, and the whole-file read the table loader will sit on; verify the exact commands run as written and record the tool versions used.
- [x] 1.2 Audit the engine and the frame record against the `entity-tables`, `entities`, and `game-time` deltas: record in the change notes what Part 4 must grow (the table loader beside the other asset loaders; the fixed entity store; the game-time scale on the update's step) and verify the audit is recorded before the first services lesson is authored.

## 2. The table as data (lesson-071…072)

- [x] 2.1 Author lesson-071 (the archetype table defined by hand: a header naming its columns, one row per definition, parsed byte by byte like the other formats in this course) — prose + one code step + 1-2 mixed exercises written before the prose with diff solutions linked after each prompt; verify a table loads completely and every definition carries the values its row states, the page renders, and the co-committed commit is tagged `lesson-071`.
- [x] 2.2 Author lesson-072 (tables loaded whole into the arena with typed failures: a missing, malformed, or wrong-shaped file refused, nothing partial kept) with its exercises and diff solutions; verify missing and malformed files fail typed as documented including a row with the wrong field count and a value where a number is required, the page renders, and the commit is tagged `lesson-072`.

## 3. Entity storage (lesson-073…075)

- [x] 3.1 Author lesson-073 (entities as rows: an entity created from a definition carries that definition's identity and attributes in named fields the game reads directly) with its exercises and diff solutions; verify an entity created from a definition carries the table's values as documented, the page renders, and the commit is tagged `lesson-073`.
- [x] 3.2 Author lesson-074 (one fixed store: capacity decided up front, creation takes the first free slot, and a full store refuses the request as a typed value — it never steals a live entity) with its exercises and diff solutions; verify the busy case refuses as documented under scripted creation and that creating entities allocates nothing, the page renders, and the commit is tagged `lesson-074`.
- [x] 3.3 Author lesson-075 (iteration and lifetime: every live entity walked exactly once per frame, retirement frees a slot, and the freed slot is reused before any never-used one) with its exercises and diff solutions; verify iteration and slot reuse behave as documented including an entity retired during the walk, the page renders, and the commit is tagged `lesson-075`.

## 4. The hero (lesson-076…077)

- [ ] 4.1 Author lesson-076 (the hero as the first entity: a row carrying position, sprite, and a speed, moved by polled input state and drawn through the camera) with its exercises and diff solutions; verify the hero moves under scripted input as documented, the page renders, and the commit is tagged `lesson-076`.
- [ ] 4.2 Author lesson-077 (movement resolved against the tilemap's collision queries — the mover habit applied to an entity, so the hero stops at walls) with its exercises and diff solutions; verify the hero stops at solid tiles and slides along walls as documented under scripted input, the page renders, and the commit is tagged `lesson-077`.

## 5. Game-time (lesson-078…079)

- [ ] 5.1 Author lesson-078 (the game-time scale: one knob the game sets — pause sets 0, hitstop a fraction, play sets full speed — and the simulation's step is the wall-clock step scaled) with its exercises and diff solutions; verify the step scales as documented at 0, at a fraction, and at full speed, the page renders, and the commit is tagged `lesson-078`.
- [ ] 5.2 Author lesson-079 (the scale reaches the simulation's step and nothing else: the platform clock stays the measurer and the frame record keeps wall-clock durations at any scale) with its exercises and diff solutions; verify the frame record's phases are wall-clock with the scale at 0 as documented, the page renders, and the commit is tagged `lesson-079`.

## 6. Close (lesson-080…081)

- [ ] 6.1 Author lesson-080 (L0\*, the vertical slice: a hero walks the tilemap with the camera following, using only finished services — every Part 4 capability at once, and nothing new invented) with its exercises and diff solutions; verify the slice runs as documented under the headless check with the hero's position and the camera's base reconciled against the map's bounds, the page renders, and the commit is tagged `lesson-080`.
- [ ] 6.2 Author lesson-081 (the slice's cost in the frame record: the update phase's attribution to entity work, measured from real frames rather than guessed) with its exercises and diff solutions; verify the reported numbers are real measurements of the slice's frames, the page renders, and the commit is tagged `lesson-081`.

## 7. Part 4 boundary review and integration checks

- [ ] 7.1 Write `plan/part4-review.md` recording authoring velocity against the 10-20 h/week review budget from `plan/part0-review.md`, exercise counts per lesson against the conventions density table (Parts 3-4: 1-2 mixed exercises), the toolchain versions used, the measured claims re-verified, and the frozen-prefix recommendation updated per `plan/part3-review.md`; verify every Part 4 lesson is counted and every Part 4 density expectation is checked.
- [ ] 7.2 From a clean checkout following only `README.md`: run `./build.sh`, run `mdbook build`, run `openspec validate`, and run the vertical slice headlessly; verify all succeed and `git tag` shows consecutive `lesson-071`…`lesson-081` where each consecutive tag diff equals that lesson's code step.
