# Solution: exercise 2 — The acceptance table

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 057 — the closing demo](../../lessons/part-2/lesson-057-demo.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch makes the demo name what it uses — one line per capability
group, printed before the first frame:

```
engine: drawing: one blit for sprites, glyphs, and tiles; clipping and transparency included
engine: text: strings laid out one slot per character, missing glyphs skipped
engine: world: the map loaded whole, drawn through the camera's summed offset
engine: input: polled movement, collision-gated per axis; the camera follows, the additive shakes
engine: measurement: one record per frame, render attributed per subsystem
```

Those five lines are the table's left column. The rest is yours to fill —
and the shape that makes it worth keeping is this:

| Spec scenario | Lesson | Evidence from a run |
| ------------- | ------ | ------------------- |
| Sprite pixels drawn unchanged | 045 | `blit check: 130 opaque pixels drawn unchanged, 0 mismatches` |
| Transparency writes nothing | 045 | `126 key pixels wrote nothing over the background` |
| Off-screen drawing is clipped | 045 | `clip at -4,-4 landed 144 pixels … 0 touched outside` |
| Text is drawn from the font | 050-051 | `font check: glyph 'A' … 0 mismatches`; the HUD on screen |
| Missing characters do not corrupt layout | 051 | `ink pixels per slot: 18, 0, 0, 23` |
| The map appears at the camera offset | 053-054 | `tilemap check … 0 mismatches`; `camera check: base (100,50) scrolls the scene` |
| Scrolling keeps the map intact | 053 | `map 768x512 px over frame 640x480 — 274365 pixels compared, 0 mismatches` |
| The additive offset stacks / clears | 054 | `additive (7,-3) stacks over base … 0 mismatches` / `restores the base view` |
| A map loads completely | 052 | `1536 cells, 0 unknown` (counts close against the file) |
| Missing or malformed file is a typed failure | 052, 044 | `not a complete map`, `missing or unreadable` — by name |
| A solid tile reports overlap | 055 | `collision check: 12 of 12 answers as documented` |
| Empty region reports no overlap | 055 | same table, the free rows |
| Out-of-bounds queries have a defined answer | 055 | the three policy rows — solid, documented |
| Kinds' solidity alone decides | 055 | floor vs pillar rows, no drawing code consulted |
| Per-frame record / log / account | 036, 046+ | every `frame N:` line; the account's per-subsystem sums |
| Measurement behind the boundary | 036 | `check-boundary.sh` — no OS timing in engine code |

The last question — which rows break first when the engine changes — is
where the table earns its keep:

- **The pixel-exact rows break first under any drawing change.** Touch
  the blit's clip, the key check, or the RGB→BGR swap and lessons 045,
  050, 053's checks go red immediately. That is deliberate: those rows
  are cheap to re-run and precise about what broke.
- **The layout rows break silently.** A missing-glyph or camera change
  rarely crashes anything — the screen just subtly disagrees with the
  spec. The slot maps and the two-draw comparisons exist because
  "looks fine" is not a check.
- **The account rows drift rather than break.** Numbers move with the
  machine and the build; what must not change is the *shape* and the
  format. The rows to watch are the ones that would corrupt the data
  Part 5 reads.

How would you notice? Exactly the way this course has done it since
lesson 001: the check is a command, its output is a number, and the
numbers are re-run — not re-read — when the engine changes.
