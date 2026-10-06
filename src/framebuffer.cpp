// framebuffer.cpp — the engine's pixels, by hand.
//
// Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
// the window. Offset math, byte order, clipping — nothing else.
//
// Lesson 040: the buffer's memory comes from an OS-level reservation, not
// from static storage and not from an allocator — whole pages, zeroed,
// released when the engine is done with them.

#include "framebuffer.h"

#include "platform.h"

namespace engine {

static Framebuffer framebuffer = { 0, FRAME_WIDTH, FRAME_HEIGHT };
static platform::Reservation storage;

Framebuffer *GetFramebuffer(void)
{
    if (!framebuffer.pixels) {
        storage = platform::ReserveMemory((size_t)FRAME_WIDTH *
                                          FRAME_HEIGHT * 4);
        framebuffer.pixels = storage.bytes;
    }
    return &framebuffer;
}

void ReleaseFramebuffer(void)
{
    platform::ReleaseMemory(storage);
    framebuffer.pixels = 0;
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
