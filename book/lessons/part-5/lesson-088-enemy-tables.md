# Lesson 088 — enemy archetype tables

{{#include ../../stability-horizon.md}}

## Prose

The world has a hero, a slime, and combat that works. What it does not
have is enemies — the three types and the boss the game was scoped
around. This lesson brings them, and the whole of it is about *how*
they arrive: **as rows.** No enemy code, no per-type attributes, no
switch on a kind's name — the roster is data, and the data is the
grown format of lesson 087 doing exactly what it was grown for.

### The roster is rows

`assets/enemies.txt` is the game's roster. Its header names the
columns its rows use — thirteen of the fifteen the format knows — and
every per-type fact is a value in a row:

```
name x y facing speed health sprite damage rate fires behavior wave count
bat 560 72 2 160 2 assets/bat.ppm 1 60 bolt chase 1 2
wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 1 1
spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2
golem 384 96 1 72 8 assets/golem.ppm 2 30 shell boss 3 1
```

Read the rows as the game's design document: the **bat** is fast and
fragile and chases; the **wisp** is fragile and flees; the **spitter**
is tougher, keeps its distance, and spits shells; the **golem** is the
boss — slow, heavy (eight health, two damage), and its `behavior` names
the one thing that is only its own: the pattern. Their attacks ride
along too (`damage`, `rate`, `fires` — lesson 090 makes them fire), and
so do their wave facts (`wave`, `count` — lesson 091 spends those).
Nothing about these four kinds exists anywhere in the code.

The columns a file does *not* name sit at the format's defaults — the
roster names no `accel` and no `range` column, and the run says so:

```
engine: table assets/enemies.txt: 4 definitions
engine: def bat: x 560 y 72 facing 2 speed 160 health 2 sprite assets/bat.ppm accel 120 damage 1 rate 60 fires bolt range 0 behavior chase wave 1 count 2
engine: def wisp: x 640 y 336 facing 2 speed 120 health 1 sprite assets/wisp.ppm accel 120 damage 1 rate 20 fires bolt range 0 behavior flee wave 1 count 1
engine: def spitter: x 120 y 400 facing 0 speed 96 health 3 sprite assets/spitter.ppm accel 120 damage 1 rate 30 fires shell range 0 behavior keep wave 2 count 2
engine: def golem: x 384 y 96 facing 1 speed 72 health 8 sprite assets/golem.ppm accel 120 damage 2 rate 30 fires shell range 0 behavior boss wave 3 count 1
```

`accel 120` and `range 0` on every row — the defaults, carried along
where the file stays silent.

### Every enemy carries its row's values

A row becomes an entity the way every entity has since lesson 073:
`EntityFromDef` copies the row's values into named fields, and the game
reads and writes those fields directly. The spawn's report prints each
entity in the same words as its definition, so the carrying is checkable
by eye against the file above:

```
engine: entity bat: x 560 y 72 facing 2 speed 160 health 2 sprite 16x16 accel 120 damage 1 rate 60 fires bolt range 0 behavior chase wave 1 count 2
engine: entity wisp: x 640 y 336 facing 2 speed 120 health 1 sprite 16x16 accel 120 damage 1 rate 20 fires bolt range 0 behavior flee wave 1 count 1
engine: entity spitter: x 120 y 400 facing 0 speed 96 health 3 sprite 16x16 accel 120 damage 1 rate 30 fires shell range 0 behavior keep wave 2 count 2
engine: entity golem: x 384 y 96 facing 1 speed 72 health 8 sprite 16x16 accel 120 damage 2 rate 30 fires shell range 0 behavior boss wave 3 count 1
engine: roster: 4 enemies from the table's rows, live 6 of 64
```

Line for line: the same positions, the same speeds, the same health,
the same weapons, the same behaviors, the same wave facts. The one
field that changes shape is the sprite — the entity carries the row's
art *loaded*, the image the path named, and prints its dimensions. The
entity is the row, alive.

### No per-type copy in code

The spawn is one loop over the table's rows. There is no `if (isBat)`,
no golem special case, no constant anywhere in `src/` holding a speed
or a health or a damage that belongs to a kind. The only names the code
knows are the ones the *game* asks for by name — the hero's row, the
weapon rows — and a lookup of a row is not a copy of an attribute.

The claim is easiest to prove by spending it: add a fifth kind to the
file and change nothing else. From a scratch copy of the run, one row
more —

```
swarmling 240 160 0 200 1 assets/spitter.ppm 1 90 bolt chase 1 3
```

— and the run grows an enemy, carrying its row's values like all the
rest:

```
engine: table assets/enemies.txt: 5 definitions
engine: def swarmling: x 240 y 160 facing 0 speed 200 health 1 sprite assets/spitter.ppm accel 120 damage 1 rate 90 fires bolt range 0 behavior chase wave 1 count 3
engine: entity swarmling: x 240 y 160 facing 0 speed 200 health 1 sprite 16x16 accel 120 damage 1 rate 90 fires bolt range 0 behavior chase wave 1 count 3
engine: roster: 5 enemies from the table's rows, live 7 of 64
```

`git diff --stat src/` is empty: a new enemy is a new row. That is the
whole point of the archetype table, and it is why the per-type facts
never moved into code when the types got interesting.

### What this run verified, and what it did not

- **Every enemy carries its row's values** — the spawn prints each
  entity in the same words as its definition, and every field matches:
  positions, speeds, health, weapons, behaviors, wave facts.
- **No per-type copy of the attributes appears in code** — the spawn is
  one loop over rows; a fifth kind added as a row alone spawns carrying
  its values with an empty `src/` diff.

What this run did **not** verify is the enemies doing anything. Their
`behavior` values are carried, not yet acted on — lesson 089 turns
`chase`, `keep`, and `flee` into movement through the mover, and `boss`
into the pattern that composes them. Their `damage`, `rate`, and
`fires` are carried too, and lesson 090 makes an armed enemy fire. The
roster standing still in a row's `x` and `y` is exactly as much game as
this lesson promised: the data, carried, waiting.

## Code step

One change: the roster, as rows. `assets/enemies.txt` is new — the
three types and the boss, every per-type fact its own row's value —
and `assets/bat.ppm`, `assets/wisp.ppm`, `assets/spitter.ppm`,
`assets/golem.ppm` are their art (four 16×16 magenta-keyed images, one
colour and set of eyes per kind). `src/entity.h/.cpp` carry the row's
last two facts (`wave`, `count`) like they carry all the others;
`src/main.cpp` loads the roster's table, spawns one entity per row
through the store, and prints each entity beside its definition. No
other file changes — and that is the lesson. Its end state is tagged
`lesson-088`.

```diff
diff --git a/assets/enemies.txt b/assets/enemies.txt
new file mode 100644
index 0000000..ffd931d
--- /dev/null
+++ b/assets/enemies.txt
@@ -0,0 +1,5 @@
+name x y facing speed health sprite damage rate fires behavior wave count
+bat 560 72 2 160 2 assets/bat.ppm 1 60 bolt chase 1 2
+wisp 640 336 2 120 1 assets/wisp.ppm 1 20 bolt flee 1 1
+spitter 120 400 0 96 3 assets/spitter.ppm 1 30 shell keep 2 2
+golem 384 96 1 72 8 assets/golem.ppm 2 30 shell boss 3 1
diff --git a/src/entity.cpp b/src/entity.cpp
index 77cdd42..98511cc 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -27,6 +27,8 @@ Entity EntityFromDef(const EntityDef &def)
     entity.rate = def.rate;
     entity.range = def.range;
     entity.behavior = def.behavior;
+    entity.wave = def.wave;
+    entity.count = def.count;
     return entity;
 }
 
diff --git a/src/entity.h b/src/entity.h
index c214bb8..a6872fb 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -54,6 +54,9 @@ struct Entity {
                                   never hits its owner */
     int behavior;              /* BehaviorKind, its row's: what this
                                   entity does each frame */
+    int wave;                  /* lesson 088: which wave spawns this
+                                  kind — carried like every row value */
+    int count;                 /* and how many join that wave */
     double cooldown;           /* seconds until it may fire again */
 };
 
diff --git a/src/main.cpp b/src/main.cpp
index 58ff0bd..6a3320a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -163,6 +163,20 @@ static void PrintDefs(const char *path, const EntityTable &table)
     }
 }
 
+/* Lesson 088: one live entity, carrying its row's values — printed in
+   the same words as the definition above, so the carrying is checkable
+   by eye against the file's rows. The sprite prints as its dimensions
+   because the entity carries the row's art *loaded* — the image, not
+   the path that named it. */
+static void PrintEntity(const char *kind, const Entity &e)
+{
+    std::printf("engine: %s %s: x %d y %d facing %d speed %d health %d sprite %dx%d accel %d damage %d rate %d fires %s range %d behavior %s wave %d count %d\n",
+                kind, e.name, (int)e.x, (int)e.y, e.facing, e.speed, e.health,
+                e.sprite->width, e.sprite->height, e.accel, e.damage, e.rate,
+                e.fires[0] ? e.fires : "none", e.range, BehaviorName(e.behavior),
+                e.wave, e.count);
+}
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -242,10 +256,13 @@ int Run(void)
        are rows that name the projectile kind they fire and carry their
        rate and damage; the projectile kinds are rows a fired shot is an
        entity of. Each file's header names the columns it uses — and only
-       those; what it leaves unnamed sits at the format's defaults. */
-    EntityTable weapons, shots;
+       those; what it leaves unnamed sits at the format's defaults.
+       Lesson 088: and the enemy roster — the three types and the boss,
+       every per-type fact its own row's value. */
+    EntityTable weapons, shots, foes;
     if (!LoadRunTable(arena, "assets/weapons.txt", weapons) ||
-        !LoadRunTable(arena, "assets/projectiles.txt", shots)) {
+        !LoadRunTable(arena, "assets/projectiles.txt", shots) ||
+        !LoadRunTable(arena, "assets/enemies.txt", foes)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
@@ -260,10 +277,12 @@ int Run(void)
     PrintDefs("assets/entities.txt", table);
     PrintDefs("assets/weapons.txt", weapons);
     PrintDefs("assets/projectiles.txt", shots);
+    PrintDefs("assets/enemies.txt", foes);
 
     /* Lesson 073: the definitions' art, loaded at startup. A row that
        names no sprite (a weapon row) has no art and needs none. */
-    if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots)) {
+    if (!LoadRunArt(arena, table) || !LoadRunArt(arena, shots) ||
+        !LoadRunArt(arena, foes)) {
         platform::CloseWindow(opened.window);
         ArenaRelease(arena);
         return 1;
@@ -341,6 +360,25 @@ int Run(void)
     std::printf("engine: world: %d entities from the table's rows, live %d of %d\n",
                 created, store.live, ENTITY_CAP);
 
+    /* Lesson 088: the enemy roster is data. The three types and the
+       boss are rows of the game's table; each row becomes one entity,
+       carrying its row's values in named fields the game reads
+       directly. A new row is a new enemy — the run has no per-kind code
+       to grow, and no per-type copy of any attribute to keep honest. */
+    for (int i = 0; i < foes.count; ++i) {
+        EntityResult made = EntityCreate(store, foes.rows[i]);
+        if (made.error != ENTITY_OK) {
+            std::fprintf(stderr, "engine: the store refused %s\n",
+                         foes.rows[i].name);
+            platform::CloseWindow(opened.window);
+            ArenaRelease(arena);
+            return 1;
+        }
+        PrintEntity("entity", *made.entity);
+    }
+    std::printf("engine: roster: %d enemies from the table's rows, live %d of %d\n",
+                foes.count, store.live, ENTITY_CAP);
+
     /* Lesson 087: weapons are rows. The hero starts armed with the
        weapons table's first row; the number keys arm the rest (HeroFire).
        The demonstration stand-in arms the foe with the second row — its
```

## Exercises

Two larger challenges. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — the kinds nobody made yet *(extend-the-code)*

Design the roster's next three kinds as **rows only**: a swarm type
that comes in numbers (its `count` says so), a fast and fragile
sprinter, and a slow heavy tank with health to spare. Give each a row
and art of your own (or reuse the art already on disk) — and then prove
the lesson's claim: the run must spawn each one carrying its row's
values, and `git diff --stat src/` must be empty. Quote the run's
roster lines and the diff. What did the columns let you express about
your three kinds that code would have hidden?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-088/ex1.md)

### Exercise 2 — why the boss does not deserve code *(explain-in-prose)*

The instinct is old and strong: *the boss is special — give it its own
update function, its own fields, its own file even.* Argue against it
in your own words, using this course's own arguments: what lesson 075's
walk rule (every live entity exactly once, one per-entity shape) costs
when five kinds have five update functions; what happens to that shape
when lesson 091's waves spawn the same kinds by the dozen; and what
"per-type attributes are data" protects that a code constant cannot.
Then steelman it: name the *one* thing a boss genuinely needs that a
row cannot carry — and say which lesson in this batch gives it that
thing instead.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-088/ex2.md)

---

**Part:** [Part 5 — the game](../../index.md) ·
**Previous:** [Lesson 087 — projectiles and two weapons](lesson-087-projectiles-weapons.md) ·
**Next:** [Lesson 089 — enemy AI](lesson-089-enemy-ai.md) ·
**Code tag:** [`lesson-088`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-088)
