# Lesson 043 — the closing demo: platform layer done

{{#include ../../stability-horizon.md}}

## Prose

Eighteen lessons ago the repository had an empty `src/` and a promise. This
is what the promise looks like running: **one measured frame loop** — a
window kept alive, input polled, an arena-backed framebuffer presented,
every frame timed — using every part of the platform layer at once. Nothing
new is invented today; today the parts *fit*, and the fit is the point.
This is "platform layer done".

### The demo

```
$ DISPLAY=:99 ./build/game
engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames
engine: arrow keys move the marker; close the window to stop
engine: marker at 308,228
engine: marker at 548,228 (t=1.004)
engine: marker at 553,228 (t=1.045)
...
engine: marker at 616,253 (t=1.781)
frame 1: update 0.000 ms, render 0.772 ms, present 1.644 ms, total 2.416 ms
frame 2: update 0.000 ms, render 0.427 ms, present 0.313 ms, total 0.740 ms
...
engine: 41 frames — avg 0.967 ms (update 0.000, render 0.444, present 0.523)
engine: worst frame 2.416 ms (frame 1); present is 54% of the frame
engine: arena: 1228800 of 4194304 bytes used
engine: close reported
engine: closed
```

Read the run as a tour of the part:

- **The window** opens at the size the engine asked for and stays alive on
  the event pump (lessons 027-029). Closing it is reported, and the run
  ends with its resources released — the window gone, the connection
  closed, the memory back with the OS.
- **The input** moves the marker (lessons 032-034). Scripted holds moved it
  right until the clamp stopped it at the edge, then down — twenty steps,
  each one a frame that *polled* the state rather than handling events.
- **The clock** made the movement honest (lessons 035-036): the marker
  traveled at 240 pixels per second across frames that came at whatever
  rate the news came, and every frame's cost was measured into the record.
- **The memory** is the engine's own (lessons 039-041): the framebuffer's
  1,228,800 bytes came out of the arena — which is exactly what the last
  line shows — and the arena sits on one OS reservation. One allocation
  for the whole run's pixels, and one release at the end.
- **The presentation** put those pixels in the window, synchronously,
  every frame (lessons 030-031) — and it is still the single most
  expensive thing the engine does per frame, 54% of the average.

The readback check confirms what the demo claims: the marker's pixels are
in the window at the marker's reported position (`616,254 .. 638,276`
against a reported `616,253` — the marker's top-left corner). The loop is
not moving a variable; it is moving a thing you can see, at a speed it
chose, at a cost it measured.

### The shape Part 2 inherits

The loop is four moves and it will stay four moves:

```
pump  ->  update  ->  render  ->  present
news      state      our bytes   the copy
```

Part 2 replaces `DrawMarker` with a renderer and the loop does not change.
Part 5 measures the loop's phases and the record is already there. Part 3
and 4 add services that live *beside* the loop — sound, assets, entities —
and the platform layer keeps answering for the machine underneath all of
it. The frame log's format (one line, one record) is the seed those parts
grow.

### What "done" means

Done is not "finished" — the engine does nothing yet. Done is:

- the **contract** is complete (lesson 042's audit: fifteen functions, no
  OS type in the interface, engine code OS-free);
- the **scenarios** are verified (each spec requirement has a run behind
  it — exercise 2 collects them into one table);
- the **costs** are measured (the frame account, the presentation copy,
  the arena's usage — nothing about the loop is guesswork);
- and the **limits** are named (frames happen when news happens; the copy
  is the biggest cost; ASan cannot see inside the arena).

Every one of those was earned in a lesson with a run behind it. That is
what the platform layer being done means — and it is the state Part 2
starts from.

## Code step

One change for this lesson: `main.cpp` becomes the closing demo — the one
measured frame loop, with the framebuffer's memory moved into the engine's
arena (the arena owns it now; there is no separate release) and the
startup experiments retired in favor of the demo — and `framebuffer.h` /
`framebuffer.cpp` grow the arena-backed allocation. Its end state is tagged
`lesson-043`.

```diff
diff --git a/src/framebuffer.cpp b/src/framebuffer.cpp
index b8ddcfa..88080d5 100644
--- a/src/framebuffer.cpp
+++ b/src/framebuffer.cpp
@@ -3,35 +3,25 @@
 // Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
 // the window. Offset math, byte order, clipping — nothing else.
 //
-// Lesson 040: the buffer's memory comes from an OS-level reservation, not
-// from static storage and not from an allocator — whole pages, zeroed,
-// released when the engine is done with them.
+// Lesson 043: the buffer's memory comes from the engine's arena — one
+// allocation, owned like everything else in the arena, released by
+// releasing the arena.
 
 #include "framebuffer.h"
 
-#include "platform.h"
-
 namespace engine {
 
 static Framebuffer framebuffer = { 0, FRAME_WIDTH, FRAME_HEIGHT };
-static platform::Reservation storage;
 
-Framebuffer *GetFramebuffer(void)
+Framebuffer *GetFramebuffer(Arena &arena)
 {
     if (!framebuffer.pixels) {
-        storage = platform::ReserveMemory((size_t)FRAME_WIDTH *
-                                          FRAME_HEIGHT * 4);
-        framebuffer.pixels = storage.bytes;
+        framebuffer.pixels = (unsigned char *)ArenaAlloc(
+            arena, (size_t)FRAME_WIDTH * FRAME_HEIGHT * 4, 4096);
     }
     return &framebuffer;
 }
 
-void ReleaseFramebuffer(void)
-{
-    platform::ReleaseMemory(storage);
-    framebuffer.pixels = 0;
-}
-
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                  unsigned char b)
 {
diff --git a/src/framebuffer.h b/src/framebuffer.h
index 871fed7..dd09e45 100644
--- a/src/framebuffer.h
+++ b/src/framebuffer.h
@@ -6,6 +6,8 @@
 #ifndef FRAMEBUFFER_H
 #define FRAMEBUFFER_H
 
+#include "arena.h"
+
 namespace engine {
 
 /* The size the window is opened at (lesson 027) and the size of the
@@ -23,12 +25,10 @@ struct Framebuffer {
     int height;
 };
 
-/* The engine's framebuffer. Its bytes come from an OS-level reservation
-   (lesson 040): whole pages, zeroed, released with ReleaseFramebuffer. */
-Framebuffer *GetFramebuffer(void);
-
-/* Gives the framebuffer's pages back to the OS. */
-void ReleaseFramebuffer(void);
+/* The engine's framebuffer: its bytes are allocated from the caller's
+   arena (lesson 043's shape — the arena owns everything in it, and the
+   framebuffer is no exception). */
+Framebuffer *GetFramebuffer(Arena &arena);
 
 /* Fills every pixel with one color. */
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
diff --git a/src/main.cpp b/src/main.cpp
index 06eba6e..75dfaa6 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -1,9 +1,9 @@
-// main.cpp — the engine, born.
+// main.cpp — the engine: one measured frame loop.
 //
-// Lesson 030: the engine writes its own pixels. The framebuffer is our
-// bytes — Part 0's paint intuition at window size — and Present carries
-// them through the seam. Still no OS headers here; the language law of
-// lesson 026 holds.
+// Lesson 043: the closing demo. The platform layer does its whole job at
+// once — a window kept alive, input polled, an arena-backed framebuffer
+// presented, every frame measured — and this loop is the shape Part 2
+// draws into. The language law of lesson 026 still holds over all of it.
 
 #include <cstdio>
 
@@ -26,13 +26,13 @@ static void DrawMarker(Framebuffer &fb, int x, int y)
             PutPixel(fb, x + i, y + j, 240, 220, 80);
 }
 
-int Run(int argc, char **argv)
+int Run(void)
 {
     platform::WindowResult opened =
         platform::OpenWindow(FRAME_WIDTH, FRAME_HEIGHT);
     if (!opened.window) {
-        /* The error path: nothing was taken that the platform layer did not
-           put back, and the failure is reported by name. */
+        /* The error path: nothing was taken that the platform layer did
+           not put back, and the failure is reported by name. */
         switch (opened.error) {
         case platform::OPEN_NO_DISPLAY:
             std::fprintf(stderr, "engine: no display to open a window on\n");
@@ -48,121 +48,23 @@ int Run(int argc, char **argv)
         return 1;
     }
 
-    /* The clock's contract, checked before anything depends on it: the
-       readings never go backwards, and the finest step between two of them
-       is far below a frame. */
-    double prev = platform::Now();
-    double finest = 1e9;
-    int backwards = 0;
-    for (int i = 0; i < 100000; ++i) {
-        double t = platform::Now();
-        if (t < prev)
-            ++backwards;
-        else if (t > prev && t - prev < finest)
-            finest = t - prev;
-        prev = t;
-    }
-    std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
-                backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);
-
-    /* Whole-file writes and reads: bytes leave the engine, come back, and
-       had better be the same bytes — or the failure is the answer. */
-    if (argc > 1) {
-        unsigned char payload[256];
-        for (int i = 0; i < (int)sizeof payload; ++i)
-            payload[i] = (unsigned char)(i * 7); /* a pattern, byte by byte */
-
-        platform::FileError wrote =
-            platform::WriteFile(argv[1], payload, sizeof payload);
-        if (wrote != platform::FILE_OK) {
-            std::printf("engine: %s: could not write\n", argv[1]);
-            platform::CloseWindow(opened.window);
-            return 1;
-        }
-        std::printf("engine: wrote %s: %zu bytes\n", argv[1], sizeof payload);
-
-        platform::FileData file = platform::ReadFile(argv[1]);
-        if (file.error != platform::FILE_OK) {
-            std::printf("engine: %s: could not read back\n", argv[1]);
-            platform::CloseWindow(opened.window);
-            return 1;
-        }
-
-        long mismatch = -1;
-        size_t checked = file.size < sizeof payload ? file.size : sizeof payload;
-        for (size_t i = 0; i < checked; ++i)
-            if (file.data[i] != payload[i]) {
-                mismatch = (long)i;
-                break;
-            }
-        if (file.size == sizeof payload && mismatch < 0) {
-            std::printf("engine: round-trip ok: %zu bytes match\n", file.size);
-        } else {
-            std::printf("engine: round-trip FAILED: %zu bytes back (wanted %zu), first mismatch %ld\n",
-                        file.size, sizeof payload, mismatch);
-        }
-        platform::ReleaseFile(file);
-    }
+    /* The engine's memory: one arena over one reservation. Everything the
+       engine allocates lives in here and is released together. */
+    Arena arena;
+    ArenaInit(arena, 4 * 1024 * 1024);
+    Framebuffer *fb = GetFramebuffer(arena);
 
-    /* The scene: a marker the arrow keys move. The report below is its
-       position and the time it moved — the interactive frame makes itself
-       observable. */
-    Framebuffer *fb = GetFramebuffer();
     double marker_x = (FRAME_WIDTH - MARKER_SIZE) / 2.0;
     double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
     double started = platform::Now();
     double last = started;
 
-    /* The virtual-memory report: every byte the engine owns lives in a
-       mapping the OS keeps, counted in pages. Since lesson 040 the
-       framebuffer's pages are a reservation — sized in whole pages and
-       aligned like one. */
-    size_t page = platform::PageSize();
-    size_t fb_bytes = (size_t)fb->width * fb->height * 4;
-    std::printf("engine: page size %zu bytes\n", page);
-    std::printf("engine: framebuffer reserved at %p — %zu bytes = %.2f pages (page-aligned: %s)\n",
-                (void *)fb->pixels, fb_bytes,
-                (double)fb_bytes / (double)page,
-                (size_t)fb->pixels % page == 0 ? "yes" : "no");
-
+    std::printf("engine: platform layer done — window, polled input, arena-backed framebuffer, measured frames\n");
     std::printf("engine: arrow keys move the marker; close the window to stop\n");
     std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
 
-    /* The arena: bump allocation over a reservation. This report is the
-       allocator's behavior — the same numbers Part 4's services will rely
-       on. */
-    Arena arena;
-    ArenaInit(arena, 65536);
-    std::printf("engine: arena over %zu bytes (%zu pages)\n",
-                arena.memory.size, arena.memory.size / page);
-
-    void *a = ArenaAlloc(arena, 100, 16);
-    std::printf("engine: alloc 100 (align 16) -> offset %ld, used %zu\n",
-                (long)((unsigned char *)a - arena.memory.bytes), arena.used);
-    void *b = ArenaAlloc(arena, 50, 32);
-    std::printf("engine: alloc 50 (align 32) -> offset %ld, used %zu\n",
-                (long)((unsigned char *)b - arena.memory.bytes), arena.used);
-
-    size_t mark = ArenaMark(arena);
-    std::printf("engine: mark at %zu\n", mark);
-    void *c = ArenaAlloc(arena, 3, 1);
-    std::printf("engine: alloc 3 (align 1) -> offset %ld, used %zu\n",
-                (long)((unsigned char *)c - arena.memory.bytes), arena.used);
-    ArenaRollback(arena, mark);
-    std::printf("engine: rollback to %zu — used %zu\n", mark, arena.used);
-    void *d = ArenaAlloc(arena, 3, 1);
-    std::printf("engine: alloc 3 (align 1) -> offset %ld, used %zu\n",
-                (long)((unsigned char *)d - arena.memory.bytes), arena.used);
-
-    void *e = ArenaAlloc(arena, 999999, 16);
-    std::printf("engine: exhausted: alloc 999999 -> %s\n",
-                e ? "handed out (!)" : "0 (as it should be)");
-    ArenaRelease(arena);
-
-    /* The frame step: read news, update from polled state, draw, present.
-       This is the shape every later part fills in — Part 2 draws into it,
-       Part 5 measures it. Every phase is now measured: the frame record is
-       data, not guesswork. */
+    /* The frame step: read news, update from polled state, draw, present —
+       every phase measured, one record per frame. */
     int exit_code = 0;
     long frame_number = 0;
     FrameStats stats = {};
@@ -175,9 +77,7 @@ int Run(int argc, char **argv)
         frame.number = ++frame_number;
         double t0 = platform::Now();
 
-        /* Update: a frame reads state — it never handles events. The step
-           is speed × elapsed: the marker moves 240 pixels per second no
-           matter how often frames happen. */
+        /* Update: a frame reads state — it never handles events. */
         double now = platform::Now();
         double dt = now - last;
         last = now;
@@ -232,8 +132,7 @@ int Run(int argc, char **argv)
         frame.total = platform::Now() - t0;
         AccountFrame(stats, frame);
 
-        /* The frame log: one line per record. This is the format Part 2
-           grows and Part 5's frame-budget report reads. */
+        /* The frame log: one line per record — the format Part 2 grows. */
         std::printf("frame %ld: update %.3f ms, render %.3f ms, present %.3f ms, total %.3f ms\n",
                     frame.number, frame.update * 1e3, frame.render * 1e3,
                     frame.present * 1e3, frame.total * 1e3);
@@ -251,18 +150,20 @@ int Run(int argc, char **argv)
                     stats.worst * 1e3, stats.worst_number,
                     100.0 * stats.present_sum / stats.total_sum);
     }
+    std::printf("engine: arena: %zu of %zu bytes used\n", arena.used,
+                arena.memory.size);
 
     if (platform::CloseRequested(opened.window))
         std::printf("engine: close reported\n");
     platform::CloseWindow(opened.window);
-    ReleaseFramebuffer(); /* the pages go back to the OS (lesson 029's rule) */
+    ArenaRelease(arena);
     std::printf("engine: closed\n");
     return exit_code;
 }
 
 } /* namespace engine */
 
-int main(int argc, char **argv)
+int main(void)
 {
-    return engine::Run(argc, argv);
+    return engine::Run();
 }
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Platform layer done, on your machine *(port-to-your-own-machine)*

The book's numbers came from Xvfb on the authoring machine; yours should
come from your desktop. Grow the account with one line — how long the run
measured and how many frames per second that gave — then run the demo on
your own machine and compare shapes: what moved between the machines (the
present cost? the rate?), what did not (the update?), and what the
differences say about where the numbers come from. Record the machine with
the numbers; a measurement without its machine is a rumor.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-043/ex1.md)

### Exercise 2 — The acceptance table *(explain-in-prose)*

"Platform layer done" is a claim, and a claim wants a table. Make the
demo name the contract it uses — one line per group: window and
presentation, polled input, monotonic clock, whole-file I/O, reservations
and arena. Then fill the acceptance table: for every scenario in the
platform spec, the lesson that built it and the *evidence* from your own
run (the command and what it printed). Finish with the question that makes
the table worth keeping: which rows break first when the engine changes,
and how would you notice?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-043/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 042 — the interface as a contract](lesson-042-contract.md) ·
**Next:** — ·
**Code tag:** [`lesson-043`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-043)
