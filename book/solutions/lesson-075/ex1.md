# Solution: exercise 1 — The walk that retires ahead

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 075 — the walk and the free slot](../../lessons/part-4/lesson-075-lifetime.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is one probe, run twice: `ProbeWalk` builds a scratch store of
six entities at slots 0-5 and walks it with a body that retires *one
other entity* when the walk reaches a given slot — then reports the
visit list and the live count. The first call retires slot 3 while
standing at slot 1 (ahead of the walk); the second retires slot 1 while
standing at slot 3 (behind it).

The predictions, before any run. In the **first** case the walk visits
slot 0 and slot 1; at slot 1 its work retires slot 3; the walk then
continues to slot 2, finds slot 3 dead and skips it, and visits 4 and 5.
The list is `0 1 2 4 5` — five visits — and the live count is 5. In the
**second** case the walk has already visited slot 1 before slot 3's work
retires it, so every slot is visited once and the list is `0 1 2 3 4 5`
— six visits — while the live count again lands on 5.

The runs, from this lesson's end state plus the patch:

```
engine: walk (retire slot 3 at slot 1) visited 0 1 2 4 5, live 5
engine: walk (retire slot 1 at slot 3) visited 0 1 2 3 4 5, live 5
```

Both hold. Note what the two lists share with the lesson's own walk: no
slot is visited twice in any of them, and no live entity is skipped. The
asymmetry is exactly what the contract says — an entity retired *ahead*
of the walk is never visited in that walk; an entity retired *behind* it
was visited before it went and is not visited again.

What makes both answers safe is one sentence: **the slots never move.**
The walk's index is its own position in the store, and "is this entity
live?" is answered at arrival — so nothing a body does to any slot can
shift the ground under the walk. A walk over a structure that compacts
itself on removal (an array that shifts, a list that relinks) would lose
or repeat entities the moment its body retired one — which is exactly
why the store is fixed slots and not a growable collection. Retirement
here is one flag and one count; the geometry is untouched.

That is also the answer to the objection "why not just collect the
entities into an array each frame and walk that?" — collecting is a
per-frame copy of the very thing the walk exists to avoid, and it moves
the entities the moment you want to remove one. The fixed store makes
the walk, the retirement, and the reuse all one rule each.

Nothing here touches the demo's walk, the store's policy, or the game's
loop: the probe is a scratch store beside the run's own.
