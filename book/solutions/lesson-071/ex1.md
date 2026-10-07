# Solution: exercise 1 — The header, reordered

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 071 — the archetype table](../../lessons/part-4/lesson-071-table.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is two files and one refactor: `assets/reordered.txt` (the two
definitions under a header in a different order) and `assets/swapped.txt`
(the same, with the hero row's `3` and `240` exchanged), beside the
course's own table. The report moves out of `Run` into `PrintTable` —
same line, same fields — so one run prints all three tables and the
difference is a comparison of three blocks of output instead of three
runs.

The prediction, before any run. The loader does not read "the first value
is the name": it reads the header, finds which field each column names,
and fills the fields in the header's order. So in `assets/reordered.txt`
the hero's row `240 hero assets/sprite.ppm 3 232 0 312` lands exactly
where the original row lands — `240` is under `speed`, `3` is under
`health`, `232` is under `y` — and the report must be *identical*, line
for line, to the report for `assets/entities.txt`. Same fields, same
values, nothing reordered in the output because the output prints fields,
not columns.

The swapped row is the one to think about twice. `3 hero
assets/sprite.ppm 240 232 0 312` is a well-formed row: seven values,
`3` and `240` are both whole numbers, the facing is still 0, the name is
still unique in the file. The loader has no opinion about whether a
health of 240 is *plausible* — that is the game's business, not the
format's. So nothing fails, the run reports `speed 3 health 240`, and
the hero in the report is a slower, much healthier one.

The runs, from this lesson's end state plus the patch:

```
engine: table: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: reordered: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: swapped: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 3 health 240 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
```

Prediction one holds exactly: the reordered table's report is identical
character for character. Prediction two holds as predicted: the swapped
values are carried by their columns, and the only thing the loader
guarantees is that each value is what its column *requires* — a whole
number where a number is required — not what the game would want there.

That distinction is the exercise's point. The format's checks are
syntactic and cheap — types, counts, uniqueness, bounds — and they are
exactly the checks a parser can make without knowing what a game is. A
value that is well-typed but wrong for the game (a health of 240, a speed
of 3) sails through, which is why the report prints every field: the
byte-level check against the file is yours to make, and it is the only
check that catches the well-typed lie. When a table grows a column you
did not expect to tune — the boss's health, a projectile's speed — the
file is where you look first, and the run's report is how you look.

Nothing here touches the loader, the parse, or the format: the probe is
three loads and three reports beside the lesson's own, and the two files
it reads are yours to keep or delete.
