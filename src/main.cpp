// main.cpp — the engine, born.
//
// Lesson 028: the engine stays alive by reading the OS's news through the
// seam. Still no OS headers, still no OS types — the pump is one more
// platform function and the engine polls what it leaves behind. The
// language law of lesson 026 holds.

#include <cstdio>

#include "platform.h"

namespace engine {

constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;

int Run(void)
{
    platform::WindowResult opened =
        platform::OpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!opened.window) {
        std::fprintf(stderr, "engine: no window (platform error %d)\n",
                     opened.error);
        return 1;
    }

    std::printf("engine: window %dx%d open — waiting for news\n",
                WINDOW_WIDTH, WINDOW_HEIGHT);

    /* The event pump: read news, fold it into state, react to state,
       repeat. This loop is what keeps the window alive. */
    while (!platform::CloseRequested(opened.window))
        platform::PumpEvents(opened.window);

    std::printf("engine: close reported\n");
    platform::CloseWindow(opened.window);
    std::printf("engine: closed\n");
    return 0;
}

} /* namespace engine */

int main(void)
{
    return engine::Run();
}
