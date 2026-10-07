# Solution: exercise 1 — Two heroes, one row

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 073 — entities as rows](../../lessons/part-4/lesson-073-rows.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is a second creation and one act of vandalism: `twin` from the
same `hero` definition, then the first entity's `health` set to 0 and its
`x` set to 0, and one report naming all three sources at once — the
twin's fields, the hero's fields, and the row's fields.

The prediction, before any run. The twin was created *before* the
vandalism, and `EntityFromDef` copies every attribute — so the twin keeps
what the row stated at its creation: `x 312`, `health 3`. The hero is the
one that was written to: `x 0`, `health 0`. And the definition — the
table's row — still says `x 312`, `health 3`, because nothing in this
lesson writes *to* a definition. Three answers, two of them identical,
and only one of them moved.

The run, from this lesson's end state plus the patch:

```
engine: entity hero: x 312 y 232 facing 0 speed 240 health 3 sprite 16x16
engine: twin hero: x 312 y 232 health 3 (hero at x 0 health 0; the row says x 312 health 3)
engine: table: "dragon" -> unknown
```

Exactly as predicted. The twin's line is the whole answer in one string:
two entities from one row, one of them changed, and the row untouched.

What makes this the design and not an accident is worth saying once more,
because it is the reason `EntityFromDef` copies instead of pointing. The
game *writes* entity fields — the hero's position every frame its player
holds a key, its health every time something lands — and those writes
must land on one entity's life, not on the definition every entity of
that kind is created from. A view onto the row would make moving one hero
move the twins, and would quietly rewrite the file's stated values while
the run went on; the table would stop being data and become a shared
variable. With copies, the table is the past — what the file said — and
the store's rows are the present. Lesson 074 makes "the store" real;
this lesson's two locals are the whole of it.

One field is *not* copied by value and that is deliberate too: the
`sprite` pointer. Every hero shares the art, because art is loaded once
and read-only — the definition carries it (the run loaded it at startup)
and the entity points at it. Copies where the game writes, sharing where
it does not.

Nothing here touches the loader, the table's parse, or the game's loop:
the probe is one more creation and one report beside the lesson's own.
