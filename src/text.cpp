// text.cpp — the layout loop.
//
// Lesson 051: every character advances the layout; only characters with
// a glyph draw. Two rules, and the second one is the whole of the
// missing-glyph behavior.

#include "text.h"

#include "blit.h"

namespace engine {

void DrawText(Framebuffer &fb, const Font &font, const char *text, int x,
              int y)
{
    for (int i = 0; text[i]; ++i) {
        const Sprite *glyph = FontGlyph(font, text[i]);
        if (glyph) /* the missing-glyph rule: draw nothing, keep the slot */
            BlitSprite(fb, *glyph, x + i * FONT_CELL, y);
    }
}

int TextWidth(const char *text)
{
    int n = 0;
    while (text[n])
        ++n;
    return n * FONT_CELL;
}

} /* namespace engine */
