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
#include <X11/Xutil.h>

#include <cstdio>
#include <poll.h>
#include <signal.h>

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

/* The interrupt: Ctrl+C is an exit too, and the run owes it the same clean
   close as any other. The handler does the only thing a signal handler may
   safely do here — set a flag (lesson 020's rule). */
static volatile sig_atomic_t interrupted;

static void OnInterrupt(int)
{
    interrupted = 1;
}

/* X errors are values in this layer, not process death. The default X
   error handler would end the run on the spot — but a window can die while
   a present is in flight, and that is news, not a crash. The handler
   records what happened; Present decides what it means. */
static int last_xerror;
static unsigned long last_xerror_resource;

static int OnXError(Display *display, XErrorEvent *event)
{
    (void)display;
    last_xerror = event->error_code;
    last_xerror_resource = event->resourceid;
    return 0;
}

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
       window's lifecycle news (map, configure, destroy) — plus Expose, the
       "your pixels are gone" news. The engine handles Expose by doing the
       only thing that repairs a window: presenting again. */
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
    XSelectInput(display, xwindow, StructureNotifyMask | ExposureMask);

    XStoreName(display, xwindow, "the framebuffer engine");
    XMapWindow(display, xwindow);
    XFlush(display);

    window_state.display = display;
    window_state.xwindow = xwindow;
    window_state.close_requested = false;

    /* The interrupt is part of the window's take: from here on, Ctrl+C is
       news like any other — and X errors are recorded, not fatal. */
    interrupted = 0;
    signal(SIGINT, OnInterrupt);
    XSetErrorHandler(OnXError);
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

    /* Wait for news where a signal can wake us. Lesson 028 slept inside
       XNextEvent, where Ctrl+C could not reach it; poll on the OS
       connection returns when there is news *or* when a signal interrupts
       it — then the flag below is folded in like any other news. */
    struct pollfd pfd = { ConnectionNumber(window->display), POLLIN, 0 };
    poll(&pfd, 1, -1);

    if (interrupted)
        window->close_requested = true;

    /* Drain whatever piled up: one blocking wait, then the whole batch. */
    while (XPending(window->display)) {
        XEvent event;
        XNextEvent(window->display, &event);
        HandleEvent(window, event);
    }
}

bool CloseRequested(const Window *window)
{
    return window && window->close_requested;
}

bool Present(Window *window, const unsigned char *pixels, int width,
             int height)
{
    if (!window || !window->display || !window->xwindow)
        return false; /* nothing to present to (the OS may have destroyed it) */

    int screen = DefaultScreen(window->display);

    /* An XImage is a *view*: XCreateImage wraps the engine's bytes without
       copying them — the description of a pixel buffer, not the buffer.
       It carries our bytes exactly as the seam's contract defines them. */
    XImage *image = XCreateImage(window->display,
                                 DefaultVisual(window->display, screen),
                                 DefaultDepth(window->display, screen),
                                 ZPixmap, 0, (char *)pixels,
                                 width, height, 32, width * 4);
    if (!image)
        return false;

    /* XPutImage is where the copy happens — our bytes to the server. Its
       cost is real; lesson 036 measures it. (Its return value is not a
       status in practice — this call either copies or raises an X error,
       which is the OS error handler's territory.) */
    XPutImage(window->display, window->xwindow,
              DefaultGC(window->display, screen),
              image, 0, 0, 0, 0, width, height);

    /* The image struct is ours to destroy; the bytes under it are the
       engine's, so they are detached before destruction (the same rule as
       lesson 004's ownership drills). */
    image->data = 0;
    XDestroyImage(image);

    /* The contract: when Present returns, the pixels are on screen. XFlush
       would only send the copy; XSync waits for the server to have done
       it. */
    last_xerror = 0;
    XSync(window->display, False);

    if (last_xerror) {
        /* An error on our own window means it died mid-copy — the same
           news as DestroyNotify, the deed: report it and stop presenting. */
        if ((last_xerror == BadDrawable || last_xerror == BadWindow) &&
            last_xerror_resource == (unsigned long)window->xwindow) {
            window->close_requested = true;
            window->xwindow = 0;
        }
        return false;
    }
    return true;
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
    signal(SIGINT, SIG_DFL); /* the handler is taken and released like the rest */
    window->display = 0;
    window->xwindow = 0;
}

} /* namespace platform */
