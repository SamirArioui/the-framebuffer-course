// load.h — the run's asset wiring: what it loads, and the byte-level
// checks the loads print.
//
// Lesson 097: these five were born inside main.cpp, one per lesson that
// grew the run's data (066's samples, 073's art, 087's tables) — the
// loading dock with no door of its own. They are the run's wiring of the
// finished loaders (LoadSample, LoadTable, LoadSprite): which failure
// ends the run by name, and what the run prints to check its bytes
// before anything uses them. The services below stay exactly what they
// are; this pair only says how the run uses them.
#ifndef LOAD_H
#define LOAD_H

#include "arena.h"
#include "audio.h"
#include "sprite.h"
#include "table.h"

namespace engine {

/* Lesson 066: a loaded sample's facts, printed — the run's byte-level
   check on its sounds. The peak is the largest frame the sample holds,
   and it is what says how much room the format still has above the
   sound. */
void PrintSample(const char *name, const Sample &sample);

/* Lesson 066: one asset load's whole failure path — a failed load is
   named typed and ends the run by name, exactly like every other load
   the run makes. */
bool LoadRunSample(Arena &arena, const char *path, Sample &into);

/* Lesson 087: one table load's whole failure path, the same shape — the
   load either hands over every definition or names what went wrong typed
   and the run ends by name. Used for every table file the game loads. */
bool LoadRunTable(Arena &arena, const char *path, EntityTable &into);

/* Lesson 073/087: the definitions' art, loaded at startup. The sprite
   column names the file; the run loads each one and hands the definition
   its image, so an entity created from the definition is answered from
   the definition alone. A row that names no sprite (a weapon row) has no
   art and needs none. */
bool LoadRunArt(Arena &arena, EntityTable &table);

/* Lesson 087: the byte-level check on a table, before anything uses it —
   every definition, carrying every field: the values its row states and
   the format's defaults for the columns its file did not name. */
void PrintDefs(const char *path, const EntityTable &table);

} /* namespace engine */

#endif
