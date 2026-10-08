# Solution: exercise 1 — the scatter shot

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 087 — projectiles and two weapons](../../lessons/part-5/lesson-087-projectiles-weapons.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

A weapon is a row, so a weapon that behaves differently is a row that
says so. The scatter gun is one row more (`scatter 1 120 bolt 3`) and
one named column more in the format: **`burst`** — how many shots one
trigger pull fires. The growth is the lesson's own move: the column is
named, additive, and defaulted (`burst` defaults to `1`), so every file
that shipped keeps loading byte-for-byte — their rows simply carry one
shot per pull — and the arm report shows the row's value where it is
carried:

```
engine: arm: hero arms scatter (damage 1, rate 120, burst 3, fires bolt)
engine: fire: hero -> bolt (damage 1, range 160)
engine: fire: hero -> bolt (damage 1, range 160)
engine: fire: hero -> bolt (damage 1, range 160)
engine: shot bolt retired — wall
engine: shot bolt retired — wall
engine: shot bolt retired — range
```

One trigger pull, three `fire` lines — and then three different fates:
two of the spread's lanes met walls, the third flew its whole range. The
spread is real: the shots leave along three directions, not one.

The directions are the interesting part. The middle shot flies the aim;
either side is the aim **turned one 45-degree step**, and a turn of a
unit direction by 45 degrees — `((x − y)/√2, (x + y)/√2)` — keeps its
length exactly. So every shot of the spread flies at the projectile's
own speed (no scaling fudge), and turning a direction repeatedly stays
unit. No square root is computed at run time: `AIM_DIAG` is the same
1/√2 the hero's diagonal intent has used since lesson 085.

The third number key arms the third row — `HeroFire`'s arming block
grows one line, exactly the shape the lesson promised: a weapon is a
row, and the weapons grow as rows.
