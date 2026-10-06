// blit.h — the blitter: one clipped, transparent copy from a sprite's
// bytes into the framebuffer.
//
// Lesson 045: every drawn pixel in the game comes through this loop —
// sprites now, glyphs and tiles later. One copy, one place where clipping
// and transparency live, and one honest unit of work for the deep dives of
// lessons 047-049 to measure and read.
#ifndef BLIT_H
#define BLIT_H

#include "framebuffer.h"
#include "sprite.h"

namespace engine {

/* Draws a sprite with its top-left corner at (x, y): each source pixel
   becomes one framebuffer pixel carrying the exact color the sprite has,
   except the sprite's transparent color, which writes nothing at all.
   Pixels whose destination falls outside the framebuffer are dropped —
   lesson 015's fold at rectangle scale, never a wrap into other pixels. */
void BlitSprite(Framebuffer &fb, const Sprite &sprite, int x, int y);

} /* namespace engine */

#endif
