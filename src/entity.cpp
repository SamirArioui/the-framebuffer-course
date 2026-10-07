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

EntityResult EntityCreate(EntityStore &store, const EntityDef &def)
{
    EntityResult result = { 0, ENTITY_OK };

    /* The first free slot — the lowest one that holds nothing. With the
       slots fixed in place that rule also answers which slot a new
       entity takes after one is retired: the freed one, before any slot
       that has never been used (lesson 075 makes that visible). */
    for (int i = 0; i < ENTITY_CAP; ++i) {
        if (store.slots[i].live)
            continue;
        store.slots[i] = EntityFromDef(def);
        store.slots[i].live = true;
        store.live += 1;
        result.entity = &store.slots[i];
        return result;
    }

    /* Every slot busy: the request is refused as a value. The game
       decides what a refusal means; the store decides only this — that
       it is never a stolen entity. */
    result.error = ENTITY_FULL;
    return result;
}

void EntityRetire(EntityStore &store, Entity &entity)
{
    if (!entity.live)
        return; /* retiring nothing is nothing */
    entity.live = false;
    store.live -= 1;
}

void MoveEntity(const TileMap &map, Entity &entity, double dx, double dy)
{
    /* One axis at a time: a wall blocks the movement into it and the
       movement along it still works — the slide is this shape, not a
       special case. The rectangle the map is asked about is one frame of
       the entity's art (lesson 086: an animated kind's sheet is several
       frames wide, but what it draws — and so what it collides as — is
       one ANIM_FRAME_W-wide frame). */
    double next_x = entity.x + dx;
    if (!TileRectSolid(map, (int)next_x, (int)entity.y,
                       ANIM_FRAME_W, entity.sprite->height))
        entity.x = next_x;
    double next_y = entity.y + dy;
    if (!TileRectSolid(map, (int)entity.x, (int)next_y,
                       ANIM_FRAME_W, entity.sprite->height))
        entity.y = next_y;
}

} /* namespace engine */
