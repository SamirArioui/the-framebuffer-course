# Solution: exercise 1 — the kinds nobody made yet

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 088 — enemy archetype tables](../../lessons/part-5/lesson-088-enemy-tables.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

Three kinds, three rows — the diff is `assets/enemies.txt` and nothing
else:

```
swarm 240 160 0 140 1 assets/wisp.ppm 1 90 bolt chase 2 6
sprinter 240 200 0 240 1 assets/bat.ppm 1 60 bolt chase 1 3
tank 600 440 0 48 12 assets/golem.ppm 3 20 shell keep 3 1
```

(The art is reused on purpose — the swarmling wears the wisp's colours,
the sprinter the bat's. New art is a new `.ppm`, also data, also no
code.) The run spawns each one carrying its row's values:

```
engine: entity swarm: x 240 y 160 facing 0 speed 140 health 1 sprite 16x16 accel 120 damage 1 rate 90 fires bolt range 0 behavior chase wave 2 count 6
engine: entity sprinter: x 240 y 200 facing 0 speed 240 health 1 sprite 16x16 accel 120 damage 1 rate 60 fires bolt range 0 behavior chase wave 1 count 3
engine: entity tank: x 600 y 440 facing 0 speed 48 health 12 sprite 16x16 accel 120 damage 3 rate 20 fires shell range 0 behavior keep wave 3 count 1
engine: roster: 7 enemies from the table's rows, live 9 of 64
```

and `git diff --stat src/` prints nothing at all — the source tree did
not move.

What the columns expressed that code would have hidden: the swarm is
six of a thing (`count 6`), the sprinter is speed 240 with one health
point — the whole "fast and fragile" trade written where a designer can
read it — and the tank is twelve health, three damage, speed 48: a
different *position in the design space*, and the only difference
between it and the sprinter is fourteen bytes of text in a file. In
code these would be constants nobody can find, three `if` chains, and
an argument in review about which file the boss's speed belongs in.
Here they are a table you can diff.
