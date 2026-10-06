# Solution: exercise 1 — The failure that names itself

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 052 — the tilemap asset format](../../lessons/part-2/lesson-052-tilemap.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch splits `TILE_MALFORMED` into four named cases and tracks the
*stage* the parse was in when it gave up — a single `stage` counter that
only advances while the load still holds, then a mapping from stage to
enum value. The run's switch reports each case in its own words. Every
corrupt copy from the lesson, re-run:

```
bad-header: engine: assets/map.txt: the first line is not width height kinds
bad-row:    engine: assets/map.txt: the rows are wrong (length, count, or an unknown character)
bad-cell:   engine: assets/map.txt: the rows are wrong (length, count, or an unknown character)
bad-rows:   engine: assets/map.txt: the rows are wrong (length, count, or an unknown character)
trailing:   engine: assets/map.txt: there is content after the last row
```

Each report names its stage — and notice the lesson's corrupt copies
already group themselves: the short row, the unknown character, and the
missing rows all surface in the rows stage, because that is where the
parse *discovered* each of them.

Which is exactly the boundary question the prompt asks. A row using a
character the table never defined is detected while reading a **row** —
but the mistake was written in the **table** (a kind line missing, or a
row typed before the kind that names it). The loader can only report
where it stopped; the human has to decide who wrote the bug. The honest
statement of the limit: *the report names the stage of detection, not the
stage of cause.*

The next step up — and a fine follow-on exercise — is location-level
diagnostics: `row 7, column 5: character '?' names no kind`, `row 4: 19
characters, expected 20`. That is what real asset pipelines grow into,
and the parse loops already carry every number needed (`y`, `x`, `len`).
This course stops at stage-level names because the file is 20 lines long
and the stage *plus a text editor* finds the bug in seconds. When your
maps are 20,000 lines, you will want the coordinates — and the loader's
shape will not change to get them.

One design detail worth keeping from the patch: the `stage` counter must
only advance **while `ok` is still true**. The first version of this
exercise advanced it unconditionally — and every failure reported
"content after the last row", because the counter had marched on past the
wreck. State machines lie when their state moves without their
conditions; this one is two `if`s away from honest.
