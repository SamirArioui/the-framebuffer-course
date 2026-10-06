// platform.h — the platform layer's interface: the engine's only view of the OS.
//
// Lesson 027: the seam. Nothing in this header names an OS type — no display
// connection, no window handle, no X11 anything — and it includes no OS
// headers. A second OS implements the functions below in its own file, and
// no engine file changes when it does (lesson 042 audits that promise).
#ifndef PLATFORM_H
#define PLATFORM_H

namespace platform {

/* What a window is made of is the OS implementation's business. The engine
   holds pointers to it and never looks inside. */
struct Window;

/* Failure is a value: OpenWindow either hands the engine a window or names
   the step that failed. No partial result is ever presented as success. */
enum OpenError {
    OPEN_OK = 0,
    OPEN_NO_DISPLAY, /* the OS's display could not be opened */
    OPEN_NO_WINDOW,  /* the OS refused to create the window */
};

struct WindowResult {
    Window *window;  /* the window, or 0 on failure */
    OpenError error; /* OPEN_OK exactly when window is non-0 */
};

/* Opens a window of exactly the requested size on the OS's display. */
WindowResult OpenWindow(int width, int height);

/* The keys the engine tracks. Plain values — no OS key code ever crosses
   the seam. */
enum Key {
    KEY_UP = 0,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_SPACE,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_COUNT
};

/* The polled input state: true while the key is down at the moment of the
   call. State, not events — the engine asks, it never consumes a stream. */
bool KeyDown(const Window *window, Key key);

/* Reads whatever news the OS has about this window and folds it into the
   platform layer's state. The engine never sees an event object — it polls
   state afterwards. Blocks until there is news or the run is interrupted. */
void PumpEvents(Window *window);

/* True once the user has asked for this window to close. An interrupted run
   counts: every way the run can end reports here, so the engine has exactly
   one ending to get right. */
bool CloseRequested(const Window *window);

/* Presents the engine's framebuffer in the window: the pixels the engine
   wrote are the pixels the window shows. The format is this interface's
   contract, not any OS's — width * height pixels of 4 bytes each (blue,
   green, red, one unused byte), one row after another. Returns false if
   the platform could not carry the pixels at all; when it returns true the
   pixels are on screen — the copy has happened. */
bool Present(Window *window, const unsigned char *pixels, int width,
             int height);

/* Releases everything OpenWindow took from the OS. */
void CloseWindow(Window *window);

} /* namespace platform */

#endif
