# Lesson 087 — projectiles and two weapons

{{#include ../../stability-horizon.md}}

## Prose

The hero walks with weight and animates while it does — and nothing in
the world fights back. This lesson is combat: **two weapons, and the
projectiles they fire.** It is also, first, a lesson about data —
because the table format of lessons 071-072 cannot carry what a weapon
and a projectile know, and the way it grows is the design decision this
whole batch of lessons stands on.

### The format grows by named columns

What does combat need to know? How much damage a hit does. How often a
weapon fires. Which projectile kind a weapon sends out. How far a
projectile flies before its life ends. The seven columns of
`assets/entities.txt` — `name x y facing speed health sprite` — carry
none of it. There were tempting cheats: health could be read as damage
"for the kinds where that fits", the facing column could hold a weapon
id. That is the failure lesson 071 named: *the wrong game that works* —
the values lie about what they are, and no report ever catches it.

So the format grows **by named columns, additively**. `EntityDef` and
the loader gain fields; a file's header names the columns it uses; the
loader fills the named fields and **leaves the rest at their defaults**.
The grown format knows eight columns more:

| Column | Type | Default | What it carries |
| ------ | ---- | ------- | --------------- |
| `accel` | ms | 120 | the eased-move time constant — the feel |
| `damage` | points | 0 | what a hit removes (a weapon row's damage) |
| `rate` | rounds/min | 0 | how often a weapon fires; 0 = never |
| `fires` | text | none | the projectile kind a weapon fires |
| `range` | world px | 0 | a projectile's flight budget — its life |
| `behavior` | text | `none` | what a kind does each frame: `none`, `fly`, `chase`, `keep`, `flee`, `boss` |
| `wave` | number | 0 | which wave spawns this kind; 0 = never by wave |
| `count` | number | 1 | how many of this kind join that wave |

The feel is data now, exactly as lesson 085 said it would be: the hero's
ease reads its own `accel`, not a constant in its code. (The hero's row
predates the column and keeps loading byte-for-byte — its weight is the
default, the 120 ms lesson 085 shipped.)

This is a **deliberate fix-forward** of a published refusal edge.
Lesson 071/072's loader refused "a column not named at all": every file
had to name every column. That rule made sense when there were seven
columns and one kind of row; it cannot survive a format where a weapon
row has no sprite and a projectile row has no damage. The edge is
relaxed exactly one notch: **a file may omit any column the format
knows, and the unnamed fields take their defaults.** A column the
format does *not* know is still a malformed file — and so is a name
said twice, a row with one value too few or too many, a behavior the
format does not define. From a scratch run, one of each:

```
engine: assets/entities.txt: could not load (malformed)      <- the header named "sprit"
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none wave 0 count 1
engine: assets/entities.txt: could not load (malformed)      <- a row one value short
```

The middle line is the other half of the proof: a file naming only
`name x y speed health sprite` loaded, and the fields it never named
read as the format's defaults — `facing 0`, `accel 120`, `behavior
none`.

And the compatibility rule holds without negotiation: **every file the
course has shipped keeps loading byte-for-byte.** `assets/entities.txt`
is not touched by this lesson at all — `git diff lesson-086 lesson-087
-- assets/entities.txt` is empty — and the run's byte-level check still
prints its two rows exactly as lesson 071 wrote them, now beside the
defaults their file never names:

```
engine: table: unnamed fields at their defaults — accel 120, damage 0, rate 0, fires none, range 0, behavior none, wave 0, count 1
engine: table assets/entities.txt: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/hero.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none wave 0 count 1
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm accel 120 damage 0 rate 0 fires none range 0 behavior none wave 0 count 1
```

### Weapons are rows

The game's combat data is two new files in the grown format — each
file's header naming exactly the columns its rows use. `assets/weapons.txt`
is what a weapon is: a row that names the projectile kind it fires and
carries its rate and damage. `assets/projectiles.txt` is what a
projectile kind is: its speed, its art, its life, and `behavior fly` —
the fact its row carries, like every other.

```
name damage rate fires
blaster 1 300 bolt
cannon 2 60 shell
```

A weapon row is never an entity. It has no position, no life, no
per-frame work — its facts are only *carried*. Arming a shooter with a
weapon row copies those facts onto it, and from then on the shooter
carries them like any entity carries its row's values:

```
engine: arm: hero arms blaster (damage 1, rate 300, fires bolt)
engine: arm: slime arms cannon (damage 2, rate 60, fires shell)
```

The number keys arm the weapons table's rows (`HeroFire`), and the fire
key sends a shot at the row's rate — the rate is a ceiling: the trigger
answers again only when its cooldown has run out. The two weapons trade
off exactly as their rows state: the blaster is fast and weak (a shot
every fifth of a second, one point), the cannon slow and strong (a shot
a second, two points) and — because the projectile rows say so — the
cannon's shell flies farther than the blaster's bolt. From a run, each
weapon firing the projectile kind its row states:

```
engine: arm: hero arms blaster (damage 1, rate 300, fires bolt)
engine: fire: hero -> bolt (damage 1, range 160)
engine: shot bolt retired — range
...
engine: arm: hero arms cannon (damage 2, rate 60, fires shell)
engine: fire: hero -> shell (damage 2, range 400)
engine: shot shell retired — wall
```

The hero aims the way it runs: the shot flies the compass point of the
hero's motion — the same eight directions, the same 1/√2 diagonal — or
its facing at rest.

### Projectiles are entities

The projectile is not a particle system and not a special case: it is
an entity, created from its definition through the store, exactly like
the hero and the slime. Its row gives it its speed, its art, its range;
its shooter gives it the damage it will deal and the fact that it was
fired (`owner` — a shot never hits the shooter). What the store's
lifetime, the walk's one-visit-per-entity rule, and the mover already
do for entities, they now do for projectiles for free.

Its flight is the per-entity work's one behavior branch — `fly` —
sub-stepped through the mover a few pixels at a time, so a fast shot
cannot pass what it hits inside one long frame. At every position —
including the one it is fired at, so a shot fired point-blank lands —
it looks at what it must notice, and it retires at exactly one of three
ends: **a wall** (the mover refused the step), **its range's end** (its
row's flight budget is spent), or **the entity it hit**. From the same
run, all three:

```
engine: shot bolt retired — wall
engine: shot bolt retired — range
engine: hit: bolt hits slime — damage 1, health 1 -> 0
engine: shot bolt retired — hit slime
engine: slime retired — zero health
```

The hit is the row's damage, and the row's damage is all it is:
`damage 1` takes the slime from `1` to `0` exactly. A zero-health
entity is retired — with one exception worth naming: **the hero is
never retired.** Its zero health is the game's own defeat condition,
which the state machine reads; the game's actor is not retired out from
under the game. A run that drives the hero's health to zero twice —
once dying, once after a fresh game has restored it — shows both the
rule and the exception:

```
engine: fire: slime -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 3 -> 1
engine: shot shell retired — hit hero
engine: fire: slime -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 1 -> 0
engine: shot shell retired — hit hero
engine: state play -> death (the hero's health reached zero)
engine: state death -> title (the player returned to the title)
engine: state title -> play (the player started)
engine: fire: slime -> shell (damage 2, range 400)
engine: hit: shell hits hero — damage 2, health 3 -> 1
```

`damage 2` twice: `3 -> 1`, `1 -> 0` — the numbers are the row's, and
nothing else. The last line is the exception working: after the fresh
game restored the hero's health to 3, the hero is hit again — still
live, still in the store. And the shots in that run were the **enemy
fire stand-in**: `G` makes the slime spit at the hero, a keyed
demonstration like lesson 082's, because what stands in today is only
*who shoots at the hero and when*. The hit itself is real combat — that
is what this lesson landed.

### What this run verified, and what it did not

- **Every shipped table file loads unchanged** — `assets/entities.txt`
  byte-for-byte (this lesson does not touch it; its diff is empty), its
  rows printing the same values they always did, the unnamed fields at
  the format's defaults.
- **Each weapon fires the projectile kind its row states** — the arm
  and fire reports: `blaster … fires bolt`, `cannon … fires shell`, and
  the shots that appear are `bolt` and `shell`.
- **Projectiles retire at walls, at their range's end, and at the
  entities they hit** — all three retirement lines, in real runs.
- **A hit reduces health by the row's damage** — `damage 1` took the
  slime `1 -> 0`, `damage 2` took the hero `3 -> 1` and `1 -> 0` — and
  a zero-health entity is retired, the hero excepted.

What this run did **not** verify is how the weapons *feel* at their
rates — the authoring machine's loop is paced by scripted input
(roughly 25 frames a second here, and the trigger fires at most one
shot per frame), so the blaster's five-shots-a-second ceiling reads as
one shot per frame under the script. The rate is a row's fact either
way; tuning it is the game's judgment, made later. Nor does the lesson
yet say *who* the enemies are or *when* they attack: the enemy rows
that carry their own weapons arrive in lesson 088, the movement
behaviors in 089, and the attacks that replace the `G` stand-in in 090.

## Code step

One change: the format grows, and combat stands on it. `src/table.h`
and `src/table.cpp` grow the named columns (the defaults, the behavior
spellings, a header that names any subset — the fix-forward of
lesson 072's refusal edge); `src/entity.h/.cpp` carry the new facts
from the row; `src/combat.h` and `src/combat.cpp` are new — arming,
firing, and the projectile's flight (`CombatArm`, `CombatFire`,
`CombatFly`); `src/hero.h/.cpp` read the feel from the row's `accel`
and fire the hero's weapon; `src/game.cpp`'s walk branches on the
behavior the row carries (a projectile flies); `src/main.cpp` loads the
game's two new tables and arms the shooters; `src/platform.h` and
`src/platform_x11.cpp` grow the three keys the combat needs.
`assets/weapons.txt` and `assets/projectiles.txt` are the game's combat
data (new), and `assets/bolt.ppm` and `assets/shell.ppm` are the two
shot sprites — two 16×16 magenta-keyed images, the bolt a bright core
with a streak, the shell a warm disc with a hot centre. Its end state
is tagged `lesson-087`.

```diff
diff --git a/assets/projectiles.txt b/assets/projectiles.txt
new file mode 100644
index 0000000..beb5ef6
--- /dev/null
+++ b/assets/projectiles.txt
@@ -0,0 +1,3 @@
+name speed sprite range behavior
+bolt 480 assets/bolt.ppm 160 fly
+shell 240 assets/shell.ppm 400 fly
diff --git a/assets/weapons.txt b/assets/weapons.txt
new file mode 100644
index 0000000..ee42dd4
--- /dev/null
+++ b/assets/weapons.txt
@@ -0,0 +1,3 @@
+name damage rate fires
+blaster 1 300 bolt
+cannon 2 60 shell
diff --git a/src/combat.cpp b/src/combat.cpp
new file mode 100644
index 0000000..905cfc7
--- /dev/null
+++ b/src/combat.cpp
@@ -0,0 +1,183 @@
+// combat.cpp — the combat's mechanics: arm, fire, fly, hit, retire.
+//
+// Lesson 087: every mechanic here is the data's. The damage a hit does is
+// the row's damage the shooter carries; the projectile is the kind its
+// row names; the flight is the projectile's speed over game time; the
+// life is its row's range. Nothing here knows which weapon or which
+// projectile exists — the tables know that.
+
+#include "combat.h"
+
+#include <cstdio>
+
+namespace engine {
+namespace {
+
+/* The two boxes of a shot and its target: what each draws is what each
+   is hit as — one ANIM_FRAME_W-wide frame of its art (lesson 086). */
+bool Overlaps(const Entity &a, const Entity &b)
+{
+    return a.x < b.x + ANIM_FRAME_W && b.x < a.x + ANIM_FRAME_W &&
+           a.y < b.y + b.sprite->height && b.y < a.y + a.sprite->height;
+}
+
+} /* namespace */
+
+void CombatAim(double vx, double vy, double &dir_x, double &dir_y)
+{
+    /* The aim is the compass point of the motion — one of the eight
+       directions the movement already knows, a diagonal at 1/sqrt(2) —
+       never a direction of some other length. At rest this is (0, 0). */
+    dir_x = 0.0;
+    dir_y = 0.0;
+    if (vx > 0.0)
+        dir_x = 1.0;
+    else if (vx < 0.0)
+        dir_x = -1.0;
+    if (vy > 0.0)
+        dir_y = 1.0;
+    else if (vy < 0.0)
+        dir_y = -1.0;
+    if (dir_x != 0.0 && dir_y != 0.0) {
+        dir_x *= AIM_DIAG;
+        dir_y *= AIM_DIAG;
+    }
+}
+
+void CombatArm(Entity &shooter, const EntityDef &weapon)
+{
+    /* A weapon is a row; carrying it means carrying its values. The
+       shooter is not tied to the row afterwards — it carries the facts
+       and may be armed with another row's. */
+    shooter.damage = weapon.damage;
+    shooter.rate = weapon.rate;
+    for (int i = 0; i < TABLE_NAME_MAX; ++i)
+        shooter.fires[i] = weapon.fires[i];
+    shooter.cooldown = 0.0;
+    std::printf("engine: arm: %s arms %s (damage %d, rate %d, fires %s)\n",
+                shooter.name, weapon.name, weapon.damage, weapon.rate,
+                weapon.fires[0] ? weapon.fires : "none");
+}
+
+bool CombatFire(EntityStore &store, const EntityTable &shots,
+                Entity &shooter, double dir_x, double dir_y)
+{
+    if (!shooter.fires[0])
+        return false; /* an unarmed shooter fires nothing */
+
+    /* The projectile kind is the row's fact, looked up where the kinds
+       live. A kind the table does not hold is a typed failure — never a
+       shot with assumed attributes. */
+    DefResult kind = TableFind(shots, shooter.fires);
+    if (kind.error != DEF_OK) {
+        std::printf("engine: fire refused — %s is no projectile kind\n",
+                    shooter.fires);
+        return false;
+    }
+
+    /* The projectile is an entity: created from its definition, through
+       the store, in a slot like every entity. */
+    EntityResult made = EntityCreate(store, *kind.def);
+    if (made.error != ENTITY_OK) {
+        std::printf("engine: fire refused — the store is full\n");
+        return false;
+    }
+
+    Entity &shot = *made.entity;
+    shot.x = shooter.x; /* a shot leaves its shooter's box */
+    shot.y = shooter.y;
+    shot.owner = &shooter; /* a shot never hits its owner */
+    shot.damage = shooter.damage; /* the row's damage, carried into the hit */
+    shot.move_x = dir_x;
+    shot.move_y = dir_y;
+    if (dir_x > 0.0)
+        shot.facing = 0;
+    else if (dir_y > 0.0)
+        shot.facing = 1;
+    else if (dir_x < 0.0)
+        shot.facing = 2;
+    else
+        shot.facing = 3;
+    std::printf("engine: fire: %s -> %s (damage %d, range %d)\n", shooter.name,
+                shot.name, shot.damage, shot.range);
+    return true;
+}
+
+void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
+               Entity &shot, double dt)
+{
+    /* The flight, in game time: the shot's speed over dt, sub-stepped
+       through the mover so each sub-step is small. What is checked at
+       every position — including the one it is fired at, so a shot fired
+       point-blank lands — is what a projectile must notice: the entity
+       it hit, a wall (the step refused), and its range running out. */
+    double left = (double)shot.speed * dt;
+    for (;;) {
+        /* The first actor the shot overlaps, other than itself and its
+           owner. Slots are checked in order; the first hit is the hit.
+           A shot flies through other shots — a projectile hits the
+           living, and crossing fire does not cancel in mid-air. */
+        Entity *target = 0;
+        for (int i = 0; i < ENTITY_CAP && !target; ++i) {
+            Entity &e = store.slots[i];
+            if (!e.live || &e == &shot || &e == shot.owner)
+                continue;
+            if (e.behavior == BEHAVIOR_FLY)
+                continue;
+            if (Overlaps(shot, e))
+                target = &e;
+        }
+        if (target) {
+            /* The hit: the target's health falls by the row's damage,
+               and the shot is spent on it. */
+            int was = target->health;
+            target->health -= shot.damage;
+            if (target->health < 0)
+                target->health = 0;
+            std::printf("engine: hit: %s hits %s — damage %d, health %d -> %d\n",
+                        shot.name, target->name, shot.damage, was,
+                        target->health);
+            std::printf("engine: shot %s retired — hit %s\n", shot.name,
+                        target->name);
+            EntityRetire(store, shot);
+            if (target->health == 0 && target != &hero) {
+                /* A zero-health entity is retired — the hero excepted:
+                   its zero health is the game's defeat condition, which
+                   the state machine reads; the game's actor is not
+                   retired out from under the game. */
+                std::printf("engine: %s retired — zero health\n",
+                            target->name);
+                EntityRetire(store, *target);
+            }
+            return;
+        }
+
+        if (left <= 0.0)
+            return; /* this frame's flight is spent */
+        if (shot.traveled >= shot.range) {
+            /* The range is the shot's life: a projectile that has flown
+               its row's range retires in the air — after its last look
+               at what it might have hit. */
+            std::printf("engine: shot %s retired — range\n", shot.name);
+            EntityRetire(store, shot);
+            return;
+        }
+
+        double step = left < COMBAT_STEP ? left : COMBAT_STEP;
+        double was_x = shot.x, was_y = shot.y;
+        MoveEntity(map, shot, shot.move_x * step, shot.move_y * step);
+        bool moved_x = shot.move_x == 0.0 || shot.x != was_x;
+        bool moved_y = shot.move_y == 0.0 || shot.y != was_y;
+        if (!moved_x || !moved_y) {
+            /* The mover refused the step where the shot meant to go: a
+               projectile that meets a wall retires at it. */
+            std::printf("engine: shot %s retired — wall\n", shot.name);
+            EntityRetire(store, shot);
+            return;
+        }
+        shot.traveled += step;
+        left -= step;
+    }
+}
+
+} /* namespace engine */
diff --git a/src/combat.h b/src/combat.h
new file mode 100644
index 0000000..b052eaf
--- /dev/null
+++ b/src/combat.h
@@ -0,0 +1,65 @@
+// combat.h — the combat: weapons armed, shots fired, shots in flight.
+//
+// Lesson 087: weapons are rows, projectiles are entities (design D5). A
+// weapon is a table row that names the projectile definition it fires and
+// carries its rate and damage; arming a shooter carries those values onto
+// it. Firing creates a projectile entity from that definition — through
+// the store, like every entity — and the projectile moves through the
+// mover in game time. It retires at a wall, at its range's end, and at
+// the entity it hit; a hit reduces the target's health by the row's
+// damage and retires the projectile. A zero-health entity is retired —
+// the hero excepted, whose zero health is the game's defeat condition.
+//
+// This is the game layer's combat file pair, beside the services (D2):
+// the store, the mover, and the table stay exactly what they are.
+#ifndef COMBAT_H
+#define COMBAT_H
+
+#include "entity.h"
+#include "table.h"
+
+namespace engine {
+
+/* Lesson 087: the flight's sub-step, in world pixels. A projectile moves
+   through the mover a few pixels at a time, checking what it hit along
+   the way — a fast shot cannot pass what it hits, or cross a thin wall
+   inside one long frame. */
+constexpr double COMBAT_STEP = 4.0;
+
+/* 1 / sqrt(2): a diagonal aim is scaled by this, exactly as the hero's
+   diagonal intent is (lesson 085), so a shot covers ground at its speed
+   whichever of the eight directions it flies. */
+constexpr double AIM_DIAG = 0.70710678;
+
+/* The eight compass points: the aim of the motion (vx, vy), normalized.
+   At rest the result is (0, 0) and the caller picks its own fallback —
+   the hero fires along its facing. */
+void CombatAim(double vx, double vy, double &dir_x, double &dir_y);
+
+/* Arm a shooter from a weapon row: the row's damage, rate, and the
+   projectile kind it fires become the shooter's own values — the weapon
+   is a row, and carrying a weapon is carrying its row's facts. */
+void CombatArm(Entity &shooter, const EntityDef &weapon);
+
+/* Fire: a projectile entity created from the definition the shooter's
+   `fires` names, sent along (dir_x, dir_y) — a direction of unit length.
+   Answers false — the report names why — when the shooter names no
+   projectile kind, when the table holds no such definition, or when the
+   store has no slot: never a stolen entity, never a shot with assumed
+   attributes. */
+bool CombatFire(EntityStore &store, const EntityTable &shots,
+                Entity &shooter, double dir_x, double dir_y);
+
+/* One projectile's flight, once a frame of game time: sub-stepped
+   through the mover, retiring at a wall (the step refused), at its
+   range's end, and at the entity it hit. A hit reduces the target's
+   health by the shot's damage and retires the shot; a zero-health target
+   is retired too — the hero excepted, whose zero health is the game's
+   defeat condition (the state machine reads it, the game's actor is not
+   retired out from under the game). */
+void CombatFly(const TileMap &map, EntityStore &store, const Entity &hero,
+               Entity &shot, double dt);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/entity.cpp b/src/entity.cpp
index b45cc1f..77cdd42 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -12,14 +12,21 @@ namespace engine {
 Entity EntityFromDef(const EntityDef &def)
 {
     Entity entity = {};
-    for (int i = 0; i < TABLE_NAME_MAX; ++i)
+    for (int i = 0; i < TABLE_NAME_MAX; ++i) {
         entity.name[i] = def.name[i];
+        entity.fires[i] = def.fires[i];
+    }
     entity.x = def.x;
     entity.y = def.y;
     entity.facing = def.facing;
     entity.speed = def.speed;
     entity.health = def.health;
     entity.sprite = def.image;
+    entity.accel = def.accel;
+    entity.damage = def.damage;
+    entity.rate = def.rate;
+    entity.range = def.range;
+    entity.behavior = def.behavior;
     return entity;
 }
 
diff --git a/src/entity.h b/src/entity.h
index 560296f..c214bb8 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -38,6 +38,23 @@ struct Entity {
                                   into motion */
     bool live;                 /* lesson 074: this entity exists — the
                                   slot's state, set by the store */
+
+    /* Lesson 087: the combat facts, carried from the row the entity was
+       created from (or, for a shooter armed with a weapon row, from that
+       row — a weapon's values are data like any other's). */
+    int accel;                 /* ms: the eased-move time constant — the
+                                  feel; the format's default if its row
+                                  named no accel column */
+    int damage;                /* points a hit from this entity removes */
+    int rate;                  /* rounds per minute; 0 = never fires */
+    char fires[TABLE_NAME_MAX]; /* the projectile kind it fires */
+    int range;                 /* a projectile's flight budget, pixels */
+    double traveled;           /* how far a projectile has flown */
+    const Entity *owner;       /* the shooter of a projectile — a shot
+                                  never hits its owner */
+    int behavior;              /* BehaviorKind, its row's: what this
+                                  entity does each frame */
+    double cooldown;           /* seconds until it may fire again */
 };
 
 /* An entity created from a definition: every attribute its row states,
diff --git a/src/game.cpp b/src/game.cpp
index 967f072..33101e3 100644
--- a/src/game.cpp
+++ b/src/game.cpp
@@ -12,23 +12,24 @@
 #include <cstdio>
 
 #include "blit.h"
+#include "combat.h"
 #include "text.h"
 #include "tilemap.h"
 #include "tiles.h"
 
 namespace engine {
 
-/* The demonstration stand-in, named so it cannot be mistaken for the
-   game. Lesson 082 has the state machine but not yet the gameplay that
-   drives two of its named conditions: combat reduces the hero's health
-   (lesson 087) and the waves spend the game's completion (lesson 091).
-   Until those land, keys stand in for them: SPACE is a hit on the hero,
-   and ENTER in play says the game is complete — the same kind of keyed
-   demonstration, and like lesson 078's script before the juice toolkit
-   drove it. Both are removed when the real triggers arrive; the
-   transitions they fire are the game's own (defeat on zero health,
-   completion on no waves). Keyed, they never fire on their own during a
-   gameplay test. */
+/* The demonstration stand-ins, named so they cannot be mistaken for the
+   game. Lesson 087 made the hit real: a projectile reduces its target's
+   health by its row's damage, and a zero-health entity is retired. What
+   still stands in is the *enemy's fire* — who shoots at the hero, and
+   when. Until the enemies' attacks land (lesson 090), the run's G key
+   makes the slime spit at the hero (a keyed stand-in in the run, like
+   the feel demonstration beside it), and ENTER in play below says the
+   game is complete — lesson 091's waves spend it for real. Both die
+   when the real triggers arrive; the transitions they fire are the
+   game's own (defeat on zero health, completion on no waves). Keyed,
+   they never fire on their own during a gameplay test. */
 
 /* A transition, named once here and printed the moment it happens, so a
    run shows the machine moving between states and why. */
@@ -86,19 +87,10 @@ void GameInput(Game &game, platform::Window *window, Entity &hero,
         break;
 
     case GAME_PLAY: {
-        /* Play's movement is the hero's own (HeroMove, lesson 085) — the
-           held direction read and eased into motion there. What is left
-           here is the play state's other input and the named conditions. */
-
-        /* The stand-in for combat: SPACE is a hit on the hero (lesson 087
-           makes real hits land). The named condition below reads the
-           health this lowers. */
-        if (platform::KeyPressed(window, platform::KEY_SPACE) &&
-            hero.health > 0) {
-            hero.health -= 1;
-            std::printf("engine: hero takes a hit — health %d (t=%.3f)\n",
-                        hero.health, game.play_clock);
-        }
+        /* Play's movement is the hero's own (HeroMove, lesson 085) and
+           its fire is the hero's weapon's (HeroFire, lesson 087) — both
+           the run's, in play. What is left here is the play state's
+           other input and the named conditions. */
 
         game.play_clock += wall_dt;
 
@@ -234,7 +226,8 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
     }
 }
 
-int GameWalk(EntityStore &store, const TileMap &map, double dt)
+int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
+             double dt)
 {
     /* Lesson 084: the walk — every live entity, once per frame, in slot
        order, its movement resolved against the tilemap. The per-entity
@@ -242,13 +235,22 @@ int GameWalk(EntityStore &store, const TileMap &map, double dt)
        turns the request into motion one axis at a time, so an entity
        that meets a solid tile stops on that axis and slides along the
        wall on the other — and the facing follows where it is going.
-       The hero and every other entity resolve the same way. */
+
+       Lesson 087: the per-entity work branches on the entity's behavior
+       — the fact its row carries. A projectile flies: its own
+       sub-stepped flight through the same mover, retiring at walls, at
+       its range's end, and at the entity it hit. Every other behavior
+       leaves the request for the mover below. */
     int visited = 0;
     for (int i = 0; i < ENTITY_CAP; ++i) {
         if (!store.slots[i].live)
             continue;
         visited += 1;
         Entity &e = store.slots[i];
+        if (e.behavior == BEHAVIOR_FLY) {
+            CombatFly(map, store, hero, e, dt);
+            continue;
+        }
         MoveEntity(map, e, e.move_x * e.speed * dt, e.move_y * e.speed * dt);
         if (e.move_x > 0.0)
             e.facing = 0;
diff --git a/src/game.h b/src/game.h
index a031ef7..bde62f0 100644
--- a/src/game.h
+++ b/src/game.h
@@ -103,8 +103,13 @@ void GameDrawSprites(const Game &game, Framebuffer &fb,
 /* Lesson 084: the walk — the game resolves every live entity's movement
    against the tilemap. Each entity's movement request becomes motion
    through the mover (MoveEntity), one axis at a time, so it stops at a
-   solid tile and slides along a wall. Returns the visit count. */
-int GameWalk(EntityStore &store, const TileMap &map, double dt);
+   solid tile and slides along a wall. Lesson 087: a projectile's
+   behavior flies it (CombatFly — its own sub-stepped flight, retiring at
+   walls, at its range, at what it hits) instead of the request. The hero
+   is handed along for the combat's rules to know the game's actor by.
+   Returns the visit count. */
+int GameWalk(EntityStore &store, const TileMap &map, const Entity &hero,
+             double dt);
 
 } /* namespace engine */
 
diff --git a/src/hero.cpp b/src/hero.cpp
index a2798ab..8945263 100644
--- a/src/hero.cpp
+++ b/src/hero.cpp
@@ -6,6 +6,8 @@
 
 #include "hero.h"
 
+#include "combat.h"
+
 namespace engine {
 
 void HeroMove(Entity &hero, platform::Window *window, double dt)
@@ -31,13 +33,15 @@ void HeroMove(Entity &hero, platform::Window *window, double dt)
         intent_y *= HERO_DIAG;
     }
 
-    /* The ease: the velocity closes on the intent by dt/HERO_TIME each
-       frame — toward the intent when the player steers (acceleration),
-       toward rest when they let go (deceleration). A turn passes through
-       the ease instead of snapping to full speed the other way. The
-       hero's movement request carries the eased velocity; the walk turns
-       it into motion (move x speed = the velocity). */
-    double k = dt / HERO_TIME;
+    /* The ease: the velocity closes on the intent by dt/accel each frame
+       — toward the intent when the player steers (acceleration), toward
+       rest when they let go (deceleration). A turn passes through the
+       ease instead of snapping to full speed the other way. The feel is
+       the hero's own `accel` — its row's fact, milliseconds (lesson
+       087); an accel of 0 is instant weightless motion. The hero's
+       movement request carries the eased velocity; the walk turns it
+       into motion (move x speed = the velocity). */
+    double k = hero.accel > 0 ? dt / (hero.accel / 1000.0) : 1.0;
     if (k > 1.0)
         k = 1.0;
     hero.move_x += (intent_x - hero.move_x) * k;
@@ -61,4 +65,47 @@ void HeroMove(Entity &hero, platform::Window *window, double dt)
     }
 }
 
+void HeroFire(Entity &hero, platform::Window *window,
+              const EntityTable &weapons, const EntityTable &shots,
+              EntityStore &store, double dt)
+{
+    /* Lesson 087: the number keys arm the weapons table's rows. A weapon
+       is a row — arming carries its values — so the weapons grow as
+       rows: one row more is one key more, and no weapon code. */
+    if (platform::KeyPressed(window, platform::KEY_1) && weapons.count > 0)
+        CombatArm(hero, weapons.rows[0]);
+    if (platform::KeyPressed(window, platform::KEY_2) && weapons.count > 1)
+        CombatArm(hero, weapons.rows[1]);
+
+    /* The rate is the row's, in rounds per minute: the trigger answers
+       again only when the cooldown it earns has run out. At most one
+       shot per frame — a long frame is caught up by the next shot, never
+       by a burst of them. */
+    if (hero.cooldown > 0.0) {
+        hero.cooldown -= dt;
+        return;
+    }
+    if (!platform::KeyDown(window, platform::KEY_SPACE))
+        return;
+    if (hero.rate <= 0 || !hero.fires[0])
+        return; /* unarmed, or a row that never fires */
+
+    /* The aim: the compass point of the hero's motion — the eight
+       directions, a diagonal at 1/sqrt(2) — or its facing at rest. */
+    double dir_x = 0.0, dir_y = 0.0;
+    CombatAim(hero.move_x, hero.move_y, dir_x, dir_y);
+    if (dir_x == 0.0 && dir_y == 0.0) {
+        if (hero.facing == 0)
+            dir_x = 1.0;
+        else if (hero.facing == 1)
+            dir_y = 1.0;
+        else if (hero.facing == 2)
+            dir_x = -1.0;
+        else
+            dir_y = -1.0;
+    }
+    if (CombatFire(store, shots, hero, dir_x, dir_y))
+        hero.cooldown = 60.0 / (double)hero.rate;
+}
+
 } /* namespace engine */
diff --git a/src/hero.h b/src/hero.h
index c602349..6804096 100644
--- a/src/hero.h
+++ b/src/hero.h
@@ -17,13 +17,6 @@
 
 namespace engine {
 
-/* The hero's accel/decel time constant — the feel: roughly how long it
-   takes to ease from rest to full speed (or back). Lesson 085 keeps it
-   here as the hero's own fact; when the table format grows named
-   columns (lesson 087) the feel becomes data, like the hero's speed
-   already is. */
-constexpr double HERO_TIME = 0.12; /* seconds to close on the intent */
-
 /* 1 / sqrt(2): a diagonal intent is scaled by this so the hero covers
    ground at the straight-line speed, not sqrt(2) times it. */
 constexpr double HERO_DIAG = 0.70710678;
@@ -36,9 +29,22 @@ constexpr double ANIM_STEP = 0.12;
    hero's velocity eases toward that intent (accel) and toward rest
    (decel) — a turn passes through the ease rather than snapping. The
    result is left in the hero's own movement request, which the walk
-   turns into motion against the map. */
+   turns into motion against the map. The ease's time constant is the
+   hero's own `accel` — its row's fact since lesson 087 grew the format
+   by named columns (the hero's row predates the column and keeps loading
+   byte-for-byte, its weight the format's default). */
 void HeroMove(Entity &hero, platform::Window *window, double dt);
 
+/* Lesson 087: the hero's weapon, once per frame of play. The number keys
+   arm the weapons table's rows — a weapon is a row, and carrying it is
+   carrying its values — and the fire key sends a shot along the hero's
+   motion (the eight compass points of its velocity) or, at rest, along
+   its facing. The shot is an entity like any other; the rate its row
+   states is the ceiling on how often the trigger answers. */
+void HeroFire(Entity &hero, platform::Window *window,
+              const EntityTable &weapons, const EntityTable &shots,
+              EntityStore &store, double dt);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index e1993f9..58ff0bd 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -14,6 +14,7 @@
 #include "arena.h"
 #include "audio.h"
 #include "blit.h"
+#include "combat.h"
 #include "entity.h"
 #include "feel.h"
 #include "font.h"
@@ -93,6 +94,75 @@ static bool LoadRunSample(Arena &arena, const char *path, Sample &into)
     return false;
 }
 
+/* Lesson 087: one table load's whole failure path, the same shape — the
+   load either hands over every definition or names what went wrong typed
+   and the run ends by name. Used for every table file the game loads. */
+static bool LoadRunTable(Arena &arena, const char *path, EntityTable &into)
+{
+    TableResult loaded = LoadTable(arena, path);
+    if (loaded.error == TABLE_OK) {
+        into = loaded.table;
+        return true;
+    }
+    switch (loaded.error) {
+    case TABLE_MISSING:
+        std::fprintf(stderr, "engine: %s: could not load (missing)\n", path);
+        break;
+    case TABLE_MALFORMED:
+        std::fprintf(stderr, "engine: %s: could not load (malformed)\n", path);
+        break;
+    default:
+        std::fprintf(stderr, "engine: %s: could not load (no room)\n", path);
+        break;
+    }
+    return false;
+}
+
+/* Lesson 073/087: the definitions' art, loaded at startup. The sprite
+   column names the file; the run loads each one and hands the definition
+   its image, so an entity created from the definition is answered from
+   the definition alone. A row that names no sprite (a weapon row) has no
+   art and needs none. */
+static bool LoadRunArt(Arena &arena, EntityTable &table)
+{
+    Sprite *images = (Sprite *)ArenaAlloc(
+        arena, (size_t)table.count * sizeof(Sprite), 4);
+    if (!images) {
+        std::fprintf(stderr, "engine: no room for the definitions' art\n");
+        return false;
+    }
+    for (int i = 0; i < table.count; ++i) {
+        EntityDef &def = table.rows[i];
+        if (!def.sprite[0])
+            continue;
+        SpriteResult art = LoadSprite(arena, def.sprite);
+        if (art.error != SPRITE_OK) {
+            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
+            return false;
+        }
+        images[i] = art.sprite;
+        def.image = &images[i];
+    }
+    return true;
+}
+
+/* Lesson 087: the byte-level check on a table, before anything uses it —
+   every definition, carrying every field: the values its row states and
+   the format's defaults for the columns its file did not name. */
+static void PrintDefs(const char *path, const EntityTable &table)
+{
+    std::printf("engine: table %s: %d definition%s\n", path, table.count,
+                table.count == 1 ? "" : "s");
+    for (int i = 0; i < table.count; ++i) {
+        const EntityDef &def = table.rows[i];
+        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
+                    def.name, def.x, def.y, def.facing, def.speed, def.health,
+                    def.sprite[0] ? def.sprite : "none", def.accel, def.damage,
+                    def.rate, def.fires[0] ? def.fires : "none", def.range,
+                    BehaviorName(def.behavior), def.wave, def.count);
+    }
+}
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -153,68 +223,51 @@ int Run(void)
     }
     TileSheet &sheet = tiles_loaded.sheet;
 
-    /* Lesson 071: the run's entities are data. The table file holds one
+    /* Lesson 071: the run's entities are data. A table file holds one
        row per definition — its columns named by its header — and the load
        either hands over every definition or names what went wrong, like
        every asset above. Lesson 072: the rows are the arena's, and a
-       refused load keeps none of them. */
-    TableResult table_loaded = LoadTable(arena, "assets/entities.txt");
-    if (table_loaded.error != TABLE_OK) {
-        switch (table_loaded.error) {
-        case TABLE_MISSING:
-            std::fprintf(stderr,
-                         "engine: assets/entities.txt: could not load (missing)\n");
-            break;
-        case TABLE_MALFORMED:
-            std::fprintf(stderr,
-                         "engine: assets/entities.txt: could not load (malformed)\n");
-            break;
-        default:
-            std::fprintf(stderr,
-                         "engine: assets/entities.txt: could not load (no room)\n");
-            break;
-        }
+       refused load keeps none of them. Lesson 087: the format grew by
+       named columns — and this file keeps loading byte-for-byte, its
+       seven columns exactly as lesson 071 wrote them, every field it
+       never named at the format's default. */
+    EntityTable table;
+    if (!LoadRunTable(arena, "assets/entities.txt", table)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
-    EntityTable &table = table_loaded.table;
 
-    /* The byte-level check, before anything uses the table: every
-       definition, carrying the values its row states. */
-    std::printf("engine: table: %d definition%s\n", table.count,
-                table.count == 1 ? "" : "s");
-    for (int i = 0; i < table.count; ++i) {
-        const EntityDef &def = table.rows[i];
-        std::printf("engine: def %s: x %d y %d facing %d speed %d health %d sprite %s\n",
-                    def.name, def.x, def.y, def.facing, def.speed, def.health,
-                    def.sprite);
+    /* Lesson 087: the game's own data, in the grown format. The weapons
+       are rows that name the projectile kind they fire and carry their
+       rate and damage; the projectile kinds are rows a fired shot is an
+       entity of. Each file's header names the columns it uses — and only
+       those; what it leaves unnamed sits at the format's defaults. */
+    EntityTable weapons, shots;
+    if (!LoadRunTable(arena, "assets/weapons.txt", weapons) ||
+        !LoadRunTable(arena, "assets/projectiles.txt", shots)) {
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
     }
 
-    /* Lesson 073: the definitions' art, loaded at startup. The table's
-       sprite column names the file; the run loads each one and hands the
-       definition its image, so an entity created from a definition is
-       answered from the definition alone. */
-    Sprite *images = (Sprite *)ArenaAlloc(
-        arena, (size_t)table.count * sizeof(Sprite), 4);
-    if (!images) {
-        std::fprintf(stderr, "engine: no room for the definitions' art\n");
+    /* The byte-level check, before anything uses the tables: every
+       definition of every table, carrying every field — the values its
+       row states and the format's defaults for the columns its file did
+       not name. */
+    std::printf("engine: table: unnamed fields at their defaults — accel %d, damage 0, rate 0, fires none, range 0, behavior none, wave 0, count 1\n",
+                TABLE_ACCEL_DEFAULT);
+    PrintDefs("assets/entities.txt", table);
+    PrintDefs("assets/weapons.txt", weapons);
+    PrintDefs("assets/projectiles.txt", shots);
+
+    /* Lesson 073: the definitions' art, loaded at startup. A row that
+       names no sprite (a weapon row) has no art and needs none. */
+    if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
     }
-    for (int i = 0; i < table.count; ++i) {
-        EntityDef &def = table.rows[i];
-        SpriteResult art = LoadSprite(arena, def.sprite);
-        if (art.error != SPRITE_OK) {
-            std::fprintf(stderr, "engine: %s: could not load\n", def.sprite);
-            platform::CloseWindow(opened.window);
-            ArenaRelease(arena);
-            return 1;
-        }
-        images[i] = art.sprite;
-        def.image = &images[i];
-    }
 
     /* Lesson 073: the game's first entity — created from the hero's
        definition, carrying the values its row states in named fields the
@@ -263,6 +316,7 @@ int Run(void)
        same table, one entity per row. A new row is a new entity; the
        run has no per-kind code to grow. */
     int created = 1;
+    Entity *foe = 0; /* the world's one enemy row (the slime) */
     for (int i = 0; i < table.count; ++i) {
         if (&table.rows[i] == hero_def.def)
             continue;
@@ -280,11 +334,24 @@ int Run(void)
            walls and stops it at solid tiles. */
         made.entity->move_x = 1.0;
         made.entity->move_y = 1.0;
+        if (!foe)
+            foe = made.entity;
         created += 1;
     }
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
                 created, store.live, ENTITY_CAP);
 
+    /* Lesson 087: weapons are rows. The hero starts armed with the
+       weapons table's first row; the number keys arm the rest (HeroFire).
+       The demonstration stand-in arms the foe with the second row — its
+       projectile is what G spits at the hero. The enemy rows that carry
+       their own attacks arrive in lesson 088; this stand-in and its key
+       die when those attacks land (lesson 090). */
+    if (weapons.count > 0)
+        CombatArm(hero, weapons.rows[0]);
+    if (weapons.count > 1 && foe)
+        CombatArm(*foe, weapons.rows[1]);
+
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
        attributes. */
@@ -348,7 +415,7 @@ int Run(void)
     std::printf("engine: sound %d-frame music looping on channel %d, %d-frame effect on the pool; one mixer of %d channels\n",
                 music.frame_count, AUDIO_MUSIC_CHANNEL, effect.frame_count,
                 AUDIO_MIXER_CHANNELS);
-    std::printf("engine: arrow keys move the hero, space shakes the camera; close the window to stop\n");
+    std::printf("engine: arrows move the hero, 1 and 2 arm the weapons, space fires, G is the enemy spit; close the window to stop\n");
     std::printf("engine: hero at %.0f,%.0f\n", hero.x, hero.y);
 
     /* Lesson 059: the run's sound is a run of amplitude at the engine's
@@ -451,16 +518,34 @@ int Run(void)
 
         /* Lesson 085: the hero's movement — the held direction eased into
            motion (accel/decel, the diagonal at the straight-line speed).
-           Only in play; the walk turns the eased velocity into steps. */
-        if (game.state == GAME_PLAY)
+           Lesson 087: and its weapon — the number keys arm the weapons
+           table's rows, the fire key sends a shot. Only in play; the
+           walk turns the eased velocity into steps and the shot into its
+           flight. */
+        if (game.state == GAME_PLAY) {
             HeroMove(hero, opened.window, dt);
+            HeroFire(hero, opened.window, weapons, shots, store, dt);
+
+            /* Lesson 087: the enemy-fire stand-in — G makes the slime
+               spit at the hero. What it demonstrates is real: the shot
+               is an entity, its hit reduces the hero's health by the
+               row's damage, and the hero's zero health is the game's
+               defeat. Only the shooter and its aim are scripted — the
+               enemies' own attacks (lesson 090) replace this key. */
+            if (foe && platform::KeyPressed(opened.window, platform::KEY_G)) {
+                double dir_x = 0.0, dir_y = 0.0;
+                CombatAim(hero.x - foe->x, hero.y - foe->y, dir_x, dir_y);
+                CombatFire(store, shots, *foe, dir_x, dir_y);
+            }
+        }
 
         /* Lesson 084: the game resolves its movement against its map —
            the walk is the game's now (GameWalk, in game.cpp), turning
-           every live entity's request into motion through the mover.
-           The loop times it as the frame record's entity sub-phase. */
+           every live entity's request into motion through the mover (and
+           every projectile into its flight). The loop times it as the
+           frame record's entity sub-phase. */
         double t_entities = platform::Now();
-        int visited = GameWalk(store, map, dt);
+        int visited = GameWalk(store, map, hero, dt);
         frame.entities = platform::Now() - t_entities;
         walk_visits += visited;
 
diff --git a/src/platform.h b/src/platform.h
index 4473e68..cd860ae 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -32,7 +32,9 @@ struct WindowResult {
 WindowResult OpenWindow(int width, int height);
 
 /* The keys the engine tracks. Plain values — no OS key code ever crosses
-   the seam. */
+   the seam. Lesson 087: the game's combat needs three more (the two
+   weapon rows the number keys arm, and the enemy-fire demonstration
+   key); a second OS maps its own three. */
 enum Key {
     KEY_UP = 0,
     KEY_DOWN,
@@ -41,6 +43,9 @@ enum Key {
     KEY_SPACE,
     KEY_ENTER,
     KEY_ESCAPE,
+    KEY_1,
+    KEY_2,
+    KEY_G,
     KEY_COUNT
 };
 
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index a251f11..bd91201 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -164,6 +164,9 @@ static int KeyIndex(KeySym sym)
     case XK_space:  return KEY_SPACE;
     case XK_Return: return KEY_ENTER;
     case XK_Escape: return KEY_ESCAPE;
+    case XK_1:      return KEY_1;
+    case XK_2:      return KEY_2;
+    case XK_g:      return KEY_G;
     default:        return -1;
     }
 }
diff --git a/src/table.cpp b/src/table.cpp
index ec1b59e..fd179ab 100644
--- a/src/table.cpp
+++ b/src/table.cpp
@@ -12,6 +12,13 @@
 // file's fact, so the file is walked once to count them and once to fill
 // them, and the whole load is bracketed by a mark — a refused load rolls
 // the arena back and keeps nothing.
+//
+// Lesson 087: the format grows by named columns, additively. The header
+// names the columns a file uses — any subset of the ones below — and the
+// fill starts every row at the format's defaults (DefaultRow), writing
+// only the named fields. A file that omits a column is no longer
+// refused; its fields sit at their defaults. A column the format does
+// not know is still malformed.
 
 #include "table.h"
 
@@ -98,7 +105,7 @@ bool ReadText(const unsigned char *line, int len, int &at, char *out,
     return true;
 }
 
-/* The columns the format knows. */
+/* The columns the format knows (lesson 087: grown by named columns). */
 enum Column {
     COL_NAME,
     COL_X,
@@ -107,13 +114,54 @@ enum Column {
     COL_SPEED,
     COL_HEALTH,
     COL_SPRITE,
-    COL_COUNT
+    COL_ACCEL,
+    COL_DAMAGE,
+    COL_RATE,
+    COL_FIRES,
+    COL_RANGE,
+    COL_BEHAVIOR,
+    COL_WAVE,
+    COL_SPAWN_COUNT,
+    COL_COUNT /* how many columns the format knows, not a column */
 };
 
 const char *const COLUMN_NAMES[COL_COUNT] = {
-    "name", "x", "y", "facing", "speed", "health", "sprite"
+    "name", "x", "y", "facing", "speed", "health", "sprite",
+    "accel", "damage", "rate", "fires", "range", "behavior", "wave",
+    "count"
+};
+
+/* Lesson 087: the behavior column's spellings, the format's own. */
+const char *const BEHAVIOR_NAMES[BEHAVIOR_COUNT] = {
+    "none", "fly", "chase", "keep", "flee", "boss"
 };
 
+/* Lesson 087: one row at the format's defaults, before the named fields
+   are written. A file may omit any column; whatever it omits keeps the
+   value set here — the defaults are part of the format's contract. */
+void DefaultRow(EntityDef &def)
+{
+    for (int i = 0; i < TABLE_NAME_MAX; ++i) {
+        def.name[i] = 0;
+        def.fires[i] = 0;
+    }
+    for (int i = 0; i < TABLE_PATH_MAX; ++i)
+        def.sprite[i] = 0;
+    def.x = 0;
+    def.y = 0;
+    def.facing = 0;
+    def.speed = 0;
+    def.health = 0;
+    def.image = 0;
+    def.accel = TABLE_ACCEL_DEFAULT;
+    def.damage = 0;
+    def.rate = 0;
+    def.range = 0;
+    def.behavior = BEHAVIOR_NONE;
+    def.wave = 0;
+    def.count = 1;
+}
+
 bool TokenIs(const unsigned char *token, int token_len, const char *name)
 {
     int n = 0;
@@ -144,8 +192,31 @@ int FindColumn(const unsigned char *token, int token_len)
     return -1;
 }
 
+/* Lesson 087: a behavior value — one of the format's spellings, mapped
+   to its number. A spelling the format does not define is refused, like
+   a facing that is not one of the four. */
+bool ReadBehavior(const unsigned char *line, int len, int &at, int &out)
+{
+    char text[TABLE_NAME_MAX];
+    if (!ReadText(line, len, at, text, TABLE_NAME_MAX))
+        return false;
+    for (int b = 0; b < BEHAVIOR_COUNT; ++b)
+        if (SameText(text, BEHAVIOR_NAMES[b])) {
+            out = b;
+            return true;
+        }
+    return false;
+}
+
 } /* namespace */
 
+const char *BehaviorName(int behavior)
+{
+    if (behavior < 0 || behavior >= BEHAVIOR_COUNT)
+        return "?";
+    return BEHAVIOR_NAMES[behavior];
+}
+
 TableResult LoadTable(Arena &arena, const char *path)
 {
     TableResult result = {};
@@ -199,23 +270,29 @@ TableResult LoadTable(Arena &arena, const char *path)
     ok = ok && NextLine(lines, line, len);
     int at = 0;
     int order[COL_COUNT];
+    int named = 0; /* lesson 087: how many columns this file names */
     for (int c = 0; c < COL_COUNT; ++c)
         order[c] = -1;
-    for (int i = 0; ok && i < COL_COUNT; ++i) {
+    /* Lesson 087: the header names the columns this file's rows carry —
+       any subset of the format's, each at most once, at least one. A
+       column the header does not name is not a refusal any more: the
+       field sits at the format's default. A name the format does not
+       know is still refused here, and so is a name said twice. */
+    while (ok) {
         const unsigned char *token = 0;
         int token_len = 0;
-        ok = ok && NextToken(line, len, at, token, token_len);
+        if (!NextToken(line, len, at, token, token_len))
+            break; /* the header's line ends */
+        int column = FindColumn(token, token_len);
+        ok = ok && column >= 0;
+        for (int prev = 0; ok && prev < named; ++prev)
+            ok = ok && order[prev] != column; /* one name, one column */
         if (ok) {
-            int column = FindColumn(token, token_len);
-            ok = ok && column >= 0;
-            for (int prev = 0; ok && prev < i; ++prev)
-                ok = ok && order[prev] != column; /* one name, one column */
-            order[i] = column;
+            order[named] = column;
+            named += 1;
         }
     }
-    const unsigned char *extra = 0;
-    int extra_len = 0;
-    ok = ok && !NextToken(line, len, at, extra, extra_len);
+    ok = ok && named > 0;
 
     while (ok) {
         if (!NextLine(lines, line, len))
@@ -228,9 +305,12 @@ TableResult LoadTable(Arena &arena, const char *path)
         }
 
         EntityDef &def = defs[result.table.count];
-        def.image = 0; /* the run hands the definition its art, not the file */
+        /* Lesson 087: the row begins at the format's defaults. The named
+           fields below overwrite theirs; every field the header does not
+           name keeps its default. */
+        DefaultRow(def);
         at = 0;
-        for (int i = 0; ok && i < COL_COUNT; ++i) {
+        for (int i = 0; ok && i < named; ++i) {
             switch (order[i]) {
             case COL_NAME:
                 ok = ReadText(line, len, at, def.name, TABLE_NAME_MAX);
@@ -254,6 +334,30 @@ TableResult LoadTable(Arena &arena, const char *path)
             case COL_SPRITE:
                 ok = ReadText(line, len, at, def.sprite, TABLE_PATH_MAX);
                 break;
+            case COL_ACCEL:
+                ok = ReadInt(line, len, at, def.accel);
+                break;
+            case COL_DAMAGE:
+                ok = ReadInt(line, len, at, def.damage);
+                break;
+            case COL_RATE:
+                ok = ReadInt(line, len, at, def.rate);
+                break;
+            case COL_FIRES:
+                ok = ReadText(line, len, at, def.fires, TABLE_NAME_MAX);
+                break;
+            case COL_RANGE:
+                ok = ReadInt(line, len, at, def.range);
+                break;
+            case COL_BEHAVIOR:
+                ok = ReadBehavior(line, len, at, def.behavior);
+                break;
+            case COL_WAVE:
+                ok = ReadInt(line, len, at, def.wave);
+                break;
+            case COL_SPAWN_COUNT:
+                ok = ReadInt(line, len, at, def.count);
+                break;
             default:
                 ok = false;
                 break;
diff --git a/src/table.h b/src/table.h
index fae166d..c4d7572 100644
--- a/src/table.h
+++ b/src/table.h
@@ -10,11 +10,22 @@
 //   slime 400 320 2 96 1 assets/sprite.ppm
 //
 // The header names the columns, and the loader fills the fields the
-// header declares — so the columns may come in any order, but every
-// column the format knows comes exactly once. `name` and `sprite` are
-// text (a run of non-space bytes); x, y, facing, speed, and health are
-// whole numbers, and facing is one of the four the format defines:
-// 0 right, 1 down, 2 left, 3 up.
+// header declares — so the columns may come in any order, and every
+// column the format knows comes at most once. `name`, `sprite`, and
+// `fires` are text (a run of non-space bytes); the rest are whole
+// numbers, and facing is one of the four the format defines: 0 right,
+// 1 down, 2 left, 3 up.
+//
+// Lesson 087: the format grows by named columns, additively. A file's
+// header names the columns it uses — and may omit any column the format
+// knows; the unnamed fields take the defaults the format defines below.
+// This is a deliberate fix-forward of lesson 071/072's refusal edge ("a
+// column not named at all"): a file may now omit columns, because the
+// game's facts (a weapon's damage, a projectile's life) must not be
+// crammed into columns that would lie about them, and every file the
+// course has shipped keeps loading byte-for-byte. A column the format
+// does *not* know is still a malformed file — and so is a name the
+// table already holds, or a row with one value too few or too many.
 #ifndef TABLE_H
 #define TABLE_H
 
@@ -31,12 +42,29 @@ namespace engine {
 constexpr int TABLE_NAME_MAX = 16;
 constexpr int TABLE_PATH_MAX = 64;
 
+/* Lesson 087: the behavior column's values — what a kind does each
+   frame, the fact its row carries. The format defines the spellings,
+   like it defines facing's four numbers: `none` stands still, `fly` is a
+   projectile in flight, and chase / keep / flee / boss are the enemy
+   behaviors (lessons 089-090) — the boss's value names its pattern. */
+enum BehaviorKind {
+    BEHAVIOR_NONE = 0,
+    BEHAVIOR_FLY,
+    BEHAVIOR_CHASE,
+    BEHAVIOR_KEEP,
+    BEHAVIOR_FLEE,
+    BEHAVIOR_BOSS,
+    BEHAVIOR_COUNT
+};
+
 /* One definition: a row of the table, carrying every value its row
    states — the identity and the attributes an entity is created from.
    The image is the one fact the file states as a name: the run loads the
    art the sprite column names and hands the definition its image, so an
    entity created from the definition is answered from the definition
-   alone. */
+   alone. A weapon is one of these rows (lesson 087): it names the
+   projectile kind it fires and carries its rate and damage — and is
+   never itself an entity. */
 struct EntityDef {
     char name[TABLE_NAME_MAX];   /* the definition's identity */
     int x, y;                    /* where it starts, in world pixels */
@@ -45,8 +73,34 @@ struct EntityDef {
     int health;                  /* points */
     char sprite[TABLE_PATH_MAX]; /* the art file it draws */
     const Sprite *image;         /* that art, loaded at startup */
+
+    /* Lesson 087: the format's named columns. Every field below takes
+       its default when a file's header does not name it — the defaults
+       are the format's contract, not a gap in it. */
+    int accel;                   /* ms: the eased-move time constant —
+                                    the feel (the hero's weight) */
+    int damage;                  /* points a hit removes — a weapon
+                                    row's damage, carried by its shots */
+    int rate;                    /* rounds per minute; 0 = never fires */
+    char fires[TABLE_NAME_MAX];  /* the projectile kind this row fires */
+    int range;                   /* world pixels: a projectile's flight
+                                    budget — its life */
+    int behavior;                /* BehaviorKind, its row's */
+    int wave;                    /* which wave spawns this kind;
+                                    0 = never by wave */
+    int count;                   /* how many of this kind join the wave */
 };
 
+/* Lesson 087: the defaults the format defines. A file may omit any
+   column; the field takes the value named here. The accel default is
+   the feel lesson 085 shipped as a constant — the hero's row predates
+   the column and keeps loading byte-for-byte, and its weight is exactly
+   this. */
+constexpr int TABLE_ACCEL_DEFAULT = 120; /* ms */
+
+/* A behavior value's spelling (and back), for the format's own reports. */
+const char *BehaviorName(int behavior);
+
 /* A loaded table: one definition per row, in the arena — as many rows as
    the file has, and not one more. */
 struct EntityTable {
@@ -71,11 +125,14 @@ struct TableResult {
 
 /* Loads an entity table from a file read whole. The header and the rows
    are parsed byte by byte — no library reads it — and anything the format
-   does not describe is refused typed: a column it does not know, a row
-   with the wrong number of values, a value where a number is required, a
-   value where text is, a name the table already holds. The rows are
-   copied into the arena behind a mark, and every refusal path rolls back
-   to it: a load that refuses leaves nothing behind. */
+   does not describe is refused typed: a column it does not know, a column
+   named twice, a row with the wrong number of values (exactly as many as
+   the header names), a value where a number is required, a value where
+   text is, a behavior the format does not define, a name the table
+   already holds. The rows are copied into the arena behind a mark, and
+   every refusal path rolls back to it: a load that refuses leaves
+   nothing behind. (Lesson 087: a column the header does *not* name is
+   no longer a refusal — its field takes the format's default.) */
 TableResult LoadTable(Arena &arena, const char *path);
 
 /* Lesson 073: a definition lookup — the request the game makes when it
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the scatter shot *(extend-the-code)*

The two weapons cover a straight line each. Give the game a scatter
gun: a third weapon row whose trigger pulls a **spread of three shots
at once** — the middle along the aim, one turned a step to either side
of it — and arm it on a third number key. The row must *state* how many
shots its trigger pulls at once, so the format grows again: do it the
way this lesson grew it (named, additive, defaulted) and verify that
every file that has shipped still loads byte-for-byte. One constraint
on the spread: every shot of it must fly at the projectile's own speed
— a spread whose side shots crawl is a spread that lies. Then run it
and watch the three shots' retirements: what do their fates tell you
about the fan you built?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-087/ex1.md)

### Exercise 2 — the projectile with no art *(fix-the-crash)*

Give the projectile table's file a header that names no `sprite` column
— the format allows it (a weapon row names no art either) — and fire
the weapon that names one of those projectile kinds. The run dies.
Work out exactly *where* the run dies and *why* it cannot survive a
definition with no art; then fix it so the failure is **typed and
named**, like every other refusal this engine answers with — and so the
weapon rows, which carry no art either, keep working exactly as they
did.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-087/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 086 — feedback and animation](lesson-086-feedback-animation.md) ·
**Next:** [Lesson 088 — enemy archetype tables](lesson-088-enemy-tables.md) ·
**Code tag:** [`lesson-087`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-087)
