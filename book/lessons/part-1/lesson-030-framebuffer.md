# Lesson 030 — the framebuffer as our own bytes

{{#include ../../stability-horizon.md}}

## Prose

The window is alive but it shows nothing of ours — every pixel on it came
from the OS. That ends today. The engine gets a **framebuffer**: one
screenful of memory that belongs to us, laid out by a contract we define,
written by code we own. This is Part 0's `paint` buffer grown to window
size — the same instincts (`ClearBuffer`, `PutPixel`, `GetPixel`), the same
offset arithmetic — and at the end of the lesson the window shows those
bytes for the first time.

### The buffer

```c++
struct Framebuffer {
    unsigned char *pixels;
    int width;
    int height;
};
```

One screenful of bytes and two numbers. `FRAME_WIDTH`×`FRAME_HEIGHT` is
640×480 — the window's size from lesson 027 — and at four bytes per pixel
that is `1228800` bytes of pixels: the biggest thing the engine owns so far,
and the reason lesson 039's virtual-memory deep dive is coming.

Where do 1.2 MB of pixels live? In static storage, for now: an array inside
`framebuffer.cpp`, handed out by `GetFramebuffer`. The language law of
lesson 026 keeps `new`/`delete` out of the engine, so the buffer is simply
*there* — and this is the thread lesson 040 pulls: a buffer this size should
come from an OS-level memory *reservation*, not from anyone's allocator, and
the framebuffer will be the first one to move.

### The format is the seam's

Four bytes per pixel, in this order in memory: **blue, green, red, one
unused byte**. That is the packed pixel of lesson 013 — but notice *whose*
contract this is. It is not X11's format and it is not a hardware fact the
engine has to live with; it is the **platform seam's** format, declared in
`platform.h` next to `Present`:

> width × height pixels of 4 bytes each (blue, green, red, one unused byte),
> one row after another.

Any OS implementation behind the seam must carry these bytes to its window.
On the Linux/X11 main line it happens for free — this byte order is what X11
carries natively on little-endian x86-64 — and a Win32 epilogue would
translate on its way out. Engine code writes colors through `PutPixel` and
never reasons about byte order again.

The arithmetic is lesson 013's and lesson 007's, once and for all:

```
pixel (x, y) starts at (y * width + x) * 4
one row is width * 4 bytes — the stride
```

At 640 wide the stride is 2560 bytes: a whole number of 8-byte words, so no
row boundary ever straddles a machine word. The unused byte is the cheapest
stride there is.

### Writing pixels

`PutPixel` is paint's, with one difference worth the whole function: it
**clips**. An out-of-bounds write is dropped — lesson 015's fold — never
wrapped into some other pixel's bytes. The proof is arithmetic: a write at
`(700, 100)` without the clip would land at pixel index
`100 * 640 + 700 = 64700` — row 101, column 60 — corrupting a pixel that
has nothing to do with the request. With the fold, it changes nothing, and
the self-check can *show* the untouched pixel.

`ClearBuffer` fills the whole buffer with one color and `GetPixel` reads a
pixel back — the same bytes `PutPixel` wrote. Together they make the buffer
checkable without any window at all.

### The XImage that can carry it

The bytes are ours; getting them to the window is the platform layer's job,
and its tool is X11's `XImage`. An `XImage` is not a picture — it is a
*view*: a description of a pixel buffer (geometry, depth, byte layout) that
Xlib can act on. `XCreateImage` **wraps our bytes without copying them**;
the engine's `pixels` pointer becomes the image's data, and the image struct
is just the description around it.

Two things happen in `Present`, and only one of them costs:

1. `XCreateImage` wraps the buffer — no copy, just a description.
2. `XPutImage` **copies** the pixels to the server — this is the copy whose
   cost is real, and lesson 036 will measure it honestly. (Shared memory —
   MIT-SHM — can skip this copy; it is a named later optimization, not a
   Part 1 topic.)

Then the image struct is destroyed — and note *how*: `image->data` is
zeroed first. The struct is ours to free; the bytes under it are the
engine's. Lesson 004's ownership rule in its smallest form: you release what
you took, and you took only the struct.

### First light

The engine paints and presents: clear to a dark blue-gray, a red pixel at
the origin, green at the far corner, and one out-of-bounds write that the
fold drops. The report reads the buffer back:

```
$ ./build.sh
build: compiling 3 source(s) from src/
  CC  src/framebuffer.cpp
  CC  src/main.cpp
  CC  src/platform_x11.cpp
  LD  build/game
build: OK (3 source(s) compiled -> build/game)
$ DISPLAY=:99 ./build/game
engine: framebuffer 640x480, 1228800 bytes, stride 2560
engine: pixel (0,0) = 255 0 0
engine: pixel (639,479) = 0 255 0
engine: pixel (60,101) = 32 32 64
engine: presented
engine: close reported
engine: closed
```

`(0,0)` and `(639,479)` are the two pixels we wrote. `(60,101)` is the
clip's proof: it still reads the background `32 32 64` — the write at
`(700, 100)` that would have landed there was dropped at the fold.

And the window — on a real desktop you can see it — shows exactly these
pixels: dark blue-gray, one red dot in the top-left corner, one green dot in
the bottom-right. "The pixels the engine wrote are the pixels the window
shows" is now true in fact; lesson 031 turns it into a contract and proves
it by reading the window back.

## Code step

One change for this lesson: the framebuffer is born — `framebuffer.h` and
`framebuffer.cpp` with the bytes, the layout, and the clipped writes — the
seam grows `Present`, whose `XImage` wraps the engine's bytes and whose
`XPutImage` copies them to the window, and `main.cpp` paints, checks, and
presents its first frame. Its end state is tagged `lesson-030`.

```diff
diff --git a/src/framebuffer.cpp b/src/framebuffer.cpp
new file mode 100644
index 0000000..7e6fe13
--- /dev/null
+++ b/src/framebuffer.cpp
@@ -0,0 +1,59 @@
+// framebuffer.cpp — the engine's pixels, by hand.
+//
+// Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
+// the window. Offset math, byte order, clipping — nothing else.
+
+#include "framebuffer.h"
+
+namespace engine {
+
+/* One screenful of pixels, in static storage. */
+static unsigned char pixels[FRAME_WIDTH * FRAME_HEIGHT * 4];
+static Framebuffer framebuffer = { pixels, FRAME_WIDTH, FRAME_HEIGHT };
+
+Framebuffer *GetFramebuffer(void)
+{
+    return &framebuffer;
+}
+
+void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
+                 unsigned char b)
+{
+    int count = fb.width * fb.height;
+    for (int i = 0; i < count; ++i) {
+        unsigned char *p = fb.pixels + i * 4;
+        p[0] = b;
+        p[1] = g;
+        p[2] = r;
+        p[3] = 0;
+    }
+}
+
+void PutPixel(Framebuffer &fb, int x, int y, unsigned char r,
+              unsigned char g, unsigned char b)
+{
+    if (x < 0 || x >= fb.width || y < 0 || y >= fb.height)
+        return; /* dropped, not wrapped (lesson 015's fold) */
+
+    unsigned char *p = fb.pixels + (y * fb.width + x) * 4;
+    p[0] = b;
+    p[1] = g;
+    p[2] = r;
+    p[3] = 0;
+}
+
+void GetPixel(const Framebuffer &fb, int x, int y, unsigned char &r,
+              unsigned char &g, unsigned char &b)
+{
+    if (x < 0 || x >= fb.width || y < 0 || y >= fb.height) {
+        r = g = b = 0;
+        return;
+    }
+
+    const unsigned char *p = fb.pixels + (y * fb.width + x) * 4;
+    b = p[0];
+    g = p[1];
+    r = p[2];
+}
+
+} /* namespace engine */
diff --git a/src/framebuffer.h b/src/framebuffer.h
new file mode 100644
index 0000000..1f38723
--- /dev/null
+++ b/src/framebuffer.h
@@ -0,0 +1,47 @@
+// framebuffer.h — the engine's pixels: our own bytes, our own layout.
+//
+// Lesson 030: the framebuffer is memory the engine owns — Part 0's paint
+// buffer, grown to window size. The pixel format is the platform seam's
+// contract, not any OS's: platform.h's Present carries these exact bytes.
+#ifndef FRAMEBUFFER_H
+#define FRAMEBUFFER_H
+
+namespace engine {
+
+/* The size the window is opened at (lesson 027) and the size of the
+   framebuffer behind it: one format, one geometry. */
+constexpr int FRAME_WIDTH = 640;
+constexpr int FRAME_HEIGHT = 480;
+
+/* 32 bits per pixel in memory: blue, green, red, one unused byte — the
+   packed pixel of lesson 013, four bytes for the alignment lesson 007
+   explained. Rows run top to bottom, one pixel after another:
+   the pixel at (x, y) starts at (y * width + x) * 4. */
+struct Framebuffer {
+    unsigned char *pixels;
+    int width;
+    int height;
+};
+
+/* The engine's framebuffer. Its bytes live in static storage — the
+   language law of lesson 026 keeps allocation out of the engine, and
+   lesson 040 gives buffers like this a real home. */
+Framebuffer *GetFramebuffer(void);
+
+/* Fills every pixel with one color. */
+void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
+                 unsigned char b);
+
+/* Writes one pixel. Out-of-bounds writes are dropped — the fold of
+   lesson 015, never a wrap into someone else's memory. */
+void PutPixel(Framebuffer &fb, int x, int y, unsigned char r,
+              unsigned char g, unsigned char b);
+
+/* Reads one pixel back — the same bytes PutPixel wrote. Out of bounds, the
+   result is black. */
+void GetPixel(const Framebuffer &fb, int x, int y, unsigned char &r,
+              unsigned char &g, unsigned char &b);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index efb0b34..3245656 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,23 +1,21 @@
 // main.cpp — the engine, born.
 //
-// Lesson 028: the engine stays alive by reading the OS's news through the
-// seam. Still no OS headers, still no OS types — the pump is one more
-// platform function and the engine polls what it leaves behind. The
-// language law of lesson 026 holds.
+// Lesson 030: the engine writes its own pixels. The framebuffer is our
+// bytes — Part 0's paint intuition at window size — and Present carries
+// them through the seam. Still no OS headers here; the language law of
+// lesson 026 holds.
 
 #include <cstdio>
 
+#include "framebuffer.h"
 #include "platform.h"
 
 namespace engine {
 
-constexpr int WINDOW_WIDTH = 640;
-constexpr int WINDOW_HEIGHT = 480;
-
 int Run(void)
 {
     platform::WindowResult opened =
-        platform::OpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
+        platform::OpenWindow(FRAME_WIDTH, FRAME_HEIGHT);
     if (!opened.window) {
         /* The error path: nothing was taken that the platform layer did not
            put back, and the failure is reported by name. */
@@ -36,11 +34,30 @@ int Run(void)
         return 1;
     }
 
-    std::printf("engine: window %dx%d open — waiting for news\n",
-                WINDOW_WIDTH, WINDOW_HEIGHT);
+    /* Paint: clear, then pixels — the two instincts Part 0's paint taught,
+       now onto the engine's own buffer. */
+    Framebuffer *fb = GetFramebuffer();
+    ClearBuffer(*fb, 32, 32, 64);
+    PutPixel(*fb, 0, 0, 255, 0, 0);
+    PutPixel(*fb, 639, 479, 0, 255, 0);
+    PutPixel(*fb, 700, 100, 0, 0, 255); /* out of bounds: dropped */
+
+    /* The byte-level report: what is actually in the buffer. */
+    unsigned char r, g, b;
+    std::printf("engine: framebuffer %dx%d, %d bytes, stride %d\n",
+                fb->width, fb->height, fb->width * fb->height * 4,
+                fb->width * 4);
+    GetPixel(*fb, 0, 0, r, g, b);
+    std::printf("engine: pixel (0,0) = %d %d %d\n", r, g, b);
+    GetPixel(*fb, 639, 479, r, g, b);
+    std::printf("engine: pixel (639,479) = %d %d %d\n", r, g, b);
+    GetPixel(*fb, 60, 101, r, g, b);
+    std::printf("engine: pixel (60,101) = %d %d %d\n", r, g, b);
+
+    /* First light: the bytes go to the window through the seam. */
+    platform::Present(opened.window, fb->pixels, fb->width, fb->height);
+    std::printf("engine: presented\n");
 
-    /* The event pump: read news, fold it into state, react to state,
-       repeat. This loop is what keeps the window alive. */
     while (!platform::CloseRequested(opened.window))
         platform::PumpEvents(opened.window);
 
diff --git a/src/platform.h b/src/platform.h
index da3e0f1..a2e0298 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -39,6 +39,13 @@ void PumpEvents(Window *window);
    one ending to get right. */
 bool CloseRequested(const Window *window);
 
+/* Presents the engine's framebuffer in the window: the pixels the engine
+   wrote are the pixels the window shows. The format is this interface's
+   contract, not any OS's — width * height pixels of 4 bytes each (blue,
+   green, red, one unused byte), one row after another. */
+void Present(Window *window, const unsigned char *pixels, int width,
+             int height);
+
 /* Releases everything OpenWindow took from the OS. */
 void CloseWindow(Window *window);
 
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 0438d3f..8c93ccc 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -11,6 +11,7 @@
 #include "platform.h"
 
 #include <X11/Xlib.h>
+#include <X11/Xutil.h>
 
 #include <poll.h>
 #include <signal.h>
@@ -132,6 +133,39 @@ bool CloseRequested(const Window *window)
     return window && window->close_requested;
 }
 
+void Present(Window *window, const unsigned char *pixels, int width,
+             int height)
+{
+    if (!window || !window->display)
+        return;
+
+    int screen = DefaultScreen(window->display);
+
+    /* An XImage is a *view*: XCreateImage wraps the engine's bytes without
+       copying them — the description of a pixel buffer, not the buffer.
+       It carries our bytes exactly as the seam's contract defines them. */
+    XImage *image = XCreateImage(window->display,
+                                 DefaultVisual(window->display, screen),
+                                 DefaultDepth(window->display, screen),
+                                 ZPixmap, 0, (char *)pixels,
+                                 width, height, 32, width * 4);
+    if (!image)
+        return;
+
+    /* XPutImage is where the copy happens — our bytes to the server. Its
+       cost is real; lesson 036 measures it. */
+    XPutImage(window->display, window->xwindow,
+              DefaultGC(window->display, screen),
+              image, 0, 0, 0, 0, width, height);
+
+    /* The image struct is ours to destroy; the bytes under it are the
+       engine's, so they are detached before destruction (the same rule as
+       lesson 004's ownership drills). */
+    image->data = 0;
+    XDestroyImage(image);
+    XFlush(window->display);
+}
+
 void CloseWindow(Window *window)
 {
     if (!window || !window->display)
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — FillRect, back from Part 0 *(extend-the-code)*

Paint's `FillRect` belongs here as much as `PutPixel` does. Grow the
framebuffer module with a clipped rectangle fill — and put the clip where
lesson 015's fold says it belongs: in rectangle space, before the pixel
loops. Use it to paint three blocks: one fully inside the buffer, one
crossing the left edge, one crossing the right and bottom corners. Read
pixels back to show which parts of each block landed — and watch what your
third block does to the green pixel at (639, 479).

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-030/ex1.md)

### Exercise 2 — The four bytes *(explain-in-prose)*

The report reads pixels through `GetPixel`, which translates bytes back into
colors. Make the machine visible instead: two instrumenting lines that dump
the raw four bytes at the start of pixel (0,0) and the raw four bytes at the
start of pixel (1,0) — the way lesson 013 dumped image files. Predict both
dumps before running (pixel (0,0) is red; (1,0) is the background). Then
explain, in prose, why the order in memory is blue-green-red-x and not
red-green-blue — what the window system has to do with it, what the unused
byte buys (lesson 007's arithmetic at buffer scale), and where in the code
the contract that defines it lives.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-030/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 029 — clean close and error paths: OS resources released on every exit](lesson-029-clean-close.md) ·
**Next:** [Lesson 031 — presentation through the platform layer](lesson-031-present.md) ·
**Code tag:** [`lesson-030`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-030)
