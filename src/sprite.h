// sprite.h — a sprite as loaded bytes: a PPM image's pixels in our arena.
//
// Lesson 044: an asset is a file, read whole through the seam (lesson 037)
// and kept in the engine's own memory (lesson 041). The format is PPM (P6)
// — the one Part 0's lesson 017 wrote by hand: a tiny text header, then
// every pixel's three bytes in order. No library parses it; we do, and the
// bytes stay inspectable.
#ifndef SPRITE_H
#define SPRITE_H

#include "arena.h"

namespace engine {

/* The transparent color of the course's sprites: magenta. PPM carries no
   key field, so the format's convention is the loader's job — every sprite
   loaded here names 255,0,255 as "draw nothing". */
constexpr unsigned char SPRITE_KEY_R = 255;
constexpr unsigned char SPRITE_KEY_G = 0;
constexpr unsigned char SPRITE_KEY_B = 255;

/* A sprite: one image's pixels in the engine's memory — row after row,
   three bytes each (red, green, blue), exactly the file's pixel section —
   and the color that means "nothing" when it is drawn. */
struct Sprite {
    unsigned char *pixels; /* width * height * 3 bytes */
    int width;
    int height;
    unsigned char key_r, key_g, key_b; /* the transparent color */
};

/* A load either hands over a complete sprite or names what went wrong —
   never a half-loaded sprite presented as success. */
enum SpriteError {
    SPRITE_OK = 0,
    SPRITE_MISSING,   /* the file is not there or cannot be read */
    SPRITE_MALFORMED, /* the bytes are not a complete P6 image */
    SPRITE_NO_ROOM,   /* the arena had no room for the pixels */
};

struct SpriteResult {
    Sprite sprite;
    SpriteError error; /* SPRITE_OK exactly when sprite.pixels is non-0 */
};

/* Loads a PPM (P6) image from a file. The header is parsed byte by byte,
   the pixel bytes are copied into the arena, and the file's own bytes go
   back to the OS — what the engine keeps is its copy. */
SpriteResult LoadSprite(Arena &arena, const char *path);

} /* namespace engine */

#endif
