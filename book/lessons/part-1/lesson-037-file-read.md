# Lesson 037 — whole-file reads

{{#include ../../stability-horizon.md}}

## Prose

The engine can see, move, and measure — and it cannot yet *load*. Every
asset the game will ever need is a file, and the contract for getting one
into memory is the platform layer's narrowest promise:

> **A read returns a file's complete bytes or a typed failure — never
> partial data presented as success.**

Part 0 read files with `fopen`/`fread` inside the sandbox programs
(wordcount's first lesson). The engine reads **against the OS**: the OS's
own open-read-close calls, behind the seam, where a second OS substitutes
its own.

### The contract

```c++
enum FileError {
    FILE_OK = 0,
    FILE_NOT_FOUND,
    FILE_UNREADABLE,
};

struct FileData {
    const unsigned char *data;
    size_t size;
    FileError error;
};

FileData ReadFile(const char *path);
void ReleaseFile(FileData &file);
```

Three things to read in it:

- **Complete or failed.** There is no third outcome. A read that ends
  early, a file that changes under the read, a path that is not a file —
  all of them are failures, and none of them leave partial bytes lying
  around pretending to be a file.
- **Failure is typed by what the caller can act on.** `FILE_NOT_FOUND`
  ("nothing is there") and `FILE_UNREADABLE` ("something is there and the
  OS says no") are the two cases an engine reacts to differently — try
  another path, or report a broken install. The OS has dozens of error
  numbers; the seam has the ones with meanings.
- **The bytes belong to the caller.** On success the platform has taken
  memory from the OS to hold the file and hands it over; `ReleaseFile`
  gives it back. Lesson 004's ownership rule in its plainest form: you
  release what you took, through the same door you took it.

### The OS side

The implementation is the file API of the OS — `open`, `fstat`, `read`,
`close` — and the shape of the code is dictated by the contract:

1. **Open.** A missing path is `FILE_NOT_FOUND` (the one OS error worth
   naming); anything else the OS refuses is `FILE_UNREADABLE`.
2. **Take the size first.** `fstat` says how big the file is *before* the
   first byte is read — and what kind of thing the path is. A directory
   opens happily on this OS and reads as nothing; a character device like
   `/dev/zero` has no size at all. Both fail the "is it a regular file"
   check and become `FILE_UNREADABLE` — before any memory is taken.
3. **Read exactly that much, and check.** The loop fills the buffer until
   the size is reached; then one more byte is attempted and must find
   nothing. A file that ends early fails. A file that *grew* under the
   read fails. The buffer never holds "most of a file".

The engine's demo takes a path on the command line — lesson 001's `argv`,
now through `main(int argc, char **argv)` — and reports what it got:

```
$ DISPLAY=:99 ./build/game README.md
engine: read README.md: 6113 bytes, 143 lines
```

The complete bytes: 6113 of them, counted by the engine from the buffer it
was handed, along with the line count wordcount would recognize.

### The failures, as written

```
$ DISPLAY=:99 ./build/game no-such-file.txt
engine: no-such-file.txt: file not found
$ echo $?
1
$ DISPLAY=:99 ./build/game src
engine: src: unreadable
$ DISPLAY=:99 ./build/game /dev/zero
engine: /dev/zero: unreadable
```

Each one exits non-zero, and each one leaves **nothing** behind: the error
path closes the window it opened (lesson 029's rule — every exit releases
its takes, and an exit with a typed failure is an exit) and no partial data
reached the engine.

`/dev/zero` deserves its stare. It is "a file" that reads successfully
forever — the naive whole-file loop never returns from it. The contract
does not chase it: the size comes first, the size is zero, the
regular-file check fails, and the answer is a typed failure in constant
time. Whole-file means *the file's* size, not "until the stream stops".

An empty file, for completeness, is success — 0 complete bytes:

```
$ DISPLAY=:99 ./build/game /tmp/opencode/xcheck/empty.txt
engine: read /tmp/opencode/xcheck/empty.txt: 0 bytes, 0 lines
```

## Code step

One change for this lesson: the seam grows `FileError`, `FileData`,
`ReadFile`, and `ReleaseFile`, the OS side implements whole-file reads with
open/fstat/read/close and the two typed failure reasons, and `main.cpp`
takes a path from `argv`, reads it whole, and reports — or reports the
typed failure and exits clean. Its end state is tagged `lesson-037`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 942d825..26aca64 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -25,7 +25,7 @@ static void DrawMarker(Framebuffer &fb, int x, int y)
             PutPixel(fb, x + i, y + j, 240, 220, 80);
 }
 
-int Run(void)
+int Run(int argc, char **argv)
 {
     platform::WindowResult opened =
         platform::OpenWindow(FRAME_WIDTH, FRAME_HEIGHT);
@@ -64,6 +64,27 @@ int Run(void)
     std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
                 backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);
 
+    /* Whole-file reads: the file's complete bytes, or a typed failure —
+       never partial data dressed as success. */
+    if (argc > 1) {
+        platform::FileData file = platform::ReadFile(argv[1]);
+        if (file.error != platform::FILE_OK) {
+            std::printf("engine: %s: %s\n", argv[1],
+                        file.error == platform::FILE_NOT_FOUND
+                            ? "file not found"
+                            : "unreadable");
+            platform::CloseWindow(opened.window);
+            return 1;
+        }
+        long lines = 0;
+        for (size_t i = 0; i < file.size; ++i)
+            if (file.data[i] == '\n')
+                ++lines;
+        std::printf("engine: read %s: %zu bytes, %ld lines\n", argv[1],
+                    file.size, lines);
+        platform::ReleaseFile(file);
+    }
+
     /* The scene: a marker the arrow keys move. The report below is its
        position and the time it moved — the interactive frame makes itself
        observable. */
@@ -177,7 +198,7 @@ int Run(void)
 
 } /* namespace engine */
 
-int main(void)
+int main(int argc, char **argv)
 {
-    return engine::Run();
+    return engine::Run(argc, argv);
 }
diff --git a/src/platform.h b/src/platform.h
index cea5fc6..73ec7e5 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -7,6 +7,8 @@
 #ifndef PLATFORM_H
 #define PLATFORM_H
 
+#include <stddef.h> /* size_t */
+
 namespace platform {
 
 /* What a window is made of is the OS implementation's business. The engine
@@ -62,6 +64,28 @@ bool HasFocus(const Window *window);
    clock Part 2's frame timing and Part 5's frame-budget report stand on. */
 double Now(void);
 
+/* A file's complete bytes — or a typed failure. Never partial data
+   presented as success. */
+enum FileError {
+    FILE_OK = 0,
+    FILE_NOT_FOUND,  /* nothing is there */
+    FILE_UNREADABLE, /* something is there and the OS says no */
+};
+
+struct FileData {
+    const unsigned char *data; /* the file's complete bytes, or 0 */
+    size_t size;
+    FileError error;
+};
+
+/* Reads a whole file from the OS. On success the bytes are the file —
+   all of it — and they belong to the caller: give them back with
+   ReleaseFile. */
+FileData ReadFile(const char *path);
+
+/* Gives the file's bytes back to the OS. */
+void ReleaseFile(FileData &file);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index d35cf08..3205c28 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -11,6 +11,10 @@
 // Lesson 035: the platform clock. POSIX, not ISO C — clock_gettime is the
 // OS's clock interface (the one lesson 020 taught inside snek, now behind
 // the seam), so the feature-test macro goes before the includes.
+//
+// Lesson 037: whole-file reads. File I/O is OS surface too — POSIX here,
+// a second OS's own calls there. Everything in this file is one
+// implementation behind the seam.
 #define _POSIX_C_SOURCE 200809L
 
 #include "platform.h"
@@ -20,9 +24,14 @@
 #include <X11/Xutil.h>
 #include <X11/keysym.h>
 
+#include <errno.h>
+#include <fcntl.h>
 #include <poll.h>
 #include <signal.h>
+#include <stdlib.h>
+#include <sys/stat.h>
 #include <time.h>
+#include <unistd.h>
 
 namespace platform {
 
@@ -212,6 +221,71 @@ double Now(void)
     return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
 }
 
+/* File I/O is the OS side too — POSIX here, Win32's own calls in a second
+   implementation. The bytes the OS reads for us live in memory the OS
+   gives us (its allocator) and leave through ReleaseFile. */
+FileData ReadFile(const char *path)
+{
+    FileData file = { 0, 0, FILE_UNREADABLE };
+
+    int fd = open(path, O_RDONLY);
+    if (fd < 0) {
+        file.error = (errno == ENOENT) ? FILE_NOT_FOUND : FILE_UNREADABLE;
+        return file;
+    }
+
+    /* Whole file means whole file: the size is known before the first
+       byte, and a read that ends early is a failure, not a smaller file. */
+    struct stat st;
+    if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)) {
+        close(fd);
+        return file; /* still FILE_UNREADABLE */
+    }
+
+    size_t capacity = (size_t)st.st_size;
+    unsigned char *bytes =
+        (unsigned char *)malloc(capacity ? capacity : 1);
+    if (!bytes) {
+        close(fd);
+        return file;
+    }
+
+    size_t total = 0;
+    while (total < capacity) {
+        ssize_t n = read(fd, bytes + total, capacity - total);
+        if (n < 0) {
+            free(bytes);
+            close(fd);
+            return file;
+        }
+        if (n == 0)
+            break; /* the file ended early — checked below */
+        total += (size_t)n;
+    }
+
+    /* One byte past what the size promised must find nothing, or the file
+       changed under the read — and a moving file is not a whole file. */
+    unsigned char extra;
+    if (total != capacity || read(fd, &extra, 1) != 0) {
+        free(bytes);
+        close(fd);
+        return file;
+    }
+
+    close(fd);
+    file.data = bytes;
+    file.size = total;
+    file.error = FILE_OK;
+    return file;
+}
+
+void ReleaseFile(FileData &file)
+{
+    free((void *)file.data);
+    file.data = 0;
+    file.size = 0;
+}
+
 void PumpEvents(Window *window)
 {
     if (!window || !window->display)
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Read into your own memory *(extend-the-code)*

`ReadFile` hands out bytes the platform allocated and you must release.
When the engine already knows where its memory should come from — an arena,
a static buffer, a scratch pad — it wants the bytes *there*. Grow the seam
with `ReadFileInto(path, into, capacity)`: the file's complete bytes in the
caller's buffer, or a typed failure when it does not fit. Nothing is
allocated; nothing is released. Show both paths working in one run — and
show the "does not fit" failure with a file bigger than your buffer.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-037/ex1.md)

### Exercise 2 — The file that never ends *(predict-the-output)*

Three paths, three predictions — write them down before running anything:
what does `ReadFile` answer for a **directory**, for **`/dev/zero`**, and
for a **path that does not exist**? Then make the implementation narrate —
one instrumenting line at each refusal, naming the OS's own error — and run
all three. Reconcile the predictions with what the OS said, and explain why
`/dev/zero` is the case that tells you what "whole file" really means.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-037/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 036 — frame time as measured data](lesson-036-frame-time.md) ·
**Next:** [Lesson 038 — whole-file writes and round-trips](lesson-038-file-write.md) ·
**Code tag:** [`lesson-037`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-037)
