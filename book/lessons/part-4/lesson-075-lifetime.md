# Lesson 075 — the walk and the free slot

{{#include ../../stability-horizon.md}}

## Prose

Lesson 074 ended with a store full of entities and nothing that looked at
them: creation, refusal, and a count. A store nobody walks is a list, not
a game — the per-entity work of a frame (move, think, draw, die) needs a
way to reach every live entity exactly once. And once entities can end,
two questions follow that the store has to answer without ambiguity:
when is a slot free again, and which slot does the next entity take. So
this lesson's idea is the other half of storage: **every live entity is
walked exactly once per frame, retirement frees a slot, and the freed
slot is reused before any never-used one.**

### The walk is a loop over fixed slots

The walk has one shape, and it is the whole API:

```cpp
for (int i = 0; i < ENTITY_CAP; ++i) {
    if (!store.slots[i].live)
        continue;
    ... one entity's work ...
}
```

Every live entity exactly once, in slot order; no retired or empty slot
visited. The work is expressed *here*, once — not once per kind of
entity. Whether the entity at slot 12 is the hero or a projectile, the
work that applies to all of them is this one body.

The contract's interesting half is what happens when that body retires
something. **The slots never move.** Unlike a list that shifts its
elements when one is removed, the store's slots are fixed in place —
so a walk that retires the entity it is looking at (or one further
along, or one behind) cannot lose or repeat anyone by doing it. Whoever
is not live when the walk arrives is not visited; whoever is, is. The
run's first walk does exactly this: it retires three entities in passing
and visits all eight.

A scratch probe (not engine code) checked the visit order to the slot,
with retirements in three positions:

```
walk visited 8: 0 1 2 3 4 5 6 7  (retired at -1 -> slot -1), live 5
walk visited 5: 0 1 2 4 5  (retired at 1 -> slot 3), live 5
walk visited 6: 0 1 2 3 4 5  (retired at 2 -> slot 0), live 5
```

Line one is the demo's walk: eight live entities at slots 0-7, three of
them killed beforehand, all eight visited once each — the three retired
in passing *after* being visited, because retiring is what the work did
to them. Line two retires slot 3 while the walk stands at slot 1 —
ahead of it — and slot 3 is simply not there when the walk arrives:
visited five, no one else skipped. Line three retires slot 0 while at
slot 2 — behind, already visited — and all six are visited once each:
the retired one was seen before it went, and is not seen again.

### Retirement, and what the count means

```cpp
void EntityRetire(EntityStore &store, Entity &entity);
```

One entity gone: its slot's `live` flag clears and the count falls.
That is the entire lifetime model — no destruction, no copying, no
compacting. The slot keeps the bytes of the entity that was; they are
not the store's business to clear, and the next creation overwrites
them.

The count is `store.live`, and it is deliberately *not* the number of
slots ever filled. The game reads "how many things are alive right now"
— for the HUD, for the wave logic, for the frame budget — and a count
that only ever grows would answer a question nobody asked. The run's
report shows the count fall and rise within one second:

```
engine: walk (frame 1): visited 8 live entities, once each, in slot order; live 5 of 64
engine: store: live 8 of 64
```

### The freed slot comes back first

Creation takes the first free slot — the lowest index holding nothing
(lesson 074's rule). With slots fixed in place, that rule answers the
reuse question for free: a freed slot's index is lower than every slot
that has never been used, so **the freed slot is taken before any
never-used one**. There is no free list to maintain and no ordering to
get wrong; the scan is the policy.

The demo makes it visible. The store holds eight entities at slots 0-7;
the walk retires 2, 4, and 6; the next three requests land where they
went:

```
engine: store: created in slot 2 (freed before never-used)
engine: store: created in slot 4 (freed before never-used)
engine: store: created in slot 6 (freed before never-used)
engine: store: live 8 of 64
```

Slots 2, 4, 6 — the freed ones — while slots 8-63 have never been used
and stay unused. Had the store taken a never-used slot instead, the
report would read `created in slot 8`, and the game's memory would creep
while perfectly good slots sat empty. (One honest note: when the store
has been filled to capacity once, every slot has been used and the
distinction disappears — which is why this demo keeps its world at eight
entities rather than lesson 074's full store.)

### What this run verified, and what it did not

From a real run of this lesson's end state on this machine:

- **Every live entity is walked exactly once per frame.** The first
  walk's report: `visited 8 live entities, once each, in slot order` —
  and the probe above names the slots. The run's account closes the
  arithmetic: `engine: walk: 29 visits over 4 frames — one per live
  entity per frame`, which is 8 + 5 + 8 + 8 — the live count of each
  frame, summed, and nothing else. The walk is the update's per-entity
  work; every frame runs it once.
- **An entity retired during the walk behaves.** The walk retired slots
  2, 4, and 6 in passing and still visited all eight; the probe's
  retire-ahead and retire-behind walks confirm both directions.
- **Retirement frees a slot and the count falls.** `live 8` → `live 5`
  across one walk.
- **The freed slot is reused before any never-used one.** The three
  creations landed in slots 2, 4, and 6, not in 8, 9, 10.

What this lesson does **not** do is give the walk real work. The
per-entity body is the demo's kill check — a game's body will be
movement, behavior, drawing. That arrives lesson 076, where the hero
becomes the first entity a frame actually *does* something to.

## Code step

One change for this lesson, from storage to lifetime: `src/entity.h` /
`src/entity.cpp` grow `EntityRetire` and the walk's contract — the shape
the game's per-entity work takes, and why retiring during it is safe.
`src/main.cpp` grows the run's frame: the walk in the update phase,
every live entity visited once and the demo's killed entities retired in
passing, and the reuse script on the next frame — three requests landing
in the slots just freed. The demo's world shrinks from lesson 074's full
store to eight entities so the report can tell a freed slot from one
that has never been used. The demo's sprite, the map, and the loop are
untouched. Its end state is tagged `lesson-075`.

```diff
diff --git a/src/entity.cpp b/src/entity.cpp
index 36bb098..04ff1f3 100644
--- a/src/entity.cpp
+++ b/src/entity.cpp
@@ -48,4 +48,12 @@ EntityResult EntityCreate(EntityStore &store, const EntityDef &def)
     return result;
 }
 
+void EntityRetire(EntityStore &store, Entity &entity)
+{
+    if (!entity.live)
+        return; /* retiring nothing is nothing */
+    entity.live = false;
+    store.live -= 1;
+}
+
 } /* namespace engine */
diff --git a/src/entity.h b/src/entity.h
index d31886b..785f868 100644
--- a/src/entity.h
+++ b/src/entity.h
@@ -65,6 +65,26 @@ struct EntityResult {
    inaudible; a stolen enemy is a bug the player experiences. */
 EntityResult EntityCreate(EntityStore &store, const EntityDef &def);
 
+/* Lesson 075: retirement — the entity is gone and its slot is free
+   again. The live count falls; the slot is reusable, and creation's
+   first-free-slot rule hands it back before any slot that has never
+   been used. Retiring an entity that is already gone is nothing. */
+void EntityRetire(EntityStore &store, Entity &entity);
+
+/* Lesson 075: the walk — the shape the game's per-entity work takes:
+
+     for (int i = 0; i < ENTITY_CAP; ++i) {
+         if (!store.slots[i].live)
+             continue;
+         ... one entity's work ...
+     }
+
+   Every live entity exactly once, in slot order; no retired or empty
+   slot visited. The slots never move, so the work may retire the entity
+   it is looking at — or one further along — without the walk repeating
+   or skipping anyone: whoever is not live when the walk arrives is not
+   visited, and everyone who is, is. */
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index d84675f..51ae3de 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -252,22 +252,24 @@ int Run(void)
                 hero.name, hero.x, hero.y, hero.facing, hero.speed, hero.health,
                 hero.sprite->width, hero.sprite->height);
 
-    /* Lesson 074: the creation script — request after request, each an
-       entity from the table's rows in turn, until the store answers with
-       its typed failure. This is the policy under load: the first free
-       slot, and never a live entity's. */
-    size_t before_script = arena.used;
+    /* Lesson 074: the creation policy — the first free slot, and never
+       a live entity's. Lesson 075: the demo keeps a small world — eight
+       entities, three of them killed before the first walk — so the
+       store has free slots and the report can tell a freed slot from one
+       that has never been used. */
     int created = 0;
-    for (;;) {
-        EntityResult made = EntityCreate(store, table.rows[created % table.count]);
+    while (store.live < 8) {
+        EntityResult made =
+            EntityCreate(store, table.rows[store.live % table.count]);
         if (made.error != ENTITY_OK)
             break;
         created += 1;
     }
-    std::printf("engine: store: live %d of %d — the hero and %d from the script\n",
+    store.slots[2].health = 0;
+    store.slots[4].health = 0;
+    store.slots[6].health = 0;
+    std::printf("engine: store: live %d of %d — the hero and %d from the script; slots 2, 4, 6 killed\n",
                 store.live, ENTITY_CAP, created);
-    std::printf("engine: store: creation refused (full), arena %zu -> %zu — creation allocates nothing\n",
-                before_script, arena.used);
 
     /* The lookup's typed failure, checked on purpose: a definition the
        table does not hold is a value — never an entity with assumed
@@ -379,6 +381,7 @@ int Run(void)
        account of what it carried is the frame record's audio phase now,
        measured like every other phase of the frame. */
     int feeds = 0;         /* buffers handed to the device */
+    long walk_visits = 0;  /* lesson 075: entities visited by the walk */
     while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
@@ -393,6 +396,42 @@ int Run(void)
         double dt = now - last;
         last = now;
 
+        /* Lesson 075: the walk — every live entity, once per frame, in
+           slot order. The per-entity work is expressed here, once, and
+           not per type; today it is the demo's kill check. An entity
+           retired in passing is not visited again and no other is
+           skipped — the slots do not move under the walk. */
+        int visited = 0;
+        for (int i = 0; i < ENTITY_CAP; ++i) {
+            if (!store.slots[i].live)
+                continue;
+            visited += 1;
+            if (store.slots[i].health <= 0) {
+                std::printf("engine: walk (frame %ld): retiring slot %d in passing\n",
+                            frame.number, i);
+                EntityRetire(store, store.slots[i]);
+            }
+        }
+        walk_visits += visited;
+        if (frame_number == 1)
+            std::printf("engine: walk (frame 1): visited %d live entities, once each, in slot order; live %d of %d\n",
+                        visited, store.live, ENTITY_CAP);
+
+        /* Lesson 075: the reuse — three requests once the walk has
+           freed three slots. Each lands in a freed slot, before any
+           slot that has never been used. */
+        if (frame_number == 2) {
+            for (int k = 0; k < 3; ++k) {
+                EntityResult made =
+                    EntityCreate(store, table.rows[k % table.count]);
+                if (made.error != ENTITY_OK)
+                    break;
+                std::printf("engine: store: created in slot %d (freed before never-used)\n",
+                            (int)(made.entity - store.slots));
+            }
+            std::printf("engine: store: live %d of %d\n", store.live, ENTITY_CAP);
+        }
+
         double was_x = sprite_x, was_y = sprite_y;
         double move_x = 0.0, move_y = 0.0;
         if (platform::KeyDown(opened.window, platform::KEY_LEFT))
@@ -606,6 +645,11 @@ int Run(void)
     std::printf("engine: demo: %ld frames measured, %d buffers fed, %d effects fired, %d music wraps\n",
                 frame_number, feeds, effect_count, music_wraps);
 
+    /* Lesson 075: the walk's account — one visit per live entity per
+       frame, and nothing else. */
+    std::printf("engine: walk: %ld visits over %ld frames — one per live entity per frame\n",
+                walk_visits, frame_number);
+
     /* The account as the frame-budget table (lesson 058): the frame
        count, the average, the worst frame — and the render attributed to
        its subsystems, the report Part 5's finale grows. */
```

## Exercises

Two mixed exercises. Each ends with its solution — a diff against this
lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The walk that retires ahead *(predict-the-output)*

The walk's body may retire *any* entity, not just the one it is looking
at. Take a store with six live entities at slots 0-5 and walk it with a
body that, upon reaching slot 1, retires the entity at slot 3. Before
running anything, write down the list of slots the walk visits, in
order, and the live count after. Then write down the same two answers
for the mirrored case: a body that, upon reaching slot 3, retires the
entity at slot 1. Run both and reconcile — and say, in one sentence,
what makes both answers safe.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-075/ex1.md)

### Exercise 2 — The store's map *(extend-the-code)*

Numbers say how many; a map says where. Teach the run to print the
store's occupancy as one character per slot — a live entity and a free
slot are two characters — at each of the demo's three moments: after the
world is created, after the walk's retirements, and after the reuse.
What does the map look like at each moment? Then answer the question the
map cannot: a slot that was freed and a slot that has never been used
are both free — where does this lesson's report get the distinction
between them, and what would the store have to remember to show it in
the map?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-075/ex2.md)

---

**Part:** [Part 4 — services](../../index.md) ·
**Previous:** [Lesson 074 — one fixed store](lesson-074-store.md) ·
**Next:** [Lesson 076 — the hero as an entity](lesson-076-hero.md) ·
**Code tag:** [`lesson-075`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-075)
