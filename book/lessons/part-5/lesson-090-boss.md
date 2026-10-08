# Lesson 090 — the boss

{{#include ../../stability-horizon.md}}

## Prose

The golem has stood at its row's `x` and `y` through two lessons of
everyone else moving. Its `behavior` says `boss` — the one value the
walk's switch does not act on yet. This lesson is what that value
means: **the three behaviors, composed by a pattern of the boss's own
— its own schedule, and nothing else of its own.** And while the boss
learns to move, the world learns to shoot: the enemy rows have carried
their weapons since lesson 088, and today the walk fires them.

### A schedule is not movement machinery

The instinct the roster lesson argued against — *the boss deserves its
own code* — was half right. What the boss deserves is **time**: phases,
a cadence, "chase now, hold and fire, break away, repeat". A row
carries facts; it cannot carry a schedule. So the boss gets one —
per-entity state and timing, the entity's `phase` and `phase_t` — and
what the schedule schedules is the same three functions everyone else
uses:

```cpp
e.phase_t += dt;                       /* how long this phase has run */
if (e.phase_t >= length) {
    e.phase_t = 0.0;
    e.phase = (e.phase + 1) % 3;       /* chase -> keep -> flee -> … */
    ...
}
if (e.phase == 0)      AiChase(e, hero);
else if (e.phase == 1) AiKeep(e, hero);
else                   AiFlee(e, hero);
```

That is the entire boss: a timer, a counter, and three calls. No
movement math, no boss-sized mover, no phase-specific physics — the
phases *are* `AiChase`, `AiKeep`, and `AiFlee`, in an order and for
lengths the pattern owns (3 s, 2 s, 1 s). The timer counts in game
time, so the freeze of lesson 078 freezes the pattern too.

### The pattern, measured

One run with the boss alone on the map (a scratch roster holding just
its row — the boss is easier to watch without company) and the hero
standing still. The pattern announces its phases, and the entity
reports show each phase's motion doing exactly its behavior's work:

```
engine: golem at 384,172 — 93 px of the hero (t=1.504)
engine: golem at 365,190 — 67 px of the hero (t=1.866)
engine: golem at 352,212 — 44 px of the hero (t=2.297)
engine: golem at 337,232 — 25 px of the hero (t=2.857)
engine: boss: golem's pattern -> keep (2 s)
engine: golem at 358,217 — 48 px of the hero (t=3.849)
engine: golem at 376,200 — 71 px of the hero (t=4.196)
engine: golem at 393,182 — 95 px of the hero (t=4.541)
engine: golem at 411,165 — 119 px of the hero (t=4.885)
engine: golem at 428,147 — 144 px of the hero (t=5.231)
engine: boss: golem's pattern -> flee (1 s)
engine: golem at 446,129 — 168 px of the hero (t=5.576)
engine: golem at 464,112 — 193 px of the hero (t=5.923)
engine: golem at 481,94 — 218 px of the hero (t=6.268)
engine: boss: golem's pattern -> chase (3 s)
engine: golem at 464,112 — 193 px of the hero (t=6.701)
engine: golem at 446,129 — 168 px of the hero (t=7.046)
...
engine: golem at 338,233 — 26 px of the hero (t=9.161)
engine: boss: golem's pattern -> keep (2 s)
```

Read the distances: `93 → 67 → 44 → 25` is the **chase** closing.
`48 → 71 → 95 → 119 → 144` is the **keep** backing off to its 160 —
the same `AiKeep` the spitter used, the same band, the same math.
`168 → 193 → 218` is the **flee** running. Then `chase (3 s)` and the
distance falls again. The boss is three behaviors and a watch.

### The world shoots back

The enemy rows have carried `damage`, `rate`, and `fires` since lesson
088. Today the walk's per-entity work gains one more line — the
**attack**: an armed entity fires its row's weapon at the hero, at its
row's rate, while the hero is within the shot's reach. The hero is
exempt (its trigger is the player's); an unarmed kind — like the
slime, whose row names no weapon — fires nothing.

With that, lesson 087's last stand-in dies. The `G` key and the armed
slime are gone: what they demonstrated — a real projectile hit
reducing the hero's health by the row's damage — is now the enemy
rows' own work. A run with the hero standing in the boss's line:

```
engine: hit: shell hits hero — damage 2, health 3 -> 1
engine: hit: shell hits hero — damage 2, health 1 -> 0
engine: state play -> death (the hero's health reached zero)
```

`damage 2` twice — the golem's row — and the named condition fires on
real combat, no key held down. (The seam shrinks with the stand-in:
`KEY_G` is gone from `platform.h`.)

### What this run verified, and what it did not

- **The boss composes the behaviors plus its pattern** — the phase
  reports and the distances show `AiChase`, `AiKeep`, and `AiFlee`
  doing the boss's moving; the pattern's own machinery is a timer and a
  counter.
- **No separate movement machinery** — `AiBoss` contains no movement
  math at all; every step the boss takes is a shared behavior's
  request through the same `MoveEntity` as everyone else.
- **The enemy attacks are real** — the boss's shells hit the hero for
  its row's damage and the hero's zero health is the game's defeat.

What this run did **not** verify is the boss fight as a *fight* — with
the hero shooting back, dodging, and winning. The golem's eight health
is a row's fact waiting for a player; whether the numbers make a good
fight is the game's judgment, not the machine's. Nor does the schedule
know anything but its three phases: an enraged second half, a
telegraphed wind-up, a phase that spends three shots — all patterns of
the same shape (a schedule that composes shared work), and exercises
below.

## Code step

One change: the pattern and the attacks. `src/ai.h/.cpp` grow
`AiBoss` — the schedule (the entity's `phase` and `phase_t`) composing
the three shared behaviors; `src/entity.h` carries that pattern state;
`src/combat.h/.cpp` grow `CombatAttack` (an armed entity fires its
row's weapon at its rate, at the hero, within the shot's reach);
`src/game.cpp`'s walk gains the `boss` case and the attack line;
`src/main.cpp` and `src/platform.h`/`src/platform_x11.cpp` drop
lesson 087's enemy-fire stand-in and its key — the enemy rows fire for
real. `assets/enemies.txt` tunes the golem's rate to the fight this
lesson demonstrates (a shell every three seconds). Its end state is
tagged `lesson-090`.

```diff
diff --git a/assets/enemies.txt b/assets/enemies.txt
index ffd931d..0659107 100644
--- a/assets/enemies.txt
+++ b/assets/enemies.txt
@@ -2,4 +2,4 @@ name x y facing speed health sprite damage rate fires behavior wave count
 bat 560 72 2 160 2 assets/bat.ppm 1 60 bolt chase 1 2
 wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 1 1
 spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2
-golem 384 96 1 72 8 assets/golem.ppm 2 30 shell boss 3 1
+golem 384 96 1 72 8 assets/golem.ppm 2 20 shell boss 3 1
diff --git a/src/ai.cpp b/src/ai.cpp
index 1ed2dc4..67b9ceb 100644
--- a/src/ai.cpp
+++ b/src/ai.cpp
@@ -8,6 +8,8 @@
 
 #include "ai.h"
 
+#include <cstdio>
+
 #include "combat.h" /* CombatAim — the same eight compass points */
 
 namespace engine {
@@ -45,4 +47,35 @@ void AiFlee(Entity &e, const Entity &hero)
     CombatAim(e.x - hero.x, e.y - hero.y, e.move_x, e.move_y);
 }
 
+void AiBoss(Entity &e, const Entity &hero, double dt)
+{
+    /* The schedule: one timer per entity (its `phase_t`) counting how
+       long the current phase has run, in game time — so a frozen world
+       freezes the pattern too — and the phase index cycling when a
+       phase's length is met. This is the whole of the boss's own
+       machinery: state and timing. */
+    e.phase_t += dt;
+    double length = e.phase == 0 ? BOSS_CHASE_S
+                                 : e.phase == 1 ? BOSS_KEEP_S : BOSS_FLEE_S;
+    if (e.phase_t >= length) {
+        e.phase_t = 0.0;
+        e.phase = (e.phase + 1) % 3;
+        length = e.phase == 0 ? BOSS_CHASE_S
+                              : e.phase == 1 ? BOSS_KEEP_S : BOSS_FLEE_S;
+        std::printf("engine: boss: %s's pattern -> %s (%.0f s)\n", e.name,
+                    e.phase == 0 ? "chase" : e.phase == 1 ? "keep" : "flee",
+                    length);
+    }
+
+    /* And what the schedule schedules: the same three behaviors every
+       other entity uses. The boss composes them; it does not own any
+       movement of its own. */
+    if (e.phase == 0)
+        AiChase(e, hero);
+    else if (e.phase == 1)
+        AiKeep(e, hero);
+    else
+        AiFlee(e, hero);
+}
+
 } /* namespace engine */
diff --git a/src/ai.h b/src/ai.h
index a2b1ee7..f5fd324 100644
--- a/src/ai.h
+++ b/src/ai.h
@@ -37,6 +37,21 @@ void AiKeep(Entity &e, const Entity &hero);
 /* Flee: the request points away from the hero, every frame. */
 void AiFlee(Entity &e, const Entity &hero);
 
+/* Lesson 090: the boss's pattern — its own schedule, and the one thing
+   a row cannot carry. The schedule is per-entity state and timing (the
+   entity's `phase` and `phase_t`), and what it schedules is the same
+   three behaviors above: the boss has no movement machinery of its
+   own. The phases' lengths are the pattern's own facts, here beside
+   the keep distance. */
+constexpr double BOSS_CHASE_S = 3.0;
+constexpr double BOSS_KEEP_S = 2.0;
+constexpr double BOSS_FLEE_S = 1.0;
+
+/* One boss's pattern, once per frame of game time: the schedule
+   advances, and the phase it lands on writes the request — through
+   AiChase, AiKeep, or AiFlee, like every other entity. */
+void AiBoss(Entity &e, const Entity &hero, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/combat.cpp b/src/combat.cpp
index 905cfc7..0673f23 100644
--- a/src/combat.cpp
+++ b/src/combat.cpp
@@ -180,4 +180,36 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
     }
 }
 
+void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
+                  const Entity &hero, double dt)
+{
+    /* An unarmed entity never fires — and the hero is never its own
+       attacker: its trigger is the player's (HeroFire). */
+    if (!e.fires[0] || e.rate <= 0 || &e == &hero)
+        return;
+    if (e.cooldown > 0.0) {
+        e.cooldown -= dt;
+        return;
+    }
+
+    /* The shot's reach is its kind's row: an attacker threatens only
+       as far as its shot flies, and stays quiet beyond it. */
+    DefResult kind = TableFind(shots, e.fires);
+    if (kind.error != DEF_OK)
+        return;
+    double dx = hero.x - e.x, dy = hero.y - e.y;
+    double reach = (double)kind.def->range;
+    if (dx * dx + dy * dy > reach * reach)
+        return;
+
+    /* The attack aims the way everything else does: the compass point
+       at the hero. */
+    double dir_x = 0.0, dir_y = 0.0;
+    CombatAim(dx, dy, dir_x, dir_y);
+    if (dir_x == 0.0 && dir_y == 0.0)
+        return;
+    if (CombatFire(store, shots, e, dir_x, dir_y))
+        e.cooldown = 60.0 / (double)e.rate;
+}
+
 } /* namespace engine */
diff --git a/src/combat.h b/src/combat.h
index b052eaf..77df2ba 100644
--- a/src/combat.h
+++ b/src/combat.h
@@ -60,6 +60,14 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                Entity &shot, double dt);
 
+/* Lesson 090: the enemy attack, once per frame of game time. An armed
+   entity — one whose row names a projectile kind — fires it at the
+   hero at its row's rate, while the hero is within the shot's reach.
+   The hero itself is never its own attacker: its trigger is the
+   player's. */
+void CombatAttack(EntityStore &store, const EntityTable &shots, Entity &e,
+                  const Entity &hero, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/entity.h b/src/entity.h
index a6872fb..2797cc7 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -57,6 +57,10 @@ struct Entity {
     int wave;                  /* lesson 088: which wave spawns this
                                   kind — carried like every row value */
     int count;                 /* and how many join that wave */
+    int phase;                 /* lesson 090: where a pattern is in its
+                                  schedule — per-entity state, the one
+                                  thing a row cannot carry */
+    double phase_t;            /* and how long this phase has run */
     double cooldown;           /* seconds until it may fire again */
 };
 
diff --git a/src/game.cpp b/src/game.cpp
index 11276a1..f9350ae 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -22,15 +22,12 @@ namespace engine {
 
 /* The demonstration stand-ins, named so they cannot be mistaken for the
    game. Lesson 087 made the hit real: a projectile reduces its target's
-   health by its row's damage, and a zero-health entity is retired. What
-   still stands in is the *enemy's fire* — who shoots at the hero, and
-   when. Until the enemies' attacks land (lesson 090), the run's G key
-   makes the slime spit at the hero (a keyed stand-in in the run, like
-   the feel demonstration beside it), and ENTER in play below says the
-   game is complete — lesson 091's waves spend it for real. Both die
-   when the real triggers arrive; the transitions they fire are the
-   game's own (defeat on zero health, completion on no waves). Keyed,
-   they never fire on their own during a gameplay test. */
+   health by its row's damage, and a zero-health entity is retired.
+   Lesson 090 made the enemy fire real: the enemy rows carry their own
+   weapons and the walk's attack fires them at the hero — the `G` key's
+   stand-in is gone. What remains is ENTER in play below: the game's
+   completion stood in for, until lesson 091's waves spend it for real.
+   Keyed, it never fires on its own during a gameplay test. */
 
 /* A transition, named once here and printed the moment it happens, so a
    run shows the machine moving between states and why. */
@@ -228,7 +225,7 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 }
 
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             double dt)
+             const EntityTable &shots, double dt)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -267,12 +264,20 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
         case BEHAVIOR_FLEE:
             AiFlee(e, hero);
             break;
+        case BEHAVIOR_BOSS:
+            AiBoss(e, hero, dt);
+            break;
         default:
             /* `none` stands where it stands — the request is its row's
-               (lesson 084's stand-in walk writes one) — and `boss` is
-               lesson 090's pattern, composed of these same behaviors. */
+               (lesson 084's stand-in walk used to write one). */
             break;
         }
+
+        /* Lesson 090: the attack, once per entity — an armed kind fires
+           its row's weapon at the hero at its rate. The hero is exempt
+           (its trigger is the player's); an unarmed kind fires
+           nothing. */
+        CombatAttack(store, shots, e, hero, dt);
         MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
         if (e.move_x > 0.0)
             e.facing = 0;
diff --git a/src/game.h b/src/game.h
index bde62f0..386c098 100644
--- a/src/game.h
+++ b/src/game.h
@@ -105,11 +105,13 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
    through the mover (MoveEntity), one axis at a time, so it stops at a
    solid tile and slides along a wall. Lesson 087: a projectile's
    behavior flies it (CombatFly — its own sub-stepped flight, retiring at
-   walls, at its range, at what it hits) instead of the request. The hero
-   is handed along for the combat's rules to know the game's actor by.
-   Returns the visit count. */
+   walls, at its range, at what it hits) instead of the request. Lesson
+   089-090: the enemy behaviors and the boss's pattern write the request
+   the way the player's input writes the hero's, and an armed entity
+   attacks at its row's rate. The hero is handed along for the combat's
+   rules to know the game's actor by. Returns the visit count. */
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             double dt);
+             const EntityTable &shots, double dt);
 
 } /* namespace engine */
 
diff --git a/src/main.cpp b/src/main.cpp
index 4c5a16f..3271085 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -336,7 +336,6 @@ int Run(void)
        same table, one entity per row. A new row is a new entity; the
        run has no per-kind code to grow. */
     int created = 1;
-    Entity *foe = 0; /* the world's one enemy row (the slime) */
     for (int i = 0; i < table.count; ++i) {
         if (&table.rows[i] == hero_def.def)
             continue;
@@ -352,8 +351,6 @@ int Run(void)
            stand-in for the AI. Lesson 089 replaced it: the behaviors
            are real now, and the world's kinds move the ways their rows
            say (the slime's row says `none`, so it stands). */
-        if (!foe)
-            foe = made.entity;
         created += 1;
     }
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
@@ -379,15 +376,12 @@ int Run(void)
                 foes.count, store.live, ENTITY_CAP);
 
     /* Lesson 087: weapons are rows. The hero starts armed with the
-       weapons table's first row; the number keys arm the rest (HeroFire).
-       The demonstration stand-in arms the foe with the second row — its
-       projectile is what G spits at the hero. The enemy rows that carry
-       their own attacks arrive in lesson 088; this stand-in and its key
-       die when those attacks land (lesson 090). */
+       weapons table's first row; the number keys arm the rest
+       (HeroFire). Lesson 090: the enemy-fire stand-in and its key are
+       gone — the enemy rows carry their own weapons and the walk's
+       attack fires them. */
     if (weapons.count > 0)
         CombatArm(hero, weapons.rows[0]);
-    if (weapons.count > 1 && foe)
-        CombatArm(*foe, weapons.rows[1]);
 
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
@@ -454,7 +448,7 @@ int Run(void)
     std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
                 music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
                 AUDIO_MIXER_CHANNELS);
-    std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires, G is the enemy spit; close the window to stop\n");
+    std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires; close the window to stop\n");
     std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);
 
     /* Lesson 059: the run's sound is a run of amplitude at the engine's
@@ -565,17 +559,6 @@ int Run(void)
             HeroMove(hero, opened.window, dt);
             HeroFire(hero, opened.window, weapons, shots, store, dt);
 
-            /* Lesson 087: the enemy-fire stand-in — G makes the slime
-               spit at the hero. What it demonstrates is real: the shot
-               is an entity, its hit reduces the hero's health by the
-               row's damage, and the hero's zero health is the game's
-               defeat. Only the shooter and its aim are scripted — the
-               enemies' own attacks (lesson 090) replace this key. */
-            if (foe && platform::KeyPressed(opened.window, platform::KEY_G)) {
-                double dir_x = 0.0, dir_y = 0.0;
-                CombatAim(hero.x - foe->x, hero.y - foe->y, dir_x, dir_y);
-                CombatFire(store, shots, *foe, dir_x, dir_y);
-            }
         }
 
         /* Lesson 084: the game resolves its movement against its map —
@@ -584,7 +567,7 @@ int Run(void)
            every projectile into its flight). The loop times it as the
            frame record's entity sub-phase. */
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, hero, dt);
+        int visited = GameWalk(store, map, hero, shots, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
diff --git a/src/platform.h b/src/platform.h
index cd860ae..f2408c0 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -45,7 +45,6 @@ enum Key {
     KEY_ESCAPE,
     KEY_1,
     KEY_2,
-    KEY_G,
     KEY_COUNT
 };
 
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index bd91201..2991183 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -166,7 +166,6 @@ static int KeyIndex(KeySym sym)
     case XK_Escape: return KEY_ESCAPE;
     case XK_1:      return KEY_1;
     case XK_2:      return KEY_2;
-    case XK_g:      return KEY_G;
     default:        return -1;
     }
 }
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the pattern owns its attacks *(extend-the-code)*

Right now the boss fires whenever its row's rate says so, whatever the
phase — the attack line is everybody's. Give the boss's *pattern* its
attacks instead: while the pattern is in its `keep` phase it fires its
weapon at the hero at the row's rate (its hold-and-fire beat); in
`chase` and `flee` it holds its fire. The generic attack rule stays for
every other armed kind. Then run the boss alone again: do the `fire`
lines land inside the `keep` phases and only there?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-090/ex1.md)

### Exercise 2 — the schedule's timeline *(predict-the-output)*

The pattern is 3 s of chase, 2 s of keep, 1 s of flee, repeating —
starting at play. Predict the run's phase reports: in the first
fifteen seconds of play, which phases announce themselves and when
(approximately — the run's frames are its own), and what is the shape
of the boss's distance-to-hero curve over one full cycle? One caveat to
build into your prediction: the headless run's first frame can be a
long one. Then run it and compare against the pattern's reports.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-090/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 089 — enemy AI](lesson-089-enemy-ai.md) ·
**Next:** [Lesson 091 — waves](lesson-091-waves.md) ·
**Code tag:** [`lesson-090`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-090)
