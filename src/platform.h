// platform.h — the platform layer's interface: the engine's only view of the OS.
//
// Lesson 027: the seam. Nothing in this header names an OS type — no display
// connection, no window handle, no X11 anything — and it includes no OS
// headers. A second OS implements the functions below in its own file, and
// no engine file changes when it does (lesson 042 audits that promise).
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h> /* size_t */

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

/* True if the key went down since the last time that key was polled this
   way. A press that ended before the poll is not lost; the latch clears
   when the engine has seen it. */
bool KeyPressed(Window *window, Key key);

/* True while the window has keyboard focus. Keys held when focus is lost
   are dropped by the platform layer — no release event will ever arrive
   for them. */
bool HasFocus(const Window *window);

/* The platform clock: seconds since an arbitrary starting point. It is
   monotonic — it never goes backwards, and the wall clock cannot move it —
   and its resolution is fine enough to measure one frame. This is the
   clock Part 2's frame timing and Part 5's frame-budget report stand on. */
double Now(void);

/* The OS's memory page: the unit every mapping is counted in. */
size_t PageSize(void);

/* A memory reservation: whole pages of OS memory, zeroed, owned by the
   engine until it releases them. This is where engine buffers come from
   (lesson 040) — not from an allocator. */
enum MemoryError {
    MEMORY_OK = 0,
    MEMORY_NO_MEMORY, /* the OS refused the reservation */
};

struct Reservation {
    unsigned char *bytes; /* the reserved bytes, or 0 */
    size_t size;          /* whole pages */
    MemoryError error;
};

/* Reserves at least this many bytes from the OS, rounded to whole pages. */
Reservation ReserveMemory(size_t bytes);

/* Gives the reservation back to the OS. */
void ReleaseMemory(Reservation &reservation);

/* A file's complete bytes — or a typed failure. Never partial data
   presented as success. */
enum FileError {
    FILE_OK = 0,
    FILE_NOT_FOUND,   /* nothing is there */
    FILE_UNREADABLE,  /* something is there and the OS says no */
    FILE_UNWRITABLE,  /* the OS refused to take the bytes */
};

struct FileData {
    const unsigned char *data; /* the file's complete bytes, or 0 */
    size_t size;
    FileError error;
};

/* Reads a whole file from the OS. On success the bytes are the file —
   all of it — and they belong to the caller: give them back with
   ReleaseFile. */
FileData ReadFile(const char *path);

/* Gives the file's bytes back to the OS. */
void ReleaseFile(FileData &file);

/* Writes a whole file to the OS: the file is created, or replaced if it
   already exists, with exactly the bytes given — or the write is a typed
   failure. */
FileError WriteFile(const char *path, const unsigned char *data, size_t size);

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
