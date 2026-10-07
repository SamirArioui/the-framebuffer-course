# Solution: exercise 2 — The capacity you would pick

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 074 — one fixed store](../../lessons/part-4/lesson-074-store.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The diff is one line of accounting: the run names the slot's size and the
store's total, so "capacity is a decision" is a decision made against a
number. From a real run of this lesson's end state plus the patch:

```
engine: store: live 64 of 64 — the hero and 63 from the script
engine: store: creation refused (full), arena 1253648 -> 1253648 — creation allocates nothing
engine: store: 64 slots of 56 bytes — 3592 bytes decided up front
```

Fifty-six bytes a slot. That is the number the whole discussion stands
on: an entity is its name, five `int`s, a sprite pointer, and the `live`
flag, padded to the machine's liking. Everything below is arithmetic
with it.

**The count, for the finished game.** The MVD's world holds: the hero
(1); a wave's worth of enemies alive at once — three types plus the boss
with its parts (call it 32); projectiles in flight from two weapons
(64 is a busy second); and particle bursts (the design deliberately
leaves open whether bursts get their own, narrower store — Part 5's L12
decides). That is about a hundred entities with headroom for a bad
moment, so **256 slots** is the capacity this solution picks: 256 × 56
= 14,336 bytes, plus the count — under 15 KB of a 32 MB arena, and the
same 15 KB whether the game ever spawns a hundred entities or one.

**When it is too small.** Requests past the capacity are refused as
typed values (exercise 1 is what handling one looks like). The game
decides what each refusal means: a projectile that cannot be created is
the shot that did not fire — the honest version is to spend it where it
matters (the player's shot) and drop it where it does not (the fifth
particle of a burst), and to *count* the refusals in the run's report so
a full store is visible in testing rather than in a review. What the
game must never do is the thing exercise 1 crashed on: use the entity of
a request it did not check.

**When it is too large.** The cost is the store's bytes, taken up front
and never returned — 3.5 KB at 64 slots, 14 KB at 256, 56 KB at 1024.
Against a 32 MB arena that is nothing; against a smaller machine or a
bigger store of something else it is not. The rule of thumb the engine
uses everywhere: *capacity is a decision*, made where the need is known,
and named where a reviewer will find it (this part's closing review names
this one).

**Why a wrong capacity is tuning, not correctness.** Three things in the
store's design, all in this lesson's code: the refusal is *typed* — a
full store answers `ENTITY_FULL` and the game handles it, rather than
writing somewhere it should not; it never *steals* — no live entity is
silently replaced, so nothing the player can see disappears because
something invisible asked for room; and it never *allocates* — the run's
memory does not move under it, so a full store cannot become a slow
store or a fragmented one. Get the number wrong and the game plays worse
or spawns less. It does not corrupt, leak, or vanish anything — which is
what separates a tuning knob from a bug.

If your own count came out different, that is the exercise working: the
number belongs to *your* game's worst second, not to this lesson. Change
`ENTITY_CAP`, re-run the script, and read the store's line — the
capacity is one constant and the report tells you what it costs.
