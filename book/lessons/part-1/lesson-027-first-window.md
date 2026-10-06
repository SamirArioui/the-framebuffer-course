# Lesson 027 — the platform seam and the first X11 window

{{#include ../../stability-horizon.md}}

## Prose

The engine is born; today it gets a window. Not by reaching into the OS — by
asking the platform layer for one. This lesson builds the **seam**: the one
place in the codebase where an OS is allowed to speak, and the OS-idiom-free
interface that hides which one it is. On this main line the implementation is
X11, the window system of Linux and every Unix worth the name; the engine
will never know that.

### The seam

The promise the platform layer makes is old — the game is built "against the
OS behind a platform layer we write ourselves" — and this is where it is
kept. Two files with opposite jobs appear today:

- **`src/platform.h`** — the interface. It names functions, plain types, and
  a failure enum. It includes no OS header and mentions no OS type. Engine
  code includes this file and no other view of the OS exists for it.
- **`src/platform_x11.cpp`** — one implementation. Every OS header in the
  codebase is included here, every OS type is spelled here, and the X11 calls
  live their whole lives inside this file.

A second OS — say a Win32 epilogue — means a second implementation file and
one line of build configuration. No engine file changes. That claim is
checkable, not aspirational; exercise 2 checks it with `nm`, and lesson 042
audits the whole seam against exactly this test.

Why a plain header of functions and not an interface class with a vtable?
Because lesson 025 already taught you to see what a vtable compiles down to,
and this seam does not need one: there is one implementation in the program
at a time, the choice is made by which file the build compiles, and a
function call is all the dispatch there ever is. The law says a feature is
admitted if its cost is explainable — the cheapest explainable dispatch is no
dispatch at all.

### The interface, symbol by symbol

```c++
namespace platform {

struct Window;   /* opaque */

enum OpenError {
    OPEN_OK = 0,
    OPEN_NO_DISPLAY,
    OPEN_NO_WINDOW,
};

struct WindowResult {
    Window *window;
    OpenError error;
};

WindowResult OpenWindow(int width, int height);
void CloseWindow(Window *window);

}
```

**`struct Window`** is declared and never defined in the header — an
*incomplete type*. The engine can hold `Window *`, pass it around, test it
against zero; it cannot look inside, because there is nothing to see from out
here. What a window is made of is the implementation's business. This is
lesson 010's `void *` pain answered properly: instead of erasing the type, we
declared it and withheld its definition.

**`OpenError` and `WindowResult`** make failure a value. `OpenWindow` either
hands the engine a window or names the step that failed — the OS's display
could not be opened (`OPEN_NO_DISPLAY`), or the OS refused to create the
window (`OPEN_NO_WINDOW`). There is no third option where a half-open window
is presented as success; `WindowResult`'s two fields agree by construction
(`error` is `OPEN_OK` exactly when `window` is non-zero). Lesson 037 grows
this same shape for files: complete bytes or a typed failure, never partial
data dressed as success.

**`OpenWindow` / `CloseWindow`** are the whole window lifecycle. Open takes
the size the engine wants — *exactly* that size, because the engine's pixels
will be laid out for it — and Close releases everything Open took from the
OS. Nothing else is exposed. No show/hide, no resize, no title, no events —
not because they will never come, but because the interface grows one
contract at a time and every function in it is a promise a second OS must
keep.

### The OS side: display and window

`platform_x11.cpp` is where X11 finally speaks. Two OS concepts run the
show:

- **The display** — `XOpenDisplay` opens a *connection* to the X server that
  owns the screen. X11 is a client/server system: your program is a client,
  the screen belongs to a server process, and almost every X call is really a
  request pushed across that connection. `XOpenDisplay(0)` connects to the
  display named by the environment (on a real desktop, that is just there; on
  a headless check, it is `DISPLAY=:99` under Xvfb).
- **The window** — `XCreateSimpleWindow` asks the server to create a
  rectangle of exactly the requested size, with a background color and no
  border. What comes back is an **XID**: an integer the server made up for
  the window. The window is server-side state; our program holds its id.

Then two more requests: `XMapWindow` makes the window visible (creating it
does not show it), and `XFlush` pushes the queued requests across the
connection to the server — X calls accumulate as requests; nothing happens
on screen until they are sent.

One naming trap, defused by the language law's namespaces: X11 has a type
called `Window` too (it is the XID typedef). Inside `namespace platform`, the
OS type is spelled `::Window` — the global namespace's — and ours is
`platform::Window`:

```c++
struct Window {
    Display *display;
    ::Window xwindow;
};
```

The definition of `platform::Window` lives in this file, next to the Xlib
include — the header's forward declaration is all the engine ever sees.

And notice what is *not* in this file: no `new`, no `delete`. The one window
worth of OS state sits in static storage (`window_state`) and the engine only
ever holds the pointer to it. The language law of lesson 026 keeps allocation
out of the engine, and it will stay out: the engine's large buffers come from
OS-level memory reservations later in this part, not from an allocator the
lesson would have to un-teach later.

### The engine side

`main.cpp` is engine code and it looks like it: it includes `<cstdio>` and
`platform.h`, and it calls `platform::OpenWindow(WINDOW_WIDTH,
WINDOW_HEIGHT)`. If the result's `window` is zero, it reports the typed
failure and exits — the error paths around this grow in lesson 029. If a
window came back, the engine reports it, holds it open, and closes it:

```
$ ./build.sh
build: compiling 2 source(s) from src/
  CC  src/main.cpp
  CC  src/platform_x11.cpp
  LD  build/game
build: OK (2 source(s) compiled -> build/game)
$ ./build/game
engine: window 640x480 open — press enter to close
```

On a real desktop a 640×480 window titled *the framebuffer engine* appears
and waits. The wait itself is a stand-in — `std::getchar()` blocking on the
keyboard — because the thing that *should* hold a window open is an event
pump, and that is exactly what lesson 028 builds. This is a deliberate
temporary state: the seam is done, the liveness is borrowed.

### The build line

The one thing `build.sh` had to learn: the link line now carries the OS
library the platform layer wraps, `-lX11`. The compiler flags are unchanged
from lesson 026 — the seam is made of plain C++; it is only the *link* that
needs Xlib's code. A Win32 implementation would change this one default to
the Win32 libraries and nothing else about the build.

### Checking the window headlessly

The window's behavior is observable without a desktop. On a machine with no
display (this book's authoring machine is one), Xvfb provides a virtual
screen and `xdotool` queries it the way a window manager would:

```
$ Xvfb :99 -screen 0 800x600x24 &
$ (sleep 2; echo) | DISPLAY=:99 ./build/game &
engine: window 640x480 open — press enter to close
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
2097153
$ DISPLAY=:99 xdotool getwindowgeometry 2097153
Window 2097153
  Position: 0,0 (screen: 0)
  Geometry: 640x480
engine: closed
```

A window of exactly the size the engine requested opened, carried the title
the implementation gave it, and closed cleanly when the stand-in pump let it
go. The machine is the same either way; only the screen is virtual.

## Code step

One change for this lesson: the seam is born — `platform.h` as the
OS-idiom-free interface, `platform_x11.cpp` as its first implementation,
`main.cpp` grown into engine code that opens its window through the seam, and
`build.sh`'s link line taught the one OS library. Its end state is tagged
`lesson-027`.

```diff
diff --git a/build.sh b/build.sh
index 2674470..f852f8c 100755
--- a/build.sh
+++ b/build.sh
@@ -13,7 +13,8 @@
 # Environment overrides:
 #   CC, CFLAGS       compiler and flags for C sources
 #   CXX, CXXFLAGS    compiler and flags for C++ sources
-#   LDFLAGS          extra link flags
+#   LDFLAGS          extra link flags (default: the OS library the platform
+#                    layer wraps — -lX11 on the Linux/X11 main line)
 #   BUILD_DIR        output directory (default: build)
 
 set -euo pipefail
@@ -24,7 +25,7 @@ CC="${CC:-gcc}"
 CXX="${CXX:-g++}"
 CFLAGS="${CFLAGS:--std=c11 -O0 -g -Wall -Wextra}"
 CXXFLAGS="${CXXFLAGS:--std=c++17 -O0 -g -Wall -Wextra}"
-LDFLAGS="${LDFLAGS:-}"
+LDFLAGS="${LDFLAGS:--lX11}"
 BUILD_DIR="${BUILD_DIR:-build}"
 OBJ_DIR="$BUILD_DIR/obj"
 BIN="$BUILD_DIR/game"
diff --git a/src/main.cpp b/src/main.cpp
index 7ecbde3..106b3b0 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,13 +1,45 @@
 // main.cpp — the engine, born.
 //
-// Lesson 026: the codebase is born from a blank file. This file obeys the
-// language law documented in the lesson: the admitted C++ subset of
-// lesson 025 — references, overloading, namespaces, constexpr, and classes
-// with vtables — and nothing else. `main` is the one global function the
-// runtime looks up; every engine symbol that follows lives in
-// `namespace engine`.
+// Lesson 027: the engine meets the OS through the platform seam. This file
+// includes no OS headers and names no OS type — it sees platform.h and
+// nothing else. The language law of lesson 026 holds: engine code lives in
+// namespace engine, main stays global, and every feature here is one the
+// law admits.
 
-int main(void)
+#include <cstdio>
+
+#include "platform.h"
+
+namespace engine {
+
+constexpr int WINDOW_WIDTH = 640;
+constexpr int WINDOW_HEIGHT = 480;
+
+int Run(void)
 {
+    platform::WindowResult opened =
+        platform::OpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
+    if (!opened.window) {
+        std::fprintf(stderr, "engine: no window (platform error %d)\n",
+                     opened.error);
+        return 1;
+    }
+
+    std::printf("engine: window %dx%d open — press enter to close\n",
+                WINDOW_WIDTH, WINDOW_HEIGHT);
+
+    /* Stand-in for the event pump: hold the window open until enter.
+       Lesson 028 replaces exactly this. */
+    std::getchar();
+
+    platform::CloseWindow(opened.window);
+    std::printf("engine: closed\n");
     return 0;
 }
+
+} /* namespace engine */
+
+int main(void)
+{
+    return engine::Run();
+}
diff --git a/src/platform.h b/src/platform.h
new file mode 100644
index 0000000..361251c
--- /dev/null
+++ b/src/platform.h
@@ -0,0 +1,37 @@
+// platform.h — the platform layer's interface: the engine's only view of the OS.
+//
+// Lesson 027: the seam. Nothing in this header names an OS type — no display
+// connection, no window handle, no X11 anything — and it includes no OS
+// headers. A second OS implements the functions below in its own file, and
+// no engine file changes when it does (lesson 042 audits that promise).
+#ifndef PLATFORM_H
+#define PLATFORM_H
+
+namespace platform {
+
+/* What a window is made of is the OS implementation's business. The engine
+   holds pointers to it and never looks inside. */
+struct Window;
+
+/* Failure is a value: OpenWindow either hands the engine a window or names
+   the step that failed. No partial result is ever presented as success. */
+enum OpenError {
+    OPEN_OK = 0,
+    OPEN_NO_DISPLAY, /* the OS's display could not be opened */
+    OPEN_NO_WINDOW,  /* the OS refused to create the window */
+};
+
+struct WindowResult {
+    Window *window;  /* the window, or 0 on failure */
+    OpenError error; /* OPEN_OK exactly when window is non-0 */
+};
+
+/* Opens a window of exactly the requested size on the OS's display. */
+WindowResult OpenWindow(int width, int height);
+
+/* Releases everything OpenWindow took from the OS. */
+void CloseWindow(Window *window);
+
+} /* namespace platform */
+
+#endif
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
new file mode 100644
index 0000000..203b181
--- /dev/null
+++ b/src/platform_x11.cpp
@@ -0,0 +1,70 @@
+// platform_x11.cpp — the X11 implementation of the platform layer.
+//
+// Lesson 027: the OS side of the seam. Every OS header in the codebase is
+// included here and nowhere else, and every OS type is spelled here and
+// nowhere else. An implementation for a second OS would live in its own
+// file beside this one and nothing outside would change.
+
+#include "platform.h"
+
+#include <X11/Xlib.h>
+
+namespace platform {
+
+/* What a window is made of on this OS. The definition lives here, where
+   Xlib is visible; the engine sees only the forward declaration. X11 has a
+   type called Window too — `::Window` is its, `platform::Window` is ours. */
+struct Window {
+    Display *display;
+    ::Window xwindow;
+};
+
+/* The OS state for one window, in static storage: no new, no delete — the
+   language law of lesson 026 keeps allocation out of the engine, and the
+   engine's big buffers will come from memory reservations later in this
+   part. The engine only ever holds the pointer. */
+static Window window_state;
+
+WindowResult OpenWindow(int width, int height)
+{
+    WindowResult result = { &window_state, OPEN_NO_DISPLAY };
+
+    Display *display = XOpenDisplay(0);
+    if (!display) {
+        result.window = 0;
+        return result;
+    }
+
+    int screen = DefaultScreen(display);
+    ::Window xwindow = XCreateSimpleWindow(display, RootWindow(display, screen),
+                                           0, 0, width, height, 0,
+                                           BlackPixel(display, screen),
+                                           WhitePixel(display, screen));
+    if (!xwindow) {
+        XCloseDisplay(display);
+        result.window = 0;
+        result.error = OPEN_NO_WINDOW;
+        return result;
+    }
+
+    XStoreName(display, xwindow, "the framebuffer engine");
+    XMapWindow(display, xwindow);
+    XFlush(display);
+
+    window_state.display = display;
+    window_state.xwindow = xwindow;
+    return result;
+}
+
+void CloseWindow(Window *window)
+{
+    if (!window || !window->display)
+        return;
+
+    XDestroyWindow(window->display, window->xwindow);
+    XCloseDisplay(window->display);
+    window->display = 0;
+    window->xwindow = 0;
+}
+
+} /* namespace platform */
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The window gets your title *(extend-the-code)*

The seam names no OS concept — but a window title is not an OS concept, it is
just text. Give the engine a voice on its own window: add a title to the
interface (a `const char *` parameter on `OpenWindow`), pass it through the
X11 implementation to the OS, and have the engine open its window as `the
framebuffer engine — lesson 027`. Show it working: run the engine and find
your window by its title — on your desktop, or headlessly the way this lesson
did with `xdotool search --name`. How many files did the change touch, and
which of them is allowed to know what a title is made of?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-027/ex1.md)

### Exercise 2 — Where the boundary is *(explain-in-prose)*

The seam's promise is checkable: engine code never names an OS type. Check
it. Build the lesson's end state, then compile `src/main.cpp` and
`src/platform_x11.cpp` to objects on their own (`g++ -std=c++17 -Wall
-Wextra -c`) and inspect what each one references with `nm` (through
`c++filt`). Before you look, predict which object names `XOpenDisplay` and
which names `platform::OpenWindow`. Then write down, in prose, what a Win32
port of this program would touch — and what in the two object files proves
it. Make the boundary visible at runtime too, with one instrumenting print
inside the implementation that names which OS is serving the call.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-027/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 026 — the codebase is born](lesson-026-birth.md) ·
**Next:** [Lesson 028 — the event pump](lesson-028-event-pump.md) ·
**Code tag:** [`lesson-027`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-027)
