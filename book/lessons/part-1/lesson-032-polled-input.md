# Lesson 032 — polled input state

{{#include ../../stability-horizon.md}}

## Prose

The engine can open a window, keep it alive, fill it with its own pixels —
and it has no idea whether anyone is touching the keyboard. Today that
changes, and it changes the way the engine thinks: not by handing the engine
an event stream to consume, but by giving it **state to poll**. Which keys
are down, right now, whenever the engine asks. That is the contract Part 2's
movement code and Part 5's feel toolkit will stand on.

### State, not events

The seam grows two things:

```c++
enum Key {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_SPACE, KEY_ENTER, KEY_ESCAPE, KEY_COUNT
};

bool KeyDown(const Window *window, Key key);
```

`KeyDown` answers one question — *is this key down at the moment of the
call?* — and that question is what game logic actually asks. "Move the
marker while left is held" does not want a queue of key events with
timestamps; it wants to know, once per frame, whether left is down. Events
are how the OS thinks; polled state is how a frame thinks.

The division of labor from lesson 028 is now doing real work: **news goes
into state; the engine reads state**. Key events are folded into a small
table of bits inside the platform layer and thrown away. The engine polls
the table through one function and never sees an event object, an OS key
code, or an event queue.

### The keys are the seam's

`Key` is the seam's vocabulary, not X11's. The OS has its own key codes
(X11 calls them keysyms — `XK_Left`, `XK_space`) and they never cross the
boundary. The translation lives in one function in one file:

```c++
static int KeyIndex(KeySym sym)
{
    switch (sym) {
    case XK_Up:     return KEY_UP;
    ...
    case XK_space:  return KEY_SPACE;
    ...
    }
}
```

A second OS translates its own key codes to the same `Key` values and the
engine does not notice. Which keys the engine *tracks* is a contract
decision — seven is plenty for a game that moves with arrows and confirms
with space — and exercise 1 grows the set in exactly three places.

### Held keys stay held

One OS behavior deserves its own paragraph, because it breaks naive
implementations: **auto-repeat**. Hold a key on a real keyboard and the OS
streams repeated presses — but with the plain setting it interleaves
*releases* too: press, release, press, release, press... A fold that trusts
every event would flicker the state between down and up while your finger
never moved.

The OS can tell the truth instead. `XkbSetDetectableAutoRepeat` makes
repeats arrive as presses *only* — no phantom releases — so the folded state
stays down exactly as long as the key is held:

```
$ DISPLAY=:99 xdotool keydown --window <id> Left
$ DISPLAY=:99 xdotool keydown --window <id> Right
$ DISPLAY=:99 xdotool keyup --window <id> Right
$ DISPLAY=:99 xdotool keyup --window <id> Left
engine: polled: left
engine: polled: left right
engine: polled: left
engine: polled: -
```

Four polls, one per gesture. `left` appears in the first poll and is *still
there* in the third — held keys stay held across polls — while `right`
arrives and leaves in between. The last poll sees an empty state again.

### What the poll reports, and when

The engine polls once per frame step — after the pump has folded whatever
news arrived. That timing is a contract in itself: **the poll reports the
state as of the last fold**. It is exactly what "which keys are down at the
moment of the poll" means.

Which is also where this contract's edge is. A key can be pressed *and*
released between two polls — faster than the frame — and then the state
never shows it: down and up cancel before anyone asks. The state is honest
and the press is lost. That is not a bug in the fold (both events are
folded; both assignments run); it is the shape of state-only input, and it
is lesson 033's whole subject: the engine needs one more piece of state —
not just *is down now*, but *went down since the last poll*.

## Code step

One change for this lesson: the seam grows `Key` and `KeyDown`, the X11 side
folds key events into a state table (with detectable auto-repeat so held
keys stay held), and `main.cpp` polls the state each frame step and reports
what it sees. Its end state is tagged `lesson-032`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 2dc455a..91be4f0 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -12,6 +12,11 @@
 
 namespace engine {
 
+/* The seam's keys, by name — for the report below. */
+static const char *const key_names[platform::KEY_COUNT] = {
+    "up", "down", "left", "right", "space", "enter", "escape",
+};
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -73,6 +78,18 @@ int Run(void)
         platform::PumpEvents(opened.window);
         if (platform::CloseRequested(opened.window))
             break;
+
+        /* The polled state: what is down right now, as of this poll. */
+        std::printf("engine: polled:");
+        bool any = false;
+        for (int k = 0; k < platform::KEY_COUNT; ++k) {
+            if (platform::KeyDown(opened.window, (platform::Key)k)) {
+                std::printf(" %s", key_names[k]);
+                any = true;
+            }
+        }
+        std::printf(any ? "\n" : " -\n");
+
         if (!platform::Present(opened.window, fb->pixels, fb->width,
                                fb->height)) {
             /* A present can fail because the window died mid-copy — that
diff --git a/src/platform.h b/src/platform.h
index e23434d..aac8d48 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -29,6 +29,23 @@ struct WindowResult {
 /* Opens a window of exactly the requested size on the OS's display. */
 WindowResult OpenWindow(int width, int height);
 
+/* The keys the engine tracks. Plain values — no OS key code ever crosses
+   the seam. */
+enum Key {
+    KEY_UP = 0,
+    KEY_DOWN,
+    KEY_LEFT,
+    KEY_RIGHT,
+    KEY_SPACE,
+    KEY_ENTER,
+    KEY_ESCAPE,
+    KEY_COUNT
+};
+
+/* The polled input state: true while the key is down at the moment of the
+   call. State, not events — the engine asks, it never consumes a stream. */
+bool KeyDown(const Window *window, Key key);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 19bff08..22f4c19 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -10,8 +10,10 @@
 
 #include "platform.h"
 
+#include <X11/XKBlib.h>
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
+#include <X11/keysym.h>
 
 #include <cstdio>
 #include <poll.h>
@@ -26,6 +28,7 @@ struct Window {
     Display *display;
     ::Window xwindow;
     bool close_requested;
+    bool keys[KEY_COUNT];
 };
 
 /* The OS state for one window, in static storage: no new, no delete — the
@@ -89,11 +92,20 @@ WindowResult OpenWindow(int width, int height)
 
     /* Register the close request as the way to go, and subscribe to the
        window's lifecycle news (map, configure, destroy) — plus Expose, the
-       "your pixels are gone" news. The engine handles Expose by doing the
-       only thing that repairs a window: presenting again. */
+       "your pixels are gone" news, and the keyboard. The engine handles
+       Expose by doing the only thing that repairs a window: presenting
+       again. */
     wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
     XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
-    XSelectInput(display, xwindow, StructureNotifyMask | ExposureMask);
+    XSelectInput(display, xwindow,
+                 StructureNotifyMask | ExposureMask | KeyPressMask |
+                     KeyReleaseMask);
+
+    /* Auto-repeat would otherwise look like release-then-press every
+       repeat: a held key would flicker in polled state. Detectable
+       auto-repeat makes repeats arrive as presses only, so "held" stays
+       held. */
+    XkbSetDetectableAutoRepeat(display, True, 0);
 
     XStoreName(display, xwindow, "the framebuffer engine");
     XMapWindow(display, xwindow);
@@ -102,6 +114,8 @@ WindowResult OpenWindow(int width, int height)
     window_state.display = display;
     window_state.xwindow = xwindow;
     window_state.close_requested = false;
+    for (int i = 0; i < KEY_COUNT; ++i)
+        window_state.keys[i] = false;
 
     /* The interrupt is part of the window's take: from here on, Ctrl+C is
        news like any other — and X errors are recorded, not fatal. */
@@ -111,6 +125,23 @@ WindowResult OpenWindow(int width, int height)
     return result;
 }
 
+/* Which of our keys an OS key event is about, or -1 for keys we do not
+   track. The translation from OS key codes to the seam's Key lives here
+   and nowhere else. */
+static int KeyIndex(KeySym sym)
+{
+    switch (sym) {
+    case XK_Up:     return KEY_UP;
+    case XK_Down:   return KEY_DOWN;
+    case XK_Left:   return KEY_LEFT;
+    case XK_Right:  return KEY_RIGHT;
+    case XK_space:  return KEY_SPACE;
+    case XK_Return: return KEY_ENTER;
+    case XK_Escape: return KEY_ESCAPE;
+    default:        return -1;
+    }
+}
+
 /* One piece of news, folded into state. The request and the deed both mean
    the same thing to the engine. */
 static void HandleEvent(Window *window, XEvent &event)
@@ -121,9 +152,18 @@ static void HandleEvent(Window *window, XEvent &event)
     } else if (event.type == DestroyNotify) {
         window->close_requested = true; /* the window is already gone */
         window->xwindow = 0;
+    } else if (event.type == KeyPress || event.type == KeyRelease) {
+        int key = KeyIndex(XLookupKeysym(&event.xkey, 0));
+        if (key >= 0)
+            window->keys[key] = (event.type == KeyPress);
     }
 }
 
+bool KeyDown(const Window *window, Key key)
+{
+    return window && key >= 0 && key < KEY_COUNT && window->keys[key];
+}
+
 void PumpEvents(Window *window)
 {
     if (!window || !window->display)
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Your own keys *(extend-the-code)*

Arrows are not the only way to move. Grow the seam's tracked keys with
W/A/S/D — the enum, the mapping, and the report — and show them tracking
independently: hold `w`, add and remove `d`, watch each poll. Where does the
translation from OS key codes to seam keys live, and how many files did your
change have to touch? (Check what `XK_w` and `XK_W` are, and think about
which one movement input should care about.)

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-032/ex1.md)

### Exercise 2 — The press that vanished *(predict-the-output)*

A key can be pressed and released faster than the engine polls. *Predict*
what the report shows for a single gesture with no waiting —
`xdotool key --delay 0 --window <id> space` puts a press and a release into
one pump batch. Write the prediction down before running anything. Then
make the fold visible — one instrumenting line per key event, printed from
inside the implementation — run the gesture, and reconcile. What does the
state-based contract lose here, and what would the engine need in order not
to lose it?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-032/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 031 — presentation through the platform layer](lesson-031-present.md) ·
**Next:** [Lesson 033 — latching brief presses and tracking focus](lesson-033-latching.md) ·
**Code tag:** [`lesson-032`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-032)
