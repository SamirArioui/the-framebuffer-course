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

/* The marker: one square the arrow keys move. Its speed is the engine's —
   pixels per second — and the clock's dt turns it into a per-frame step. */
constexpr int MARKER_SIZE = 24;
constexpr double MARKER_SPEED = 240.0; /* pixels per second */

static void DrawMarker(Framebuffer &fb, int x, int y)
{
    for (int j = 0; j < MARKER_SIZE; ++j)
        for (int i = 0; i < MARKER_SIZE; ++i)
            PutPixel(fb, x + i, y + j, 240, 220, 80);
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
       engine allocates lives in here and is released together. */
    Arena arena;
    ArenaInit(arena, 4 * 1024 * 1024);
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

    double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
    double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
    double started = platform::Now();
    double last = started;

    std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
    std::printf("engine: arrow keys move the marker; close the window to stop\n");
    std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);

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

        int old_x = (int)marker_x, old_y = (int)marker_y;
        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
            marker_x -= MARKER_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
            marker_x += MARKER_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_UP))
            marker_y -= MARKER_SPEED * dt;
        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
            marker_y += MARKER_SPEED * dt;

        /* The marker stays on screen — lesson 015's fold at frame scale. */
        if (marker_x < 0)
            marker_x = 0;
        if (marker_x > FRAME_WIDTH - MARKER_SIZE)
            marker_x = FRAME_WIDTH - MARKER_SIZE;
        if (marker_y < 0)
            marker_y = 0;
        if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
            marker_y = FRAME_HEIGHT - MARKER_SIZE;

        frame.update = platform::Now() - t0;
        double t1 = platform::Now();

        if ((int)marker_x != old_x || (int)marker_y != old_y)
            std::printf("engine: marker at %d,%d (t=%.3f)\n", (int)marker_x,
                        (int)marker_y, platform::Now() - started);

        /* Render: every frame draws the whole scene — clear, then the
           sprite through the one blit. */
        ClearBuffer(*fb, 32, 32, 64);
        BlitSprite(*fb, sprite, 32, 32);
        DrawMarker(*fb, (int)marker_x, (int)marker_y);

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

        /* The frame log: one line per record — the format Part 2 grows. */
        std::printf("frame %ld: update %.3f ms, render %.3f ms, present %.3f ms, total %.3f ms\n",
                    frame.number, frame.update * 1e3, frame.render * 1e3,
                    frame.present * 1e3, frame.total * 1e3);
    }

    /* The account: what the frames actually cost, including the honest
       price of the presentation copy. */
    if (stats.frames) {
        double n = (double)stats.frames;
        std::printf("engine: %ld frames — avg %.3f ms (update %.3f, render %.3f, present %.3f)\n",
                    stats.frames, stats.total_sum / n * 1e3,
                    stats.update_sum / n * 1e3, stats.render_sum / n * 1e3,
                    stats.present_sum / n * 1e3);
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
