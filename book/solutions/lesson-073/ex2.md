# Solution: exercise 2 — Every definition, an entity

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 073 — entities as rows](../../lessons/part-4/lesson-073-rows.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff replaces the by-name request for `hero` with a walk over the
table: one `EntityFromDef` per row, one report line per entity. The
run's own `TableFind(table, "dragon")` check stays — the typed failure
is not about *how many* definitions the game wants, it is about asking
for one the table does not hold.

The run, from this lesson's end state plus the patch:

```
engine: table: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: entity hero: x 312 y 232 facing 0 speed 240 health 3 sprite 16x16
engine: entity slime: x 400 y 320 facing 2 speed 96 health 1 sprite 16x16
engine: table: "dragon" -> unknown
```

Now the part that makes the exercise worth doing. Add one row to
`assets/entities.txt` —

```
bat 96 320 1 120 2 assets/sprite.ppm
```

— and run again **without rebuilding**:

```
engine: entity hero: x 312 y 232 facing 0 speed 240 health 3 sprite 16x16
engine: entity slime: x 400 y 320 facing 2 speed 96 health 1 sprite 16x16
engine: entity bat: x 96 y 320 facing 1 speed 120 health 2 sprite 16x16
engine: table: "dragon" -> unknown
```

What in the run's code changed to make the bat appear? **Nothing.** The
engine's source was not touched and not recompiled; the third entity's
facts — its position, its facing, its speed, its health — came from a
file. That is the requirement this whole part exists for, made visible in
two runs: *changing a table's values changes the game's behavior without
recompiling the engine*, and the engine's own code holds no per-type copy
of those values to get out of date. Search `src/` for the bat's speed and
it is not there.

Two things this exercise does *not* remove. `TableFind` still has its
place — the game wants the `hero` row specifically when the player is
handed one entity to control, and "one of each" is not that request. And
the by-name request's failure path is still worth having in the run: a
table whose `hero` row is missing is a named failure, not a run that
grows its own hero. The loop and the lookup are two different questions
about the same data.

Where this goes next is lesson 074: two locals and a loop are fine for a
report and useless for a game. The entities the loop creates are dropped
on the floor — there is nowhere to *keep* them — and the fixed store is
exactly the answer to "the table lists the kinds; where do the live ones
live?"

Nothing here touches the loader or the parse: the probe is one loop
replacing one lookup.
