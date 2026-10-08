# Lesson 089 — enemy AI

{{#include ../../stability-horizon.md}}

## Prose

The roster stands at its row's `x` and `y` and does nothing. This
lesson gives it intent: **chase, keep-distance, flee** — the three
behaviors the enemy types were scoped around, each a small function
that writes an entity's movement request. The requests go through the
mover like every entity's; the walk's per-entity work gains one branch
on the behavior the row already carries. No per-type update, no entity
machinery — design D6's shape, exactly.

### A behavior writes a request

The player's input writes the hero's movement request; a behavior
writes an enemy's, the same way — a direction of the eight the
movement knows, scaled by the entity's own speed when the walk moves
it. Three functions, each about one number: the distance between the
entity and the hero.

- **Chase** points at the hero, every frame. The distance shrinks.
- **Flee** points away, every frame. The distance grows.
- **Keep-distance** points at the hero when it is too far, away when it
  is too close, and rests in between — a band around `AI_KEEP` (160 px,
  the behavior's fact, not any kind's) so a keeper at its distance
  stands instead of twitching across the line.

```cpp
double dx = hero.x - e.x, dy = hero.y - e.y;
double d2 = dx * dx + dy * dy;          /* the distance, squared */
if (d2 > far * far)        AiChase(e, hero);
else if (d2 < near * near) AiFlee(e, hero);
else { e.move_x = 0.0; e.move_y = 0.0; }   /* at its distance */
```

The distance is compared squared — no square root runs at run time
anywhere in the behaviors (the report's arithmetic is the one
exception, and it is a report). And the direction is the **compass
point** of the delta, the same eight directions the hero's aim uses:
the world moves in eight lanes, and the enemies run in them too.

### One branch, per-entity work expressed once

The walk's per-entity work branches on the entity's **behavior — the
fact its row carries** — and nothing else:

```cpp
switch (e.behavior) {
case BEHAVIOR_FLY:   CombatFly(map, store, hero, e, dt); continue;
case BEHAVIOR_CHASE: AiChase(e, hero); break;
case BEHAVIOR_KEEP:  AiKeep(e, hero); break;
case BEHAVIOR_FLEE:  AiFlee(e, hero); break;
default: break;  /* `none` stands; `boss` is lesson 090's pattern */
}
```

Every behavior then falls through to the same `MoveEntity` call the
hero uses — intent becomes motion only where the map allows it. The
bat, the wisp, the spitter, and the golem do not exist in this code at
all; they are four values of one switch. The boss, next lesson, is a
fifth value — composed of these same three behaviors plus a schedule —
not a fifth shape.

### The three behaviors, measured

One run with the hero standing still moves all three kinds at once. The
report samples each entity as it travels a tile, its distance to the
hero beside it — the number every behavior is about:

```
engine: bat at 389,72 — 177 px of the hero (t=1.504)
engine: bat at 368,93 — 149 px of the hero (t=1.692)
engine: bat at 353,112 — 126 px of the hero (t=1.865)
engine: bat at 349,137 — 101 px of the hero (t=2.081)
engine: bat at 329,156 — 77 px of the hero (t=2.253)
engine: bat at 310,176 — 55 px of the hero (t=2.426)
engine: wisp at 640,463 — 401 px of the hero (t=1.504)
engine: wisp at 659,479 — 426 px of the hero (t=1.736)
engine: wisp at 685,479 — 447 px of the hero (t=2.038)
engine: wisp at 710,479 — 469 px of the hero (t=2.340)
engine: wisp at 736,479 — 491 px of the hero (t=2.642)
engine: spitter at 222,297 — 111 px of the hero (t=1.504)
engine: spitter at 203,316 — 137 px of the hero (t=1.779)
```

The **chase** shrinks: 177 → 149 → 126 → 101 → 77 → 55, the bat
closing on the hero down its diagonal lane. The **flee** grows: 401 →
426 → 447 → 469 → 491 — and look at the wisp's `y`: pinned at `479`,
`479`, `479` while its `x` slides right. That is the **mover** at
work: the wisp ran out of world, the wall refused the step, and the
movement along the wall still happened — the same slide rule the hero
has used since lesson 084. Every behavior moves through `MoveEntity`;
the map has the last word.

The **keep-distance** is the quietest one. Its first report already
shows it correcting: the spitter sat 111 px away — too close for its
160 — and backed off. Where it settled, from the run's closing
account:

```
engine: world: slime ends at 400,320 — 124 px of the hero
engine: world: bat ends at 316,176 — 55 px of the hero
engine: world: wisp ends at 736,480 — 491 px of the hero
engine: world: spitter ends at 191,328 — 154 px of the hero
engine: world: golem ends at 384,96 — 153 px of the hero
```

`154 px` — inside the band the behavior keeps (152 to 168), and
holding. The golem is at `153 px` only by coincidence of where its row
stands: its behavior is `boss`, and the boss's pattern is next
lesson's. The slime is at its row's spot because its row says `none`
and lesson 084's stand-in walk is gone now that the real behaviors
exist.

### What this run verified, and what it did not

- **Each behavior moves its entity through the mover** — the chase's
  distance shrinking (177 → 55), the flee's growing (401 → 491) and
  held at the wall by the mover (its `y` fixed, its `x` sliding), the
  keeper settling at 154 px of its 160 and holding.
- **Per-entity work is expressed once** — the walk's one switch on the
  row's behavior; the kinds appear in the run's reports and nowhere in
  the code.

What this run did **not** verify is anything *attacking*: the
behaviors write movement and nothing else — the enemy rows' weapons
(`damage`, `rate`, `fires`) are still only carried. Nor does the boss
do anything but stand. Lesson 090 assembles the boss from these three
behaviors plus a schedule of its own, and that is where the world
starts shooting back.

## Code step

One change: the behaviors. `src/ai.h` and `src/ai.cpp` are new — three
small functions writing an entity's movement request (`AiChase`,
`AiKeep`, `AiFlee`) and the keep distance the behavior keeps.
`src/game.cpp`'s walk grows the one branch on the row's behavior (the
projectile's `fly` was already there); `src/main.cpp` drops lesson
084's stand-in walk (the behaviors are real now) and reports the
world's motion as it happens — each entity as it travels a tile, its
distance to the hero beside it — and where it ended. Its end state is
tagged `lesson-089`.

```diff
diff --git a/src/ai.cpp b/src/ai.cpp
new file mode 100644
index 0000000..1ed2dc4
--- /dev/null
+++ b/src/ai.cpp
@@ -0,0 +1,48 @@
+// ai.cpp — the enemy behaviors, one request at a time.
+//
+// Lesson 089: a behavior is a small function of the entity, the hero,
+// and what the entity's row already carried — nothing else. Each writes
+// the entity's movement request; the walk turns every request into
+// motion through the mover, exactly as it does for the hero. No
+// behavior knows about kinds, rows, or the store.
+
+#include "ai.h"
+
+#include "combat.h" /* CombatAim — the same eight compass points */
+
+namespace engine {
+
+void AiChase(Entity &e, const Entity &hero)
+{
+    /* The request is the compass point at the hero — the same eight
+       directions the player's arrows write for the hero, so the chaser
+       covers ground at its own speed whichever way it runs. */
+    CombatAim(hero.x - e.x, hero.y - e.y, e.move_x, e.move_y);
+}
+
+void AiKeep(Entity &e, const Entity &hero)
+{
+    /* The distance is squared throughout — no square root runs at run
+       time. Beyond the band the keeper closes; inside it, it backs
+       away; within it, it stands at its distance. */
+    double dx = hero.x - e.x, dy = hero.y - e.y;
+    double d2 = dx * dx + dy * dy;
+    double near = AI_KEEP - AI_KEEP_BAND;
+    double far = AI_KEEP + AI_KEEP_BAND;
+    if (d2 > far * far)
+        AiChase(e, hero);
+    else if (d2 < near * near)
+        AiFlee(e, hero);
+    else {
+        e.move_x = 0.0;
+        e.move_y = 0.0;
+    }
+}
+
+void AiFlee(Entity &e, const Entity &hero)
+{
+    /* Away — the compass point of the reverse delta. */
+    CombatAim(e.x - hero.x, e.y - hero.y, e.move_x, e.move_y);
+}
+
+} /* namespace engine */
diff --git a/src/ai.h b/src/ai.h
new file mode 100644
index 0000000..a2b1ee7
--- /dev/null
+++ b/src/ai.h
@@ -0,0 +1,42 @@
+// ai.h — the enemy behaviors: chase, keep-distance, flee.
+//
+// Lesson 089: three small functions, each writing one entity's movement
+// request the way the player's input writes the hero's — a direction of
+// the eight the movement knows — and every behavior moves through the
+// mover (MoveEntity) like every entity does. The walk branches once on
+// the entity's behavior, the fact its row carries (design D6):
+// per-entity work is expressed once, not per type. The boss (lesson
+// 090) composes these same three with a schedule of its own — its own
+// pattern, never its own movement machinery.
+//
+// This is the game layer's AI file pair, beside the services (D2).
+#ifndef AI_H
+#define AI_H
+
+#include "entity.h"
+
+namespace engine {
+
+/* Lesson 089: the distance the keep-distance behavior keeps, in world
+   pixels — the behavior's fact, not any kind's: every kind that keeps,
+   keeps this far. */
+constexpr double AI_KEEP = 160.0;
+
+/* The band inside which "close enough" holds — a keeper at its
+   distance stands instead of twitching across the line. */
+constexpr double AI_KEEP_BAND = 8.0;
+
+/* Chase: the request points at the hero, every frame. */
+void AiChase(Entity &e, const Entity &hero);
+
+/* Keep-distance: the request points at the hero when it is too far,
+   away when it is too close, and is rest at the distance — the ranged
+   kind's habit: near enough to shoot, far enough to live. */
+void AiKeep(Entity &e, const Entity &hero);
+
+/* Flee: the request points away from the hero, every frame. */
+void AiFlee(Entity &e, const Entity &hero);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/game.cpp b/src/game.cpp
index 33101e3..11276a1 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -12,6 +12,7 @@
 #include <cstdio>
 
 #include "blit.h"
+#include "ai.h"
 #include "combat.h"
 #include "text.h"
 #include "tilemap.h"
@@ -239,17 +240,38 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
        Lesson 087: the per-entity work branches on the entity's behavior
        — the fact its row carries. A projectile flies: its own
        sub-stepped flight through the same mover, retiring at walls, at
-       its range's end, and at the entity it hit. Every other behavior
-       leaves the request for the mover below. */
+       its range's end, and at the entity it hit.
+
+       Lesson 089: the rest of the branch is the enemy behaviors —
+       chase, keep-distance, flee — each a small function writing this
+       entity's movement request the way the player's input writes the
+       hero's. One branch on the behavior, per-entity work expressed
+       once: the boss (lesson 090) is one more value here, not one more
+       shape. */
     int visited = 0;
     for (int i = 0; i < ENTITY_CAP; ++i) {
         if (!store.slots[i].live)
             continue;
         visited += 1;
         Entity &e = store.slots[i];
-        if (e.behavior == BEHAVIOR_FLY) {
+        switch (e.behavior) {
+        case BEHAVIOR_FLY:
             CombatFly(map, store, hero, e, dt);
-            continue;
+            continue; /* the flight moves itself, through the mover */
+        case BEHAVIOR_CHASE:
+            AiChase(e, hero);
+            break;
+        case BEHAVIOR_KEEP:
+            AiKeep(e, hero);
+            break;
+        case BEHAVIOR_FLEE:
+            AiFlee(e, hero);
+            break;
+        default:
+            /* `none` stands where it stands — the request is its row's
+               (lesson 084's stand-in walk writes one) — and `boss` is
+               lesson 090's pattern, composed of these same behaviors. */
+            break;
         }
         MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
         if (e.move_x > 0.0)
diff --git a/src/main.cpp b/src/main.cpp
index 6a3320a..4c5a16f 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -9,6 +9,7 @@
 // is invented here; today the parts fit, and the fit is what the demo
 // shows. The language law of lesson 026 still holds over all of it.
 
+#include <cmath>
 #include <cstdio>
 
 #include "arena.h"
@@ -347,12 +348,10 @@ int Run(void)
             ArenaRelease(arena);
             return 1;
         }
-        /* Lesson 084: a non-hero entity walks (down-right) so its
-           movement is resolved against the map like the hero's — a stand
-           in for the AI lesson 089 brings. The walk slides it along
-           walls and stops it at solid tiles. */
-        made.entity->move_x = 1.0;
-        made.entity->move_y = 1.0;
+        /* Lesson 084: a non-hero entity walked (down-right) here — a
+           stand-in for the AI. Lesson 089 replaced it: the behaviors
+           are real now, and the world's kinds move the ways their rows
+           say (the slime's row says `none`, so it stands). */
         if (!foe)
             foe = made.entity;
         created += 1;
@@ -441,6 +440,8 @@ int Run(void)
     bool was_blocked = false; /* lesson 077: the mover's state report */
     int was_vx = 0, was_vy = 0; /* lesson 085: the hero's velocity, as it eases */
     int was_frame = 0;          /* lesson 086: the hero's walk-cycle frame */
+    double seen_x[ENTITY_CAP] = {}, seen_y[ENTITY_CAP] = {}; /* lesson 089:
+                                  where each entity was last reported */
 
     /* The slice's identity: what the run is, named at once — L0*, the
        gate this part closes on. Every service it uses was finished
@@ -587,6 +588,26 @@ int Run(void)
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
+        /* Lesson 089: the world's motion, as the behaviors produce it —
+           every non-hero entity reported as it travels about a tile, its
+           distance to the hero beside it (the number all three behaviors
+           are about: chase shrinks it, flee grows it, keep holds it). */
+        for (int i = 0; i < ENTITY_CAP; ++i) {
+            Entity &e = store.slots[i];
+            if (!e.live || &e == &hero)
+                continue;
+            double dx = e.x - seen_x[i], dy = e.y - seen_y[i];
+            if (dx * dx + dy * dy < 24.0 * 24.0)
+                continue;
+            seen_x[i] = e.x;
+            seen_y[i] = e.y;
+            double to_x = hero.x - e.x, to_y = hero.y - e.y;
+            std::printf("engine: %s at %d,%d — %d px of the hero (t=%.3f)\n",
+                        e.name, (int)e.x, (int)e.y,
+                        (int)std::sqrt(to_x * to_x + to_y * to_y),
+                        platform::Now() - started);
+        }
+
         /* The score, and the hero's own report: where the entity the
            game moves has got to. */
         distance += (hero.x > was_x ? hero.x - was_x : was_x - hero.x) +
@@ -790,6 +811,21 @@ int Run(void)
     std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
                 frame_number, feeds, effect_count, music_wraps);
 
+    /* Lesson 089: where the behaviors left the world — every live
+       entity's position and its distance to the hero, the number all
+       three behaviors are about (chase shrinks it, flee grows it, keep
+       holds it). The report above samples a moving world; this one
+       states where it ended. */
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        Entity &e = store.slots[i];
+        if (!e.live || &e == &hero)
+            continue;
+        double to_x = hero.x - e.x, to_y = hero.y - e.y;
+        std::printf("engine: world: %s ends at %d,%d — %d px of the hero\n",
+                    e.name, (int)e.x, (int)e.y,
+                    (int)std::sqrt(to_x * to_x + to_y * to_y));
+    }
+
     /* Lesson 075: the walk's account — one visit per live entity per
        frame, and nothing else. */
     std::printf("engine: walk: %ld visits over %ld frames — one per live entity per frame\n",
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the keeper's distance is data *(extend-the-code)*

`AI_KEEP` is one number for every keeper — but "per-type attributes are
data" has a cousin: the behavior's tuning should be where tuning lives.
Make the keep distance **a row's fact**: grow the format the way lesson
087 grew it (named, additive, defaulted — the files that name the
column state it, the files that do not keep the default), carry it like
every other value, and have `AiKeep` use the entity's own number. Then
run two keepers at two distances in one world and quote the closing
account's lines. What does the default buy a file that stays silent?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-089/ex1.md)

### Exercise 2 — the chaser's staircase *(predict-the-output)*

The behaviors write **compass points** — the eight directions — never
a direction between them. So a chaser's path is a staircase. Predict
the bat's path precisely: starting at its row's `(560, 72)` with the
hero standing at `(312, 232)`, what sequence of moves does `AiChase`
ask for, when does the path stop being diagonal, and what does the last
leg look like? Then say what happens when a wall sits across the
diagonal: which axis does the mover resolve first, and what does the
staircase look like as the bat slides? Run it and compare against the
lesson's quoted path.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-089/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 088 — enemy archetype tables](lesson-088-enemy-tables.md) ·
**Next:** [Lesson 090 — the boss](lesson-090-boss.md) ·
**Code tag:** [`lesson-089`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-089)
