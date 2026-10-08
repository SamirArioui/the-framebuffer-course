# Solution: exercise 1 — the pattern owns its attacks

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 090 — the boss](../../lessons/part-5/lesson-090-boss.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The generic attack line is every armed kind's — so the boss opts out of
it (`e.behavior != BEHAVIOR_BOSS` in the walk) and its pattern fires
for itself: `AiBoss` gains the store and the projectile table, and the
`keep` phase — the hold-and-fire beat — calls `CombatAttack` while the
other two phases hold their fire. The rule for everyone else is
untouched.

One run with the boss alone, the hero standing in its line:

```
engine: boss: golem's pattern -> keep (2 s)
engine: fire: golem -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 3 -> 1
engine: boss: golem's pattern -> flee (1 s)
engine: boss: golem's pattern -> chase (3 s)
engine: boss: golem's pattern -> keep (2 s)
engine: fire: golem -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 1 -> 0
```

Every `fire` line falls inside a `keep` phase and nowhere else: one
shot per keep (the row's rate — one shell every three seconds — is
slower than the two-second beat, so each hold gets its shot). The
chase and the flee are silent: the boss closes and the boss runs, and
the player can read when the gun is coming up. The hero dies at the
second shot — `damage 2` twice, exactly the row's — which is the fight
reading as a fight: the pattern now *means* something to the player,
not just to the movement.

That is the shape of every pattern this game will ever have: a schedule
that composes shared work and owns its own timing. The wind-up, the
enrage, the volley are the same two lines — a phase that calls the
shared functions — and no new machinery.
