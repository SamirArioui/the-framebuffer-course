// main.cpp — the engine, born.
//
// Lesson 030: the engine writes its own pixels. The framebuffer is our
// bytes — Part 0's paint intuition at window size — and Present carries
// them through the seam. Still no OS headers here; the language law of
// lesson 026 holds.

#include <cstdio>

#include "framebuffer.h"
#include "platform.h"

namespace engine {

/* The marker: one square the arrow keys move. Its position is whole
   pixels and its speed is pixels-per-frame — lesson 035's clock turns
   that into pixels-per-second. */
constexpr int MARKER_SIZE = 24;
constexpr int MARKER_STEP = 8;

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

    /* The scene: a marker the arrow keys move. The report below is its
       position — the interactive frame makes itself observable. */
    Framebuffer *fb = GetFramebuffer();
    int marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2;
    int marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2;
    std::printf("engine: arrow keys move the marker; close the window to stop\n");
    std::printf("engine: marker at %d,%d\n", marker_x, marker_y);

    /* The frame step: read news, update from polled state, draw, present.
       This is the shape every later part fills in — Part 2 draws into it,
       Part 5 measures it. */
    int exit_code = 0;
    while (!platform::CloseRequested(opened.window)) {
        platform::PumpEvents(opened.window);
        if (platform::CloseRequested(opened.window))
            break;

        /* Update: a frame reads state — it never handles events. */
        int old_x = marker_x, old_y = marker_y;
        if (platform::KeyDown(opened.window, platform::KEY_LEFT))
            marker_x -= MARKER_STEP;
        if (platform::KeyDown(opened.window, platform::KEY_RIGHT))
            marker_x += MARKER_STEP;
        if (platform::KeyDown(opened.window, platform::KEY_UP))
            marker_y -= MARKER_STEP;
        if (platform::KeyDown(opened.window, platform::KEY_DOWN))
            marker_y += MARKER_STEP;

        /* The marker stays on screen — lesson 015's fold at frame scale. */
        if (marker_x < 0)
            marker_x = 0;
        if (marker_x > FRAME_WIDTH - MARKER_SIZE)
            marker_x = FRAME_WIDTH - MARKER_SIZE;
        if (marker_y < 0)
            marker_y = 0;
        if (marker_y > FRAME_HEIGHT - MARKER_SIZE)
            marker_y = FRAME_HEIGHT - MARKER_SIZE;

        if (marker_x != old_x || marker_y != old_y)
            std::printf("engine: marker at %d,%d\n", marker_x, marker_y);

        /* Render: every frame draws the whole scene — clear, then marker. */
        ClearBuffer(*fb, 32, 32, 64);
        DrawMarker(*fb, marker_x, marker_y);

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
