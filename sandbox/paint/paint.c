// paint.c — Lesson 014: endianness and image-header layout.
//
// A pixel buffer is bytes (lesson 013); an image FILE is bytes too, with a
// header whose field order and byte order the format pins down.  PutU16LE
// and PutU32LE write integers byte by byte in little-endian order, and
// BuildBmpHeader lays out the 54-byte BMP header field by field.
#include <stdio.h>
#include <stdlib.h>

#define W 8
#define H 6

static unsigned char *pixels;

static void PutPixel(unsigned char *px, int w, int h, int x, int y,
                     unsigned char r, unsigned char g, unsigned char b)
{
    (void)h; /* unused for now — no bounds checks yet */
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

static void DumpBytes(const unsigned char *p, int n)
{
    for (int i = 0; i < n; i++) {
        printf("%02X", p[i]);
        if (i % 16 == 15 || i == n - 1) putchar('\n');
        else putchar(' ');
    }
}

static void PutU16LE(unsigned char *dst, unsigned int v)
{
    dst[0] = (unsigned char)(v & 0xFF);
    dst[1] = (unsigned char)((v >> 8) & 0xFF);
}

static void PutU32LE(unsigned char *dst, unsigned int v)
{
    dst[0] = (unsigned char)(v & 0xFF);
    dst[1] = (unsigned char)((v >> 8) & 0xFF);
    dst[2] = (unsigned char)((v >> 16) & 0xFF);
    dst[3] = (unsigned char)((v >> 24) & 0xFF);
}

static unsigned int GetU16LE(const unsigned char *src)
{
    return (unsigned int)src[0] | ((unsigned int)src[1] << 8);
}

static unsigned int GetU32LE(const unsigned char *src)
{
    return (unsigned int)src[0]
         | ((unsigned int)src[1] << 8)
         | ((unsigned int)src[2] << 16)
         | ((unsigned int)src[3] << 24);
}

// BuildBmpHeader — lay out the 54-byte BMP header field by field.
// 14-byte file header:  "BM", file size, reserved, data offset.
// 40-byte info header: size, width, height, planes, bpp, compression,
//                      image size, resolution, colors.
static void BuildBmpHeader(unsigned char *header, int w, int h)
{
    unsigned int row_size = (unsigned int)w * 3;
    unsigned int pad = (4 - row_size % 4) % 4;
    unsigned int image_size = (row_size + pad) * (unsigned int)h;

    header[0] = 'B';
    header[1] = 'M';
    PutU32LE(header + 2, 54 + image_size);      /* file size */
    PutU32LE(header + 6, 0);                    /* reserved */
    PutU32LE(header + 10, 54);                  /* data offset */

    PutU32LE(header + 14, 40);                  /* info header size */
    PutU32LE(header + 18, (unsigned int)w);     /* width */
    PutU32LE(header + 22, (unsigned int)h);     /* height */
    PutU16LE(header + 26, 1);                   /* planes */
    PutU16LE(header + 28, 24);                  /* bits per pixel */
    PutU32LE(header + 30, 0);                   /* compression: none */
    PutU32LE(header + 34, image_size);          /* image size */
    PutU32LE(header + 38, 2835);                /* x pixels per meter */
    PutU32LE(header + 42, 2835);                /* y pixels per meter */
    PutU32LE(header + 46, 0);                   /* colors used */
    PutU32LE(header + 50, 0);                   /* important colors */
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

    /* What does a multi-byte integer look like in memory? */
    unsigned int probe = 0x01020304;
    printf("0x01020304 in memory:");
    for (int i = 0; i < 4; i++)
        printf(" %02X", ((unsigned char *)&probe)[i]);
    putchar('\n');

    unsigned char header[54];
    BuildBmpHeader(header, W, H);
    printf("BMP header:\n");
    DumpBytes(header, 54);

    printf("file size   %u\n", GetU32LE(header + 2));
    printf("data offset %u\n", GetU32LE(header + 10));
    printf("header size %u\n", GetU32LE(header + 14));
    printf("width       %u\n", GetU32LE(header + 18));
    printf("height      %u\n", GetU32LE(header + 22));
    printf("planes      %u\n", GetU16LE(header + 26));
    printf("bpp         %u\n", GetU16LE(header + 28));
    printf("image size  %u\n", GetU32LE(header + 34));

    free(pixels);
    return 0;
}
