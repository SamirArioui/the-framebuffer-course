// tiles.cpp — cutting the tile sheet, and the map's walk.
//
// Lesson 053: the cut is lesson 050's, the walk is lesson 045's blit in
// a loop at computed origins. Nothing here draws a pixel by any other
// path.

#include "tiles.h"

#include "blit.h"

namespace engine {

TileSheetResult LoadTileSheet(Arena &arena, const char *path, int kind_count)
{
    TileSheetResult result = { { { { 0, 0, 0, 0, 0, 0, 0 } } }, TILES_OK };

    if (kind_count <= 0 || kind_count > TILE_MAX_KINDS) {
        result.error = TILES_MALFORMED;
        return result;
    }

    /* One allocation for every kind's tile pixels. */
    size_t tile_bytes = (size_t)kind_count * TILE_SIZE * TILE_SIZE * 3;
    unsigned char *pixels = (unsigned char *)ArenaAlloc(arena, tile_bytes, 4);
    if (!pixels) {
        result.error = TILES_NO_ROOM;
        return result;
    }

    size_t mark = ArenaMark(arena);
    SpriteResult sheet = LoadSprite(arena, path);
    if (sheet.error == SPRITE_MISSING) {
        result.error = TILES_MISSING;
        return result;
    }
    if (sheet.error != SPRITE_OK) {
        result.error = TILES_MALFORMED;
        return result;
    }

    /* The sheet is exactly kind_count cells in one row. */
    if (sheet.sprite.width != kind_count * TILE_SIZE ||
        sheet.sprite.height != TILE_SIZE) {
        ArenaRollback(arena, mark);
        result.error = TILES_MALFORMED;
        return result;
    }

    for (int k = 0; k < kind_count; ++k) {
        Sprite &tile = result.sheet.kinds[k];
        tile.pixels = pixels + (size_t)k * TILE_SIZE * TILE_SIZE * 3;
        tile.width = TILE_SIZE;
        tile.height = TILE_SIZE;
        tile.key_r = sheet.sprite.key_r;
        tile.key_g = sheet.sprite.key_g;
        tile.key_b = sheet.sprite.key_b;
        for (int r = 0; r < TILE_SIZE; ++r)
            for (int c = 0; c < TILE_SIZE; ++c) {
                const unsigned char *src =
                    &sheet.sprite.pixels[(((size_t)r * sheet.sprite.width) +
                                          (k * TILE_SIZE + c)) * 3];
                unsigned char *dst =
                    &tile.pixels[(((size_t)r * TILE_SIZE) + c) * 3];
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
            }
        tile.key_count = CountKeyPixels(tile); /* lesson 099 */
    }
    ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
    result.error = TILES_OK;
    return result;
}

void DrawTileMap(Framebuffer &fb, const TileMap &map, const TileSheet &sheet,
                 int x, int y)
{
    /* Lesson 099: copy less — the walk visits only the cells the frame
       can show. The window is the visible span of cells computed once
       from the map's offset: the first cell whose right edge passes the
       frame's left (at offset x, cell -x/TILE_SIZE is the first one
       with a pixel on screen), through the last whose left edge is
       inside the frame (the fold's exclusive bound). A cell outside
       writes nothing through the blit's clipping either way — the
       pixels drawn are the same pixels; the walk is shorter. */
    int cx0 = x < 0 ? -x / TILE_SIZE : 0;
    int cy0 = y < 0 ? -y / TILE_SIZE : 0;
    int cx1 = (fb.width - x + TILE_SIZE - 1) / TILE_SIZE;
    int cy1 = (fb.height - y + TILE_SIZE - 1) / TILE_SIZE;
    if (cx1 > map.width)
        cx1 = map.width;
    if (cy1 > map.height)
        cy1 = map.height;

    for (int cy = cy0; cy < cy1; ++cy)
        for (int cx = cx0; cx < cx1; ++cx) {
            int kind = map.cells[cy * map.width + cx];
            BlitSprite(fb, sheet.kinds[kind],
                       x + cx * TILE_SIZE, y + cy * TILE_SIZE);
        }
}

} /* namespace engine */
