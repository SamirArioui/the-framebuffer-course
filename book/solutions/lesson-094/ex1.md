# Solution: exercise 1 — the score for the kills

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 094 — the HUD](../../lessons/part-5/lesson-094-hud.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The format grows exactly the way lesson 087 grew it — once, by named
columns, additively. `points` joins the column list; `EntityDef` and
`Entity` carry it like every other row value; the default
(`TABLE_POINTS_DEFAULT = 0`) is the format's contract for every file
that never names the column. The enemies' rows price their kinds, and
the header names the new column the way the format's headers always do:

```
name x y facing speed health sprite damage rate fires behavior wave count points
bat 560 72 2 160 2 assets/bat.ppm 1 20 bolt chase 1 2 100
wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 2 1 150
spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2 250
golem 384 96 1 72 8 assets/golem.ppm 2 20 shell boss 3 1 1000
```

**The compatibility rule holds.** `assets/entities.txt` is untouched —
and the run shows every shipped file loading at the default its header
never names, beside the rows that do name it:

```
engine: table assets/entities.txt: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none wave 0 count 1 points 0
engine: def bat: x 560 y 72 facing 2 speed 160 health 2 sprite assets/bat.ppm … wave 1 count 2 points 100
engine: def wisp: x 640 y 336 facing 2 speed 120 health 1 sprite assets/wisp.ppm … wave 2 count 1 points 150
engine: def spitter: x 120 y 400 facing 0 speed 96 health 3 sprite assets/spitter.ppm … wave 2 count 2 points 250
engine: def golem: x 384 y 96 facing 1 speed 72 health 8 sprite assets/golem.ppm … wave 3 count 1 points 1000
```

`points 0` for the hero and the slime and every weapon and projectile
row — the files that shipped in lesson 071 load byte-for-byte, their
unnamed field at the format's default. That is the whole promise of
growing by named columns: the data moves, the loader does not.

**The kill earns its row's points** in the flight's death branch — the
same line where the death retires — and the score is the game's, handed
to the flight to add to. The scratch roster (one fragile kind priced at
500, the numbers its own) shows the score moving in the kill's own
frame:

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: bag retired — zero health
engine: hud: score 000500, health 3/3, time 0:00, wave 1/3 — at 8,8 over camera 8,0
engine: hud: score 000500, health 3/3, time 0:00, wave 2/3 — at 8,8 over camera 8,0
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: bag retired — zero health
engine: hud: score 001000, health 3/3, time 0:00, wave 2/3 — at 8,8 over camera 8,0
```

One kill, `+500`; the second kill, `001000` — and the readout that
shows it is in the retirement's frame, exactly like the health was in
the hit's. The walk-ground part of the score still accumulates beside
it: `game.score` is one number, the game's own, and the HUD reads it
the same way whatever fills it.

What the exercise deliberately keeps out: nothing about *which* prices
are fun. `100/150/250/1000` is a spreadsheet guess — what a bat is
worth next to a golem is game design, and the whole point of the column
is that tuning it is an edit to a text file, never a recompile.
