// font.cpp — cutting the glyph sheet into sprites.
//
// Lesson 050: the sheet is one sprite; a glyph is a cell of it. The cut
// is the same byte-by-byte habit as every asset here — each cell's
// pixels are copied, in order, into their own sprite.

#include "font.h"

namespace engine {

FontResult LoadFont(Arena &arena, const char *path)
{
    FontResult result = { {}, FONT_OK };

    /* One allocation for every glyph's pixels — 96 cells of 8x8x3. */
    size_t glyph_bytes =
        (size_t)FONT_COUNT * FONT_CELL * FONT_CELL * 3;
    unsigned char *pixels =
        (unsigned char *)ArenaAlloc(arena, glyph_bytes, 4);
    if (!pixels) {
        result.error = FONT_NO_ROOM;
        return result;
    }

    /* The sheet loads through the sprite loader (it is a P6 image like
       any other), and its bytes go back to the arena when the cut is
       done — the mark sits after the glyphs, so only the sheet rolls
       back. */
    size_t mark = ArenaMark(arena);
    SpriteResult sheet = LoadSprite(arena, path);
    if (sheet.error == SPRITE_MISSING) {
        result.error = FONT_MISSING;
        return result;
    }
    if (sheet.error != SPRITE_OK) {
        result.error = FONT_MALFORMED;
        return result;
    }

    /* The format is fixed: the sheet is exactly FONT_COLS x FONT_ROWS
       cells. A sheet of any other size is a different format's file. */
    if (sheet.sprite.width != FONT_COLS * FONT_CELL ||
        sheet.sprite.height != FONT_ROWS * FONT_CELL) {
        ArenaRollback(arena, mark);
        result.error = FONT_MALFORMED;
        return result;
    }

    for (int k = 0; k < FONT_COUNT; ++k) {
        int cell_x = (k % FONT_COLS) * FONT_CELL;
        int cell_y = (k / FONT_COLS) * FONT_CELL;
        Sprite &glyph = result.font.glyphs[k];
        glyph.pixels =
            pixels + (size_t)k * FONT_CELL * FONT_CELL * 3;
        glyph.width = FONT_CELL;
        glyph.height = FONT_CELL;
        glyph.key_r = sheet.sprite.key_r;
        glyph.key_g = sheet.sprite.key_g;
        glyph.key_b = sheet.sprite.key_b;
        for (int r = 0; r < FONT_CELL; ++r)
            for (int c = 0; c < FONT_CELL; ++c) {
                const unsigned char *src =
                    &sheet.sprite.pixels[(((cell_y + r) * sheet.sprite.width) +
                                          (cell_x + c)) * 3];
                unsigned char *dst =
                    &glyph.pixels[((r * FONT_CELL) + c) * 3];
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
            }
    }
    ArenaRollback(arena, mark); /* the sheet's bytes are copied; give back */
    result.error = FONT_OK;
    return result;
}

const Sprite *FontGlyph(const Font &font, char c)
{
    int index = (int)(unsigned char)c - FONT_FIRST;
    if (index < 0 || index >= FONT_COUNT)
        return 0; /* outside the sheet: the character has no glyph */
    return &font.glyphs[index];
}

} /* namespace engine */
