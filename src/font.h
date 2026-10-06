// font.h — the bitmap font: a glyph sheet cut into sprites.
//
// Lesson 050: the font is an asset like the sprite — a PPM (P6) sheet of
// 8x8 glyph cells, 16 columns by 6 rows, covering ASCII 32..127 in
// reading order. The loader cuts the sheet into one Sprite per glyph, so
// every glyph draws through the one blitter (lesson 045) with no special
// case, no second drawing path, and no new rules.
#ifndef FONT_H
#define FONT_H

#include "sprite.h"

namespace engine {

/* The sheet's format, defined by hand (lesson 050): fixed 8x8 cells in a
   16x6 grid, one character per cell starting at ASCII 32. */
constexpr int FONT_CELL = 8;
constexpr int FONT_COLS = 16;
constexpr int FONT_ROWS = 6;
constexpr int FONT_FIRST = 32;
constexpr int FONT_COUNT = FONT_COLS * FONT_ROWS; /* ASCII 32..127 */

/* A font: one sprite per glyph, cut from the sheet at load. */
struct Font {
    Sprite glyphs[FONT_COUNT];
};

/* A load either hands over a font or names what went wrong. */
enum FontError {
    FONT_OK = 0,
    FONT_MISSING,   /* the sheet is not there or cannot be read */
    FONT_MALFORMED, /* the bytes are not the 16x6 sheet the format fixes */
    FONT_NO_ROOM,   /* the arena had no room for the glyphs */
};

struct FontResult {
    Font font;
    FontError error;
};

/* Loads a glyph sheet and cuts it into FONT_COUNT glyph sprites — one
   arena allocation for all the glyph pixels, the sheet's own bytes given
   back when the copy is done. */
FontResult LoadFont(Arena &arena, const char *path);

/* The glyph sprite for a character, or 0 for a character the sheet does
   not cover. Lesson 051's layout rule builds on this answer. */
const Sprite *FontGlyph(const Font &font, char c);

} /* namespace engine */

#endif
