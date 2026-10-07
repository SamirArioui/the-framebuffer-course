# Solution: exercise 2 — The table written back

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 071 — the archetype table](../../lessons/part-4/lesson-071-table.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The writer is the reader's inverse, and the patch builds it the way every
loader in this engine is built: two small `Put` helpers in the anonymous
namespace beside the `Read` helpers, then one function that assembles a
file. Where `ReadInt` takes digits off the line, `PutInt` lays digits
down — remainders into a small buffer, then written back to front, which
is the same arithmetic lesson 052's reader ran in the other direction.
Where `ReadText` copies a run of non-space bytes into a field, `PutText`
copies the field's bytes out. If those two do not agree on what a value
*is*, nothing else in the format can save them.

`WriteTable` writes the lesson's walk in order: the header this lesson's
format names, one space between the column names and a newline after the
last, then one row per definition — name, x, y, facing, speed, health,
sprite — each value followed by the separator the reader's tokenizer
skips. Every field a reader will check is written to pass that check: the
writer is not a second opinion about the format, it is the same opinion
in the other direction. Note that the writer picks one layout — the
format's canonical column order — while the reader accepts any order the
header names; they agree on the *fields*, which is what "the format"
means.

One habit worth noticing: the assembled bytes are pure scratch, so the
mark and the rollback bracket them — kept or refused, the arena does not
keep the file image. `platform::WriteFile` takes the bytes before the
rollback, exactly as `ReadFile`'s bytes go back with `ReleaseFile`.

The round trip in `Run` writes the loaded table to `assets/entities.txt`'s
sibling, `assets/table-roundtrip.txt`, reads it back through `LoadTable`,
and compares every field of every definition — naming the first field that
disagrees. From a real run of this lesson's end state plus the patch:

```
engine: table: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: round trip: 2 definitions written and read back, every field agrees
```

And the file itself, compared against the course's asset outside the run
(`cmp assets/table-roundtrip.txt assets/entities.txt`): **byte-identical**
— header and rows, all of it. The writer did not just produce a file the
loader tolerates; it produced *this* file.

The disagreeing path is real too, and cheap to see: make the writer put
`def.health + 1` into the image instead of `def.health`, rebuild, and the
run says `engine: round trip: definition 0 disagrees on health` — the
first field that differs, named, with the definition it belongs to. That
probe is not part of the patch; it is the five-minute check that the
comparison is comparing.

That is what the round trip proves that a single read cannot. A read
shows one implementation of the format meeting one file; the round trip
puts two implementations across a real file and checks they agree on
every field. A hexdump of the written file would not catch a digit-order
bug in `PutInt` (the file would still "look" like a table); comparing
240 against 240 catches it at once. When you later write a table the
loader refuses — put the values one column off the header, write a facing
of 4 — the two halves argue in public, and the typed failure names which
claim lost.

Nothing here touches the loader, the parse, or the format: the round trip
is a check beside the load, and the file it writes is yours to keep or
delete.
