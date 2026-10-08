# Solution: exercise 1 — the keeper's distance is data

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 089 — enemy AI](../../lessons/part-5/lesson-089-enemy-ai.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The distance becomes a named column — `keep` — grown exactly the way
lesson 087 grew the format: the column exists, the loader defaults it
(`TABLE_KEEP_DEFAULT`, the 160 the behavior used to hard-code), and
each file decides whether to name it. `assets/enemies.txt` names it, so
every roster row states its own distance; `assets/entities.txt` stays
silent and loads byte-for-byte, its rows carrying the default:

```
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none keep 160 wave 0 count 1
engine: def spitter: x 120 y 400 facing 0 speed 96 health 3 sprite assets/spitter.ppm accel 120 damage 1 rate 30 fires shell range 0 behavior keep keep 120 wave 2 count 2
engine: def warden: x 240 y 320 facing 0 speed 72 health 6 sprite assets/golem.ppm accel 120 damage 1 rate 30 fires shell range 0 behavior keep keep 240 wave 3 count 1
```

(A new `warden` row joins the roster — a second keeper, so the run has
two of them at two distances. The rows' `keep` values sit beside their
`behavior keep`, which reads a little oddly and is honest: one names
what it does, one says how far.) `AiKeep` now reads the entity's own
number instead of `AI_KEEP`, and the closing account shows the two
keepers each at its own distance:

```
engine: world: spitter ends at 221,298 — 112 px of the hero
engine: world: warden ends at 156,403 — 232 px of the hero
```

112 px of a 120 (the band's inner edge), 232 px of a 240 — same
behavior, different numbers, no code between them.

**What the default buys a silent file.** Exactly what lesson 087
promised: `assets/entities.txt` names no `keep` column and still loads,
byte-for-byte, and its rows carry the format's default — visible right
in the report (`keep 160` on the hero's line). A file states only where
it disagrees with the format; the format's defaults are the shared
tuning. Change `TABLE_KEEP_DEFAULT` and every silent row moves with it,
without a single file being edited.
