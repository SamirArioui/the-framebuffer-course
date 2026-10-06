// tilemap.h — the tilemap as loadable world data.
//
// Lesson 052: the map is a file, the format is ours, and the load is
// complete or nothing. The format, defined by hand:
//
//   <width> <height> <kind-count>       one line, three numbers
//   <cell-char> <solid: 0|1>            one line per tile kind
//   <width> characters                  one line per map row
//
// Every row is exactly <width> characters; every character names a kind
// from the table; the table carries each kind's solidity, so collision
// queries (lesson 055) are answered from the map data alone.
#ifndef TILEMAP_H
#define TILEMAP_H

#include "arena.h"

namespace engine {

/* The bounds the format fixes — a map bigger than this is a different
   format's file. */
constexpr int TILE_MAX_DIM = 256;
constexpr int TILE_MAX_KINDS = 8;

/* The cell's size in pixels: the geometry every world coordinate walks
   on. Lesson 053 draws cells at this size; lesson 055's queries read
   world positions through it. */
constexpr int TILE_SIZE = 16;

/* One tile kind: the character that names it in the file, and whether it
   is solid for collision (lesson 055 reads this; the format carries it
   from the first day). */
struct TileKind {
    char cell;
    unsigned char solid;
};

/* A loaded map: its dimensions, its kinds, and one kind index per cell. */
struct TileMap {
    int width;
    int height;
    int kind_count;
    TileKind kinds[TILE_MAX_KINDS];
    unsigned char *cells; /* width * height kind indices, in the arena */
};

/* A load either hands over a complete map or names what went wrong —
   never a partial map presented as success. */
enum TileError {
    TILE_OK = 0,
    TILE_MISSING,   /* the file is not there or cannot be read */
    TILE_MALFORMED, /* the bytes do not form a complete map */
    TILE_NO_ROOM,   /* the arena had no room for the cells */
};

struct TileResult {
    TileMap map;
    TileError error;
};

/* Loads a map from a file read whole: the first line's three numbers, the
   kind table, then exactly height rows of exactly width characters. Any
   deviation — a short row, an extra line, a character no kind claims —
   is a typed failure, and nothing is handed over. */
TileResult LoadTileMap(Arena &arena, const char *path);

/* The kind of a cell, or -1 outside the map. */
int TileAt(const TileMap &map, int x, int y);

/* Lesson 055: the collision queries — answered from the map data alone
   (the kinds' solidity), never from drawing code. The out-of-bounds
   policy is defined, not accidental: a position or rectangle outside the
   map counts as solid, so the world's edge blocks like a wall and no
   query ever reads outside the map's cells. */

/* Is the cell at (x, y) solid? Cells outside the map answer solid. */
bool TileSolid(const TileMap &map, int x, int y);

/* Point query: does this world position overlap a solid tile? */
bool TilePointSolid(const TileMap &map, int world_x, int world_y);

/* Rectangle query: does this world rectangle (w, h > 0) overlap any
   solid tile — or the world's edge, which counts as solid? */
bool TileRectSolid(const TileMap &map, int x, int y, int w, int h);

} /* namespace engine */

#endif
