// world.h — the world the run plays in: its assets, its entities, its
// sound — loaded whole at startup.
//
// Lesson 097: the startup was a straight line through Run() — load,
// check, create, wire, one typed failure per stage — four hundred lines
// of composition between opening the window and the first frame. It is
// the world's construction, and it lives here: `World` is everything the
// loop touches, named in one place, and `WorldStart` builds it in the
// order the run has always reported it (the game's machine starts
// between the hero's creation and the world's fill — the reports keep
// their order because the sequence is the same sequence).
//
// Nothing here is a service's redesign (design D2): the arena, the
// table, the store, the mover, the mixer stay exactly what they are.
// This pair is the run's way of standing them up together.
#ifndef WORLD_H
#define WORLD_H

#include "arena.h"
#include "entity.h"
#include "feel.h"
#include "font.h"
#include "framebuffer.h"
#include "game.h"
#include "sound.h"
#include "table.h"
#include "tilemap.h"
#include "tiles.h"

namespace engine {

/* Everything the loop touches, in one named state: the memory, the
   frame's pixels, the world's assets (font, map, tile sheet, the five
   tables), the entities in their store, the hero the player plays, and
   the game's sound. */
struct World {
    Arena arena;
    Framebuffer *fb;
    Font font;
    TileMap map;
    TileSheet sheet;
    EntityTable table;     /* assets/entities.txt — the hero and the
                              world's scenery */
    EntityTable weapons;   /* lesson 087: weapons are rows */
    EntityTable shots;     /* the projectile kinds the weapons name */
    EntityTable foes;      /* lesson 088: the enemy roster */
    EntityTable particles; /* lesson 093: the toolkit's burst kinds */
    EntityStore store;
    Entity *hero;          /* the game's actor, the row the game asks
                              for by name */
    Sound sound;           /* lesson 095: the game's music and effects */
};

/* The run's startup, whole: every asset loaded or a named typed failure
   (the failure prints, and this answers false — the caller closes what
   the run opened), every byte-level check printed, the hero created,
   the game's machine started, the world's rows filled, the hero armed,
   the burst kind named, and the sound started. The order is the run's
   own; the reports a checklist's demonstration reads are exactly the
   reports this sequence has always printed. */
bool WorldStart(World &world, Game &game, Feedback &feel);

} /* namespace engine */

#endif
