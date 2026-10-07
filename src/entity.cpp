// entity.cpp — entities from definitions: the row, copied into a life.
//
// Lesson 073: creating an entity is a copy. The definition keeps the
// values the file stated — every entity of a kind starts from the same
// row — and the entity carries its own, which the game is then free to
// change.

#include "entity.h"

namespace engine {

Entity EntityFromDef(const EntityDef &def)
{
    Entity entity = {};
    for (int i = 0; i < TABLE_NAME_MAX; ++i)
        entity.name[i] = def.name[i];
    entity.x = def.x;
    entity.y = def.y;
    entity.facing = def.facing;
    entity.speed = def.speed;
    entity.health = def.health;
    entity.sprite = def.image;
    return entity;
}

} /* namespace engine */
