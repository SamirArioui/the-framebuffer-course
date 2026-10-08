# Solution: exercise 2 — why the boss does not deserve code

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 088 — enemy archetype tables](../../lessons/part-5/lesson-088-enemy-tables.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The probe makes the shape visible: one visit per live entity, one work
shape, every kind through it.

```
walk visit: hero (behavior none)
walk visit: slime (behavior none)
walk visit: bat (behavior chase)
walk visit: wisp (behavior flee)
walk visit: spitter (behavior keep)
walk visit: golem (behavior boss)
```

Six kinds, one loop. Now the argument.

**What five update functions would cost the walk.** Lesson 075's rule
is that the per-entity work has *one* shape: every live entity exactly
once, in slot order, no retired slot visited. With `UpdateBat`,
`UpdateWisp`, `UpdateGolem`, … the walk is still one loop — but its
body is a dispatch that grows with every kind, and the invariant "every
entity gets exactly the right work, once" is no longer enforced by the
shape; it is re-argued per function. Add a kind and the dispatch grows.
Add an effect that must run for *some* kinds and you get a second
dispatch. The one branch on `behavior` — a fact the row carries — buys
the opposite: the work is expressed once, and a kind is a value.

**What lesson 091's waves change.** The waves spawn the same rows again
and again — dozens of entities over a game, but never a new *kind* at
run time. With rows, the spawn is `EntityCreate(store, row)` and the
walk does not notice. With per-type code, every wave's population is a
set of call sites to keep in step — and the wave that spawns "three
types and the boss together" is the one that most needs the work to
scale with the *count*, not with the code.

**What "per-type attributes are data" protects.** A speed constant in
code is a value that cannot be found, compared, or tuned without a
rebuild — and two kinds cannot differ in it without *more* code. The
row's value is diffable (exercise 1's three kinds were three lines of
text), reviewable, and the run prints it twice — once as the definition
and once as the carrying entity — so nobody has to trust the code's
word for what a golem is.

**The steelman — the one thing a row cannot carry.** The boss genuinely
needs *time*: phases, a cadence, "chase now, spit three times, flee,
repeat". A row carries facts; it cannot carry a schedule. That is
exactly what lesson 090 gives it — **its own schedule**, per-entity
state and timing — and notably *not* its own movement machinery: the
schedule picks among the same three behaviors everyone else uses. The
boss gets what it needs without the code that the iteration rule
forbids. (The probe above is throwaway measurement; the real walk
prints nothing.)
