// main.cpp — the engine, born.
//
// Lesson 030: the engine writes its own pixels. The framebuffer is our
// bytes — Part 0's paint intuition at window size — and Present carries
// them through the seam. Still no OS headers here; the language law of
// lesson 026 holds.

#include <cstdio>

#include "framebuffer.h"
#include "frame.h"
#include "platform.h"

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
        /* The error path: nothing was taken that the platform layer did not
           put back, and the failure is reported by name. */
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

    /* The clock's contract, checked before anything depends on it: the
       readings never go backwards, and the finest step between two of them
       is far below a frame. */
    double prev = platform::Now();
    double finest = 1e9;
    int backwards = 0;
    for (int i = 0; i < 100000; ++i) {
        double t = platform::Now();
        if (t < prev)
            ++backwards;
        else if (t > prev && t - prev < finest)
            finest = t - prev;
        prev = t;
    }
    std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
                backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);

    /* The scene: a marker the arrow keys move. The report below is its
       position and the time it moved — the interactive frame makes itself
       observable. */
    Framebuffer *fb = GetFramebuffer();
    double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
    double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
    double started = platform::Now();
    double last = started;
    std::printf("engine: arrow keys move the marker; close the window to stop\n");
    std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);

    /* The frame step: read news, update from polled state, draw, present.
       This is the shape every later part fills in — Part 2 draws into it,
       Part 5 measures it. Every phase is now measured: the frame record is
       data, not guesswork. */
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

        /* Update: a frame reads state — it never handles events. The step
           is speed × elapsed: the marker moves 240 pixels per second no
           matter how often frames happen. */
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

        /* Render: every frame draws the whole scene — clear, then marker. */
        ClearBuffer(*fb, 32, 32, 64);
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

        /* The frame log: one line per record. This is the format Part 2
           grows and Part 5's frame-budget report reads. */
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

    if (platform::CloseRequested(opened.window))
        std::printf("engine: close reported\n");
    platform::CloseWindow(opened.window);
    std::printf("engine: closed\n");
    return exit_code;
}

} /* namespace engine */

int main(void)
{
    return engine::Run();
}
