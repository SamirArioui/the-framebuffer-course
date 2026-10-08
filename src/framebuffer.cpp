// framebuffer.cpp — the engine's pixels, by hand.
//
// Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
// the window. Offset math, byte order, clipping — nothing else.
//
// Lesson 043: the buffer's memory comes from the engine's arena — one
// allocation, owned like everything else in the arena, released by
// releasing the arena.

#include "framebuffer.h"

#include <cstring>

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
    /* Lesson 100: the clear is the frame's second-hottest work (the
       measure pass, lesson 098) — 307,200 pixels every frame — and its
       loop writes four bytes per pixel through a byte pointer. The deep
       dives' lever is "copy wider": one *word* per pixel, in a
       contiguous fill the compiler can widen (lesson 049's lens reads
       the answer below). The word is composed from the framebuffer's
       own bytes — bytes through memcpy, so whatever order this machine
       stores words in, the four bytes land exactly as the byte stores
       put them. Same pixels, one store in four. */
    const unsigned char bytes[4] = { b, g, r, 0 };
    unsigned int color;
    std::memcpy(&color, bytes, sizeof color);

    unsigned int *pixels = (unsigned int *)fb.pixels;
    int count = fb.width * fb.height;
    for (int i = 0; i < count; ++i)
        pixels[i] = color;
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
