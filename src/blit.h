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
   lesson 015's fold at rectangle scale, never a wrap into other pixels.
   Lesson 099: a sprite with no transparent pixel (`key_count` zero —
   counted where sprites are born) draws through a straight expand with
   no per-pixel decision; a sprite with one keeps the decision per
   pixel. Both paths write the same pixels. */
void BlitSprite(Framebuffer &fb, const Sprite &sprite, int x, int y);

/* Lesson 086: one frame of a sprite sheet — the `frame_w`-wide column of
   the sheet starting at source x `src_x` — drawn at (x, y) exactly like
   BlitSprite. A walk cycle is one sheet, and this draws one step. */
void BlitSpriteFrame(Framebuffer &fb, const Sprite &sprite, int src_x,
                     int frame_w, int x, int y);

} /* namespace engine */

#endif
