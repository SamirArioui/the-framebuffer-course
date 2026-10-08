// blit.cpp — the copy loop.
//
// Lesson 045: the engine's one drawing loop. It clips first — the
// intersection of the sprite's rectangle with the framebuffer is the only
// region that can be drawn — then copies bytes: three of the sprite's
// bytes into four of the framebuffer's, in the framebuffer's order,
// skipping pixels of the transparent color.

#include "blit.h"

namespace engine {

void BlitSprite(Framebuffer &fb, const Sprite &s, int x, int y)
{
    /* The fold, at rectangle scale: the in-bounds region computed once,
       so the copy loop below never checks bounds again. */
    int left = x < 0 ? 0 : x;
    int top = y < 0 ? 0 : y;
    int right = x + s.width < fb.width ? x + s.width : fb.width;
    int bottom = y + s.height < fb.height ? y + s.height : fb.height;

    /* Lesson 099: this loop is the map's draw, and the measure pass
       named it the game's hottest work — so it gets the deep dives'
       levers, and every pixel lands exactly where it always landed.

       Copy closer together: each row's source and destination pointers
       are computed once and stepped, never recomputed per pixel.

       Copy wider: a sprite with no transparent pixel — `key_count`
       zero, counted at load — takes the straight expand below, with no
       per-pixel decision: the shape lesson 049's lens reads (a guard, a
       wide loop, the tails folded into the row).

       And the key path keeps its per-pixel decision because it must:
       what would "vectorized transparency" mean? (lesson 049). */
    if (s.key_count == 0) {
        for (int j = top; j < bottom; ++j) {
            const unsigned char *src =
                s.pixels + (((size_t)(j - y) * s.width) + (left - x)) * 3;
            unsigned char *dst =
                fb.pixels + (((size_t)j * fb.width) + left) * 4;
            for (int i = left; i < right; ++i) {
                dst[0] = src[2]; /* blue */
                dst[1] = src[1]; /* green */
                dst[2] = src[0]; /* red */
                dst[3] = 0;
                src += 3;
                dst += 4;
            }
        }
        return;
    }

    for (int j = top; j < bottom; ++j) {
        const unsigned char *src =
            s.pixels + (((size_t)(j - y) * s.width) + (left - x)) * 3;
        unsigned char *dst =
            fb.pixels + (((size_t)j * fb.width) + left) * 4;
        for (int i = left; i < right; ++i) {
            if (!(src[0] == s.key_r && src[1] == s.key_g &&
                  src[2] == s.key_b)) { /* the transparent color writes
                                           nothing */
                dst[0] = src[2]; /* blue */
                dst[1] = src[1]; /* green */
                dst[2] = src[0]; /* red */
                dst[3] = 0;
            }
            src += 3;
            dst += 4;
        }
    }
}

void BlitSpriteFrame(Framebuffer &fb, const Sprite &s, int src_x,
                     int frame_w, int x, int y)
{
    /* BlitSprite, scoped to one frame_w-wide column of the sheet at
       source x src_x: the source pixel's column is src_x + (i - x). */
    int left = x < 0 ? 0 : x;
    int top = y < 0 ? 0 : y;
    int right = x + frame_w < fb.width ? x + frame_w : fb.width;
    int bottom = y + s.height < fb.height ? y + s.height : fb.height;

    for (int j = top; j < bottom; ++j) {
        for (int i = left; i < right; ++i) {
            const unsigned char *src =
                &s.pixels[(((size_t)(j - y) * s.width) +
                           (src_x + (i - x))) * 3];
            if (src[0] == s.key_r && src[1] == s.key_g && src[2] == s.key_b)
                continue; /* the transparent color writes nothing */
            unsigned char *dst =
                &fb.pixels[(((size_t)j * fb.width) + i) * 4];
            dst[0] = src[2]; /* blue */
            dst[1] = src[1]; /* green */
            dst[2] = src[0]; /* red */
            dst[3] = 0;
        }
    }
}

} /* namespace engine */
