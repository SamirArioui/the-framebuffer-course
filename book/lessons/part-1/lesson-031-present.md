# Lesson 031 — presentation through the platform layer

{{#include ../../stability-horizon.md}}

## Prose

Lesson 030 ended with pixels on a window — once. Today that becomes the
contract the whole engine will stand on: **the pixels the engine wrote are
the pixels the window shows**, true at every moment of the run, and
provable. Presentation stops being something the engine does at startup and
becomes the answer it gives to every event — including the events that mean
"your pixels are gone".

### The contract

`Present` in `platform.h` now makes a promise with teeth:

> Returns false if the platform could not carry the pixels at all; when it
> returns true the pixels are on screen — the copy has happened.

"Have happened", not "have been sent". Lesson 030's `Present` ended with
`XFlush`, which pushes the copy to the server and returns — the pixels are
*in flight* when `Present` returns, and any claim about what the window
shows would be a guess. `XSync` is the difference: it round-trips the
connection and returns only when the server has done the work. One call
changed, and the function's return value became a fact.

Why synchronous presentation at all — why not fire-and-forget? Because the
engine needs to *know*. Lesson 036 will measure what the copy costs and the
honest answer needs a stopwatch that stops when the work is done; exercise 1
of this lesson reads the window back and compares, and a comparison against
pixels in flight compares nothing. The contract is the foundation the
measurements and the checks stand on.

### Presentation is not an event

The frame step is three moves and a rule:

```c++
while (!platform::CloseRequested(opened.window)) {
    platform::PumpEvents(opened.window);
    if (platform::CloseRequested(opened.window))
        break;
    if (!platform::Present(opened.window, fb->pixels, fb->width, fb->height))
        ...
}
```

**React before you present** — if the news was "the window is gone", there
is nothing to present to. That order is what keeps the loop honest, and
lesson 034's interactive frame will slot its update step into exactly this
shape.

The rule is the interesting part: the engine **re-presents after every batch
of news**, and that is the entire repair mechanism for a damaged window. X11
windows are not retained — when another window covers yours, the covered
pixels are *gone*; the server keeps nothing and reconstructs nothing. What
it sends instead is `Expose`: "your pixels are gone". The engine does not
handle `Expose` at all. It does not track damage, does not decide what to
redraw, does not fix regions. It owns one framebuffer, and its answer to
every event is to present it again. The event only serves to wake the loop.

(That is why the pump now subscribes to `ExposureMask` — the news has to be
able to arrive. Lesson 022's double buffer had a `front` grid for exactly
this problem; here the framebuffer *is* the front buffer.)

### The proof

The scenario is checkable headlessly, and it was checked: after presenting,
a readback of the window — `XGetImage` over the whole thing — compared
against the framebuffer, pixel by pixel:

```
readback: 6348 samples, 0 mismatches; (0,0)=ff0000 (639,479)=00ff00
```

Then the window was damaged on purpose — unmapped and mapped again, which is
what damage looks like from the client side — and the loop re-presented:

```
readback: 6348 samples, 0 mismatches; (0,0)=ff0000 (639,479)=00ff00
```

Every sampled pixel still equals the engine's bytes. Exercise 1 builds this
checker into your own build so the claim is yours to verify, whenever you
want.

### The window that dies mid-present

There is one more way the contract can be tested, and it is the honest one.
Closing a window is not atomic with respect to a run that is presenting:
the OS can destroy the window **while a present is in flight**. The copy
reaches the server after the window is gone, and the server's answer is an X
error — `BadDrawable` — which, with the default handler, ends the process on
the spot. A race like that is not a crash to accept; it is a failure to
report.

So the platform layer treats X errors as **values**, the same way it treats
every other failure: a handler records what happened, `Present` interprets
it after the sync, and an error on our own window is folded into the same
close news as `DestroyNotify` — the deed, arriving by another route. The
run does not crash; it ends the way every other ending works:

```
$ DISPLAY=:99 ./build/game &
$ DISPLAY=:99 xdotool windowclose <id>
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: close reported
engine: closed
```

Whether the destroy is noticed before, during, or after the next present,
the ending is the same — resources released, exit code 0. Closing the run
eight times in a row at arbitrary points produced eight identical,
unremarkable endings. That is what "the run releases its OS resources on
every exit" was always supposed to mean.

## Code step

One change for this lesson: `Present` completes synchronously and reports
its failures as values — including the window that dies mid-copy — the pump
subscribes to `Expose` so damage can wake the engine, and `main.cpp` grows
the frame step that re-presents after every batch of news. Its end state is
tagged `lesson-031`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 3245656..2dc455a 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -55,16 +55,42 @@ int Run(void)
     std::printf("engine: pixel (60,101) = %d %d %d\n", r, g, b);
 
     /* First light: the bytes go to the window through the seam. */
-    platform::Present(opened.window, fb->pixels, fb->width, fb->height);
+    if (!platform::Present(opened.window, fb->pixels, fb->width, fb->height)) {
+        std::fprintf(stderr, "engine: presentation failed\n");
+        platform::CloseWindow(opened.window);
+        return 1;
+    }
     std::printf("engine: presented\n");
 
-    while (!platform::CloseRequested(opened.window))
+    /* The frame step: read news, react, present our pixels, repeat.
+       Presentation is not an event — it is the engine's answer to every
+       event: the pixels the engine wrote are the pixels the window shows,
+       and re-presenting is what repairs the window when the OS damaged it.
+       React first: if the news was "the window is gone", there is nothing
+       left to present to. */
+    int exit_code = 0;
+    while (!platform::CloseRequested(opened.window)) {
         platform::PumpEvents(opened.window);
+        if (platform::CloseRequested(opened.window))
+            break;
+        if (!platform::Present(opened.window, fb->pixels, fb->width,
+                               fb->height)) {
+            /* A present can fail because the window died mid-copy — that
+               is close news and the fold already said so. Anything else is
+               a real failure and is reported as one. */
+            if (platform::CloseRequested(opened.window))
+                break;
+            std::fprintf(stderr, "engine: presentation failed\n");
+            exit_code = 1;
+            break;
+        }
+    }
 
-    std::printf("engine: close reported\n");
+    if (platform::CloseRequested(opened.window))
+        std::printf("engine: close reported\n");
     platform::CloseWindow(opened.window);
     std::printf("engine: closed\n");
-    return 0;
+    return exit_code;
 }
 
 } /* namespace engine */
diff --git a/src/platform.h b/src/platform.h
index a2e0298..e23434d 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -42,8 +42,10 @@ bool CloseRequested(const Window *window);
 /* Presents the engine's framebuffer in the window: the pixels the engine
    wrote are the pixels the window shows. The format is this interface's
    contract, not any OS's — width * height pixels of 4 bytes each (blue,
-   green, red, one unused byte), one row after another. */
-void Present(Window *window, const unsigned char *pixels, int width,
+   green, red, one unused byte), one row after another. Returns false if
+   the platform could not carry the pixels at all; when it returns true the
+   pixels are on screen — the copy has happened. */
+bool Present(Window *window, const unsigned char *pixels, int width,
              int height);
 
 /* Releases everything OpenWindow took from the OS. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 8c93ccc..19bff08 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -13,6 +13,7 @@
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 
+#include <cstdio>
 #include <poll.h>
 #include <signal.h>
 
@@ -49,6 +50,21 @@ static void OnInterrupt(int)
     interrupted = 1;
 }
 
+/* X errors are values in this layer, not process death. The default X
+   error handler would end the run on the spot — but a window can die while
+   a present is in flight, and that is news, not a crash. The handler
+   records what happened; Present decides what it means. */
+static int last_xerror;
+static unsigned long last_xerror_resource;
+
+static int OnXError(Display *display, XErrorEvent *event)
+{
+    (void)display;
+    last_xerror = event->error_code;
+    last_xerror_resource = event->resourceid;
+    return 0;
+}
+
 WindowResult OpenWindow(int width, int height)
 {
     WindowResult result = { &window_state, OPEN_NO_DISPLAY };
@@ -72,10 +88,12 @@ WindowResult OpenWindow(int width, int height)
     }
 
     /* Register the close request as the way to go, and subscribe to the
-       window's lifecycle news (map, configure, destroy). */
+       window's lifecycle news (map, configure, destroy) — plus Expose, the
+       "your pixels are gone" news. The engine handles Expose by doing the
+       only thing that repairs a window: presenting again. */
     wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
     XSetWMProtocols(display, xwindow, &wm_delete_window, 1);
-    XSelectInput(display, xwindow, StructureNotifyMask);
+    XSelectInput(display, xwindow, StructureNotifyMask | ExposureMask);
 
     XStoreName(display, xwindow, "the framebuffer engine");
     XMapWindow(display, xwindow);
@@ -86,9 +104,10 @@ WindowResult OpenWindow(int width, int height)
     window_state.close_requested = false;
 
     /* The interrupt is part of the window's take: from here on, Ctrl+C is
-       news like any other. */
+       news like any other — and X errors are recorded, not fatal. */
     interrupted = 0;
     signal(SIGINT, OnInterrupt);
+    XSetErrorHandler(OnXError);
     return result;
 }
 
@@ -133,11 +152,11 @@ bool CloseRequested(const Window *window)
     return window && window->close_requested;
 }
 
-void Present(Window *window, const unsigned char *pixels, int width,
+bool Present(Window *window, const unsigned char *pixels, int width,
              int height)
 {
-    if (!window || !window->display)
-        return;
+    if (!window || !window->display || !window->xwindow)
+        return false; /* nothing to present to (the OS may have destroyed it) */
 
     int screen = DefaultScreen(window->display);
 
@@ -150,10 +169,12 @@ void Present(Window *window, const unsigned char *pixels, int width,
                                  ZPixmap, 0, (char *)pixels,
                                  width, height, 32, width * 4);
     if (!image)
-        return;
+        return false;
 
     /* XPutImage is where the copy happens — our bytes to the server. Its
-       cost is real; lesson 036 measures it. */
+       cost is real; lesson 036 measures it. (Its return value is not a
+       status in practice — this call either copies or raises an X error,
+       which is the OS error handler's territory.) */
     XPutImage(window->display, window->xwindow,
               DefaultGC(window->display, screen),
               image, 0, 0, 0, 0, width, height);
@@ -163,7 +184,24 @@ void Present(Window *window, const unsigned char *pixels, int width,
        lesson 004's ownership drills). */
     image->data = 0;
     XDestroyImage(image);
-    XFlush(window->display);
+
+    /* The contract: when Present returns, the pixels are on screen. XFlush
+       would only send the copy; XSync waits for the server to have done
+       it. */
+    last_xerror = 0;
+    XSync(window->display, False);
+
+    if (last_xerror) {
+        /* An error on our own window means it died mid-copy — the same
+           news as DestroyNotify, the deed: report it and stop presenting. */
+        if ((last_xerror == BadDrawable || last_xerror == BadWindow) &&
+            last_xerror_resource == (unsigned long)window->xwindow) {
+            window->close_requested = true;
+            window->xwindow = 0;
+        }
+        return false;
+    }
+    return true;
 }
 
 void CloseWindow(Window *window)
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The presentation check *(extend-the-code)*

The contract deserves a checker, and the checker belongs on the OS side of
the seam. Grow `platform.h` with a readback — `PresentedMatches` takes the
bytes the engine presented, snapshots the window with `XGetImage`, and
compares every pixel — and have the engine verify its own claim right after
first light: `engine: presentation verified — window matches framebuffer`.
Which pixels does the comparison have to *ignore*, and why? (Lesson 030's
exercise 2 knows the answer.)

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-031/ex1.md)

### Exercise 2 — Cover and reveal *(port-to-your-own-machine)*

X11 windows are not retained: when another window covers yours, the covered
pixels are gone and the server asks you to redraw. First *predict* what your
window would show after being covered and revealed if the engine presented
only once at startup (the lesson-030 shape). Then make the repair visible:
one instrumenting line in the fold, printed when `Expose` news arrives. On
your own desktop, cover the window with another and reveal it — watch the
news arrive and the pixels come back. Write down what woke the engine, and
why the engine's answer is "present again" instead of "redraw the exposed
rectangle".

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-031/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 030 — the framebuffer as our own bytes](lesson-030-framebuffer.md) ·
**Next:** [Lesson 032 — polled input state](lesson-032-polled-input.md) ·
**Code tag:** [`lesson-031`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-031)
