# Parts 1-4 backlog: backward obligations from the MVD

The frozen MVD (`plan/target-game-mvd.md`) propagates backward: before Part 1
is authored, these obligations are the capabilities Parts 1-4 must deliver so
Part 5 can be assembly rather than invention
*(curriculum: Backward propagation from Part 5)*. Each obligation names its
home part, what it must deliver, and the MVD checklist lines it serves.

Part 1's platform baseline — window, **input state**, **timing**, and **file
I/O** against the OS (design D2) — is the part's own definition, not a derived
obligation; the obligations below build on it.

## The eight obligations

| # | Obligation | Home part | Must deliver | Serves (MVD line) |
| - | ---------- | --------- | ------------ | ----------------- |
| O1 | **Frame-timing instrumentation** | Part 2, early — on Part 1's clock | Per-frame timing hooks and a frame-time log that the profiler lesson (L17) and the final frame-budget report (L20) both read | perf; hero (accel/decel is dt work); juice (easing rides on frame time) |
| O2 | **Camera offsets** | Part 2 (viewport offsets); Part 4 leaves the feedback hook | A framebuffer viewport offset that scrolls the map, plus an additive camera-offset hook the juice toolkit drives | world; juice (screenshake); hero (capstone camera) |
| O3 | **Bitmap text** | Part 2 | Glyph rendering from a bitmap font onto our own framebuffer | states (title, pause, death, victory screens) |
| O4 | **Tilemap + collision** | Part 2 — the sprite pipeline covers tilemaps | A tilemap asset format, tilemap drawing, and tile-level collision queries that L3 turns into resolution | world |
| O5 | **Time-scale hook** | Part 4 (services) | A game-time scale the game can set: pause sets 0, hitstop sets a fraction | juice (hitstop); states (pause) |
| O6 | **Archetype storage** | Part 4 (entity storage) | Data-driven entity/asset tables that serve 3 enemy types, the boss, projectiles, and particle bursts | enemies; combat; juice (particles) |
| O7 | **Per-channel audio** | Part 3 (our own mixer) | Mixer with per-channel playback: music and sfx routed as channels | audio |
| O8 | **L0\* services gate** | Part 4 (closing capstone) | Services complete enough that the vertical slice is trivial: a hero walks a tilemap with the camera following | hero; world |

## Checklist coverage (verification)

Every frozen MVD checklist line maps to at least one obligation:

| MVD checklist item | Obligations |
| ------------------ | ----------- |
| hero: 8-dir movement with accel/decel feel | O1, O8 |
| combat: projectiles, 2 weapon types | O6 |
| enemies: 3 types + 1 boss (chase / keep-distance / flee AI) | O6 |
| world: single scrolling map, tile collision | O4, O2, O8 |
| states: title, play, pause, death, victory | O3, O5 |
| juice toolkit: hitstop, screenshake, particle burst, easing | O5, O2, O6, O1 |
| audio: music + sfx through our own mixer | O7 |
| perf: 60 fps on modest hardware + final frame-budget report | O1 |

Each obligation also has a part home before Part 5 opens, so the capstone gate
O8 can be checked at the end of Part 4: if the vertical slice is not trivial
there, a service is missing and Part 4 is not done.
