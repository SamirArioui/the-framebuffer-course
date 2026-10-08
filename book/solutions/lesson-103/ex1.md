# Solution: exercise 1 — your game's first change

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 103 — now make YOUR game](../../lessons/part-5/lesson-103-your-game.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

One worked change, in the shape the prompt demands: one fact as data,
one rule in the game layer. The game here is *a little faster and a
little greedier* than the course's — its first wave brings a **hound**
(a chaser almost twice the bat's speed, spawned beside the hero), and
its rule is **a kill is worth 100 points**.

**The data change** is one row in `assets/enemies.txt` — the hound is
a chase kind in wave 1, `count 2`, reusing `assets/bat.ppm` as its art
until the game's own art exists. No code: the loader, the store, the
AI, and the waves all already know what to do with a row. This is the
table seam working as designed — a new kind is a new line.

**The rule change** crosses the game layer's file pairs and nothing
else: the score lives in the game's state (`Game.score`, the value the
HUD reads), the kill is an event inside `CombatFly`, and the two are
wired through `GameWalk` — `double &score` handed down the same
parameter lists that already carry `feel` and `sound`. The increment
sits in the zero-health branch beside the retirement it rewards.
`main.cpp` changes by one token — the loop's call site passes
`game.score`, which is what the loop is *for*: it is the composition
root where the game layer's pairs meet.

The proof the prompt asks for, both parts of it — the run's transcript
(stationary hero, firing right at the charging wave):

```
engine: def hound: x 448 y 232 facing 0 speed 192 health 2 sprite assets/bat.ppm … behavior chase wave 1 count 2
engine: wave 1: spawns hound at 448,232 — speed 192, health 2, chase
engine: wave 1: spawns hound at 464,232 — speed 192, health 2, chase
engine: hit: bolt hits hound — damage 1, health 2 -> 1
engine: hit: bolt hits hound — damage 1, health 1 -> 0
engine: hound retired — zero health
engine: hud: score 000100, health 1/3, time 0:00, wave 1/3 — at 8,8 over camera 8,0
…
engine: hound retired — zero health
engine: hud: score 000200, health 1/3, time 0:01, wave 1/3 — at 8,8 over camera 8,0
```

Every kill is exactly one jump of 100 (the hero stands still, so the
old travel-based score adds nothing to read through), the hounds die
to two bolts each as their row's health says, and `wave 1 cleared —
the next begins` follows when the last of them falls. And the
boundary proof:

```
 assets/enemies.txt |    1 +
 src/combat.cpp     |    9 ++++++++-
 src/combat.h       |    6 ++++--
 src/game.cpp       |    4 ++--
 src/game.h         |    5 +++--
 src/main.cpp       |    2 +-
 6 files changed, 19 insertions(+), 8 deletions(-)
```

Every file is a game-layer pair, the loop's composition root, or
`assets/` — the services (`arena.*`, `table.*`, `entity.*`,
`gametime.*`, `tilemap.*`, `audio.*`, `camera.*`, `framebuffer.*`) do
not appear, `./build.sh` is warning-free, and
`./tools/check-boundary.sh` is clean. That is the hand-over test:
your change bent the game and never the engine.

Three notes for your own first change:

1. **The score rule is deliberately plain.** A per-kind bounty would
   be better game design and would touch the *table* seam instead: one
   named column, added additively (the lesson-087 rule), so every file
   ever shipped keeps loading. When your change wants a new fact about
   kinds, that is the sanctioned way to grow — and the compatibility
   rule is the thing to keep.
2. **Where a rule crosses files is information.** The `double &score`
   threading shows exactly where this rule lives and who knows about
   it. If your rule needs to reach through four pairs and a service,
   stop: either the rule is in the wrong place or the seam is. Both
   are fixable — by you, on purpose.
3. **Keep the demonstration.** A change without a transcript line that
   proves it is a guess. The reports are already in the run; use them
   before you add new ones (and when you add probes, lesson 097's
   transcript discipline keeps them from becoming the next debt).
