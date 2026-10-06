# Lesson 040 — reservation-backed buffers

{{#include ../../stability-horizon.md}}

## Prose

The framebuffer has been living where the loader happened to leave room —
static storage in the executable's own data mapping. Today the engine stops
accepting placement and starts *asking*: **a reservation** — whole pages of
memory taken from the OS on purpose, sized by the engine, owned by the
engine, released when the engine is done. It is lesson 039's mappings
vocabulary turned into an API, and it is where every large buffer in the
engine will live from here on.

### The contract

```c++
struct Reservation {
    unsigned char *bytes;
    size_t size;   /* whole pages */
    MemoryError error;
};

Reservation ReserveMemory(size_t bytes);
void ReleaseMemory(Reservation &reservation);
```

Three properties, each one a decision:

- **Whole pages.** You ask in bytes and you get in pages — one byte is one
  page (exercise 1 measures this). The map is counted in pages, the
  page table translates pages, and a reservation is a *mapping*: pretending
  otherwise would just hide the rounding.
- **Zeroed.** The bytes are zero before anything writes them. Not because
  the OS zeroes memory for hygiene theater, but because of the next point.
- **Backed lazily.** The reservation reserves *address space*; physical
  page frames arrive only when the bytes are first touched — the
  demand-zero behavior lesson 039 named. Taking a 100 MB reservation is
  nearly free; writing 100 MB is not.

Failure is a value, as everywhere in the seam: `MEMORY_NO_MEMORY` if the
OS refuses. And the release is an ownership rule, lesson 029's in its
plainest form: `ReleaseMemory` gives the pages back and zeroes the struct
— released once, safe to call again, nothing left pointing at memory the
OS has reclaimed.

### The OS side

One call is the whole implementation on this OS:

```c++
mmap(0, rounded, PROT_READ | PROT_WRITE,
     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
```

Read it in lesson 039's words: an **anonymous** mapping (no file behind
it) with read/write **protection**, private to this process, of `rounded`
whole pages, placed wherever the OS has room (the `0`). `munmap` is its
release. On a second OS the same contract is that OS's own mapping call —
the seam keeps the word *reservation* and the implementation keeps the
idiom.

### The framebuffer, moved

`GetFramebuffer` now reserves instead of pointing into the executable:

```c++
storage = platform::ReserveMemory((size_t)FRAME_WIDTH * FRAME_HEIGHT * 4);
```

and the memory report from lesson 039 shows the difference immediately:

```
engine: page size 4096 bytes
engine: framebuffer reserved at 0x7566dd4d4000 — 1228800 bytes = 300.00 pages (page-aligned: yes)
```

**Page-aligned: yes** — it was "no" as static storage, when the buffer
started 64 bytes into a page because that is where the linker put it. A
reservation starts where a page starts, because it *is* pages. The same
300.00 pages of data now fit in exactly 300 pages of address space.

The map agrees, and the buffer's mapping is now its own row with its own
size:

```
7566dd4d4000-7566dd600000 rw-p 00000000 00:00 0   -> 1228800 bytes
```

One anonymous read/write mapping of exactly the framebuffer's size — no
file, no name, no shared owner. In lesson 041 this row is where the arena
will live.

### What demand-zero looks like from the outside

The lazy backing is measurable. A small probe (reserve 1.2 MB, watch the
process's resident set as it goes):

```
zeroprobe: rss before 1280 kB, after reserve 1408 kB, after reading 3 bytes 1408 kB, after writing all 2688 kB
zeroprobe: first three bytes read: 0 0 0 (demand-zero: 0 0 0)
```

Taking the reservation and reading bytes from it cost no physical memory
worth the name — the frames were never there. Writing every byte cost the
full 1.2 MB, all at once, in zeros-then-data. (The small step at reserve
time is the OS's own bookkeeping for the mapping, not the pages.) This is
why a reservation is the right home for *large* buffers: the engine can
reserve for what it might need and only pay for what it touches.

### Released on the way out

The framebuffer's pages go back with the rest of the run's takes:

```c++
platform::CloseWindow(opened.window);
ReleaseFramebuffer();
```

Lesson 029's rule, now covering memory: every exit releases what it took.
The OS would reclaim at process death anyway — and a rule you can check
beats a backstop you can only hope for.

## Code step

One change for this lesson: the seam grows `Reservation`, `ReserveMemory`,
and `ReleaseMemory` (anonymous whole-page mappings behind the interface),
the framebuffer moves from static storage into a reservation and is
released at the end of the run, and the memory report names the
reservation. Its end state is tagged `lesson-040`.

```diff
diff --git a/src/framebuffer.cpp b/src/framebuffer.cpp
index 7e6fe13..b8ddcfa 100644
--- a/src/framebuffer.cpp
+++ b/src/framebuffer.cpp
@@ -2,20 +2,36 @@
 //
 // Lesson 030: the same arithmetic Part 0's paint did, on a buffer sized for
 // the window. Offset math, byte order, clipping — nothing else.
+//
+// Lesson 040: the buffer's memory comes from an OS-level reservation, not
+// from static storage and not from an allocator — whole pages, zeroed,
+// released when the engine is done with them.
 
 #include "framebuffer.h"
 
+#include "platform.h"
+
 namespace engine {
 
-/* One screenful of pixels, in static storage. */
-static unsigned char pixels[FRAME_WIDTH * FRAME_HEIGHT * 4];
-static Framebuffer framebuffer = { pixels, FRAME_WIDTH, FRAME_HEIGHT };
+static Framebuffer framebuffer = { 0, FRAME_WIDTH, FRAME_HEIGHT };
+static platform::Reservation storage;
 
 Framebuffer *GetFramebuffer(void)
 {
+    if (!framebuffer.pixels) {
+        storage = platform::ReserveMemory((size_t)FRAME_WIDTH *
+                                          FRAME_HEIGHT * 4);
+        framebuffer.pixels = storage.bytes;
+    }
     return &framebuffer;
 }
 
+void ReleaseFramebuffer(void)
+{
+    platform::ReleaseMemory(storage);
+    framebuffer.pixels = 0;
+}
+
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                  unsigned char b)
 {
diff --git a/src/framebuffer.h b/src/framebuffer.h
index 1f38723..871fed7 100644
--- a/src/framebuffer.h
+++ b/src/framebuffer.h
@@ -23,11 +23,13 @@ struct Framebuffer {
     int height;
 };
 
-/* The engine's framebuffer. Its bytes live in static storage — the
-   language law of lesson 026 keeps allocation out of the engine, and
-   lesson 040 gives buffers like this a real home. */
+/* The engine's framebuffer. Its bytes come from an OS-level reservation
+   (lesson 040): whole pages, zeroed, released with ReleaseFramebuffer. */
 Framebuffer *GetFramebuffer(void);
 
+/* Gives the framebuffer's pages back to the OS. */
+void ReleaseFramebuffer(void);
+
 /* Fills every pixel with one color. */
 void ClearBuffer(Framebuffer &fb, unsigned char r, unsigned char g,
                  unsigned char b);
diff --git a/src/main.cpp b/src/main.cpp
index 0eedfe1..c345bb4 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -113,12 +113,13 @@ int Run(int argc, char **argv)
     double last = started;
 
     /* The virtual-memory report: every byte the engine owns lives in a
-       mapping the OS keeps, counted in pages. This is what the deep dive
-       explains — the numbers here are measured, not illustrative. */
+       mapping the OS keeps, counted in pages. Since lesson 040 the
+       framebuffer's pages are a reservation — sized in whole pages and
+       aligned like one. */
     size_t page = platform::PageSize();
     size_t fb_bytes = (size_t)fb->width * fb->height * 4;
     std::printf("engine: page size %zu bytes\n", page);
-    std::printf("engine: framebuffer at %p — %zu bytes = %.2f pages (page-aligned: %s)\n",
+    std::printf("engine: framebuffer reserved at %p — %zu bytes = %.2f pages (page-aligned: %s)\n",
                 (void *)fb->pixels, fb_bytes,
                 (double)fb_bytes / (double)page,
                 (size_t)fb->pixels % page == 0 ? "yes" : "no");
@@ -222,6 +223,7 @@ int Run(int argc, char **argv)
     if (platform::CloseRequested(opened.window))
         std::printf("engine: close reported\n");
     platform::CloseWindow(opened.window);
+    ReleaseFramebuffer(); /* the pages go back to the OS (lesson 029's rule) */
     std::printf("engine: closed\n");
     return exit_code;
 }
diff --git a/src/platform.h b/src/platform.h
index 1c9bf6c..29e8dd9 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -67,6 +67,26 @@ double Now(void);
 /* The OS's memory page: the unit every mapping is counted in. */
 size_t PageSize(void);
 
+/* A memory reservation: whole pages of OS memory, zeroed, owned by the
+   engine until it releases them. This is where engine buffers come from
+   (lesson 040) — not from an allocator. */
+enum MemoryError {
+    MEMORY_OK = 0,
+    MEMORY_NO_MEMORY, /* the OS refused the reservation */
+};
+
+struct Reservation {
+    unsigned char *bytes; /* the reserved bytes, or 0 */
+    size_t size;          /* whole pages */
+    MemoryError error;
+};
+
+/* Reserves at least this many bytes from the OS, rounded to whole pages. */
+Reservation ReserveMemory(size_t bytes);
+
+/* Gives the reservation back to the OS. */
+void ReleaseMemory(Reservation &reservation);
+
 /* A file's complete bytes — or a typed failure. Never partial data
    presented as success. */
 enum FileError {
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 58c6339..17dae4e 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -29,6 +29,7 @@
 #include <poll.h>
 #include <signal.h>
 #include <stdlib.h>
+#include <sys/mman.h>
 #include <sys/stat.h>
 #include <time.h>
 #include <unistd.h>
@@ -226,6 +227,38 @@ size_t PageSize(void)
     return (size_t)sysconf(_SC_PAGESIZE);
 }
 
+/* Reservations are anonymous mappings (lesson 039's vocabulary): whole
+   pages of virtual memory, zeroed by the OS, with no file behind them.
+   Physical frames arrive only when the bytes are touched — the demand-zero
+   behavior the deep dive described. */
+Reservation ReserveMemory(size_t bytes)
+{
+    Reservation reservation = { 0, 0, MEMORY_NO_MEMORY };
+
+    size_t page = PageSize();
+    size_t rounded = (bytes + page - 1) / page * page;
+    if (rounded == 0)
+        rounded = page;
+
+    void *mapping = mmap(0, rounded, PROT_READ | PROT_WRITE,
+                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
+    if (mapping == MAP_FAILED)
+        return reservation;
+
+    reservation.bytes = (unsigned char *)mapping;
+    reservation.size = rounded;
+    reservation.error = MEMORY_OK;
+    return reservation;
+}
+
+void ReleaseMemory(Reservation &reservation)
+{
+    if (reservation.bytes)
+        munmap(reservation.bytes, reservation.size);
+    reservation.bytes = 0;
+    reservation.size = 0;
+}
+
 /* File I/O is the OS side too — POSIX here, Win32's own calls in a second
    implementation. The bytes the OS reads for us live in memory the OS
    gives us (its allocator) and leave through ReleaseFile. */
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — One byte, please *(predict-the-output)*

Two predictions before running anything: what does `ReserveMemory(1)`
report as the reservation's **size**, and what are the **first bytes** of
memory that nothing has written? Add the smallest experiment to the
engine's startup — reserve one byte, report its size, its address, and its
first three bytes — and reconcile. What does the address tell you that the
size alone does not?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-040/ex1.md)

### Exercise 2 — The page that fights back *(explain-in-prose)*

**This exercise crashes the engine on purpose, once** — that is its whole
point, and the patch is flagged as a deliberate teaching state. Grow the
seam with `MakeInaccessible(bytes, size)` — behind it, `mprotect` with
`PROT_NONE` — reserve a small buffer, mark it inaccessible, and touch it.
Watch the run die, then explain, in machine terms, every step from the
`mov` to the `Segmentation fault`: what the page table said, what the CPU
did about it, what the OS did next, and why no C++ construct could have
caught it. (Print your last words to `stderr` — a crash takes `stdout`'s
buffer with it.)

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-040/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 039 — the virtual-memory deep dive](lesson-039-virtual-memory.md) ·
**Next:** [Lesson 041 — arenas](lesson-041-arenas.md) ·
**Code tag:** [`lesson-040`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-040)
