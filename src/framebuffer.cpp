// framebuffer.cpp — the engine's pixels, by hand.
//
// Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
// the window. Offset math, byte order, clipping — nothing else.
//
// Lesson 043: the buffer's memory comes from the engine's arena — one
// allocation, owned like everything else in the arena, released by
// releasing the arena.

#include "framebuffer.h"

namespace engine {

static Framebuffer framebuffer = { 0, FRAME_WIDTH, FRAME_HEIGHT };

Framebuffer *GetFramebuffer(Arena &arena)
{
    if (!framebuffer.pixels) {
        framebuffer.pixels = (unsigned char *)ArenaAlloc(
            arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 4, 4096);
    }
    return &framebuffer;
}

void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                 unsigned char b)
{
    int count = fb.width * fb.height;
    for (int i = 0; i < count; ++i) {
        unsigned char *p = fb.pixels + i * 4;
        p[0] = b;
        p[1] = g;
        p[2] = r;
        p[3] = 0;
    }
}

void PutPixel(Framebuffer &fb, int x, int y, unsigned char r,
              unsigned char g, unsigned char b)
{
    if (x < 0 || x >= fb.width || y < 0 || y >= fb.height)
        return; /* dropped, not wrapped (lesson 015's fold) */

    unsigned char *p = fb.pixels + (y * fb.width + x) * 4;
    p[0] = b;
    p[1] = g;
    p[2] = r;
    p[3] = 0;
}

void GetPixel(const Framebuffer &fb, int x, int y, unsigned char &r,
              unsigned char &g, unsigned char &b)
{
    if (x < 0 || x >= fb.width || y < 0 || y >= fb.height) {
        r = g = b = 0;
        return;
    }

    const unsigned char *p = fb.pixels + (y * fb.width + x) * 4;
    b = p[0];
    g = p[1];
    r = p[2];
}

} /* namespace engine */
