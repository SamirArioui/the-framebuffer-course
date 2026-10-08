# Lesson 093 — particle bursts and easing

{{#include ../../stability-horizon.md}}

## Prose

The toolkit is two effects from complete. Hitstop and screenshake have
been firing from the game's events since the last lesson; today come
the other two the contract allows — **particle bursts** and **easing**
— and with them the toolkit is done: four effects, and no fifth. What
arrives here is not a particle system bolted beside the engine. A
particle is **an entity** — a row of a table, a slot of the store, a
visit of the walk — and everything the store has ever promised (lesson
074's refusal that never steals) applies to it unchanged. What the
toolkit adds is the policy question the store left open: what happens
when cosmetic work and the game itself want the same slots.

### Particles are rows and slots

The spark is data — `assets/particles.txt`, one row in the grown
format, its columns named by its header:

```
name sprite accel range behavior
spark assets/spark.ppm 400 64 settle
```

The art (`assets/spark.ppm`, a 16×16 magenta-keyed glow) is its
`sprite`; the settle takes `accel` milliseconds and covers `range`
pixels — the two columns the format already had, in their same
meanings. `settle` is a new spelling of the `behavior` column: what a
kind does each frame, exactly like `fly` or `chase`. The run prints the
row beside every other kind's and nothing about it is special-cased:

```
engine: def spark: x 0 y 0 facing 0 speed 0 health 0 sprite assets/spark.ppm accel 400 damage 0 rate 0 fires none range 64 behavior settle wave 0 count 1
```

A burst fires from the event's own frame, like every feel effect — a
hit scatters a few sparks from the impact, a death scatters more from
the centre of whatever fell. The scratch roster of the killing blow
(one fragile standing kind, the numbers its own):

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: burst: spark x4 at 320,232 — 4 made, 0 dropped
engine: shot bolt retired — hit bag
engine: bag retired — zero health
engine: feel: hitstop fired (0.25x, 0.30s)
engine: feel: shake fired (10 px, 0.50s)
engine: burst: spark x8 at 344,240 — 8 made, 0 dropped
```

One frame again: the hit's burst and the death's burst sit inside the
same account as the feedback that landed them.

### The store's policy bounds the burst — and keeps the game's slots

Here is the policy question, answered: particles take the store's
slots, but never all of them. The toolkit's **cosmetic share**
(`FEEL_COSMETIC_SLOTS`) is the slice of the store the particles may
hold; the rest is kept for the game's own spawns. A burst that finds no
cosmetic slot **drops its particle** — cosmetic work may be dropped (a
dropped spark is invisible), and it is counted and reported. Gameplay
work may not be dropped: the store still refuses loudly and typed (the
contrast lesson 074 taught — a dropped enemy is a bug the player
experiences), and a flood of sparks can never cause one.

A stress run shows the whole policy at once — a flood of 24 sparks per
frame (a probe's flood, stated as such) while the hero keeps fighting:

```
engine: burst: spark x24 at 320,232 — 24 made, 0 dropped
engine: burst: spark x24 at 320,232 — 8 made, 16 dropped
engine: fire: hero -> bolt (damage 1, range 160)
engine: burst: spark x24 at 320,232 — 0 made, 24 dropped
engine: burst: spark x24 at 320,232 — 0 made, 24 dropped
engine: burst: spark x4 at 320,232 — 0 made, 4 dropped
engine: burst: spark x8 at 344,240 — 0 made, 8 dropped
engine: wave 1 cleared — the next begins
engine: wave 2: spawns bag at 336,232 — speed 0, health 1, chase
engine: wave 2 begins — 1 enemies
engine: burst: spark x24 at 320,232 — 0 made, 24 dropped
```

The first two floods fill the share (24 + 8 = 32 of the store's 64
slots) and everything after that is `0 made, N dropped` — and look at
what keeps landing through the drops: `fire: hero -> bolt` (the hero's
shot takes a slot) and `wave 2: spawns bag` (the wave's enemy takes
one). **A full cosmetic share drops particles while the game's spawns
are kept.** Not one gameplay entity was refused, displaced, or stolen;
the drops are all sparks, and the report counts every one.

### Move, settle, retire

A particle's life is one eased value. Each frame of game time it sits a
little further out along its lane — the distance follows the ease's
curve — and when its life ends it is exactly at its row's `range`, and
retires there. *Move, settle, retire.* The run reports each arrival:

```
engine: spark settled at 384,232 — 64 px out, its row's range 64 (exact)
engine: spark settled at 320,296 — 64 px out, its row's range 64 (exact)
engine: spark settled at 256,232 — 64 px out, its row's range 64 (exact)
engine: spark settled at 320,168 — 64 px out, its row's range 64 (exact)
engine: spark settled at 344,304 — 64 px out, its row's range 64 (exact)
engine: spark settled at 298,285 — 64 px out, its row's range 64 (exact)
engine: spark settled at 280,240 — 64 px out, its row's range 64 (exact)
engine: spark settled at 298,194 — 64 px out, its row's range 64 (exact)
engine: spark settled at 344,176 — 64 px out, its row's range 64 (exact)
engine: spark settled at 389,194 — 64 px out, its row's range 64 (exact)
engine: spark settled at 408,240 — 64 px out, its row's range 64 (exact)
engine: spark settled at 389,285 — 64 px out, its row's range 64 (exact)
```

Twelve arrivals, and every one of them says `exact`. The first four are
the hit's burst at `320,232` — the four cardinal lanes, 64 px out
each: `384`, `320,296`, `256`, `320,168`. The eight after are the
death's burst at `344,240` — all eight lanes: `408,240` due east,
`280,240` due west (`344 − 64`), `298,285` and `389,285` the diagonals
at `64/√2 ≈ 45.25` px out, and so on around the compass. The sparks
fly over the scene — no collision resolves them; they are cosmetic
motion through game time, and the pause freezes them mid-air like
everything else the simulation does.

One honest edge the design caught before the run did: a spark settles
in **game** time, so the burst fired by the blow that *ends* the game
would hang frozen over the end screen forever. The fresh fight's clear
sweeps the debris away with the last fight's fighters and shots — the
store starts clean, every game.

### Easing: values that arrive

The fourth effect is the smallest and the one everything else will lean
on: **easing** — a small set of shapes for values that animate from
where they are to where they belong. Each takes `t` in `[0, 1]` and
answers the fraction travelled — and here is the whole contract:

> an eased value **arrives** at its target — exactly 0 at 0, exactly 1
> at 1 — it does not approach it forever.

Three shapes ship: `EaseInQuad` (slow out, arriving with weight),
`EaseOutQuad` (fast out, settling into place — the settle's shape), and
`EaseInOutQuad`. A scratch probe of the three at the boundaries and the
midpoint (stated as such: a probe, not the shipped run):

```
engine: probe: ease at 0: in 0, out 0, inout 0
engine: probe: ease at 0.5: in 0.25, out 0.75, inout 0.5
engine: probe: ease at 1: in 1, out 1, inout 1
```

`out(0.5) = 0.75` is the settling curve's signature — three-quarters of
the way out at half the time — and every shape reads `1` at `1`, to the
last digit. The settle's own value, probed at full precision through
one spark's life:

```
engine: probe: spark traveled 48.825914620711856 at t 0.513
engine: probe: spark traveled 63.302683514143858 at t 0.896
engine: probe: spark traveled 63.695704497446457 at t 0.931
engine: probe: spark traveled 64 at t 1.000
engine: spark settled at 408,240 — 64 px out, its row's range 64 (exact)
```

Sixty-three-point-something at `t 0.931`, `63.99…` a frame later — and
then `64` at `t 1.000`, printed at 17 digits of precision as exactly
`64`: the row's range, not a neighbour of it. The intermediate frames
follow the ease (`64 × (1 − (1 − t)²)`, the shape's own arithmetic) and
the last frame **is** the target. This is what separates easing from
the exponential approach the hero's accel uses: both feel smooth, but
only one can be trusted to put a value where it belongs.

### What this run verified, and what it did not

- **A burst is bounded by the store's policy and its particles
  retire** — every burst reports `N made, M dropped` against the
  cosmetic share (`24 + 8` filled it in the stress run), and every
  particle ends with its own `settled` line — move, settle, retire.
- **A full store drops particles while gameplay spawns are kept** —
  through a flood of `0 made, 24 dropped`, the hero's `fire` and the
  wave's `spawns bag` keep taking their slots; the drops are sparks
  only, counted, and nothing already spawned is touched.
- **Eased values arrive exactly at their targets** — the ease set
  reads `1` at `1`; the settle's value is exactly `64` — its row's
  range — at `t 1.000`, at 17 digits, and the run prints `exact` from
  the arrival's own comparison.

What this run did **not** verify is how any of it *looks* — the burst's
spread and the spark's glow are drawn to a screen this machine does not
have. The numbers say the sparks go where the lanes say and settle
where the row says; whether that reads as an impact is the player's
judgment again. And the size of the cosmetic share — 32 slots — is a
policy choice, not a measurement: half the store seemed a sane budget,
and the day real play shows pressure on it is the day measurement (the
measure pass) moves it.

## Code step

One change: the toolkit's last two effects. `src/feel.h/.cpp` grow the
ease set (`EaseInQuad`/`EaseOutQuad`/`EaseInOutQuad`), the burst
(`FeelBurst` — the store's cosmetic share, the eight lanes, the
dropped-and-counted policy), and the particle's settle (`FeelParticle`
— the eased travel that arrives exactly and retires there).
`assets/particles.txt` and `assets/spark.ppm` are the spark kind —
data and art, like every kind. `src/table.h/.cpp` grow the `behavior`
vocabulary by `settle`; `src/entity.h` gives a particle its clock and
its burst point (its position is measured from there every frame, never
accumulated — the arrival must be the target, not a rounding of it);
`src/combat.cpp` fires the two bursts at the hit and the death and lets
shots fly through sparks; `src/game.cpp`'s walk settles particles and
its fresh-fight clear sweeps the debris. Its end state is tagged
`lesson-093`.

```diff
diff --git a/assets/particles.txt b/assets/particles.txt
new file mode 100644
index 0000000..df42f9f
--- /dev/null
+++ b/assets/particles.txt
@@ -0,0 +1,2 @@
+name sprite accel range behavior
+spark assets/spark.ppm 400 64 settle
diff --git a/src/combat.cpp b/src/combat.cpp
index a10ffb7..e040bb5 100644
--- a/src/combat.cpp
+++ b/src/combat.cpp
@@ -8,6 +8,9 @@
 //
 // Lesson 092: the hit and the death are the juice toolkit's events, and
 // the feedback hooks fire at the lines where they happen.
+//
+// Lesson 093: the same events burst particles — cosmetic entities of
+// the kind the game names, bounded by the store's policy.
 
 #include "combat.h"
 
@@ -116,7 +119,8 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
 }
 
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
-               Entity &shot, Feedback &feel, double dt)
+               Entity &shot, Feedback &feel, const EntityDef &spark,
+               double dt)
 {
     /* The flight, in game time: the shot's speed over dt, sub-stepped
        through the mover so each sub-step is small. What is checked at
@@ -137,6 +141,9 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                 continue;
             if (e.behavior == BEHAVIOR_FLY)
                 continue;
+            if (e.behavior == BEHAVIOR_SETTLE)
+                continue; /* a spark is cosmetic — a shot flies through
+                            it, the way it flies through other shots */
             if (shot.owner && SameName(e.name, shot.owner->name))
                 continue;
             if (Overlaps(shot, e))
@@ -161,6 +168,10 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
             FeelHitstop(feel, 0.25, 0.15);
             FeelShake(feel, 5.0, 0.25);
 
+            /* Lesson 093: and the impact scatters sparks — a burst of
+               particles from the same event's own frame. */
+            FeelBurst(store, spark, shot.x, shot.y, 4);
+
             std::printf("engine: shot %s retired — hit %s\n", shot.name,
                         target->name);
             EntityRetire(store, shot);
@@ -180,6 +191,12 @@ void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
                    as both, and the death's weights win. */
                 FeelHitstop(feel, 0.25, 0.30);
                 FeelShake(feel, 10.0, 0.50);
+
+                /* Lesson 093: and the death bursts harder — the thing
+                   that fell scatters its sparks from its own centre. */
+                FeelBurst(store, spark,
+                          target->x + ANIM_FRAME_W / 2.0,
+                          target->y + target->sprite->height / 2.0, 8);
             }
             return;
         }
diff --git a/src/combat.h b/src/combat.h
index fddb88a..abb52c9 100644
--- a/src/combat.h
+++ b/src/combat.h
@@ -59,9 +59,12 @@ bool CombatFire(EntityStore &store, const EntityTable &shots,
    defeat condition (the state machine reads it, the game's actor is not
    retired out from under the game). Lesson 092: the hit and the death
    are the toolkit's events — the feedback hooks fire here, in the
-   event's own frame, through `feel`. */
+   event's own frame, through `feel`. Lesson 093: they burst particles
+   of `spark` too — the cosmetic kind the game names, spawned in the
+   same frame from the same event. */
 void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
-               Entity &shot, Feedback &feel, double dt);
+               Entity &shot, Feedback &feel, const EntityDef &spark,
+               double dt);
 
 /* Lesson 090: the enemy attack, once per frame of game time. An armed
    entity — one whose row names a projectile kind — fires it at the
diff --git a/src/entity.h b/src/entity.h
index 2797cc7..85d0d37 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -62,6 +62,11 @@ struct Entity {
                                   thing a row cannot carry */
     double phase_t;            /* and how long this phase has run */
     double cooldown;           /* seconds until it may fire again */
+    double life_t;             /* lesson 093: how long a particle has
+                                  run — its eased settle's clock */
+    double from_x, from_y;     /* and the burst point it eases out
+                                  from — its position is measured from
+                                  here, never accumulated */
 };
 
 /* An entity created from a definition: every attribute its row states,
diff --git a/src/feel.cpp b/src/feel.cpp
index 76eb89a..5d769d8 100644
--- a/src/feel.cpp
+++ b/src/feel.cpp
@@ -1,10 +1,16 @@
-// feel.cpp — the feedback hooks: fire, decay, rest.
+// feel.cpp — the juice toolkit: the feedback hooks, the bursts, and the
+// easing that shapes them.
 //
 // Lesson 086: each hook fires, runs down its own wall-time, and returns
 // exactly to rest. Lesson 092: the game's own events fire them — a hit
 // lands, a death falls — in the event's own frame, and each hook says
 // when it fires beside the event's own line. The weights are the
 // event's, passed in from where the event happens.
+//
+// Lesson 093: the toolkit's other two effects live here too — particle
+// bursts (cosmetic entities, bounded by the store's policy) and the
+// small set of ease functions that make animated values arrive at their
+// targets instead of stepping to them.
 
 #include "feel.h"
 
@@ -43,6 +49,121 @@ double FeelTimeScale(const Feedback &feel)
     return feel.hitstop > 0.0 ? feel.hitstop_k : GAMETIME_FULL;
 }
 
+/* Lesson 093: easing. Each shape takes t in [0, 1] and answers the
+   fraction travelled — exactly 0 at 0, exactly 1 at 1. The clamps at
+   both ends are the "arrives exactly" contract: past its target the
+   value sits at its target, and an eased value never overshoots. */
+double EaseInQuad(double t)
+{
+    if (t <= 0.0)
+        return 0.0;
+    if (t >= 1.0)
+        return 1.0;
+    return t * t;
+}
+
+double EaseOutQuad(double t)
+{
+    if (t <= 0.0)
+        return 0.0;
+    if (t >= 1.0)
+        return 1.0;
+    double u = 1.0 - t;
+    return 1.0 - u * u;
+}
+
+double EaseInOutQuad(double t)
+{
+    if (t <= 0.0)
+        return 0.0;
+    if (t >= 1.0)
+        return 1.0;
+    return t < 0.5 ? 2.0 * t * t : 1.0 - 2.0 * (1.0 - t) * (1.0 - t);
+}
+
+/* Lesson 093: the burst's eight lanes — the world's compass points, a
+   diagonal at 1/sqrt(2), the same lanes the aim and the movement use.
+   A burst is reproducible: lane i of a count is the same direction on
+   every machine and every run. */
+constexpr double BURST_DIAG = 0.70710678;
+const double LANE_X[8] = { 1.0, BURST_DIAG, 0.0, -BURST_DIAG, -1.0,
+                           -BURST_DIAG, 0.0, BURST_DIAG };
+const double LANE_Y[8] = { 0.0, BURST_DIAG, 1.0, BURST_DIAG, 0.0,
+                           -BURST_DIAG, -1.0, -BURST_DIAG };
+
+int FeelBurst(EntityStore &store, const EntityDef &kind, double x, double y,
+              int count)
+{
+    /* The cosmetic share, counted before the burst asks: the toolkit's
+       particles may hold FEEL_COSMETIC_SLOTS of the store's slots and
+       no more. */
+    int cosmetic = 0;
+    for (int i = 0; i < ENTITY_CAP; ++i)
+        if (store.slots[i].live && store.slots[i].behavior == BEHAVIOR_SETTLE)
+            cosmetic += 1;
+
+    int made = 0, dropped = 0;
+    for (int i = 0; i < count; ++i) {
+        /* A burst that finds no slot drops its particle — cosmetic work
+           may be dropped (counted, and invisible); gameplay work may
+           not (lesson 074's contrast). Nothing is ever stolen. */
+        if (cosmetic >= FEEL_COSMETIC_SLOTS) {
+            dropped += 1;
+            continue;
+        }
+        EntityResult result = EntityCreate(store, kind);
+        if (result.error != ENTITY_OK) {
+            dropped += 1;
+            continue;
+        }
+        Entity &e = *result.entity;
+        int lane = count < 8 ? (i * 8) / count : i % 8;
+        e.x = x;
+        e.y = y;
+        e.from_x = x;
+        e.from_y = y;
+        e.move_x = LANE_X[lane];
+        e.move_y = LANE_Y[lane];
+        e.traveled = 0.0;
+        e.life_t = 0.0;
+        cosmetic += 1;
+        made += 1;
+    }
+    std::printf("engine: burst: %s x%d at %d,%d — %d made, %d dropped\n",
+                kind.name, count, (int)x, (int)y, made, dropped);
+    return made;
+}
+
+void FeelParticle(EntityStore &store, Entity &e, double dt)
+{
+    /* The settle's clock runs on game time: a pause freezes a spark
+       mid-air, a hitstop slows it — the world's clock, like everything
+       the simulation does. */
+    e.life_t += dt;
+    double t = e.accel > 0 ? e.life_t / (e.accel / 1000.0) : 1.0;
+    if (t > 1.0)
+        t = 1.0;
+
+    /* The eased value is the distance out: it follows the ease's curve
+       frame by frame and arrives exactly at the row's range. The
+       position is that distance along the spark's lane — measured from
+       the burst point every frame, never accumulated step by step, so
+       the arrival is the target and not a rounding of it. */
+    e.traveled = (double)e.range * EaseOutQuad(t);
+    e.x = e.from_x + e.move_x * e.traveled;
+    e.y = e.from_y + e.move_y * e.traveled;
+
+    if (t >= 1.0) {
+        /* The life ends where it settles — and the arrival is exact:
+           the eased value is the target, not a neighbour of it. */
+        bool exact = e.traveled == (double)e.range;
+        std::printf("engine: %s settled at %d,%d — %g px out, its row's range %d (%s)\n",
+                    e.name, (int)e.x, (int)e.y, e.traveled, e.range,
+                    exact ? "exact" : "drifted");
+        EntityRetire(store, e);
+    }
+}
+
 void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera)
 {
     /* The hitstop runs on its own wall-time and returns to full speed
diff --git a/src/feel.h b/src/feel.h
index 5827080..d4f1909 100644
--- a/src/feel.h
+++ b/src/feel.h
@@ -1,4 +1,4 @@
-// feel.h — the feedback hooks the juice toolkit drives.
+// feel.h — the juice toolkit the game's events drive.
 //
 // Lesson 086: two hooks — a screenshake and a hitstop — each a thing
 // that fires and then rests. Lesson 092: the toolkit fires them from the
@@ -7,14 +7,28 @@
 // are the mechanisms, each with its own fire-and-rest life; the events
 // decide when and how heavily. A hook at rest costs nothing and changes
 // nothing.
+//
+// Lesson 093: the toolkit's other two effects — particle bursts (the
+// store's cosmetic work) and easing (values that arrive at their
+// targets). Four effects, and no fifth: hitstop, screenshake, particle
+// bursts, easing.
 #ifndef FEEL_H
 #define FEEL_H
 
 #include "camera.h"
+#include "entity.h"
 #include "gametime.h"
 
 namespace engine {
 
+/* Lesson 093: the store's cosmetic share — the slots the toolkit's
+   particles may hold. A burst takes these slots and no others: the rest
+   of the store is kept for the game's own spawns, so a flood of sparks
+   drops cosmetic work and never a gameplay one. A dropped particle is
+   invisible; a dropped enemy is a bug the player experiences (lesson
+   074's contrast). */
+constexpr int FEEL_COSMETIC_SLOTS = ENTITY_CAP / 2;
+
 /* The feedback state: what is still firing. Every field is at rest at
    zero — a hook that has fired and finished leaves itself exactly here. */
 struct Feedback {
@@ -45,6 +59,30 @@ double FeelTimeScale(const Feedback &feel);
    offset (resting at exactly zero). */
 void FeelUpdate(Feedback &feel, double wall_dt, Camera &camera);
 
+/* Lesson 093: easing — a small set of shapes for values that animate
+   from where they are to where they belong. Each takes t in [0, 1] and
+   answers the fraction travelled: exactly 0 at 0, exactly 1 at 1. The
+   ends are clamped on purpose — an eased value arrives at its target,
+   it does not approach it forever. */
+double EaseInQuad(double t);     /* slow out, arriving with weight */
+double EaseOutQuad(double t);    /* fast out, settling into place */
+double EaseInOutQuad(double t);  /* both */
+
+/* Lesson 093: a particle burst — the toolkit's cosmetic spawn, fired in
+   the triggering event's own frame like every feel effect. Up to
+   `count` particles of `kind` take the store's cosmetic slots and ease
+   out from (x, y) along the burst's lanes; whatever finds no slot is
+   dropped and counted — cosmetic work may be dropped, gameplay work may
+   not. Answers how many were made. */
+int FeelBurst(EntityStore &store, const EntityDef &kind, double x, double y,
+              int count);
+
+/* One particle's settle, once a frame of game time: its eased travel
+   out from its burst point — following the ease's curve and arriving
+   exactly at its row's range — and its retirement there, when its life
+   ends. */
+void FeelParticle(EntityStore &store, Entity &e, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/game.cpp b/src/game.cpp
index 3da68a4..d52e01d 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -218,7 +218,8 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 }
 
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             const EntityTable &shots, Feedback &feel, double dt)
+             const EntityTable &shots, Feedback &feel, const EntityDef &spark,
+             double dt)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -250,8 +251,12 @@ int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
         Entity &e = store.slots[i];
         switch (e.behavior) {
         case BEHAVIOR_FLY:
-            CombatFly(map, store, hero, e, feel, dt);
+            CombatFly(map, store, hero, e, feel, spark, dt);
             continue; /* the flight moves itself, through the mover */
+        case BEHAVIOR_SETTLE:
+            FeelParticle(store, e, dt);
+            continue; /* the settle moves itself — eased travel, no
+                        collision: sparks fly over the scene */
         case BEHAVIOR_CHASE:
             AiChase(e, hero);
             break;
@@ -300,16 +305,19 @@ static bool IsFighter(const Entity &e)
 
 void GameWaves(Game &game, EntityStore &store, const EntityTable &foes)
 {
-    /* A fresh fight: the last game's fighters and shots leave the store
-       — the hero is the game's actor and the `none` kinds are the
-       world's scenery, and both stay. */
+    /* A fresh fight: the last game's fighters, shots, and debris leave
+       the store — the hero is the game's actor and the `none` kinds are
+       the world's scenery, and both stay. The debris goes too (lesson
+       093): a spark settles in game time, and a frozen one — from the
+       blow that ended the last game — would hang there forever. */
     if (game.wave == 0) {
         int cleared = 0;
         for (int i = 0; i < ENTITY_CAP; ++i) {
             Entity &e = store.slots[i];
             if (!e.live)
                 continue;
-            if (IsFighter(e) || e.behavior == BEHAVIOR_FLY) {
+            if (IsFighter(e) || e.behavior == BEHAVIOR_FLY ||
+                e.behavior == BEHAVIOR_SETTLE) {
                 EntityRetire(store, e);
                 cleared += 1;
             }
diff --git a/src/game.h b/src/game.h
index 30dcc3e..92f49dc 100644
--- a/src/game.h
+++ b/src/game.h
@@ -114,10 +114,13 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
    the way the player's input writes the hero's, and an armed entity
    attacks at its row's rate. Lesson 092: the combat's events (a hit, a
    death) fire the feedback hooks through `feel`, in their own frame.
-   The hero is handed along for the combat's rules to know the game's
-   actor by. Returns the visit count. */
+   Lesson 093: the toolkit's particles settle here (FeelParticle) and
+   burst of `spark` — the cosmetic kind the game names. The hero is
+   handed along for the combat's rules to know the game's actor by.
+   Returns the visit count. */
 int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
-             const EntityTable &shots, Feedback &feel, double dt);
+             const EntityTable &shots, Feedback &feel, const EntityDef &spark,
+             double dt);
 
 /* Lesson 091: the waves, once per frame of play. A fresh fight clears
    the last one from the store; a wave spawns its composition from the
diff --git a/src/main.cpp b/src/main.cpp
index a115e30..472c017 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -245,11 +245,14 @@ int Run(void)
        entity of. Each file's header names the columns it uses — and only
        those; what it leaves unnamed sits at the format's defaults.
        Lesson 088: and the enemy roster — the three types and the boss,
-       every per-type fact its own row's value. */
-    EntityTable weapons, shots, foes;
+       every per-type fact its own row's value. Lesson 093: and the
+       toolkit's particle kinds — cosmetic entities from rows like every
+       other kind, the burst's art and settle in the table's columns. */
+    EntityTable weapons, shots, foes, particles;
     if (!LoadRunTable(arena, "assets/weapons.txt", weapons) ||
         !LoadRunTable(arena, "assets/projectiles.txt", shots) ||
-        !LoadRunTable(arena, "assets/enemies.txt", foes)) {
+        !LoadRunTable(arena, "assets/enemies.txt", foes) ||
+        !LoadRunTable(arena, "assets/particles.txt", particles)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
@@ -265,11 +268,12 @@ int Run(void)
     PrintDefs("assets/weapons.txt", weapons);
     PrintDefs("assets/projectiles.txt", shots);
     PrintDefs("assets/enemies.txt", foes);
+    PrintDefs("assets/particles.txt", particles);
 
     /* Lesson 073: the definitions' art, loaded at startup. A row that
        names no sprite (a weapon row) has no art and needs none. */
     if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots) ||
-        !LoadRunArt(arena, foes)) {
+        !LoadRunArt(arena, foes) || !LoadRunArt(arena, particles)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
@@ -355,6 +359,19 @@ int Run(void)
     if (weapons.count > 0)
         CombatArm(hero, weapons.rows[0]);
 
+    /* Lesson 093: the burst kind — the particles table's first row. The
+       game bursts what the table puts first, the way the hero arms with
+       the weapons table's first row; a table with no particle kind is a
+       named failure, never a burst of assumed attributes. */
+    if (particles.count == 0) {
+        std::fprintf(stderr,
+                     "engine: assets/particles.txt: no particle kind\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    const EntityDef &spark = particles.rows[0];
+
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
        attributes. */
@@ -533,7 +550,7 @@ int Run(void)
             GameWaves(game, store, foes);
 
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, hero, shots, feel, dt);
+        int visited = GameWalk(store, map, hero, shots, feel, spark, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
diff --git a/src/table.cpp b/src/table.cpp
index fd179ab..0b07d3e 100644
--- a/src/table.cpp
+++ b/src/table.cpp
@@ -133,7 +133,7 @@ const char *const COLUMN_NAMES[COL_COUNT] = {
 
 /* Lesson 087: the behavior column's spellings, the format's own. */
 const char *const BEHAVIOR_NAMES[BEHAVIOR_COUNT] = {
-    "none", "fly", "chase", "keep", "flee", "boss"
+    "none", "fly", "chase", "keep", "flee", "boss", "settle"
 };
 
 /* Lesson 087: one row at the format's defaults, before the named fields
diff --git a/src/table.h b/src/table.h
index a79fe0c..9fb504e 100644
--- a/src/table.h
+++ b/src/table.h
@@ -46,7 +46,9 @@ constexpr int TABLE_PATH_MAX = 64;
    frame, the fact its row carries. The format defines the spellings,
    like it defines facing's four numbers: `none` stands still, `fly` is a
    projectile in flight, and chase / keep / flee / boss are the enemy
-   behaviors (lessons 089-090) — the boss's value names its pattern. */
+   behaviors (lessons 089-090) — the boss's value names its pattern.
+   Lesson 093: `settle` is a burst's particle — eased travel out from
+   its burst point, retiring where it settles. */
 enum BehaviorKind {
     BEHAVIOR_NONE = 0,
     BEHAVIOR_FLY,
@@ -54,6 +56,7 @@ enum BehaviorKind {
     BEHAVIOR_KEEP,
     BEHAVIOR_FLEE,
     BEHAVIOR_BOSS,
+    BEHAVIOR_SETTLE,
     BEHAVIOR_COUNT
 };
 
@@ -78,13 +81,16 @@ struct EntityDef {
        its default when a file's header does not name it — the defaults
        are the format's contract, not a gap in it. */
     int accel;                   /* ms: the eased-move time constant —
-                                    the feel (the hero's weight) */
+                                    the feel (the hero's weight; a
+                                    particle's settle takes this long,
+                                    lesson 093) */
     int damage;                  /* points a hit removes — a weapon
                                     row's damage, carried by its shots */
     int rate;                    /* rounds per minute; 0 = never fires */
     char fires[TABLE_NAME_MAX];  /* the projectile kind this row fires */
     int range;                   /* world pixels: a projectile's flight
-                                    budget — its life */
+                                    budget — its life (a particle's
+                                    settle distance, lesson 093) */
     int behavior;                /* BehaviorKind, its row's */
     int wave;                    /* the wave this kind joins — it spawns
                                     in that wave and every wave after
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the sparks inherit the blow *(extend-the-code)*

A hit's sparks and a death's sparks look the same today: both bursts
spray every lane alike. Make the burst **directional**: a hit's sparks
should spray the way the shot was flying — the blow carries its
momentum into the scatter — while a death keeps its burst in every
direction. Then run a killing blow where the shot flies east and quote
where the hit's four sparks settle beside the death's eight.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-093/ex1.md)

### Exercise 2 — the curve, predicted *(predict-the-output)*

Read `EaseOutQuad` — the shape the settle travels along — and predict
the run before you make it: for the shipped spark row (`accel 400`,
`range 64`), how far out is a spark halfway through its settle, and
where does it end? Then run it with a probe that prints the eased value
every frame at full precision, and compare — including the question the
whole effect turns on: does the last frame land **on** the target, or
merely near it?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-093/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 092 — hitstop and screenshake](lesson-092-hitstop-shake.md) ·
**Next:** [Lesson 094 — the HUD](lesson-094-hud.md) ·
**Code tag:** [`lesson-093`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-093)
