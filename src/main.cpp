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

/* The seam's keys, by name — for the report below. */
static const char *const key_names[platform::KEY_COUNT] = {
    "up", "down", "left", "right", "space", "enter", "escape",
};

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

    /* Paint: clear, then pixels — the two instincts Part 0's paint taught,
       now onto the engine's own buffer. */
    Framebuffer *fb = GetFramebuffer();
    ClearBuffer(*fb, 32, 32, 64);
    PutPixel(*fb, 0, 0, 255, 0, 0);
    PutPixel(*fb, 639, 479, 0, 255, 0);
    PutPixel(*fb, 700, 100, 0, 0, 255); /* out of bounds: dropped */

    /* The byte-level report: what is actually in the buffer. */
    unsigned char r, g, b;
    std::printf("engine: framebuffer %dx%d, %d bytes, stride %d\n",
                fb->width, fb->height, fb->width * fb->height * 4,
                fb->width * 4);
    GetPixel(*fb, 0, 0, r, g, b);
    std::printf("engine: pixel (0,0) = %d %d %d\n", r, g, b);
    GetPixel(*fb, 639, 479, r, g, b);
    std::printf("engine: pixel (639,479) = %d %d %d\n", r, g, b);
    GetPixel(*fb, 60, 101, r, g, b);
    std::printf("engine: pixel (60,101) = %d %d %d\n", r, g, b);

    /* First light: the bytes go to the window through the seam. */
    if (!platform::Present(opened.window, fb->pixels, fb->width, fb->height)) {
        std::fprintf(stderr, "engine: presentation failed\n");
        platform::CloseWindow(opened.window);
        return 1;
    }
    std::printf("engine: presented\n");

    /* The frame step: read news, react, present our pixels, repeat.
       Presentation is not an event — it is the engine's answer to every
       event: the pixels the engine wrote are the pixels the window shows,
       and re-presenting is what repairs the window when the OS damaged it.
       React first: if the news was "the window is gone", there is nothing
       left to present to. */
    int exit_code = 0;
    bool had_focus = platform::HasFocus(opened.window);
    while (!platform::CloseRequested(opened.window)) {
        platform::PumpEvents(opened.window);
        if (platform::CloseRequested(opened.window))
            break;

        /* The polled state: what is down right now, as of this poll. */
        std::printf("engine: polled:");
        bool any = false;
        for (int k = 0; k < platform::KEY_COUNT; ++k) {
            if (platform::KeyDown(opened.window, (platform::Key)k)) {
                std::printf(" %s", key_names[k]);
                any = true;
            }
        }
        std::printf(any ? "\n" : " -\n");

        /* The latches: presses that ended before this poll are not lost. */
        for (int k = 0; k < platform::KEY_COUNT; ++k)
            if (platform::KeyPressed(opened.window, (platform::Key)k))
                std::printf("engine: pressed %s\n", key_names[k]);

        /* Focus: reported when it changes. */
        bool focus = platform::HasFocus(opened.window);
        if (focus != had_focus) {
            std::printf("engine: focus %s\n", focus ? "gained" : "lost");
            had_focus = focus;
        }

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
