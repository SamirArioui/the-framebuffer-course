# Part 5 skeleton: L0* and lessons L1-L22

The Part 5 lesson skeleton, frozen as an M0 deliverable before lesson 1 exists,
so Part 5's needs can propagate backward into Parts 1-4
*(curriculum: Backward propagation from Part 5 — Skeleton first)*. The cap is
hard: **Part 5 contains at most 22 lessons**. New content replaces existing
scope or moves to extras; it never exceeds the cap
*(curriculum: Part 5 lesson cap)*.

## L0* — the vertical-slice capstone (closes Part 4)

**Goal:** one closing lesson where a hero walks a tilemap with the camera
following, using only finished services.

It turns "Part 4 done" into a testable moment and derisks Part 5's opening.
Backward obligation: Part 4 must end with services complete enough to make this
lesson trivial (see `plan/part1-4-backlog.md`).
*(curriculum: Capstone gates Part 4)*

## Part 5 — the game (L1-L22)

### Game assembly (L1-L15)

| # | Lesson | Goal |
| - | ------ | ---- |
| L1 | Game skeleton | Stand up the game on finished services: the update/render loop and the game-state machine (title, play, pause, death, victory). |
| L2 | Tilemap and camera | Load and draw the single scrolling map, with camera offsets following the hero. |
| L3 | Tile collision | Resolve hero and entity movement against the tilemap. |
| L4 | Hero movement | Eight-direction movement with accel/decel feel, read from input state. |
| L5 | Feedback and animation | Sprite animation and the feedback hooks the juice toolkit will drive. |
| L6 | Projectiles and two weapons | Combat projectiles driven by the two weapon types. |
| L7 | Enemy archetype tables | Data-driven enemy types held in archetype storage. |
| L8 | Enemy AI | Chase, keep-distance, and flee behaviors over the archetype tables. |
| L9 | The boss | Assemble the boss from the same behaviors plus its own pattern. |
| L10 | Waves | Spawn waves that bring the three enemy types and the boss together. |
| L11 | Juice toolkit I | Hitstop and screenshake, through the time-scale hook and camera offsets. |
| L12 | Juice toolkit II | Particle bursts and easing — the toolkit's other two effects. |
| L13 | The HUD | Bitmap text for score, health, and timers. |
| L14 | Audio integration | Music and sfx routed through the mixer's per-channel playback. |
| L15 | Screen polish | The title, pause, death, and victory screens in final form. |

The four themes design D8 names for L11-L15 (juice toolkit, HUD, audio
integration, screen polish) fill five slots because the juice toolkit spans two
lessons: its four effects split L11 (hitstop, screenshake) and L12 (particle
bursts, easing).

### The debt lesson (L16)

| # | Lesson | Goal |
| - | ------ | ---- |
| L16 | Pay the debt | Refactor is curriculum: fix forward the architectural debt Part 5 has accumulated. |

### The three-pass optimization menu (L17-L20)

| # | Lesson | Goal |
| - | ------ | ---- |
| L17 | Pass 1 — measure | Profile the game and name the top-2 hotspots. |
| L18 | Pass 2a — fix hotspot #1 | Fix the first measured hotspot with deep-dive techniques (cache layout, SIMD, allocation). |
| L19 | Pass 2b — fix hotspot #2 | Fix the second measured hotspot the same way. |
| L20 | Pass 3 — the report | Produce the final frame-budget report: the course finale. |

Pass 2 fixes exactly the two hotspots pass 1 named — nothing else
*(target-game: Optimization follows the three-pass menu)*.

### Closing (L21-L22)

| # | Lesson | Goal |
| - | ------ | ---- |
| L21 | Retrospective | Our engine vs. real ones, and the epilogue map (GPU port, Windows module). |
| L22 | Now make YOUR game | Turn the finished engine and toolkit toward a game of the learner's own. |

**Count check:** L1-L15 game assembly (15) + L16 debt (1) + L17-L20
optimization (4) + L21-L22 closing (2) = **22 lessons**.
