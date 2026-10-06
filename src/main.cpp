// main.cpp — the engine, born.
//
// Lesson 027: the engine meets the OS through the platform seam. This file
// includes no OS headers and names no OS type — it sees platform.h and
// nothing else. The language law of lesson 026 holds: engine code lives in
// namespace engine, main stays global, and every feature here is one the
// law admits.

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

    std::printf("engine: window %dx%d open — press enter to close\n",
                WINDOW_WIDTH, WINDOW_HEIGHT);

    /* Stand-in for the event pump: hold the window open until enter.
       Lesson 028 replaces exactly this. */
    std::getchar();

    platform::CloseWindow(opened.window);
    std::printf("engine: closed\n");
    return 0;
}

} /* namespace engine */

int main(void)
{
    return engine::Run();
}
