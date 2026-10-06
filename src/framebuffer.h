// framebuffer.h — the engine's pixels: our own bytes, our own layout.
//
// Lesson 030: the framebuffer is memory the engine owns — Part 0's paint
// buffer, grown to window size. The pixel format is the platform seam's
// contract, not any OS's: platform.h's Present carries these exact bytes.
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

namespace engine {

/* The size the window is opened at (lesson 027) and the size of the
   framebuffer behind it: one format, one geometry. */
constexpr int FRAME_WIDTH = 640;
constexpr int FRAME_HEIGHT = 480;

/* 32 bits per pixel in memory: blue, green, red, one unused byte — the
   packed pixel of lesson 013, four bytes for the alignment lesson 007
   explained. Rows run top to bottom, one pixel after another:
   the pixel at (x, y) starts at (y * width + x) * 4. */
struct Framebuffer {
    unsigned char *pixels;
    int width;
    int height;
};

/* The engine's framebuffer. Its bytes come from an OS-level reservation
   (lesson 040): whole pages, zeroed, released with ReleaseFramebuffer. */
Framebuffer *GetFramebuffer(void);

/* Gives the framebuffer's pages back to the OS. */
void ReleaseFramebuffer(void);

/* Fills every pixel with one color. */
void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                 unsigned char b);

/* Writes one pixel. Out-of-bounds writes are dropped — the fold of
   lesson 015, never a wrap into someone else's memory. */
void PutPixel(Framebuffer &fb, int x, int y, unsigned char r,
              unsigned char g, unsigned char b);

/* Reads one pixel back — the same bytes PutPixel wrote. Out of bounds, the
   result is black. */
void GetPixel(const Framebuffer &fb, int x, int y, unsigned char &r,
              unsigned char &g, unsigned char &b);

} /* namespace engine */

#endif
