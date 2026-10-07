# Solution: exercise 1 — The table with a hole in it

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 072 — the load, complete or named](../../lessons/part-4/lesson-072-load.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is one file and one load: `assets/holed.txt`, the two
definitions with a blank line between them, loaded beside the course's
own table. The probe prints what the loader answered, how many rows it
handed over, and the arena's used count on both sides of the load — the
three things the exercise asks you to predict, in one line.

The predictions, before any run. **The verdict is `TABLE_MALFORMED`.**
The format's rows end at the first blank line and the tail after them may
hold blank lines and nothing else; the `slime` line after the hole is a
row where a tail belongs, and the loader refuses the file rather than
deciding whether that row belongs to this table or another one. **Zero
definitions are handed over** — a refused load hands over nothing, not
the part that parsed. **The arena is unchanged across the load** — the
mark goes down before the rows are taken and the refusal rolls back to
it.

The run, from this lesson's end state plus the patch:

```
engine: table: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: holed: malformed, 0 rows, arena 1252040 -> 1252040
```

All three hold. Now the walk that decided each answer, which is the
part worth being able to say out loud:

- **The counting walk** looked at the header's line and then at the rows,
  and stopped at the blank line. It counted **one** row — `hero` — and
  asked the arena for exactly one definition's bytes. It never saw
  `slime` as a row; for the count, the file's rows ended at the hole.
- **The fill walk** parsed the header and filled the one row it was
  promised, then stopped at the same blank line. It is the tail check
  after it — `trailing blank lines, and nothing else` — that met the
  `slime` line and said no. That is the verdict: not "a row is missing",
  not "the file is short", but the format's rule about what may follow
  the rows.
- **The rollback** is what makes the third number the same as the
  second-to-last one. The one row the fill wrote was real bytes in the
  arena for a moment; the refusal returned them.

That last line is the reason the probe prints the arena at all. Without
the mark and the rollback the report would read `arena 1252040 ->
1252140` — the hundred bytes of a definition from a file the loader
*said* it refused, kept for the rest of the run and counted in every
arena report after it. One refused load would not end the game; a
thousand of them (a loading screen retrying a broken file, a level
editor probing candidates) would be the game's memory, gone.

What the exercise does *not* change is the good file's verdict or the
game: `assets/entities.txt` loads exactly as it did, and the run's own
report is untouched. The probe is a check beside the load.
