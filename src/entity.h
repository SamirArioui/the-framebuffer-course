// entity.h — a live entity: the facts the game acts on.
//
// Lesson 073: an entity is a row the game created from a definition. It
// carries that definition's identity and attributes in named fields the
// game reads directly — and writes directly. The hero's position changes
// every frame and its health changes when it is hit; what the game writes
// is the entity's own copy of the facts, never the table's row. The
// definition is where an entity comes from, not what it lives in.
#ifndef ENTITY_H
#define ENTITY_H

#include "sprite.h"
#include "table.h"

namespace engine {

/* One entity: the facts the game acts on, one struct of named fields.
   No key/value bag, no lookup by string — the game reads entity.speed
   and writes entity.x like any other values. */
struct Entity {
    char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
    int x, y;                  /* where it is, in world pixels */
    int facing;                /* 0 right, 1 down, 2 left, 3 up */
    int speed;                 /* world pixels per second */
    int health;                /* points */
    const Sprite *sprite;      /* the art it draws, from its row */
    bool live;                 /* lesson 074: this entity exists — the
                                  slot's state, set by the store */
};

/* An entity created from a definition: every attribute its row states,
   answered from the definition alone. */
Entity EntityFromDef(const EntityDef &def);

/* Lesson 074: the store's capacity — a decision, made here and named in
   the closing review. Sixty-four live entities: the hero, the enemy
   types, and a screenful of projectiles. What the game does when it is
   wrong is the store's policy below, not a surprise. */
constexpr int ENTITY_CAP = 64;

/* One fixed store of live entities: slots decided up front, each slot
   holding one entity or nothing. Nothing is allocated while the game
   runs — creation takes a slot and retirement gives it back. A store
   with every slot zeroed is empty. */
struct EntityStore {
    Entity slots[ENTITY_CAP];
    int live; /* how many slots hold a live entity right now */
};

/* The creation request, answered with the entity or with the typed
   failure that says there is no room. */
enum EntityError {
    ENTITY_OK = 0,
    ENTITY_FULL, /* every slot holds a live entity */
};

struct EntityResult {
    Entity *entity;    /* the entity, or 0 */
    EntityError error; /* ENTITY_OK exactly when entity is non-0 */
};

/* Creates an entity from `def` in the store's first free slot. When
   every slot is busy the request is refused as a typed value — the
   store never steals a live entity to make room. A stolen sound is
   inaudible; a stolen enemy is a bug the player experiences. */
EntityResult EntityCreate(EntityStore &store, const EntityDef &def);

/* Lesson 075: retirement — the entity is gone and its slot is free
   again. The live count falls; the slot is reusable, and creation's
   first-free-slot rule hands it back before any slot that has never
   been used. Retiring an entity that is already gone is nothing. */
void EntityRetire(EntityStore &store, Entity &entity);

/* Lesson 075: the walk — the shape the game's per-entity work takes:

     for (int i = 0; i < ENTITY_CAP; ++i) {
         if (!store.slots[i].live)
             continue;
         ... one entity's work ...
     }

   Every live entity exactly once, in slot order; no retired or empty
   slot visited. The slots never move, so the work may retire the entity
   it is looking at — or one further along — without the walk repeating
   or skipping anyone: whoever is not live when the walk arrives is not
   visited, and everyone who is, is. */

} /* namespace engine */

#endif
