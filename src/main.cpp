// main.cpp — the engine: one measured frame loop.
//
// Lesson 043: the closing demo. The platform layer does its whole job at
// once — a window kept alive, input polled, an arena-backed framebuffer
// presented, every frame measured — and this loop is the shape Part 2
// draws into. The language law of lesson 026 still holds over all of it.

#include <cstdio>

#include "arena.h"
#include "blit.h"
#include "framebuffer.h"
#include "frame.h"
#include "platform.h"
#include "sprite.h"

namespace engine {

/* The scene's one object: the sprite the arrow keys move. Its speed is
   the engine's — pixels per second — and the clock's dt turns it into a
   per-frame step. */
constexpr double SPRITE_SPEED = 240.0; /* pixels per second */

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

    double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
    double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
    double started = platform::Now();
    double last = started;

    std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
    std::printf("engine: arrow keys move the sprite; close the window to stop\n");
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

        /* The sprite stays on screen — lesson 015's fold at frame scale. */
        if (sprite_x < 0)
            sprite_x = 0;
        if (sprite_x > FRAME_WIDTH - sprite.width)
            sprite_x = FRAME_WIDTH - sprite.width;
        if (sprite_y < 0)
            sprite_y = 0;
        if (sprite_y > FRAME_HEIGHT - sprite.height)
            sprite_y = FRAME_HEIGHT - sprite.height;

        frame.update = platform::Now() - t0;
        double t1 = platform::Now();

        if ((int)sprite_x != old_x || (int)sprite_y != old_y)
            std::printf("engine: sprite at %d,%d (t=%.3f)\n", (int)sprite_x,
                        (int)sprite_y, platform::Now() - started);

        /* Render: every frame draws the whole scene — clear, then the
           sprite through the one blit. The sprite draw is timed as its own
           named phase: the first subsystem the frame record can name. */
        ClearBuffer(*fb, 32, 32, 64);
        double t_sprites = platform::Now();
        BlitSprite(*fb, sprite, (int)sprite_x, (int)sprite_y);
        frame.sprites = platform::Now() - t_sprites;

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
        std::printf("frame %ld: update %.3f ms, render %.3f ms (sprites %.3f), present %.3f ms, total %.3f ms\n",
                    frame.number, frame.update * 1e3, frame.render * 1e3,
                    frame.sprites * 1e3, frame.present * 1e3,
                    frame.total * 1e3);
    }

    /* The account: what the frames actually cost, including the honest
       price of the presentation copy. */
    if (stats.frames) {
        double n = (double)stats.frames;
        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f incl. sprites %.3f, present %.3f)\n",
                    stats.frames, stats.total_sum / n * 1e3,
                    stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
                    stats.sprites_sum / n * 1e3, stats.present_sum / n * 1e3);
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
