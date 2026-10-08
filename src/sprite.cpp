// sprite.cpp — the PPM (P6) reader: a header parsed by hand, pixels copied.
//
// Lesson 044: the format is small enough to read byte by byte, and this
// file does exactly that. Part 0's habit holds: nothing in a file is
// assumed to be there until the bytes say so.

#include "sprite.h"

#include "platform.h"

namespace engine {
namespace {

/* The header is ASCII; the pixels are anything. These three helpers are
   the whole "parser" — character tests, comment skipping, one number. */

bool IsSpace(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' ||
           c == '\f';
}

bool IsDigit(unsigned char c)
{
    return c >= '0' && c <= '9';
}

/* Skips whitespace and #-to-end-of-line comments between header fields —
   the PPM format's own rules for its text part. */
void SkipBlanks(const unsigned char *data, size_t size, size_t &at)
{
    for (;;) {
        while (at < size && IsSpace(data[at]))
            ++at;
        if (at < size && data[at] == '#') {
            while (at < size && data[at] != '\n')
                ++at;
        } else {
            return;
        }
    }
}

/* One decimal field, or false when the bytes do not form one. Sizes above
   the sanity bound are refused early — a lying header is malformed, not an
   allocation request. */
bool ReadNumber(const unsigned char *data, size_t size, size_t &at, long &out)
{
    SkipBlanks(data, size, at);
    if (at >= size || !IsDigit(data[at]))
        return false;
    long value = 0;
    while (at < size && IsDigit(data[at])) {
        value = value * 10 + (data[at] - '0');
        if (value > 1000000L)
            return false;
        ++at;
    }
    out = value;
    return true;
}

} /* namespace */

SpriteResult LoadSprite(Arena &arena, const char *path)
{
    SpriteResult result = { { 0, 0, 0, 0, 0, 0, 0 }, SPRITE_OK };

    platform::FileData file = platform::ReadFile(path);
    if (file.error != platform::FILE_OK) {
        result.error = SPRITE_MISSING;
        return result;
    }

    const unsigned char *data = file.data;
    size_t size = file.size;
    size_t at = 0;
    long width = 0, height = 0, maxval = 0;

    bool ok = size >= 2 && data[0] == 'P' && data[1] == '6';
    at = 2;
    ok = ok && ReadNumber(data, size, at, width);
    ok = ok && ReadNumber(data, size, at, height);
    ok = ok && ReadNumber(data, size, at, maxval);
    ok = ok && maxval == 255; /* one byte per channel, as lesson 017 wrote */
    ok = ok && width > 0 && width <= 4096 && height > 0 && height <= 4096;

    /* Exactly one whitespace byte separates the header from the pixels.
       Not "skip whitespace here" — the pixel bytes are arbitrary, and the
       first one may itself look like whitespace. This is the line that
       keeps the parse honest. */
    ok = ok && at < size && IsSpace(data[at]);
    ++at;

    /* Complete or nothing: the file must hold exactly the pixels the
       header claims — no short read presented as a sprite. */
    size_t pixel_bytes = (size_t)width * (size_t)height * 3;
    ok = ok && size - at == pixel_bytes;

    if (!ok) {
        result.error = SPRITE_MALFORMED;
        platform::ReleaseFile(file);
        return result;
    }

    unsigned char *pixels = (unsigned char *)ArenaAlloc(arena, pixel_bytes, 4);
    if (!pixels) {
        result.error = SPRITE_NO_ROOM;
        platform::ReleaseFile(file);
        return result;
    }
    for (size_t i = 0; i < pixel_bytes; ++i)
        pixels[i] = data[at + i];
    platform::ReleaseFile(file);

    result.sprite.pixels = pixels;
    result.sprite.width = (int)width;
    result.sprite.height = (int)height;
    result.sprite.key_r = SPRITE_KEY_R;
    result.sprite.key_g = SPRITE_KEY_G;
    result.sprite.key_b = SPRITE_KEY_B;
    result.sprite.key_count = CountKeyPixels(result.sprite); /* 099 */
    result.error = SPRITE_OK;
    return result;
}

int CountKeyPixels(const Sprite &sprite)
{
    int count = 0;
    for (int i = 0; i < sprite.width * sprite.height; ++i) {
        const unsigned char *p = &sprite.pixels[(size_t)i * 3];
        if (p[0] == sprite.key_r && p[1] == sprite.key_g &&
            p[2] == sprite.key_b)
            count += 1;
    }
    return count;
}

} /* namespace engine */
