# Solution: exercise 2 — the pool under fire

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 095 — audio integration](../../lessons/part-5/lesson-095-audio.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

**The prediction, written down first.** The pool is 16 channels: 0 is
the music's (reserved, never stolen) and 1–15 are the effects'. Each
effect takes the first free channel, so twenty sounds fired in one
frame go like this:

1. The **first fifteen** take channels **1 through 15**, in order.
2. The **sixteenth** finds no free channel and steals the **oldest**
   — the one that started earliest, which is **channel 1** (it started
   first, and stealing re-starts it with the newest order).
3. The **seventeenth** then steals the new oldest, **channel 2**, and
   so on: the twenty's last five land on **1, 2, 3, 4, 5**.

**The run** — the probe firing twenty of the game's deaths in the
fight's first frame — matches the prediction exactly:

```
engine: sound: death -> channel 1 (volume 128 of 256)
engine: sound: death -> channel 2 (volume 128 of 256)
engine: sound: death -> channel 3 (volume 128 of 256)
…
engine: sound: death -> channel 14 (volume 128 of 256)
engine: sound: death -> channel 15 (volume 128 of 256)
engine: sound: death -> channel 1 (volume 128 of 256)   <- the sixteenth: channel 1, stolen
engine: sound: death -> channel 2 (volume 128 of 256)   <- the seventeenth: channel 2
engine: sound: death -> channel 3 (volume 128 of 256)
engine: sound: death -> channel 4 (volume 128 of 256)
engine: sound: death -> channel 5 (volume 128 of 256)
```

`1 … 15`, then `1, 2, 3, 4, 5` — the sixteen steals the *oldest*, not
the last, not a random one. (The `8, 9` that follow in the run are the
game's own events continuing the fight after the probe's frame.)

**What the policy is for.** A sound that gets dropped is *inaudible* —
the player hears the fight's newest moment either way — so when the
pool is full, the right sound to lose is the one the player has already
heard the most of: the oldest. It is the exact mirror of lesson 074's
entity store, and the contrast between them is the lesson: an entity
store **never steals** (a stolen enemy is a bug the player experiences;
the store refuses as a typed value and the game decides what to do),
while the mixer **steals the oldest** (a dropped sound is inaudible, so
the resource keeps flowing). Both policies are the engine's, both are
typed, and neither is a surprise — and now the game's own events are
loud enough to meet them.
