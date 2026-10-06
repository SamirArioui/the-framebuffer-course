# Lesson 028 — the event pump: keeping the window alive and reporting close

{{#include ../../stability-horizon.md}}

## Prose

Lesson 027's window had a borrowed heartbeat: `std::getchar()` held it open
and the OS never heard from the program again. That is not how a window
stays alive. The connection to the OS is bidirectional — it carries news
*back* to the program: the user moved something, the window manager wants
something, the window died. A program that never reads that news stops
answering, and a window whose owner stops answering is exactly what an OS
calls "not responding". Today the engine learns to listen: the **event
pump**, and the first piece of news it must take seriously — *close*.

### The seam grows two functions

The platform interface gains the pump and one query:

```c++
void PumpEvents(Window *window);
bool CloseRequested(const Window *window);
```

Look at the division of labor the pair encodes, because it is the shape
everything after this follows: **news goes into state; the engine reads
state**. `PumpEvents` is the only function that ever sees an event object —
and it sees them *inside the platform layer*, folds each one into plain
fields, and throws the event away. The engine calls the pump and then polls
`CloseRequested`, the way a rider checks the road rather than catching
butterflies. Lesson 032's input works the same way: OS key events will be
folded into queryable key state, and the engine will poll it — never consume
an event stream.

### The OS side: waiting, not spinning

The X11 half of the pump is three calls doing one job:

```c++
XEvent event;
XNextEvent(window->display, &event);   /* blocks until there is news */
HandleEvent(window, event);
while (XPending(window->display)) {    /* what else piled up? */
    XNextEvent(window->display, &event);
    HandleEvent(window, event);
}
```

`XNextEvent` **blocks**: if no news has arrived, the program sleeps inside
the OS until some does — no CPU is burned waiting. Then `XPending` reports
how many events are already queued, and the loop drains them all before
returning to the engine. One blocking read plus a drain is the standard
pump: the engine wakes up when there is something to know, handles the whole
batch, and goes back to sleep.

Blocking is the right behavior *for this lesson's engine*, which does
nothing between news. It stops being right the moment the engine owes the
screen a frame every tick — lesson 034's interactive frame — and that is
when the pump becomes non-blocking on purpose. For now, waiting is honest.

### The close conversation

"Close the window" sounds like one event. On X11 it is a small protocol with
two endings:

- **The polite request.** A window manager does not destroy a window behind
  its owner's back. It *asks*. The ask is a `ClientMessage` — a plain event
  carrying two atoms — and the atom that means "please close" is
  `WM_DELETE_WINDOW`. Atoms are names the X server hands out as integers:
  `XInternAtom` turns the string `"WM_DELETE_WINDOW"` into its number, and
  `XSetWMProtocols` registers with the window manager that this program
  accepts the request. Your desktop's close button produces exactly this
  message.
- **The deed.** The window can also simply stop existing — destroyed
  directly (`xdotool windowclose` does this), or torn down by the server.
  The news is `DestroyNotify`, and it is not a request: the OS resource is
  already gone.

Both endings fold into one piece of state, because the engine's reaction is
the same: stop. But the fold remembers the difference where it matters —
after `DestroyNotify` the window id is dead, and the cleanup must not ask
the OS to destroy it a second time (that is an X error, and the default
handler for those ends the program on the spot). `HandleEvent` zeroes the id
when the window is already gone; `CloseWindow` destroys only what still
exists. Lesson 029 takes this thread — *every* exit path releases exactly
its resources, exactly once — and makes it the whole subject.

### The engine side

The engine's loop is four lines and the whole point of the lesson:

```c++
while (!platform::CloseRequested(opened.window))
    platform::PumpEvents(opened.window);
```

Read news, fold it, react to the state, repeat. When the flag turns true the
loop ends, the engine reports it, and closes what it opened:

```
$ ./build.sh
build: compiling 2 source(s) from src/
  CC  src/main.cpp
  CC  src/platform_x11.cpp
  LD  build/game
build: OK (2 source(s) compiled -> build/game)
$ DISPLAY=:99 ./build/game &
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
2097153
$ DISPLAY=:99 xdotool windowclose 2097153
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed
```

That is the headless check of the lesson's documented behavior: the window
opens at the size the engine requested, the run **stays alive** (it is still
there a second later, still answering), the close is **reported** to the
engine, and the run ends **with its OS resources released** — after the
process exits, the window is gone from the server and the display connection
is closed:

```
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
$ 
```

On a real desktop the same run opens the window, waits, and your close
button sends the polite request instead of the deed — same flag, same exit.
Exercise 2 instruments the fold so you can see which ending your machine
produces.

## Code step

One change for this lesson: the seam grows `PumpEvents` and `CloseRequested`,
the X11 side learns to wait and to listen for the two ways close news
arrives, `main.cpp` swaps its `getchar` stand-in for the real pump loop, and
`CloseWindow` learns not to destroy twice. Its end state is tagged
`lesson-028`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 106b3b0..1fa92f3 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,10 +1,9 @@
 // main.cpp — the engine, born.
 //
-// Lesson 027: the engine meets the OS through the platform seam. This file
-// includes no OS headers and names no OS type — it sees platform.h and
-// nothing else. The language law of lesson 026 holds: engine code lives in
-// namespace engine, main stays global, and every feature here is one the
-// law admits.
+// Lesson 028: the engine stays alive by reading the OS's news through the
+// seam. Still no OS headers, still no OS types — the pump is one more
+// platform function and the engine polls what it leaves behind. The
+// language law of lesson 026 holds.
 
 #include <cstdio>
 
@@ -25,13 +24,15 @@ int Run(void)
         return 1;
     }
 
-    std::printf("engine: window %dx%d open — press enter to close\n",
+    std::printf("engine: window %dx%d open — waiting for news\n",
                 WINDOW_WIDTH, WINDOW_HEIGHT);
 
-    /* Stand-in for the event pump: hold the window open until enter.
-       Lesson 028 replaces exactly this. */
-    std::getchar();
+    /* The event pump: read news, fold it into state, react to state,
+       repeat. This loop is what keeps the window alive. */
+    while (!platform::CloseRequested(opened.window))
+        platform::PumpEvents(opened.window);
 
+    std::printf("engine: close reported\n");
     platform::CloseWindow(opened.window);
     std::printf("engine: closed\n");
     return 0;
diff --git a/src/platform.h b/src/platform.h
index 361251c..d6aa25e 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -29,6 +29,14 @@ struct WindowResult {
 /* Opens a window of exactly the requested size on the OS's display. */
 WindowResult OpenWindow(int width, int height);
 
+/* Reads whatever news the OS has about this window and folds it into the
+   platform layer's state. The engine never sees an event object — it polls
+   state afterwards. Blocks until there is news. */
+void PumpEvents(Window *window);
+
+/* True once the user has asked for this window to close. */
+bool CloseRequested(const Window *window);
+
 /* Releases everything OpenWindow took from the OS. */
 void CloseWindow(Window *window);
 
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 203b181..68de824 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -4,6 +4,9 @@
 // included here and nowhere else, and every OS type is spelled here and
 // nowhere else. An implementation for a second OS would live in its own
 // file beside this one and nothing outside would change.
+//
+// Lesson 028: the event pump. The OS's news arrives here as X events and is
+// folded into state the engine polls — the engine never reads an event.
 
 #include "platform.h"
 
@@ -17,6 +20,7 @@ namespace platform {
 struct Window {
     Display *display;
     ::Window xwindow;
+    bool close_requested;
 };
 
 /* The OS state for one window, in static storage: no new, no delete — the
@@ -25,6 +29,12 @@ struct Window {
    part. The engine only ever holds the pointer. */
 static Window window_state;
 
+/* The two atoms of the window-close conversation. The window manager does
+   not destroy a window behind its owner's back — it *asks*, by sending a
+   ClientMessage whose first word is WM_DELETE_WINDOW. X atoms are names the
+   server hands out as integers; asking for them is how you spell them. */
+static Atom wm_delete_window;
+
 WindowResult OpenWindow(int width, int height)
 {
     WindowResult result = { &window_state, OPEN_NO_DISPLAY };
@@ -47,21 +57,65 @@ WindowResult OpenWindow(int width, int height)
         return result;
     }
 
+    /* Register the close request as the way to go, and subscribe to the
+       window's lifecycle news (map, configure, destroy). */
+    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
+    XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
+    XSelectInput(display, xwindow, StructureNotifyMask);
+
     XStoreName(display, xwindow, "the framebuffer engine");
     XMapWindow(display, xwindow);
     XFlush(display);
 
     window_state.display = display;
     window_state.xwindow = xwindow;
+    window_state.close_requested = false;
     return result;
 }
 
+/* One piece of news, folded into state. The request and the deed both mean
+   the same thing to the engine. */
+static void HandleEvent(Window *window, XEvent &event)
+{
+    if (event.type == ClientMessage &&
+        (Atom)event.xclient.data.l[0] == wm_delete_window) {
+        window->close_requested = true; /* the polite request */
+    } else if (event.type == DestroyNotify) {
+        window->close_requested = true; /* the window is already gone */
+        window->xwindow = 0;
+    }
+}
+
+void PumpEvents(Window *window)
+{
+    if (!window || !window->display)
+        return;
+
+    /* Block for the first piece of news, then drain whatever else piled up.
+       Blocking is the point: the engine waits here instead of spinning. */
+    XEvent event;
+    XNextEvent(window->display, &event);
+    HandleEvent(window, event);
+    while (XPending(window->display)) {
+        XNextEvent(window->display, &event);
+        HandleEvent(window, event);
+    }
+}
+
+bool CloseRequested(const Window *window)
+{
+    return window && window->close_requested;
+}
+
 void CloseWindow(Window *window)
 {
     if (!window || !window->display)
         return;
 
-    XDestroyWindow(window->display, window->xwindow);
+    /* xwindow is zero once the OS has already destroyed the window;
+       destroying it twice would be an X error. */
+    if (window->xwindow)
+        XDestroyWindow(window->display, window->xwindow);
     XCloseDisplay(window->display);
     window->display = 0;
     window->xwindow = 0;
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Resize is news too *(extend-the-code)*

The pump folds one piece of news; make it fold another. Track the window's
size through `ConfigureNotify` events — the OS reports the new width and
height every time the geometry changes — record it in the window's state
from the moment `OpenWindow` asks for a size, and report it when the run
ends (`engine: closed at 800x600`). Add the query to the seam the way this
lesson added `CloseRequested` — plain types only. Show it working: resize
the window while the engine runs (drag a corner on your desktop, or
`xdotool windowsize <id> 800 600` headlessly) and close it.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-028/ex1.md)

### Exercise 2 — Two ways the news arrives *(port-to-your-own-machine)*

Make the fold announce itself: one instrumenting line per close path — the
request (`ClientMessage`) and the deed (`DestroyNotify`) — printed from
inside the implementation. Then produce both endings on your own machine:
close the window with your desktop's close button, and again the abrupt way
(`xdotool windowclose <id>` from another terminal, which destroys the window
without asking). Write down what each run produced, which one your close
gesture gives, and why the pump folds both into the same flag while the
cleanup still treats them differently.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-028/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 027 — the platform seam and the first X11 window](lesson-027-first-window.md) ·
**Next:** [Lesson 029 — clean close and error paths](lesson-029-clean-close.md) ·
**Code tag:** [`lesson-028`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-028)
