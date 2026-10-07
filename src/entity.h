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
};

/* An entity created from a definition: every attribute its row states,
   answered from the definition alone. */
Entity EntityFromDef(const EntityDef &def);

} /* namespace engine */

#endif
