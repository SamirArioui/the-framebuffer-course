// main.cpp — the engine: one measured frame loop.
//
// Lesson 043: the closing demo. The platform layer does its whole job at
// once — a window kept alive, input polled, an arena-backed framebuffer
// presented, every frame measured — and this loop is the shape Part 2
// draws into. The language law of lesson 026 still holds over all of it.

#include <cstdio>

#include "arena.h"
#include "blit.h"
#include "camera.h"
#include "font.h"
#include "framebuffer.h"
#include "frame.h"
#include "platform.h"
#include "sprite.h"
#include "text.h"
#include "tilemap.h"
#include "tiles.h"

namespace engine {

/* The scene's one object: the sprite the arrow keys move. Its speed is
   the engine's — pixels per second — and the clock's dt turns it into a
   per-frame step. */
constexpr double SPRITE_SPEED = 240.0; /* pixels per second */

/* Lesson 050: the label the demo lays out by hand — one glyph per
   blit, one position per glyph. Lesson 051 replaces the hand with a
   layout loop. */
constexpr char HUD_LABEL[] = "SCORE";

/* Lesson 054: the scene, drawn through the camera. The camera's summed
   offset is applied once, at each draw's origin — the map's and the
   sprite's. The HUD is not scene and does not pass through here. */
static void DrawScene(Framebuffer &fb, const TileMap &map,
                      const TileSheet &sheet, const Sprite &sprite,
                      int sprite_x, int sprite_y, const Camera &camera)
{
    int x = CameraX(camera);
    int y = CameraY(camera);
    DrawTileMap(fb, map, sheet, -x, -y);
    BlitSprite(fb, sprite, sprite_x - x, sprite_y - y);
}

/* The framebuffer's whole content, copied out — the check's reference. */
static void Snapshot(Framebuffer &fb, unsigned char *snap)
{
    for (int y = 0; y < FRAME_HEIGHT; ++y)
        for (int x = 0; x < FRAME_WIDTH; ++x) {
            unsigned char r, g, b;
            GetPixel(fb, x, y, r, g, b);
            unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
            p[0] = r;
            p[1] = g;
            p[2] = b;
        }
}

/* Compares the framebuffer against a snapshot shifted by (dx, dy): the
   pixel at (x, y) now must be the snapshot's pixel at (x + dx, y + dy). */
static void CompareShift(Framebuffer &fb, const unsigned char *snap, int dx,
                         int dy, int &compared, int &mismatches)
{
    compared = 0;
    mismatches = 0;
    for (int y = 0; y < FRAME_HEIGHT; ++y) {
        if (y + dy < 0 || y + dy >= FRAME_HEIGHT)
            continue;
        for (int x = 0; x < FRAME_WIDTH; ++x) {
            if (x + dx < 0 || x + dx >= FRAME_WIDTH)
                continue;
            unsigned char r, g, b;
            GetPixel(fb, x, y, r, g, b);
            const unsigned char *p =
                &snap[(((y + dy) * FRAME_WIDTH) + (x + dx)) * 3];
            ++compared;
            if (r != p[0] || g != p[1] || b != p[2])
                ++mismatches;
        }
    }
}

/* Lesson 047: the caches deep dive's evidence — a copy walk over arena
   memory at two strides, timed at working-set sizes that cross this
   machine's caches. The walk is the blit's inner copy with the
   bookkeeping removed: source bytes into destination bytes, nothing
   else — so what it costs is what the blitter's copy costs.
   Lesson 049: the walkers lose their `static` so the compiler must emit
   each one as a named function — the SIMD lens needs a listing to read. */

void CopySequential(unsigned char *dst, const unsigned char *src, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        dst[i] = src[i];
}

void CopyStrided(unsigned char *dst, const unsigned char *src, size_t n,
                 size_t stride)
{
    for (size_t i = 0; i < n; i += stride)
        dst[i] = src[i];
}

static void CacheProbe(Arena &arena)
{
    const size_t sizes[] = { 4096, 65536, 524288, 4194304, 8388608,
                             12582912 };
    const size_t biggest = 12582912;

    /* Two allocations the compiler cannot connect: source and destination
       are separate arena blocks — 2 × size bytes of working set per walk,
       and the copy loop is free to run wide. */
    unsigned char *src = (unsigned char *)ArenaAlloc(arena, biggest, 64);
    unsigned char *dst = (unsigned char *)ArenaAlloc(arena, biggest, 64);
    if (!src || !dst) {
        std::printf("engine: cache probe: no room in the arena\n");
        return;
    }
    for (size_t i = 0; i < biggest; i += 4096) {
        src[i] = (unsigned char)(i * 7); /* touch every page first */
        dst[i] = 0;
    }

    std::printf("engine: cache probe — copy walk, useful GB/s per stride\n");
    std::printf("engine: %10s %12s %12s\n", "working set", "sequential",
                "stride 64");
    for (unsigned s = 0; s < sizeof sizes / sizeof sizes[0]; ++s) {
        double gbs[2];
        for (unsigned mode = 0; mode < 2; ++mode) {
            size_t stride = mode == 0 ? 1 : 64;
            size_t per_rep = sizes[s] / stride;
            long reps = (long)(8000000 / per_rep);
            if (reps < 1)
                reps = 1;
            double t0 = platform::Now();
            for (long r = 0; r < reps; ++r) {
                if (mode == 0)
                    CopySequential(dst, src, sizes[s]);
                else
                    CopyStrided(dst, src, sizes[s], 64);
            }
            double seconds = platform::Now() - t0;
            gbs[mode] = (double)per_rep * reps / seconds / 1e9;
        }
        std::printf("engine: %7zu KB %12.1f %12.1f\n", sizes[s] / 1024,
                    gbs[0], gbs[1]);
    }
}

int Run(void)
{
    platform::WindowResult opened =
        platform::OpenWindow(FRAME_WIDTH, FRAME_HEIGHT);
    if (!opened.window) {
        /* The error path: nothing was taken that the platform layer did
           not put back, and the failure is reported by name. */
        switch (opened.error) {
        case platform::OPEN_NO_DISPLAY:
            std::fprintf(stderr, "engine: no display to open a window on\n");
            break;
        case platform::OPEN_NO_WINDOW:
            std::fprintf(stderr, "engine: the OS refused the window\n");
            break;
        default:
            std::fprintf(stderr, "engine: platform error %d\n",
                         opened.error);
            break;
        }
        return 1;
    }

    /* The engine's memory: one arena over one reservation. Everything the
       engine allocates lives in here and is released together. Lesson 047
       grows it past every cache this machine has, so the cache probe can
       walk working sets bigger than all of them. */
    Arena arena;
    ArenaInit(arena, 32 * 1024 * 1024);
    Framebuffer *fb = GetFramebuffer(arena);

    /* Lesson 044: the sprite is a file's bytes. It is loaded once, at
       startup, through the seam's whole-file read into the arena — and
       then inspected like Part 0 inspected everything: by byte. */
    const char *sprite_path = "assets/sprite.ppm";
    SpriteResult loaded = LoadSprite(arena, sprite_path);
    if (loaded.error != SPRITE_OK) {
        switch (loaded.error) {
        case SPRITE_MISSING:
            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
                         sprite_path);
            break;
        case SPRITE_MALFORMED:
            std::fprintf(stderr,
                         "engine: %s: not a complete P6 image\n",
                         sprite_path);
            break;
        default:
            std::fprintf(stderr, "engine: %s: no room in the arena\n",
                         sprite_path);
            break;
        }
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    Sprite &sprite = loaded.sprite;
    long pixel_bytes = (long)sprite.width * sprite.height * 3;
    long byte_sum = 0;
    for (long i = 0; i < pixel_bytes; ++i)
        byte_sum += sprite.pixels[i];

    std::printf("engine: sprite %s: %dx%d, %ld pixel bytes\n", sprite_path,
                sprite.width, sprite.height, pixel_bytes);
    std::printf("engine: pixel 0,0 = %d,%d,%d\n", sprite.pixels[0],
                sprite.pixels[1], sprite.pixels[2]);
    std::printf("engine: pixel %d,%d = %d,%d,%d\n", sprite.width / 2,
                sprite.height / 2,
                sprite.pixels[(sprite.height / 2 * sprite.width +
                               sprite.width / 2) * 3 + 0],
                sprite.pixels[(sprite.height / 2 * sprite.width +
                               sprite.width / 2) * 3 + 1],
                sprite.pixels[(sprite.height / 2 * sprite.width +
                               sprite.width / 2) * 3 + 2]);
    std::printf("engine: pixel bytes sum to %ld\n", byte_sum);

    /* Lesson 045: the blitter's three claims, checked against the
       framebuffer's own bytes before anything depends on them. */
    ClearBuffer(*fb, 32, 32, 64);
    BlitSprite(*fb, sprite, 100, 100);
    int opaque = 0, key_pixels = 0, mismatches = 0;
    for (int j = 0; j < sprite.height; ++j)
        for (int i = 0; i < sprite.width; ++i) {
            const unsigned char *p =
                &sprite.pixels[(j * sprite.width + i) * 3];
            unsigned char r, g, b;
            GetPixel(*fb, 100 + i, 100 + j, r, g, b);
            bool is_key = p[0] == sprite.key_r && p[1] == sprite.key_g &&
                          p[2] == sprite.key_b;
            if (is_key) {
                ++key_pixels;
                if (r != 32 || g != 32 || b != 64)
                    ++mismatches; /* the key must have written nothing */
            } else {
                ++opaque;
                if (r != p[0] || g != p[1] || b != p[2])
                    ++mismatches;
            }
        }
    std::printf("engine: blit check: %d opaque pixels drawn unchanged, %d mismatches\n",
                opaque, mismatches);
    std::printf("engine: blit check: %d key pixels wrote nothing over the background\n",
                key_pixels);

    ClearBuffer(*fb, 32, 32, 64);
    BlitSprite(*fb, sprite, -4, -4);
    int landed = 0, wrong = 0, wrapped = 0;
    for (int j = 0; j < sprite.height; ++j)
        for (int i = 0; i < sprite.width; ++i) {
            if (i < 4 || j < 4)
                continue; /* these pixels landed outside and were dropped */
            const unsigned char *p =
                &sprite.pixels[(j * sprite.width + i) * 3];
            unsigned char r, g, b;
            GetPixel(*fb, i - 4, j - 4, r, g, b);
            ++landed;
            bool is_key = p[0] == sprite.key_r && p[1] == sprite.key_g &&
                          p[2] == sprite.key_b;
            if (is_key ? (r != 32 || g != 32 || b != 64)
                       : (r != p[0] || g != p[1] || b != p[2]))
                ++wrong;
        }
    for (int y = 0; y < FRAME_HEIGHT; ++y)
        for (int x = 0; x < FRAME_WIDTH; ++x) {
            if (x < sprite.width - 4 && y < sprite.height - 4)
                continue; /* the landed region, checked above */
            unsigned char r, g, b;
            GetPixel(*fb, x, y, r, g, b);
            if (r != 32 || g != 32 || b != 64)
                ++wrapped;
        }
    std::printf("engine: blit check: clip at -4,-4 landed %d pixels, %d wrong, %d touched outside\n",
                landed, wrong, wrapped);

    /* Lesson 047's evidence, measured before the loop starts. */
    CacheProbe(arena);

    /* Lesson 050: the font is an asset too — a glyph sheet the loader
       cuts into sprites. */
    const char *font_path = "assets/font.ppm";
    FontResult font_loaded = LoadFont(arena, font_path);
    if (font_loaded.error != FONT_OK) {
        switch (font_loaded.error) {
        case FONT_MISSING:
            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
                         font_path);
            break;
        case FONT_MALFORMED:
            std::fprintf(stderr,
                         "engine: %s: not a 16x6 sheet of 8x8 glyphs\n",
                         font_path);
            break;
        default:
            std::fprintf(stderr, "engine: %s: no room in the arena\n",
                         font_path);
            break;
        }
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    Font &font = font_loaded.font;
    std::printf("engine: font %s: %d glyphs of %dx%d from a %dx%d sheet\n",
                font_path, FONT_COUNT, FONT_CELL, FONT_CELL,
                FONT_COLS * FONT_CELL, FONT_ROWS * FONT_CELL);

    /* The glyph claim, checked against the framebuffer's bytes: a glyph
       drawn through the blit is the sheet's cell, pixel for pixel. */
    const char checked[2] = { 'A', 'g' };
    for (int gi = 0; gi < 2; ++gi) {
        const Sprite *glyph = FontGlyph(font, checked[gi]);
        ClearBuffer(*fb, 32, 32, 64);
        BlitSprite(*fb, *glyph, 200 + gi * 16, 64);
        int ink = 0, key = 0, mismatches = 0;
        for (int j = 0; j < glyph->height; ++j)
            for (int i = 0; i < glyph->width; ++i) {
                const unsigned char *p = &glyph->pixels[(j * glyph->width + i) * 3];
                unsigned char r, g, b;
                GetPixel(*fb, 200 + gi * 16 + i, 64 + j, r, g, b);
                bool is_key = p[0] == glyph->key_r && p[1] == glyph->key_g &&
                              p[2] == glyph->key_b;
                if (is_key) {
                    ++key;
                    if (r != 32 || g != 32 || b != 64)
                        ++mismatches;
                } else {
                    ++ink;
                    if (r != p[0] || g != p[1] || b != p[2])
                        ++mismatches;
                }
            }
        std::printf("engine: font check: glyph '%c' — %d pixels read back, %d mismatches (%d ink, %d key)\n",
                    checked[gi], glyph->width * glyph->height, mismatches,
                    ink, key);
    }

    /* Lesson 051: the text claims — glyphs laid out in order, and the
       missing-glyph rule: the character the font lacks draws nothing and
       the characters after it keep their slots. */
    ClearBuffer(*fb, 32, 32, 64);
    DrawText(*fb, font, "AB", 200, 120);
    {
        int slots_with_ink = 0;
        for (int s = 0; s < 2; ++s) {
            bool ink = false;
            for (int j = 0; j < FONT_CELL && !ink; ++j)
                for (int i = 0; i < FONT_CELL && !ink; ++i) {
                    unsigned char r, g, b;
                    GetPixel(*fb, 200 + s * FONT_CELL + i, 120 + j, r, g, b);
                    if (r != 32 || g != 32 || b != 64)
                        ink = true;
                }
            if (ink)
                ++slots_with_ink;
        }
        std::printf("engine: text check: \"AB\" at 200,120 — %d of 2 slots have ink, width %d\n",
                    slots_with_ink, TextWidth("AB"));
    }
    ClearBuffer(*fb, 32, 32, 64);
    DrawText(*fb, font, "A\xC2\xB5" "B", 200, 140); /* "AµB": µ is not in the sheet */
    {
        /* Four bytes, four slots: the µ is two bytes, and each keeps its
           slot — the layout follows the bytes, and B lands where the
           layout says. */
        int slot_state[4] = { 0, 0, 0, 0 };
        for (int s = 0; s < 4; ++s) {
            for (int j = 0; j < FONT_CELL; ++j)
                for (int i = 0; i < FONT_CELL; ++i) {
                    unsigned char r, g, b;
                    GetPixel(*fb, 200 + s * FONT_CELL + i, 140 + j, r, g, b);
                    if (r != 32 || g != 32 || b != 64)
                        ++slot_state[s];
                }
        }
        std::printf("engine: text check: \"A?B\" with a missing character — ink pixels per slot: %d, %d, %d, %d\n",
                    slot_state[0], slot_state[1], slot_state[2], slot_state[3]);
    }

    /* Lesson 052: the world as data — the map file, loaded whole, its
       cells counted against the file's own rows. */
    const char *map_path = "assets/map.txt";
    TileResult map_loaded = LoadTileMap(arena, map_path);
    if (map_loaded.error != TILE_OK) {
        switch (map_loaded.error) {
        case TILE_MISSING:
            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
                         map_path);
            break;
        case TILE_MALFORMED:
            std::fprintf(stderr,
                         "engine: %s: not a complete map\n", map_path);
            break;
        default:
            std::fprintf(stderr, "engine: %s: no room in the arena\n",
                         map_path);
            break;
        }
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    TileMap &map = map_loaded.map;
    std::printf("engine: map %s: %dx%d, %d kinds\n", map_path, map.width,
                map.height, map.kind_count);
    for (int k = 0; k < map.kind_count; ++k) {
        int count = 0;
        for (int y = 0; y < map.height; ++y)
            for (int x = 0; x < map.width; ++x)
                if (TileAt(map, x, y) == k)
                    ++count;
        std::printf("engine: map kind '%c' (solid %d): %d cells\n",
                    map.kinds[k].cell, map.kinds[k].solid, count);
    }
    int unknown = 0, solid_corners = 0;
    for (int y = 0; y < map.height; ++y)
        for (int x = 0; x < map.width; ++x) {
            int kind = TileAt(map, x, y);
            if (kind < 0 || kind >= map.kind_count)
                ++unknown;
        }
    if (map.kinds[TileAt(map, 0, 0)].solid)
        ++solid_corners;
    if (map.kinds[TileAt(map, map.width - 1, map.height - 1)].solid)
        ++solid_corners;
    std::printf("engine: map check: %d cells, %d unknown, %d of 2 corners solid\n",
                map.width * map.height, unknown, solid_corners);

    /* Lesson 053: tiles are sprites — the sheet, cut per kind. */
    const char *tiles_path = "assets/tiles.ppm";
    TileSheetResult tiles_loaded = LoadTileSheet(arena, tiles_path,
                                                 map.kind_count);
    if (tiles_loaded.error != TILES_OK) {
        switch (tiles_loaded.error) {
        case TILES_MISSING:
            std::fprintf(stderr, "engine: %s: missing or unreadable\n",
                         tiles_path);
            break;
        case TILES_MALFORMED:
            std::fprintf(stderr,
                         "engine: %s: not one %dx%d cell per map kind\n",
                         tiles_path, TILE_SIZE, TILE_SIZE);
            break;
        default:
            std::fprintf(stderr, "engine: %s: no room in the arena\n",
                         tiles_path);
            break;
        }
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    TileSheet &sheet = tiles_loaded.sheet;
    std::printf("engine: tiles %s: %d tiles of %dx%d\n", tiles_path,
                map.kind_count, TILE_SIZE, TILE_SIZE);

    /* The map bigger than the frame, drawn at two offsets: the same
       world pixels at world-position minus offset, everywhere the two
       draws overlap. */
    unsigned char *snap = (unsigned char *)ArenaAlloc(
        arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 3, 4);
    if (!snap) {
        std::fprintf(stderr, "engine: no room for the tilemap check\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    ClearBuffer(*fb, 32, 32, 64);
    DrawTileMap(*fb, map, sheet, 0, 0);
    for (int y = 0; y < FRAME_HEIGHT; ++y)
        for (int x = 0; x < FRAME_WIDTH; ++x) {
            unsigned char r, g, b;
            GetPixel(*fb, x, y, r, g, b);
            unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
            p[0] = r;
            p[1] = g;
            p[2] = b;
        }
    ClearBuffer(*fb, 32, 32, 64);
    DrawTileMap(*fb, map, sheet, -37, -25);
    int compared = 0, moved_mismatches = 0;
    for (int y = 25; y < FRAME_HEIGHT; ++y)
        for (int x = 37; x < FRAME_WIDTH; ++x) {
            unsigned char r, g, b;
            GetPixel(*fb, x - 37, y - 25, r, g, b);
            const unsigned char *p = &snap[((y * FRAME_WIDTH) + x) * 3];
            ++compared;
            if (r != p[0] || g != p[1] || b != p[2])
                ++moved_mismatches;
        }
    std::printf("engine: tilemap check: map %dx%d px over frame %dx%d — %d pixels compared at offset 37,25, %d mismatches\n",
                map.width * TILE_SIZE, map.height * TILE_SIZE, FRAME_WIDTH,
                FRAME_HEIGHT, compared, moved_mismatches);

    /* Lesson 054: the camera's three claims, each checked against the
       framebuffer's pixels: the base scrolls the scene, the additive
       offset stacks over it, and clearing the additive restores the
       base view exactly. */
    unsigned char *snap2 = (unsigned char *)ArenaAlloc(
        arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 3, 4);
    Camera camera = { 0, 0, 0, 0 };
    if (!snap2) {
        std::fprintf(stderr, "engine: no room for the camera check\n");
        platform::CloseWindow(opened.window);
        ArenaRelease(arena);
        return 1;
    }
    ClearBuffer(*fb, 32, 32, 64);
    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
    Snapshot(*fb, snap);

    camera.base_x = 100;
    camera.base_y = 50;
    ClearBuffer(*fb, 32, 32, 64);
    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
    int cam_cmp = 0, cam_bad = 0;
    CompareShift(*fb, snap, 100, 50, cam_cmp, cam_bad);
    std::printf("engine: camera check: base (100,50) scrolls the scene — %d pixels compared, %d mismatches\n",
                cam_cmp, cam_bad);
    Snapshot(*fb, snap2); /* the base view, for the restore check below */

    camera.add_x = 7;
    camera.add_y = -3;
    ClearBuffer(*fb, 32, 32, 64);
    DrawScene(*fb, map, sheet, sprite, 200, 150, camera); /* base + add */
    Snapshot(*fb, snap);
    Camera summed = { 107, 47, 0, 0 }; /* the same sum, written out */
    ClearBuffer(*fb, 32, 32, 64);
    DrawScene(*fb, map, sheet, sprite, 200, 150, summed);
    CompareShift(*fb, snap, 0, 0, cam_cmp, cam_bad);
    std::printf("engine: camera check: additive (7,-3) stacks over base — %d pixels compared, %d mismatches\n",
                cam_cmp, cam_bad);

    camera.add_x = 0;
    camera.add_y = 0;
    ClearBuffer(*fb, 32, 32, 64);
    DrawScene(*fb, map, sheet, sprite, 200, 150, camera);
    CompareShift(*fb, snap2, 0, 0, cam_cmp, cam_bad);
    std::printf("engine: camera check: additive cleared restores the base view — %d pixels compared, %d mismatches\n",
                cam_cmp, cam_bad);

    /* Lesson 055: the collision queries — every documented answer
       checked against the map's own data, out-of-bounds included. */
    struct CollisionCase {
        const char *what;
        bool point; /* true: a point query; false: a rectangle */
        int x, y, w, h;
        bool expected;
    };
    const CollisionCase cases[] = {
        { "point over floor", true, 256, 224, 0, 0, false },
        { "point in the border wall", true, 8, 8, 0, 0, true },
        { "point in a pillar", true, 128, 96, 0, 0, true },
        { "point in the water", true, 528, 416, 0, 0, false },
        { "point left of the map", true, -1, 100, 0, 0, true },
        { "point past the right edge", true, 768, 100, 0, 0, true },
        { "point below the map", true, 100, 512, 0, 0, true },
        { "rect over floor", false, 240, 216, 32, 32, false },
        { "rect reaching a pillar", false, 120, 88, 32, 32, true },
        { "rect leaving the map", false, -8, 100, 16, 16, true },
        { "rect past the right edge", false, 760, 100, 16, 16, true },
        { "empty rect", false, 100, 100, 0, 0, false },
    };
    int passed = 0;
    for (unsigned ci = 0; ci < sizeof cases / sizeof cases[0]; ++ci) {
        const CollisionCase &c = cases[ci];
        bool answer = c.point ? TilePointSolid(map, c.x, c.y)
                              : TileRectSolid(map, c.x, c.y, c.w, c.h);
        if (answer == c.expected) {
            ++passed;
        } else {
            std::printf("engine: collision check: %s — expected %s, got %s\n",
                        c.what, c.expected ? "solid" : "free",
                        answer ? "solid" : "free");
        }
    }
    std::printf("engine: collision check: %d of %d answers as documented (out-of-bounds is solid)\n",
                passed, (int)(sizeof cases / sizeof cases[0]));

    double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
    double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
    double started = platform::Now();
    double last = started;
    int shake_frames = 0; /* lesson 054: the additive hook's demo */

    std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
    std::printf("engine: arrow keys move the sprite, space shakes the camera; close the window to stop\n");
    std::printf("engine: sprite at %d,%d\n", (int)sprite_x, (int)sprite_y);

    /* The frame step: read news, update from polled state, draw, present —
       every phase measured, one record per frame. */
    int exit_code = 0;
    long frame_number = 0;
    FrameStats stats = {};
    while (!platform::CloseRequested(opened.window)) {
        platform::PumpEvents(opened.window);
        if (platform::CloseRequested(opened.window))
            break;

        FrameRecord frame;
        frame.number = ++frame_number;
        double t0 = platform::Now();

        /* Update: a frame reads state — it never handles events. */
        double now = platform::Now();
        double dt = now - last;
        last = now;

        int old_x = (int)sprite_x, old_y = (int)sprite_y;
        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
            sprite_x -= SPRITE_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
            sprite_x += SPRITE_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_UP))
            sprite_y -= SPRITE_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
            sprite_y += SPRITE_SPEED * dt;

        /* The sprite stays in the world — the map's bounds now, not the
           screen's: the camera moves the view, the world is bigger. */
        if (sprite_x < 0)
            sprite_x = 0;
        if (sprite_x > map.width * TILE_SIZE - sprite.width)
            sprite_x = map.width * TILE_SIZE - sprite.width;
        if (sprite_y < 0)
            sprite_y = 0;
        if (sprite_y > map.height * TILE_SIZE - sprite.height)
            sprite_y = map.height * TILE_SIZE - sprite.height;

        /* Lesson 054: the camera's base follows the sprite — the world
           scrolls under the movement — clamped to the map's bounds. */
        int base_x = (int)sprite_x + sprite.width / 2 - FRAME_WIDTH / 2;
        int base_y = (int)sprite_y + sprite.height / 2 - FRAME_HEIGHT / 2;
        if (base_x < 0)
            base_x = 0;
        if (base_y < 0)
            base_y = 0;
        if (base_x > map.width * TILE_SIZE - FRAME_WIDTH)
            base_x = map.width * TILE_SIZE - FRAME_WIDTH;
        if (base_y > map.height * TILE_SIZE - FRAME_HEIGHT)
            base_y = map.height * TILE_SIZE - FRAME_HEIGHT;
        if (base_x != camera.base_x || base_y != camera.base_y) {
            camera.base_x = base_x;
            camera.base_y = base_y;
            std::printf("engine: camera base %d,%d (t=%.3f)\n", base_x,
                        base_y, platform::Now() - started);
        }

        /* The additive offset: the hook the juice toolkit will drive.
           Here SPACE demonstrates it — a shake that ends at zero, which
           is where it lives at rest. */
        if (platform::KeyPressed(opened.window, platform::KEY_SPACE) &&
            shake_frames <= 0) {
            shake_frames = 30;
            std::printf("engine: camera additive 6,0 (shake starts)\n");
        }
        if (shake_frames > 0) {
            --shake_frames;
            camera.add_x = (shake_frames % 2) ? 6 : -6;
            camera.add_y = 0;
            if (shake_frames == 0) {
                camera.add_x = 0;
                camera.add_y = 0;
                std::printf("engine: camera additive 0,0 (at rest)\n");
            }
        }

        frame.update = platform::Now() - t0;
        double t1 = platform::Now();

        if ((int)sprite_x != old_x || (int)sprite_y != old_y)
            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
                        (int)sprite_y, platform::Now() - started);

        /* Render: every frame draws the whole scene — clear, then the
           world through the camera, then the HUD — each timed as its own
           named phase: the subsystems the frame record can name. The
           camera's summed offset is applied once, at each draw's origin. */
        int cam_x = CameraX(camera);
        int cam_y = CameraY(camera);
        ClearBuffer(*fb, 32, 32, 64);
        double t_tilemap = platform::Now();
        DrawTileMap(*fb, map, sheet, -cam_x, -cam_y);
        frame.tilemap = platform::Now() - t_tilemap;
        double t_sprites = platform::Now();
        BlitSprite(*fb, sprite, (int)sprite_x - cam_x, (int)sprite_y - cam_y);
        frame.sprites = platform::Now() - t_sprites;
        double t_text = platform::Now();
        DrawText(*fb, font, HUD_LABEL, 8, 8);
        frame.text = platform::Now() - t_text;

        frame.render = platform::Now() - t1;
        double t2 = platform::Now();

        if (!platform::Present(opened.window, fb->pixels, fb->width,
                               fb->height)) {
            /* A present can fail because the window died mid-copy — that
               is close news and the fold already said so. Anything else is
               a real failure and is reported as one. */
            if (platform::CloseRequested(opened.window))
                break;
            std::fprintf(stderr, "engine: presentation failed\n");
            exit_code = 1;
            break;
        }

        frame.present = platform::Now() - t2;
        frame.total = platform::Now() - t0;
        AccountFrame(stats, frame);

        /* The frame log: one line per record — the format grows its named
           fields, one per subsystem, as the parts name them. */
        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f, text %.3f, tilemap %.3f), present %.3f ms, total %.3f ms\n",
                    frame.number, frame.update * 1e3, frame.render * 1e3,
                    frame.sprites * 1e3, frame.text * 1e3,
                    frame.tilemap * 1e3, frame.present * 1e3,
                    frame.total * 1e3);
    }

    /* The account: what the frames actually cost, including the honest
       price of the presentation copy. */
    if (stats.frames) {
        double n = (double)stats.frames;
        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, text %.3f, tilemap %.3f, present %.3f)\n",
                    stats.frames, stats.total_sum / n * 1e3,
                    stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
                    stats.sprites_sum / n * 1e3, stats.text_sum / n * 1e3,
                    stats.tilemap_sum / n * 1e3, stats.present_sum / n * 1e3);
        std::printf("engine: worst frame %.3f ms (frame %ld); present is %.0f%% of the frame\n",
                    stats.worst * 1e3, stats.worst_number,
                    100.0 * stats.present_sum / stats.total_sum);
    }
    std::printf("engine: arena: %zu of %zu bytes used\n", arena.used,
                arena.memory.size);

    if (platform::CloseRequested(opened.window))
        std::printf("engine: close reported\n");
    platform::CloseWindow(opened.window);
    ArenaRelease(arena);
    std::printf("engine: closed\n");
    return exit_code;
}

} /* namespace engine */

int main(void)
{
    return engine::Run();
}
