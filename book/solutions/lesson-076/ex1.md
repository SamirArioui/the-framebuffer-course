# Solution: exercise 1 — The step, predicted

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 076 — the hero as an entity](../../lessons/part-4/lesson-076-hero.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is a scratch probe beside the hero's creation: a copy of the
hero's entity, its movement request written by hand, and two steps of
exactly the arithmetic the walk steps — `request × speed × dt` — at a
`dt` chosen so the numbers are checkable: 0.5 seconds.

The predictions, before any run. The hero starts at its row's `312,232`
and its row says speed 240. **Frame one** — Right at `dt = 0.5`:
`x += 1.0 × 240 × 0.5` = 120 pixels, so `432.0, 232.0`. **Frame two** —
Right *and* Down at `dt = 0.5`: both axes step 120 pixels, so `552.0,
352.0`. The second answer is the one worth a sentence: moving
diagonally covers 120 pixels *on each axis* — 169.7 pixels of actual
ground, at 339 px/s — while moving straight covers 120 at 240 px/s. The
diagonal is √2 times faster, because the model treats the two axes as
independent.

The runs, from this lesson's end state plus the patch:

```
engine: probe: Right at dt 0.5 -> 432.0,232.0
engine: probe: Right+Down at dt 0.5 -> 552.0,352.0
```

Exactly as predicted.

Now the question. **What changes:** the request is normalized before it
becomes motion — when both components are non-zero, divide both by
√2 (or write the request as a unit vector in the first place). Then
Right+Down at `dt = 0.5` steps 84.85 pixels per axis: 120 pixels of
ground, the same as straight movement. **Where:** that is a design
choice with two defensible answers. Do it where the *request* is
written — the input step or the AI — and each mover decides what its
own direction means (a projectile might want the faster diagonal; a
player usually does not). Do it in the *walk* — normalize every
request that arrives — and the rule holds for every entity without
repetition. This engine leaves it to the game on purpose: the movement
model is the game's, and Part 5's hero lesson replaces this whole step
with acceleration and deceleration feel, where "same speed on the
diagonal" is one line in a much bigger idea.

One habit worth noticing: the probe copies the hero (`Entity probe =
hero`) and steps the copy — the real hero, the real store, and the real
walk are untouched. Probes scratch; games keep their state.

Nothing here touches the walk, the input step, or the game's loop: the
probe is two forced frames beside the run's own.
