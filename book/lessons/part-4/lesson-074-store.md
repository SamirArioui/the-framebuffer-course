# Lesson 074 — one fixed store

{{#include ../../stability-horizon.md}}

## Prose

Lesson 073 ended with an entity and nowhere to put it: `hero` was one
local in the run, and the run's creation loop dropped everything it made
on the floor. A game is not one entity — the hero, the enemies, the
projectiles, the bursts — and "where do the live ones live?" has one
answer in this engine, the same answer every buffer here has. So the idea
of this lesson is: **one fixed store of live entities — capacity decided
up front, creation taking the first free slot, and a full store refusing
the request as a typed value.** Never stealing a live one.

### Capacity is a decision, not an event

```cpp
constexpr int ENTITY_CAP = 64;

struct EntityStore {
    Entity slots[ENTITY_CAP];
    int live; /* how many slots hold a live entity right now */
};
```

Sixty-four slots, each holding one entity or nothing — decided at build
time, not discovered at run time. This is the arena habit (lesson 041)
applied one level up: the language law of lesson 026 says no allocation
while the game runs, so the store's memory is *taken* — one struct, 64
slots, the bytes known the moment the program is compiled. Creating an
entity writes into a slot that already exists; nothing asks the OS for
anything.

The number 64 is a decision and this lesson is where it is made: the
hero, the enemy types, and a screenful of projectiles, with the closing
review naming it. What keeps a wrong capacity from being a *correctness*
bug is the policy below — a store that refuses loudly is a store you can
tune, and a store that silently takes from the wrong place is a bug with
a game attached.

### The first free slot

`EntityCreate` walks the slots from the start and takes the first one
that holds nothing:

```cpp
EntityResult EntityCreate(EntityStore &store, const EntityDef &def);
```

That rule is short on purpose, and it settles more than it looks like it
does: with the slots fixed in place, "the first free slot" is also
"the slot a retired entity freed, before any slot that has never been
used" — because a freed slot's index is lower than every never-used one.
Lesson 075 makes that visible with retirement; the rule does not change
when it arrives.

Creation is one struct copy (`EntityFromDef`, lesson 073) into a slot,
plus the count. It touches no memory outside the store — the check below
shows that as a number.

### A full store refuses; it never steals

When every slot is busy, the request is answered with `ENTITY_FULL` and
`entity` at 0. The game decides what that means — drop the projectile,
stop the wave, punish the player — and the store decides only this: that
the answer is never somebody else's entity.

This is the policy the mixer does *not* have, and the difference is
taught on purpose, side by side. `MixerPlayEffect` steals the **oldest**
effect channel when the pool is busy (lesson 065), and that is the right
policy there: a sound that is dropped is inaudible — the player hears
effects over music and would not notice one missing. An entity that is
stolen is not inaudible. It is an enemy that vanishes mid-screen, a
projectile that disappears before it lands, a boss part that stops
existing because a particle asked for room. One policy fits a resource
whose loss is unnoticeable; the other fits one whose loss is a bug the
player experiences. Both policies are the engine's, both are typed, and
neither is a surprise.

### What this run verified, and what it did not

From a real run of this lesson's end state on this machine — the run's
creation script, one request after another from the table's rows in
turn, until the store said no:

- **The busy case refuses as a typed value.** The script asked 64 times
  and got 64 entities — the hero and 63 more, `live 64 of 64` — and the
  sixty-fifth request was refused: `engine: store: creation refused
  (full), arena 1253648 -> 1253648 — creation allocates nothing`. The
  refusal is the result's error field, `ENTITY_FULL`; the run reports it
  and continues. A scratch probe (not engine code) made that sixty-fifth
  request and then looked: `past the full one: error 1, entity (nil),
  live 64` and `slots changed by the refused request: 0 of 64` — not one
  live entity touched. Nothing was overwritten or stolen: every one of
  the 64 still holds what its definition stated.
- **Creating entities allocates nothing.** The arena's used count across
  the whole script — 64 creations — is the same number on both sides:
  1253648. The store's bytes were taken when the run's stack frame was
  made, and no creation moved the bump pointer.

What this lesson does **not** verify is what the game does *with* the
refusal — that is the game's decision, and the exercise below is where
you make it. And nothing walks the store yet: 64 live entities sit in
their slots and no code visits them. That is lesson 075 — iteration and
lifetime — and it is what turns a store into something a frame can act
on.

## Code step

One change for this lesson, from a local to a store: `src/entity.h` /
`src/entity.cpp` grow `ENTITY_CAP`, `EntityStore`, the typed
`EntityResult`, and `EntityCreate` — the first free slot, the count, and
the refusal that never steals. `Entity` grows its `live` flag, the
slot's own state. `src/main.cpp` grows the run's startup: the hero's
entity is created into the store like everything else, and the creation
script fills the store from the table's rows until the typed refusal
arrives — the report naming the live count and the arena's, unchanged.
The demo's sprite, the map, and the loop are untouched. Its end state is
tagged `lesson-074`.

```diff
diff --git a/src/entity.cpp b/src/entity.cpp
index 3bef96c..36bb098 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -23,4 +23,29 @@ Entity EntityFromDef(const EntityDef &def)
     return entity;
 }
 
+EntityResult EntityCreate(EntityStore &store, const EntityDef &def)
+{
+    EntityResult result = { 0, ENTITY_OK };
+
+    /* The first free slot — the lowest one that holds nothing. With the
+       slots fixed in place that rule also answers which slot a new
+       entity takes after one is retired: the freed one, before any slot
+       that has never been used (lesson 075 makes that visible). */
+    for (int i = 0; i < ENTITY_CAP; ++i) {
+        if (store.slots[i].live)
+            continue;
+        store.slots[i] = EntityFromDef(def);
+        store.slots[i].live = true;
+        store.live += 1;
+        result.entity = &store.slots[i];
+        return result;
+    }
+
+    /* Every slot busy: the request is refused as a value. The game
+       decides what a refusal means; the store decides only this — that
+       it is never a stolen entity. */
+    result.error = ENTITY_FULL;
+    return result;
+}
+
 } /* namespace engine */
diff --git a/src/entity.h b/src/entity.h
index edcb991..d31886b 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -24,12 +24,47 @@ struct Entity {
     int speed;                 /* world pixels per second */
     int health;                /* points */
     const Sprite *sprite;      /* the art it draws, from its row */
+    bool live;                 /* lesson 074: this entity exists — the
+                                  slot's state, set by the store */
 };
 
 /* An entity created from a definition: every attribute its row states,
    answered from the definition alone. */
 Entity EntityFromDef(const EntityDef &def);
 
+/* Lesson 074: the store's capacity — a decision, made here and named in
+   the closing review. Sixty-four live entities: the hero, the enemy
+   types, and a screenful of projectiles. What the game does when it is
+   wrong is the store's policy below, not a surprise. */
+constexpr int ENTITY_CAP = 64;
+
+/* One fixed store of live entities: slots decided up front, each slot
+   holding one entity or nothing. Nothing is allocated while the game
+   runs — creation takes a slot and retirement gives it back. A store
+   with every slot zeroed is empty. */
+struct EntityStore {
+    Entity slots[ENTITY_CAP];
+    int live; /* how many slots hold a live entity right now */
+};
+
+/* The creation request, answered with the entity or with the typed
+   failure that says there is no room. */
+enum EntityError {
+    ENTITY_OK = 0,
+    ENTITY_FULL, /* every slot holds a live entity */
+};
+
+struct EntityResult {
+    Entity *entity;    /* the entity, or 0 */
+    EntityError error; /* ENTITY_OK exactly when entity is non-0 */
+};
+
+/* Creates an entity from `def` in the store's first free slot. When
+   every slot is busy the request is refused as a typed value — the
+   store never steals a live entity to make room. A stolen sound is
+   inaudible; a stolen enemy is a bug the player experiences. */
+EntityResult EntityCreate(EntityStore &store, const EntityDef &def);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index abb8e6e..d84675f 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -229,7 +229,8 @@ int Run(void)
 
     /* Lesson 073: the game's first entity — created from the hero's
        definition, carrying the values its row states in named fields the
-       game reads directly. */
+       game reads directly. Lesson 074: it lives in the store now, in a
+       slot of the capacity decided up front. */
     DefResult hero_def = TableFind(table, "hero");
     if (hero_def.error != DEF_OK) {
         std::fprintf(stderr,
@@ -238,11 +239,36 @@ int Run(void)
         ArenaRelease(arena);
         return 1;
     }
-    Entity hero = EntityFromDef(*hero_def.def);
+    EntityStore store = {};
+    EntityResult hero_made = EntityCreate(store, *hero_def.def);
+    if (hero_made.error != ENTITY_OK) {
+        std::fprintf(stderr, "engine: the store refused the hero\n");
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Entity &hero = *hero_made.entity;
     std::printf("engine: entity %s: x %d y %d facing %d speed %d health %d sprite %dx%d\n",
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
+    /* Lesson 074: the creation script — request after request, each an
+       entity from the table's rows in turn, until the store answers with
+       its typed failure. This is the policy under load: the first free
+       slot, and never a live entity's. */
+    size_t before_script = arena.used;
+    int created = 0;
+    for (;;) {
+        EntityResult made = EntityCreate(store, table.rows[created % table.count]);
+        if (made.error != ENTITY_OK)
+            break;
+        created += 1;
+    }
+    std::printf("engine: store: live %d of %d — the hero and %d from the script\n",
+                store.live, ENTITY_CAP, created);
+    std::printf("engine: store: creation refused (full), arena %zu -> %zu — creation allocates nothing\n",
+                before_script, arena.used);
+
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
        attributes. */
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The refusal that gets ignored *(fix-the-crash)*

`EntityResult` carries the entity *and* the error, and the contract is
that `entity` is 0 exactly when the error is not `ENTITY_OK`. Make the
run forget that: after the creation script has filled the store, keep
the last request's `entity` pointer and use it — print its `name` —
without looking at the error. Rebuild with lesson 013's instrument —
AddressSanitizer, through `./build.sh`'s `CXXFLAGS` and `LDFLAGS`
overrides — and run. Read what the run and the sanitizer say. Then fix
the run so a refused request is *handled*: the game's decision, reported
and carried past, and no path that uses a request's entity before its
error is checked. Prove the run survives the full store and still ends
cleanly (`engine: closed`).

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-074/ex1.md)

### Exercise 2 — The capacity you would pick *(explain-in-prose)*

Sixty-four is this lesson's decision; yours may differ. Name the store's
capacity *your* game needs and show the count: the hero, the enemies
alive at once, the projectiles in flight, the particles of one burst.
Then answer in your own words, with the code in front of you: what does
your game do when the capacity is too small (which requests are refused,
and what should the game do about it?), and what does it cost when it is
too large (the store's bytes are taken either way — make the run report
`sizeof(Entity)` and the store's total so the cost is a number, not a
shrug)? And why is a wrong capacity here a *tuning* problem rather than a
correctness bug — what in the store's design makes that true?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-074/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 073 — entities as rows](lesson-073-rows.md) ·
**Next:** [Lesson 075 — the walk and the free slot](lesson-075-lifetime.md) ·
**Code tag:** [`lesson-074`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-074)
