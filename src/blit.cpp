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

    for (int j = top; j < bottom; ++j) {
        for (int i = left; i < right; ++i) {
            const unsigned char *src =
                &s.pixels[(((size_t)(j - y) * s.width) + (i - x)) * 3];
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
