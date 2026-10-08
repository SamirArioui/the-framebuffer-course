# Lesson 091 — waves

{{#include ../../stability-horizon.md}}

## Prose

The game has everything but a reason to play it: a hero that fights, a
world that fights back, a boss with a pattern — and no shape to the
contest. This lesson gives it one: **waves** — the fight arriving in
stages that bring the roster together, each wave spawned from the
table's definitions, the next beginning when the last enemy of the
current one is retired. When the last wave falls, the waves are
complete — the named condition lesson 082's machine has waited for
since, and the last stand-in key dies here.

### The composition is the table's

A wave's composition is not a list in code — it is the rows' own
facts. Lesson 087 grew the columns; the roster has carried them since
lesson 088: `wave` (the wave a kind joins — it spawns in that wave and
every wave after it) and `count` (how many of it each of its waves
spawns). The wave machinery asks the table one question — *whose wave
has come?* — and spawns the answer:

```
engine: wave 1: spawns bat at 560,72 — speed 160, health 2, chase
engine: wave 1: spawns bat at 576,72 — speed 160, health 2, chase
engine: wave 1 begins — 2 enemies
...
engine: wave 1 cleared — the next begins
engine: wave 2: spawns bat at 560,72 — speed 160, health 2, chase
engine: wave 2: spawns bat at 576,72 — speed 160, health 2, chase
engine: wave 2: spawns wisp at 640,336 — speed 120, health 1, flee
engine: wave 2: spawns spitter at 120,400 — speed 96, health 3, keep
engine: wave 2: spawns spitter at 136,400 — speed 96, health 3, keep
engine: wave 2 begins — 5 enemies
```

Wave 1 is the bats (their row's `wave 1`, `count 2` — the copies stand
in a line beside their row's spot). Wave 2 adds the wisp and the two
spitters *and brings the bats back* — a kind joins at its wave and
every wave after, so each stage is thicker than the last. That rule is
what makes the last stage the one the game is built around: **wave 3
joins the golem to everything before it** — the three types and the
boss, together:

```
engine: wave 3: spawns bat at 312,232 — speed 20, health 1, chase
engine: wave 3: spawns bat at 328,232 — speed 20, health 1, chase
engine: wave 3: spawns wisp at 312,232 — speed 20, health 1, flee
engine: wave 3: spawns spitter at 312,232 — speed 20, health 1, keep
engine: wave 3: spawns spitter at 328,232 — speed 20, health 1, keep
engine: wave 3: spawns golem at 312,232 — speed 20, health 1, boss
engine: wave 3 begins — 6 enemies
```

(That last run is a scratch roster — the same waves with the enemies
slow, fragile, and unarmed, spawned at the hero's feet — because the
*mechanics* are what this lesson verifies and the hunt is what a
player does. The numbers in it are the scratch's own.)

### The next begins when the last is retired

The wave is being fought while any of its fighters lives — and the
fighters are the kinds whose behavior fights (`chase`, `keep`, `flee`,
`boss`): the hero, the scenery, and the shots in the air are none of
them. When the last one is retired — by a shot, at zero health, like
every enemy since lesson 087 — the next wave begins, in the same
frame's account:

```
engine: bat retired — zero health
engine: bat retired — zero health
engine: wave 1 cleared — the next begins
```

No timer, no wave-clear key, no "all enemies defeated" flag to keep in
step with the store: the wave's state *is* the store's live fighters.
And when the last fighter of the last wave falls:

```
engine: golem retired — zero health
engine: spitter retired — zero health
engine: the waves are complete (t=3.487)
engine: state play -> victory (the game's waves are complete)
```

`the waves are complete` — the line lesson 082's stand-in printed from
a key press, now printed by the waves themselves. The named condition
(victory follows the game's completion) is the game's own work, and
`ENTER`'s stand-in is gone from the machine. **No stand-ins remain**:
every named condition the state machine reads is real gameplay now.

A fresh game (death or victory, then the title, then play) starts the
fight over: the last game's fighters and shots leave the store — the
hero is the game's actor and the `none` kinds are the world's scenery,
and both stay — and wave 1 spawns again from the same rows.

### What this run verified, and what it did not

- **A wave spawns its composition from the table's definitions** —
  every spawn line names the kind, the count, and the values its row
  carries; change a row's `wave` or `count` and the stage changes with
  it, with no code touched.
- **The next wave begins when the last entity of the current one is
  retired** — `wave N cleared — the next begins` immediately after the
  last `retired — zero health`, and `wave 3 begins` with the three
  types and the boss together.
- **The waves complete the game** — `the waves are complete` → `state
  play -> victory`, the named condition's real trigger at last.

What this run did **not** verify is the fight *feeling* like a fight —
wave pacing, difficulty ramps, the breath between stages. The wave is
the shape of the contest; whether the shape is fun is the game's
judgment (and one exercise below adds the breath the design most
obviously wants). Nor is there a score for the kills — that is the
HUD's lesson, and the data for it (what a kill is worth) is a column
this format can grow the day it needs to.

## Code step

One change: the waves. `src/game.h/.cpp` grow `GameWaves` — the fight's
shape: a fresh game's clear, the wave's composition from the table's
rows (`wave` and `count`), the advance on the last retirement, and the
completion that spends `waves_remaining` — and `Game` carries the wave
being fought. `src/main.cpp` calls the wave fight in play and drops
lesson 088's standing roster spawn (the waves bring the enemies now);
`src/game.cpp`'s machine drops the last stand-in (ENTER's completion
key). `src/combat.cpp`'s hit rule grows one clause — a shot flies
through its own kind, so a wave's copies do not shoot each other down.
`assets/enemies.txt` tunes the bat's rate for the fight the runs show;
`src/table.h` spells the `wave` column's meaning out. Its end state is
tagged `lesson-091`.

```diff
diff --git a/assets/enemies.txt b/assets/enemies.txt
index 0659107..7bf2e74 100644
--- a/assets/enemies.txt
+++ b/assets/enemies.txt
@@ -1,5 +1,5 @@
 name x y facing speed health sprite damage rate fires behavior wave count
-bat 560 72 2 160 2 assets/bat.ppm 1 60 bolt chase 1 2
-wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 1 1
+bat 560 72 2 160 2 assets/bat.ppm 1 20 bolt chase 1 2
+wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 2 1
 spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2
 golem 384 96 1 72 8 assets/golem.ppm 2 20 shell boss 3 1
diff --git a/src/combat.cpp b/src/combat.cpp
index 0673f23..f894460 100644
--- a/src/combat.cpp
+++ b/src/combat.cpp
@@ -21,6 +21,15 @@ bool Overlaps(const Entity &a, const Entity &b)
            a.y < b.y + b.sprite->height && b.y < a.y + a.sprite->height;
 }
 
+/* Two kinds' names, the same or not. */
+bool SameName(const char *a, const char *b)
+{
+    int i = 0;
+    while (a[i] && a[i] == b[i])
+        ++i;
+    return a[i] == b[i];
+}
+
 } /* namespace */
 
 void CombatAim(double vx, double vy, double &dir_x, double &dir_y)
@@ -113,10 +122,11 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
        it hit, a wall (the step refused), and its range running out. */
     double left = (double)shot.speed * dt;
     for (;;) {
-        /* The first actor the shot overlaps, other than itself and its
-           owner. Slots are checked in order; the first hit is the hit.
-           A shot flies through other shots — a projectile hits the
-           living, and crossing fire does not cancel in mid-air. */
+        /* The first actor the shot overlaps, other than itself, its
+           owner, and its owner's kind. Slots are checked in order; the
+           first hit is the hit. A shot flies through other shots — and
+           through its own kind: a wave's copies do not shoot each
+           other down. A shot's targets are the other kinds. */
         Entity *target = 0;
         for (int i = 0; i < ENTITY_CAP && !target; ++i) {
             Entity &e = store.slots[i];
@@ -124,6 +134,8 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                 continue;
             if (e.behavior == BEHAVIOR_FLY)
                 continue;
+            if (shot.owner && SameName(e.name, shot.owner->name))
+                continue;
             if (Overlaps(shot, e))
                 target = &e;
         }
diff --git a/src/game.cpp b/src/game.cpp
index f9350ae..a2f87fe 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -20,14 +20,11 @@
 
 namespace engine {
 
-/* The demonstration stand-ins, named so they cannot be mistaken for the
-   game. Lesson 087 made the hit real: a projectile reduces its target's
+/* Lesson 087 made the hit real: a projectile reduces its target's
    health by its row's damage, and a zero-health entity is retired.
-   Lesson 090 made the enemy fire real: the enemy rows carry their own
-   weapons and the walk's attack fires them at the hero — the `G` key's
-   stand-in is gone. What remains is ENTER in play below: the game's
-   completion stood in for, until lesson 091's waves spend it for real.
-   Keyed, it never fires on its own during a gameplay test. */
+   Lesson 090 made the enemy fire real. Lesson 091 made the waves real.
+   No stand-ins remain: every named condition of this machine is the
+   game's own work now. */
 
 /* A transition, named once here and printed the moment it happens, so a
    run shows the machine moving between states and why. */
@@ -60,6 +57,7 @@ void GameInit(Game &game, int hero_health_full)
     game.waves_remaining = GAME_WAVES;
     game.hero_health_full = hero_health_full;
     game.play_clock = 0.0;
+    game.wave = 0;
     game.camera = { 0, 0, 0, 0 };
     std::printf("engine: game: %d state%s, starting on %s\n", 5, "s",
                 GameStateName(game.state));
@@ -77,6 +75,7 @@ void GameInput(Game &game, platform::Window *window, Entity &hero,
                state. */
             hero.health = game.hero_health_full;
             game.waves_remaining = GAME_WAVES;
+            game.wave = 0; /* lesson 091: the fight starts over */
             game.play_clock = 0.0;
             hero.move_x = 0.0;
             hero.move_y = 0.0;
@@ -99,15 +98,9 @@ void GameInput(Game &game, platform::Window *window, Entity &hero,
             break;
         }
 
-        /* The stand-in for the waves: ENTER in play says the game is
-           complete (lesson 091 spends the waves for real). Completion is
-           the game's waves reaching zero. A keyed stand-in, like SPACE
-           is a hit — it never fires on its own during a gameplay test. */
-        if (platform::KeyPressed(window, platform::KEY_ENTER)) {
-            game.waves_remaining = 0;
-            std::printf("engine: the waves are complete (t=%.3f)\n",
-                        game.play_clock);
-        }
+        /* Completion is the game's waves reaching zero — spent for
+           real by the wave fight (GameWaves, lesson 091) now, not by a
+           key. */
         if (game.waves_remaining == 0) {
             Transition(game, GAME_VICTORY, "the game's waves are complete");
             break;
@@ -291,4 +284,85 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
     return visited;
 }
 
+/* Lesson 091: the kinds that fight are the ones with a behavior to
+   fight with. The hero, the scenery (behavior `none`), and the shots
+   (`fly`) are none of them — so "is a fighter of the waves" is the
+   row's behavior, asked once. */
+static bool IsFighter(const Entity &e)
+{
+    return e.behavior == BEHAVIOR_CHASE || e.behavior == BEHAVIOR_KEEP ||
+           e.behavior == BEHAVIOR_FLEE || e.behavior == BEHAVIOR_BOSS;
+}
+
+void GameWaves(Game &game, EntityStore &store, const EntityTable &foes)
+{
+    /* A fresh fight: the last game's fighters and shots leave the store
+       — the hero is the game's actor and the `none` kinds are the
+       world's scenery, and both stay. */
+    if (game.wave == 0) {
+        int cleared = 0;
+        for (int i = 0; i < ENTITY_CAP; ++i) {
+            Entity &e = store.slots[i];
+            if (!e.live)
+                continue;
+            if (IsFighter(e) || e.behavior == BEHAVIOR_FLY) {
+                EntityRetire(store, e);
+                cleared += 1;
+            }
+        }
+        if (cleared)
+            std::printf("engine: wave: the last fight leaves the store (%d retired)\n",
+                        cleared);
+    }
+
+    /* The wave is being fought while any of its fighters lives. */
+    int live = 0;
+    for (int i = 0; i < ENTITY_CAP; ++i)
+        if (store.slots[i].live && IsFighter(store.slots[i]))
+            live += 1;
+    if (live > 0)
+        return;
+
+    /* The wave is clear: the next begins — or the last one ended the
+       game's waves, which is the named condition for victory. */
+    if (game.wave >= GAME_WAVES) {
+        if (game.waves_remaining > 0) {
+            std::printf("engine: the waves are complete (t=%.3f)\n",
+                        game.play_clock);
+            game.waves_remaining = 0;
+        }
+        return;
+    }
+    if (game.wave > 0)
+        std::printf("engine: wave %d cleared — the next begins\n", game.wave);
+    game.wave += 1;
+
+    /* The composition is the table's: every kind whose row's wave has
+       come joins the wave (a kind joins at its wave and every wave
+       after it), its row's count of them. The copies stand in a line
+       beside their row's spot. */
+    int spawned = 0;
+    for (int i = 0; i < foes.count; ++i) {
+        const EntityDef &def = foes.rows[i];
+        if (def.wave <= 0 || def.wave > game.wave)
+            continue;
+        for (int n = 0; n < def.count; ++n) {
+            EntityResult made = EntityCreate(store, def);
+            if (made.error != ENTITY_OK) {
+                std::printf("engine: wave %d: the store refused %s\n",
+                            game.wave, def.name);
+                continue;
+            }
+            made.entity->x = def.x + n * ANIM_FRAME_W;
+            std::printf("engine: wave %d: spawns %s at %d,%d — speed %d, health %d, %s\n",
+                        game.wave, made.entity->name, (int)made.entity->x,
+                        (int)made.entity->y, made.entity->speed,
+                        made.entity->health,
+                        BehaviorName(made.entity->behavior));
+            spawned += 1;
+        }
+    }
+    std::printf("engine: wave %d begins — %d enemies\n", game.wave, spawned);
+}
+
 } /* namespace engine */
diff --git a/src/game.h b/src/game.h
index 386c098..e171716 100644
--- a/src/game.h
+++ b/src/game.h
@@ -41,8 +41,9 @@ enum GameState {
 };
 
 /* The game's wave count — the named condition for victory reads it: the
-   game is complete when its waves are done. Lesson 091 builds the waves
-   that spend it; until then it is the game's plan, named here. */
+   game is complete when its waves are done. Lesson 091: the waves are
+   fought now — each one spawns the kinds its table rows call for, and
+   the next begins when the last enemy of the current one is retired. */
 constexpr int GAME_WAVES = 3;
 
 /* The game's own state: which state it is in, and the facts the named
@@ -52,6 +53,8 @@ struct Game {
     int waves_remaining;  /* the named condition for victory */
     int hero_health_full; /* the health a fresh game starts the hero at */
     double play_clock;    /* wall seconds spent in play this game */
+    int wave;             /* lesson 091: the wave being fought (0 = the
+                            fight has not started) */
     Camera camera;        /* lesson 083: the game's world-view — one
                             camera over the single scrolling map. Its
                             base follows the hero; its additive offset
@@ -113,6 +116,14 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
              const EntityTable &shots, double dt);
 
+/* Lesson 091: the waves, once per frame of play. A fresh fight clears
+   the last one from the store; a wave spawns its composition from the
+   table's definitions (every kind whose row's wave has come, its row's
+   count of them); and the next wave begins when the last enemy of the
+   current one is retired. When the last wave is clear the waves are
+   complete — the named condition for victory, and no stand-in. */
+void GameWaves(Game &game, EntityStore &store, const EntityTable &foes);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index 3271085..c460681 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -164,20 +164,6 @@ static void PrintDefs(const char *path, const EntityTable &table)
     }
 }
 
-/* Lesson 088: one live entity, carrying its row's values — printed in
-   the same words as the definition above, so the carrying is checkable
-   by eye against the file's rows. The sprite prints as its dimensions
-   because the entity carries the row's art *loaded* — the image, not
-   the path that named it. */
-static void PrintEntity(const char *kind, const Entity &e)
-{
-    std::printf("engine: %s %s: x %d y %d facing %d speed %d health %d sprite %dx%d accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
-                kind, e.name, (int)e.x, (int)e.y, e.facing, e.speed, e.health,
-                e.sprite->width, e.sprite->height, e.accel, e.damage, e.rate,
-                e.fires[0] ? e.fires : "none", e.range, BehaviorName(e.behavior),
-                e.wave, e.count);
-}
-
 int Run(void)
 {
     platform::WindowResult opened =
@@ -356,24 +342,10 @@ int Run(void)
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
                 created, store.live, ENTITY_CAP);
 
-    /* Lesson 088: the enemy roster is data. The three types and the
-       boss are rows of the game's table; each row becomes one entity,
-       carrying its row's values in named fields the game reads
-       directly. A new row is a new enemy — the run has no per-kind code
-       to grow, and no per-type copy of any attribute to keep honest. */
-    for (int i = 0; i < foes.count; ++i) {
-        EntityResult made = EntityCreate(store, foes.rows[i]);
-        if (made.error != ENTITY_OK) {
-            std::fprintf(stderr, "engine: the store refused %s\n",
-                         foes.rows[i].name);
-            platform::CloseWindow(opened.window);
-            ArenaRelease(arena);
-            return 1;
-        }
-        PrintEntity("entity", *made.entity);
-    }
-    std::printf("engine: roster: %d enemies from the table's rows, live %d of %d\n",
-                foes.count, store.live, ENTITY_CAP);
+    /* Lesson 091: the enemy roster is the waves' now — lesson 088's
+       standing spawn gave way to the wave fight (GameWaves), which
+       spawns the same rows wave by wave. A new row is still a new
+       enemy: no per-kind code has appeared since. */
 
     /* Lesson 087: weapons are rows. The hero starts armed with the
        weapons table's first row; the number keys arm the rest
@@ -566,6 +538,14 @@ int Run(void)
            every live entity's request into motion through the mover (and
            every projectile into its flight). The loop times it as the
            frame record's entity sub-phase. */
+        /* Lesson 091: the waves — the fight's shape. A fresh game
+           clears the last fight; a wave spawns its composition from the
+           table's rows; the next begins when the last enemy of the
+           current one is retired; and the last wave's clear is the
+           game's completion. */
+        if (game.state == GAME_PLAY)
+            GameWaves(game, store, foes);
+
         double t_entities = platform::Now();
         int visited = GameWalk(store, map, hero, shots, dt);
         frame.entities = platform::Now() - t_entities;
diff --git a/src/table.h b/src/table.h
index c4d7572..a79fe0c 100644
--- a/src/table.h
+++ b/src/table.h
@@ -86,8 +86,9 @@ struct EntityDef {
     int range;                   /* world pixels: a projectile's flight
                                     budget — its life */
     int behavior;                /* BehaviorKind, its row's */
-    int wave;                    /* which wave spawns this kind;
-                                    0 = never by wave */
+    int wave;                    /* the wave this kind joins — it spawns
+                                    in that wave and every wave after
+                                    (lesson 091); 0 = never by wave */
     int count;                   /* how many of this kind join the wave */
 };
 
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the breath between waves *(extend-the-code)*

The next wave begins the instant the last enemy falls — no moment to
breathe, no sense of a stage ending. Give the fight an **intermission**:
when a wave is cleared, the next spawns a fixed number of *game*
seconds later (two, say), and the run says so — `wave 2 incoming` at
the clear, `wave 2 begins` when the breath is over. Game time, not
wall time: the breath must freeze with the pause and slow with a
hitstop, like everything the simulation does. Then run it and quote
the two lines' times.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-091/ex1.md)

### Exercise 2 — the wave plan *(predict-the-output)*

Before running anything, read the roster's rows and predict the whole
contest: for each of the game's three waves, which kinds spawn, how
many of each, and how many enemies stand in the wave at its start.
Then say what the *last* wave's battlefield looks like and why that is
the wave the game is built around. Compare against the lesson's quoted
runs — and then against a run of your own with one row's `wave` value
changed: what moved?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-091/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 090 — the boss](lesson-090-boss.md) ·
**Next:** [Lesson 092 — hitstop and screenshake](lesson-092-hitstop-shake.md) ·
**Code tag:** [`lesson-091`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-091)
