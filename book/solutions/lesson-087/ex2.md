# Solution: exercise 2 — the projectile with no art

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 087 — projectiles and two weapons](../../lessons/part-5/lesson-087-projectiles-weapons.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The crash reproduces exactly as the exercise says: a projectile file
whose header names no `sprite` column loads fine (that is the format's
promise — a weapon row names no art either), and the run dies the
moment its kind is fired:

```
engine: state title -> play (the player started)
engine: fire: hero -> bolt (damage 1, range 160)
Segmentation fault (core dumped)
```

Where and why: the definition's image is loaded from the `sprite`
column, so an artless row hands out `image = 0`. `EntityFromDef` copies
it into the entity, and the very first flight step reads the sprite's
size — the mover's collision box is the entity's art, `ANIM_FRAME_W ×
sprite->height` — and a null sprite has no size to read. The projectile
is an entity, and every entity draws and collides *as its art*: a
definition without art cannot answer the questions an entity must.

The fix is the engine's usual answer to a value it cannot vouch for:
**refuse it typed**. `EntityCreate` now answers `ENTITY_NO_ART` for a
definition with no image, before anything reads one — and the fire
names it like every other refusal:

```
engine: fire refused — the kind has no art
```

Nothing legitimate breaks: the weapon rows carry no art and are never
created as entities, so the refusal never fires for them. The run keeps
running (the line above is from a held trigger — a refusal per pull,
the world intact) — the failure is a value again, not a crash. This is
lesson 071's rule applied one layer down: the wrong game that works is
worse than a typed failure that names itself.
