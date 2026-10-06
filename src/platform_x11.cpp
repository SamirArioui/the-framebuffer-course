// platform_x11.cpp — the X11 implementation of the platform layer.
//
// Lesson 027: the OS side of the seam. Every OS header in the codebase is
// included here and nowhere else, and every OS type is spelled here and
// nowhere else. An implementation for a second OS would live in its own
// file beside this one and nothing outside would change.
//
// Lesson 028: the event pump. The OS's news arrives here as X events and is
// folded into state the engine polls — the engine never reads an event.

#include "platform.h"

#include <X11/Xlib.h>

namespace platform {

/* What a window is made of on this OS. The definition lives here, where
   Xlib is visible; the engine sees only the forward declaration. X11 has a
   type called Window too — `::Window` is its, `platform::Window` is ours. */
struct Window {
    Display *display;
    ::Window xwindow;
    bool close_requested;
};

/* The OS state for one window, in static storage: no new, no delete — the
   language law of lesson 026 keeps allocation out of the engine, and the
   engine's big buffers will come from memory reservations later in this
   part. The engine only ever holds the pointer. */
static Window window_state;

/* The two atoms of the window-close conversation. The window manager does
   not destroy a window behind its owner's back — it *asks*, by sending a
   ClientMessage whose first word is WM_DELETE_WINDOW. X atoms are names the
   server hands out as integers; asking for them is how you spell them. */
static Atom wm_delete_window;

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

    /* Register the close request as the way to go, and subscribe to the
       window's lifecycle news (map, configure, destroy). */
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
    XSelectInput(display, xwindow, StructureNotifyMask);

    XStoreName(display, xwindow, "the framebuffer engine");
    XMapWindow(display, xwindow);
    XFlush(display);

    window_state.display = display;
    window_state.xwindow = xwindow;
    window_state.close_requested = false;
    return result;
}

/* One piece of news, folded into state. The request and the deed both mean
   the same thing to the engine. */
static void HandleEvent(Window *window, XEvent &event)
{
    if (event.type == ClientMessage &&
        (Atom)event.xclient.data.l[0] == wm_delete_window) {
        window->close_requested = true; /* the polite request */
    } else if (event.type == DestroyNotify) {
        window->close_requested = true; /* the window is already gone */
        window->xwindow = 0;
    }
}

void PumpEvents(Window *window)
{
    if (!window || !window->display)
        return;

    /* Block for the first piece of news, then drain whatever else piled up.
       Blocking is the point: the engine waits here instead of spinning. */
    XEvent event;
    XNextEvent(window->display, &event);
    HandleEvent(window, event);
    while (XPending(window->display)) {
        XNextEvent(window->display, &event);
        HandleEvent(window, event);
    }
}

bool CloseRequested(const Window *window)
{
    return window && window->close_requested;
}

void CloseWindow(Window *window)
{
    if (!window || !window->display)
        return;

    /* xwindow is zero once the OS has already destroyed the window;
       destroying it twice would be an X error. */
    if (window->xwindow)
        XDestroyWindow(window->display, window->xwindow);
    XCloseDisplay(window->display);
    window->display = 0;
    window->xwindow = 0;
}

} /* namespace platform */
