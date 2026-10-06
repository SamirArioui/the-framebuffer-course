# Lesson 029 — clean close and error paths: OS resources released on every exit

{{#include ../../stability-horizon.md}}

## Prose

A window is not the only thing a run can leave behind. Lesson 028 gave the
engine one clean ending — the user closed the window — but a run can end
other ways: the OS can refuse to give a window at all, the window can be
destroyed out from under the run, and the user can interrupt it. Each of
those is an **exit path**, and every exit path owes the same debt: whatever
was taken from the OS is released, exactly once. Today that becomes a rule
the code enforces and the runs can prove.

### The rule

> **Every OS resource is taken exactly once and released exactly once, on
> every path out.**

Not "the OS reclaims it when the process dies" — that is true and it is not
the rule. Process death is a backstop, not bookkeeping: a run that leaks a
display connection every frame would survive for hours under process-death
bookkeeping and still be broken. The rule is what makes the program's
behavior *checkable*, and it is the same discipline the engine will need
when the resources are bigger than one window — the memory reservations and
arenas coming later in this part.

### The error paths

`OpenWindow` can fail two ways, and the two failures sit at different points
in the take sequence — which is why the rule matters most right here:

- **`OPEN_NO_DISPLAY`** — the connection to the OS's display could not be
  opened. This failure happens *before any take*: there is nothing to
  release, and nothing was touched.
- **`OPEN_NO_WINDOW`** — the display connected but the OS refused the
  window. This failure happens *after one take*: the display connection is
  already ours, and the implementation puts it back before reporting the
  failure. Half-open states are released on the way out, not carried home.

The engine's error path reports the failure **by name** — a `switch` over
the typed error, each case its own message — and exits non-zero without
touching a window, because there is none to touch. Both failures are
checkable headlessly, because both are about the display:

```
$ env -u DISPLAY ./build/game
engine: no display to open a window on
$ echo $?
1
$ DISPLAY=:77 ./build/game
engine: no display to open a window on
$ echo $?
1
```

(`:77` is a display with no server listening — same failure, one step
later.) The `OPEN_NO_WINDOW` case is the contract's second reason; a real
one is rare on X11, which is exactly why the typed error names it instead of
hoping.

### The interrupted exit

There is one more way to end a run that lesson 028 could not handle at all:
Ctrl+C. The default action for interrupt is to kill the process on the spot
— no `CloseWindow`, no report, nothing but the OS's backstop. And lesson
028's pump cannot hear the signal even if a handler catches it: the run is
asleep *inside* `XNextEvent`, which does not wake for signals. A probe that
blocks there and counts signals stays blocked:

```
sigprobe: blocked in XNextEvent
STILL BLOCKED after SIGINT
```

The fix is to wait somewhere signals can interrupt. `poll` on the OS's
display connection does exactly that — it returns when there is news *or*
when a signal breaks it with `EINTR` — and it is the same `poll` lesson 021
used on the terminal:

```c++
struct pollfd pfd = { ConnectionNumber(window->display), POLLIN, 0 };
poll(&pfd, 1, -1);

if (interrupted)
    window->close_requested = true;
```

The handler itself does the one thing a signal handler may safely do — set a
`volatile sig_atomic_t` flag, lesson 020's rule — and the pump folds the
flag into state like any other news. **Every way the run can end reports
through `CloseRequested`**: the user's request, the OS's deed, and the
interrupt all turn the same flag true, because the engine's reaction to all
three is the same — stop, and release. The handler is itself a resource: it
is installed when the window opens and restored to the default when the
window closes.

### Exactly once, on the way out

`CloseWindow` releases only what still exists. If the OS already destroyed
the window (`DestroyNotify`), the fold zeroed the id — destroying it again
would be an X error — and only the display connection is left to release. If
the window is still ours, it is destroyed first, then the connection is
closed. Either way: one release per take, and the interrupt handler goes
back to the default on the same path.

The interrupted run ends like any other:

```
$ DISPLAY=:99 ./build/game &
$ kill -INT %1
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed
$ DISPLAY=:99 xdotool search --name "the framebuffer engine"
$ 
```

Window gone, connection closed, exit code 0 — an interrupt is an ending, not
an accident.

## Code step

One change for this lesson: the interrupt becomes news (a `sig_atomic_t`
flag folded in by the pump, which now waits in `poll` where signals can wake
it), `CloseWindow` releases exactly what it took — including the handler —
and the engine's error path reports each typed failure by name. Its end
state is tagged `lesson-029`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 1fa92f3..efb0b34 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -19,8 +19,20 @@ int Run(void)
     platform::WindowResult opened =
         platform::OpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
     if (!opened.window) {
-        std::fprintf(stderr, "engine: no window (platform error %d)\n",
-                     opened.error);
+        /* The error path: nothing was taken that the platform layer did not
+           put back, and the failure is reported by name. */
+        switch (opened.error) {
+        case platform::OPEN_NO_DISPLAY:
+            std::fprintf(stderr, "engine: no display to open a window on\n");
+            break;
+        case platform::OPEN_NO_WINDOW:
+            std::fprintf(stderr, "engine: the OS refused the window\n");
+            break;
+        default:
+            std::fprintf(stderr, "engine: platform error %d\n",
+                         opened.error);
+            break;
+        }
         return 1;
     }
 
diff --git a/src/platform.h b/src/platform.h
index d6aa25e..da3e0f1 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -31,10 +31,12 @@ WindowResult OpenWindow(int width, int height);
 
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
-   state afterwards. Blocks until there is news. */
+   state afterwards. Blocks until there is news or the run is interrupted. */
 void PumpEvents(Window *window);
 
-/* True once the user has asked for this window to close. */
+/* True once the user has asked for this window to close. An interrupted run
+   counts: every way the run can end reports here, so the engine has exactly
+   one ending to get right. */
 bool CloseRequested(const Window *window);
 
 /* Releases everything OpenWindow took from the OS. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 68de824..0438d3f 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -12,6 +12,9 @@
 
 #include <X11/Xlib.h>
 
+#include <poll.h>
+#include <signal.h>
+
 namespace platform {
 
 /* What a window is made of on this OS. The definition lives here, where
@@ -35,6 +38,16 @@ static Window window_state;
    server hands out as integers; asking for them is how you spell them. */
 static Atom wm_delete_window;
 
+/* The interrupt: Ctrl+C is an exit too, and the run owes it the same clean
+   close as any other. The handler does the only thing a signal handler may
+   safely do here — set a flag (lesson 020's rule). */
+static volatile sig_atomic_t interrupted;
+
+static void OnInterrupt(int)
+{
+    interrupted = 1;
+}
+
 WindowResult OpenWindow(int width, int height)
 {
     WindowResult result = { &window_state, OPEN_NO_DISPLAY };
@@ -70,6 +83,11 @@ WindowResult OpenWindow(int width, int height)
     window_state.display = display;
     window_state.xwindow = xwindow;
     window_state.close_requested = false;
+
+    /* The interrupt is part of the window's take: from here on, Ctrl+C is
+       news like any other. */
+    interrupted = 0;
+    signal(SIGINT, OnInterrupt);
     return result;
 }
 
@@ -91,12 +109,19 @@ void PumpEvents(Window *window)
     if (!window || !window->display)
         return;
 
-    /* Block for the first piece of news, then drain whatever else piled up.
-       Blocking is the point: the engine waits here instead of spinning. */
-    XEvent event;
-    XNextEvent(window->display, &event);
-    HandleEvent(window, event);
+    /* Wait for news where a signal can wake us. Lesson 028 slept inside
+       XNextEvent, where Ctrl+C could not reach it; poll on the OS
+       connection returns when there is news *or* when a signal interrupts
+       it — then the flag below is folded in like any other news. */
+    struct pollfd pfd = { ConnectionNumber(window->display), POLLIN, 0 };
+    poll(&pfd, 1, -1);
+
+    if (interrupted)
+        window->close_requested = true;
+
+    /* Drain whatever piled up: one blocking wait, then the whole batch. */
     while (XPending(window->display)) {
+        XEvent event;
         XNextEvent(window->display, &event);
         HandleEvent(window, event);
     }
@@ -117,6 +142,7 @@ void CloseWindow(Window *window)
     if (window->xwindow)
         XDestroyWindow(window->display, window->xwindow);
     XCloseDisplay(window->display);
+    signal(SIGINT, SIG_DFL); /* the handler is taken and released like the rest */
     window->display = 0;
     window->xwindow = 0;
 }
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The window that gets opened twice *(fix-the-crash)*

A teammate's code calls `OpenWindow` a second time while the first window is
still open. Nothing crashes — which is the bug. The seam hands out the same
state block again: the first display connection's handle is overwritten and
leaked, and the two windows share one state. Prove the bug first (a second
call, and watch what happens to the first window), then fix it so the seam
owns exactly one window at a time: the second open is a typed failure the
engine can name, the first window keeps working, and one close still
releases exactly one take.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-029/ex1.md)

### Exercise 2 — The ledger *(predict-the-output)*

Take a ledger out on the platform layer: every OS resource it takes gets one
line (`take display`, `take window`, `take signal handler`), every release
gets one (`release window`, `release display`, `release signal handler`),
printed to stderr at the moment it happens. Before you run anything,
*predict* the ledger for each of the four exits — the user's close request,
the destroyed window, Ctrl+C, and the missing display. Then produce all four
and reconcile. Which path's balance surprised you, and who performed the
release it surprised you with?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-029/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 028 — the event pump: keeping the window alive and reporting close](lesson-028-event-pump.md) ·
**Next:** [Lesson 030 — the framebuffer as our own bytes](lesson-030-framebuffer.md) ·
**Code tag:** [`lesson-029`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-029)
