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
#include "tilemap.h"

namespace engine {

/* One entity: the facts the game acts on, one struct of named fields.
   No key/value bag, no lookup by string — the game reads entity.speed
   and writes entity.x like any other values. */
struct Entity {
    char name[TABLE_NAME_MAX]; /* the definition's identity, carried */
    double x, y;               /* where it is, in world pixels — the
                                  row's whole pixels, the motion's
                                  doubles (lesson 076) */
    int facing;                /* 0 right, 1 down, 2 left, 3 up */
    int speed;                 /* world pixels per second */
    int health;                /* points */
    const Sprite *sprite;      /* the art it draws, from its row — for an
                                  animated kind, its sprite sheet */
    int frame;                 /* lesson 086: which frame of the sheet is
                                  showing — the walk cycle advances it */
    double frame_t;            /* and how long this frame has shown */
    double move_x, move_y;     /* lesson 076: this frame's movement
                                  request — the game sets it (the
                                  player's input for the hero, Part 5's
                                  AI for enemies) and the walk turns it
                                  into motion */
    bool live;                 /* lesson 074: this entity exists — the
                                  slot's state, set by the store */

    /* Lesson 087: the combat facts, carried from the row the entity was
       created from (or, for a shooter armed with a weapon row, from that
       row — a weapon's values are data like any other's). */
    int accel;                 /* ms: the eased-move time constant — the
                                  feel; the format's default if its row
                                  named no accel column */
    int damage;                /* points a hit from this entity removes */
    int rate;                  /* rounds per minute; 0 = never fires */
    char fires[TABLE_NAME_MAX]; /* the projectile kind it fires */
    int range;                 /* a projectile's flight budget, pixels */
    double traveled;           /* how far a projectile has flown */
    const Entity *owner;       /* the shooter of a projectile — a shot
                                  never hits its owner */
    int behavior;              /* BehaviorKind, its row's: what this
                                  entity does each frame */
    int wave;                  /* lesson 088: which wave spawns this
                                  kind — carried like every row value */
    int count;                 /* and how many join that wave */
    double cooldown;           /* seconds until it may fire again */
};

/* An entity created from a definition: every attribute its row states,
   answered from the definition alone. */
Entity EntityFromDef(const EntityDef &def);

/* Lesson 086: every frame of a sprite sheet is this wide — a walk cycle
   is a sheet of ANIM_FRAME_W-wide frames. An entity collides as one
   frame (what it draws), not as the whole sheet. */
constexpr int ANIM_FRAME_W = 16;

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

/* Lesson 077: the mover, applied to entities — the habit lesson 056
   started, as one function every entity's motion goes through. Intent
   becomes motion only where the map allows it: the move that would put
   the entity in a solid tile does not happen, and the movement along the
   wall still does (one axis at a time, which is what makes the slide
   work). Empty space is free: the entity arrives at the requested
   position. */
void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy);

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
