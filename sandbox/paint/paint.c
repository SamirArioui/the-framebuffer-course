// paint.c — Lesson 013: raw bytes and pixel formats.
//
// A pixel buffer is nothing but bytes.  Ours is width x height pixels at
// 3 bytes each (RGB888), laid out row by row.  PutPixel and GetPixel map
// (x, y, color) onto byte offsets; HexDump shows the buffer as it really
// sits in memory.
#include <stdio.h>
#include <stdlib.h>

#define W 8
#define H 6

static unsigned char *pixels;

static void PutPixel(unsigned char *px, int w, int h, int x, int y,
                     unsigned char r, unsigned char g, unsigned char b)
{
    (void)h; /* unused so far — bounds arrive in the exercises below */
    int off = y * (w * 3) + x * 3;
    px[off + 0] = r;
    px[off + 1] = g;
    px[off + 2] = b;
}

static void GetPixel(const unsigned char *px, int w, int h, int x, int y,
                     unsigned char *r, unsigned char *g, unsigned char *b)
{
    (void)h;
    int off = y * (w * 3) + x * 3;
    *r = px[off + 0];
    *g = px[off + 1];
    *b = px[off + 2];
}

static void HexDump(const unsigned char *px, int w, int h)
{
    for (int y = 0; y < h; y++) {
        printf("row %d:", y);
        for (int i = 0; i < w * 3; i++)
            printf(" %02X", px[y * (w * 3) + i]);
        putchar('\n');
    }
}

int main(void)
{
    pixels = calloc((size_t)W * H * 3, 1);
    if (pixels == NULL) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    PutPixel(pixels, W, H, 0, 0, 255, 0, 0); /* red, top-left */
    PutPixel(pixels, W, H, 1, 0, 0, 255, 0); /* green, beside it */
    PutPixel(pixels, W, H, 7, 5, 0, 0, 255); /* blue, bottom-right */

    unsigned char r, g, b;
    GetPixel(pixels, W, H, 1, 0, &r, &g, &b);
    printf("pixel (1,0) = %u %u %u\n", (unsigned)r, (unsigned)g, (unsigned)b);

    HexDump(pixels, W, H);
    free(pixels);
    return 0;
}
