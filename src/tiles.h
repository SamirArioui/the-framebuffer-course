// tiles.h — the tile sheet, and the walk that draws a map with it.
//
// Lesson 053: tiles are sprites (lesson 045's promise, second
// installment). The sheet is a PPM image of TILE_SIZE cells in one row —
// one cell per tile kind, in the map's kind-table order — and the map's
// walk is a loop of blits at computed origins.
#ifndef TILES_H
#define TILES_H

#include "framebuffer.h"
#include "sprite.h"
#include "tilemap.h"

namespace engine {

/* The format's cell size: every tile is TILE_SIZE x TILE_SIZE pixels. */
constexpr int TILE_SIZE = 16;

/* A tile sheet: one sprite per kind, cut from the sheet at load. */
struct TileSheet {
    Sprite kinds[TILE_MAX_KINDS];
};

/* A load either hands over the sheet or names what went wrong. */
enum TileSheetError {
    TILES_OK = 0,
    TILES_MISSING,   /* the sheet is not there or cannot be read */
    TILES_MALFORMED, /* the sheet is not exactly kind_count cells wide */
    TILES_NO_ROOM,   /* the arena had no room for the tiles */
};

struct TileSheetResult {
    TileSheet sheet;
    TileSheetError error;
};

/* Loads a sheet of exactly kind_count cells of TILE_SIZE, cutting each
   cell into its own sprite — the map's kinds, as art. */
TileSheetResult LoadTileSheet(Arena &arena, const char *path,
                              int kind_count);

/* Draws a map with its top-left cell at world origin (x, y): one blit
   per cell, at (x + cell_x * TILE_SIZE, y + cell_y * TILE_SIZE). The
   blitter's clipping drops the cells that fall outside the framebuffer —
   a map bigger than the screen is an ordinary case. */
void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
                 int x, int y);

} /* namespace engine */

#endif
