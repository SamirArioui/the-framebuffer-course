// platform_x11.cpp — the X11 implementation of the platform layer.
//
// Lesson 027: the OS side of the seam. Every OS header in the codebase is
// included here and nowhere else, and every OS type is spelled here and
// nowhere else. An implementation for a second OS would live in its own
// file beside this one and nothing outside would change.

#include "platform.h"

#include <X11/Xlib.h>

namespace platform {

/* What a window is made of on this OS. The definition lives here, where
   Xlib is visible; the engine sees only the forward declaration. X11 has a
   type called Window too — `::Window` is its, `platform::Window` is ours. */
struct Window {
    Display *display;
    ::Window xwindow;
};

/* The OS state for one window, in static storage: no new, no delete — the
   language law of lesson 026 keeps allocation out of the engine, and the
   engine's big buffers will come from memory reservations later in this
   part. The engine only ever holds the pointer. */
static Window window_state;

WindowResult OpenWindow(int width, int height)
{
    WindowResult result = { &window_state, OPEN_NO_DISPLAY };

    Display *display = XOpenDisplay(0);
    if (!display) {
        result.window = 0;
        return result;
    }

    int screen = DefaultScreen(display);
    ::Window xwindow = XCreateSimpleWindow(display, RootWindow(display, screen),
                                           0, 0, width, height, 0,
                                           BlackPixel(display, screen),
                                           WhitePixel(display, screen));
    if (!xwindow) {
        XCloseDisplay(display);
        result.window = 0;
        result.error = OPEN_NO_WINDOW;
        return result;
    }

    XStoreName(display, xwindow, "the framebuffer engine");
    XMapWindow(display, xwindow);
    XFlush(display);

    window_state.display = display;
    window_state.xwindow = xwindow;
    return result;
}

void CloseWindow(Window *window)
{
    if (!window || !window->display)
        return;

    XDestroyWindow(window->display, window->xwindow);
    XCloseDisplay(window->display);
    window->display = 0;
    window->xwindow = 0;
}

} /* namespace platform */
