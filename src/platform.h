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

/* Releases everything OpenWindow took from the OS. */
void CloseWindow(Window *window);

} /* namespace platform */

#endif
