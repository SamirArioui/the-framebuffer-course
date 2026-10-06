// platform_x11.cpp — the X11 implementation of the platform layer.
//
// Lesson 027: the OS side of the seam. Every OS header in the codebase is
// included here and nowhere else, and every OS type is spelled here and
// nowhere else. An implementation for a second OS would live in its own
// file beside this one and nothing outside would change.
//
// Lesson 028: the event pump. The OS's news arrives here as X events and is
// folded into state the engine polls — the engine never reads an event.
//
// Lesson 035: the platform clock. POSIX, not ISO C — clock_gettime is the
// OS's clock interface (the one lesson 020 taught inside snek, now behind
// the seam), so the feature-test macro goes before the includes.
//
// Lesson 037: whole-file reads. File I/O is OS surface too — POSIX here,
// a second OS's own calls there. Everything in this file is one
// implementation behind the seam.
//
// Lesson 060: the paced wait. The wait for news is no longer unbounded —
// with an audio output open it is bounded by how long the queued samples
// will last, so the run wakes to feed the device on schedule. The bound is
// platform state, asked for through platform_internal.h.
#define _POSIX_C_SOURCE 200809L

#include "platform.h"
#include "platform_internal.h"

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

namespace platform {

/* What a window is made of on this OS. The definition lives here, where
   Xlib is visible; the engine sees only the forward declaration. X11 has a
   type called Window too — `::Window` is its, `platform::Window` is ours. */
struct Window {
    Display *display;
    ::Window xwindow;
    bool close_requested;
    bool keys[KEY_COUNT];
    bool pressed[KEY_COUNT]; /* latched: went down since last observed */
    bool focused;
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
       "your pixels are gone" news, and the keyboard. The engine handles
       Expose by doing the only thing that repairs a window: presenting
       again. */
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
    XSelectInput(display, xwindow,
                 StructureNotifyMask | ExposureMask | KeyPressMask |
                     KeyReleaseMask | FocusChangeMask);

    /* Auto-repeat would otherwise look like release-then-press every
       repeat: a held key would flicker in polled state. Detectable
       auto-repeat makes repeats arrive as presses only, so "held" stays
       held. */
    XkbSetDetectableAutoRepeat(display, True, 0);

    XStoreName(display, xwindow, "the framebuffer engine");
    XMapWindow(display, xwindow);
    XFlush(display);

    window_state.display = display;
    window_state.xwindow = xwindow;
    window_state.close_requested = false;
    for (int i = 0; i < KEY_COUNT; ++i) {
        window_state.keys[i] = false;
        window_state.pressed[i] = false;
    }
    window_state.focused = false;

    /* The interrupt is part of the window's take: from here on, Ctrl+C is
       news like any other — and X errors are recorded, not fatal. */
    interrupted = 0;
    signal(SIGINT, OnInterrupt);
    XSetErrorHandler(OnXError);
    return result;
}

/* Which of our keys an OS key event is about, or -1 for keys we do not
   track. The translation from OS key codes to the seam's Key lives here
   and nowhere else. */
static int KeyIndex(KeySym sym)
{
    switch (sym) {
    case XK_Up:     return KEY_UP;
    case XK_Down:   return KEY_DOWN;
    case XK_Left:   return KEY_LEFT;
    case XK_Right:  return KEY_RIGHT;
    case XK_space:  return KEY_SPACE;
    case XK_Return: return KEY_ENTER;
    case XK_Escape: return KEY_ESCAPE;
    default:        return -1;
    }
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
    } else if (event.type == KeyPress || event.type == KeyRelease) {
        int key = KeyIndex(XLookupKeysym(&event.xkey, 0));
        if (key >= 0) {
            if (event.type == KeyPress) {
                /* The latch lights on the edge — a key going from up to
                   down. Auto-repeat presses (a held key) arrive as presses
                   too, but a held key did not go down again. */
                if (!window->keys[key])
                    window->pressed[key] = true;
                window->keys[key] = true;
            } else {
                window->keys[key] = false;
            }
        }
    } else if (event.type == FocusIn) {
        window->focused = true;
    } else if (event.type == FocusOut) {
        /* Keys held while focus left will never send their release here —
           drop them or they stay down forever. */
        window->focused = false;
        for (int i = 0; i < KEY_COUNT; ++i)
            window->keys[i] = false;
    }
}

bool KeyDown(const Window *window, Key key)
{
    return window && key >= 0 && key < KEY_COUNT && window->keys[key];
}

bool KeyPressed(Window *window, Key key)
{
    if (!window || key < 0 || key >= KEY_COUNT || !window->pressed[key])
        return false;
    window->pressed[key] = false; /* observed; the latch clears */
    return true;
}

bool HasFocus(const Window *window)
{
    return window && window->focused;
}

double Now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

size_t PageSize(void)
{
    return (size_t)sysconf(_SC_PAGESIZE);
}

/* Reservations are anonymous mappings (lesson 039's vocabulary): whole
   pages of virtual memory, zeroed by the OS, with no file behind them.
   Physical frames arrive only when the bytes are touched — the demand-zero
   behavior the deep dive described. */
Reservation ReserveMemory(size_t bytes)
{
    Reservation reservation = { 0, 0, MEMORY_NO_MEMORY };

    size_t page = PageSize();
    size_t rounded = (bytes + page - 1) / page * page;
    if (rounded == 0)
        rounded = page;

    void *mapping = mmap(0, rounded, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapping == MAP_FAILED)
        return reservation;

    reservation.bytes = (unsigned char *)mapping;
    reservation.size = rounded;
    reservation.error = MEMORY_OK;
    return reservation;
}

void ReleaseMemory(Reservation &reservation)
{
    if (reservation.bytes)
        munmap(reservation.bytes, reservation.size);
    reservation.bytes = 0;
    reservation.size = 0;
}

/* File I/O is the OS side too — POSIX here, Win32's own calls in a second
   implementation. The bytes the OS reads for us live in memory the OS
   gives us (its allocator) and leave through ReleaseFile. */
FileData ReadFile(const char *path)
{
    FileData file = { 0, 0, FILE_UNREADABLE };

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        file.error = (errno == ENOENT) ? FILE_NOT_FOUND : FILE_UNREADABLE;
        return file;
    }

    /* Whole file means whole file: the size is known before the first
       byte, and a read that ends early is a failure, not a smaller file. */
    struct stat st;
    if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)) {
        close(fd);
        return file; /* still FILE_UNREADABLE */
    }

    size_t capacity = (size_t)st.st_size;
    unsigned char *bytes =
        (unsigned char *)malloc(capacity ? capacity : 1);
    if (!bytes) {
        close(fd);
        return file;
    }

    size_t total = 0;
    while (total < capacity) {
        ssize_t n = read(fd, bytes + total, capacity - total);
        if (n < 0) {
            free(bytes);
            close(fd);
            return file;
        }
        if (n == 0)
            break; /* the file ended early — checked below */
        total += (size_t)n;
    }

    /* One byte past what the size promised must find nothing, or the file
       changed under the read — and a moving file is not a whole file. */
    unsigned char extra;
    if (total != capacity || read(fd, &extra, 1) != 0) {
        free(bytes);
        close(fd);
        return file;
    }

    close(fd);
    file.data = bytes;
    file.size = total;
    file.error = FILE_OK;
    return file;
}

void ReleaseFile(FileData &file)
{
    free((void *)file.data);
    file.data = 0;
    file.size = 0;
}

FileError WriteFile(const char *path, const unsigned char *data, size_t size)
{
    /* Created, or replaced if it exists — exactly these bytes or nothing
       the caller can mistake for success. */
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
        return FILE_UNWRITABLE;

    /* write() may take fewer bytes than offered — a short write is not a
       finished file. Keep going until they are all in. */
    size_t total = 0;
    while (total < size) {
        ssize_t n = write(fd, data + total, size - total);
        if (n < 0) {
            close(fd);
            return FILE_UNWRITABLE;
        }
        total += (size_t)n;
    }

    if (close(fd) < 0) /* the OS can still refuse at the very end */
        return FILE_UNWRITABLE;
    return FILE_OK;
}

void PumpEvents(Window *window)
{
    if (!window || !window->display)
        return;

    /* Wait for news where a signal can wake us. Lesson 028 slept inside
       XNextEvent, where Ctrl+C could not reach it; poll on the OS
       connection returns when there is news *or* when a signal interrupts
       it — then the flag below is folded in like any other news.

       Lesson 060: the wait is bounded. An open audio output needs its next
       buffer before long, so the wait may not outlast it — and when it
       ends early it ends because the output needs feeding, not because
       anything happened. The ceiling to whole milliseconds keeps the wait
       from ending before the buffer is due; AudioWaitSeconds is negative
       with no output to feed, and the wait is then the old unbounded one. */
    double wait = AudioWaitSeconds();
    int timeout_ms = wait < 0.0 ? -1 : (int)(wait * 1000.0 + 0.999);
    struct pollfd pfd = { ConnectionNumber(window->display), POLLIN, 0 };
    poll(&pfd, 1, timeout_ms);

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
