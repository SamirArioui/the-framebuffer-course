// text.h — strings drawn through the blit, glyph by glyph.
//
// Lesson 051: text is sprites with bookkeeping — a layout rule and the
// one blit. The missing-glyph rule lives here: a character the font does
// not have draws nothing, and the layout does not notice.
#ifndef TEXT_H
#define TEXT_H

#include "font.h"
#include "framebuffer.h"

namespace engine {

/* Draws a string with its top-left at (x, y): one glyph per character,
   each FONT_CELL pixels to the right of the last — including characters
   with no glyph, which draw nothing but keep their slot. The layout is
   per character, so the string's shape never depends on which glyphs the
   font happens to have. Clipping and transparency are the blitter's. */
void DrawText(Framebuffer &fb, const Font &font, const char *text, int x,
              int y);

/* How many pixels a string's layout covers: its length x FONT_CELL. */
int TextWidth(const char *text);

} /* namespace engine */

#endif
