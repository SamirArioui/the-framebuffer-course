# Lesson 038 — whole-file writes and round-trips

{{#include ../../stability-horizon.md}}

## Prose

Reads get bytes *in*; today's half of the contract gets them *out*:

> **A write creates or replaces the file with exactly the bytes given — or
> it is a typed failure.**

And the scenario that makes the word "whole" mean something on both sides
at once: **write bytes to a path, read that path back, and the bytes match.**
The round-trip is the smallest proof that the engine's data survives the
trip through the OS and back.

### The contract

```c++
FileError WriteFile(const char *path, const unsigned char *data, size_t size);
```

The read side returned bytes; the write side consumes them and returns only
the verdict. Three words in the contract do the work:

- **Creates or replaces.** A path that does not exist becomes a file; a
  path that exists comes back with exactly these bytes and nothing of what
  was there before. `O_CREAT | O_TRUNC` on this OS — the two flags *are*
  these two words.
- **Exactly the bytes given.** Not "most of them". `write()` on this OS is
  allowed to take fewer bytes than offered — a short write is a normal
  event, not an error — so the loop keeps writing until the count says the
  file has all of them. The unrolled "one `write()` call" version is a
  bug that only shows up on some files, on some days.
- **Or a typed failure.** `FILE_UNWRITABLE` covers the OS saying no — and
  the OS says no for at least three distinguishable reasons (exercise 2
  makes them visible). The seam keeps them in one typed answer until the
  engine has a reason to care which.

### The round-trip

The engine writes a 256-byte pattern — byte `i` is `i * 7`, a pattern no
compression or translation could accidentally preserve — reads the path
back through `ReadFile`, and compares, byte by byte:

```
$ DISPLAY=:99 ./build/game /tmp/opencode/xcheck/roundtrip.bin
engine: wrote /tmp/opencode/xcheck/roundtrip.bin: 256 bytes
engine: round-trip ok: 256 bytes match
```

256 bytes out, 256 bytes back, every one of them the byte that went out.
Checked from outside the engine as well — the file on disk holds exactly
the payload:

```
$ python3 -c "import sys; sys.stdout.buffer.write(bytes((i*7)&0xFF for i in range(256)))" > payload.bin
$ cmp payload.bin /tmp/opencode/xcheck/roundtrip.bin && echo "cmp: file bytes are exactly the payload"
cmp: file bytes are exactly the payload
```

Writing the same path a second time replaces it, exactly as the contract
says — a second run is another `round-trip ok` over the same file. A
"save" is this, a hundred times over, in the game's future.

### The failures

Two of the ways a write fails, both ending in the same typed answer and
the same clean exit (the window is closed on the way out — lesson 029's
rule holds on every path):

```
$ DISPLAY=:99 ./build/game /no-such-dir/out.bin
engine: /no-such-dir/out.bin: could not write
$ echo $?
1
$ DISPLAY=:99 ./build/game /dev/full
engine: /dev/full: could not write
$ echo $?
1
```

`/no-such-dir/out.bin` fails because the OS cannot create a file in a
directory that is not there. `/dev/full` is stranger and more useful: it
is a file that opens perfectly and fails every write with "no space left
on device" — the OS's own always-failing disk. It is the test that proves
the write loop's failure path does not pretend a short file is a whole one.

One honest footnote on both: a failed write can leave a partial file on
disk — the OS wrote some bytes before refusing. The engine is not misled
(it has the failure, not the data), but the leftover bytes are real.
Making writes all-or-nothing — write a temporary, then rename over the
target — is a real technique and good exercise material.

### Both halves, one rule

Reads: complete bytes or a typed failure. Writes: exactly the bytes or a
typed failure. The symmetry is the point — the platform layer's file I/O
never returns "most of a file" in either direction, and the engine never
has to guess whether it got what it asked for. Part 4's asset loading will
lean on exactly this when it starts filling arenas from files on disk.

## Code step

One change for this lesson: the seam grows `WriteFile` and
`FILE_UNWRITABLE`, the OS side implements create-or-replace with a
short-write-proof loop, and `main.cpp` runs the round-trip: a byte pattern
written to the path on the command line, read back, compared, reported. Its
end state is tagged `lesson-038`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 26aca64..fafec04 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -64,24 +64,42 @@ int Run(int argc, char **argv)
     std::printf("engine: clock %s over 100000 samples, finest step %.0f ns\n",
                 backwards ? "WENT BACKWARDS" : "never backwards", finest * 1e9);
 
-    /* Whole-file reads: the file's complete bytes, or a typed failure —
-       never partial data dressed as success. */
+    /* Whole-file writes and reads: bytes leave the engine, come back, and
+       had better be the same bytes — or the failure is the answer. */
     if (argc > 1) {
+        unsigned char payload[256];
+        for (int i = 0; i < (int)sizeof payload; ++i)
+            payload[i] = (unsigned char)(i * 7); /* a pattern, byte by byte */
+
+        platform::FileError wrote =
+            platform::WriteFile(argv[1], payload, sizeof payload);
+        if (wrote != platform::FILE_OK) {
+            std::printf("engine: %s: could not write\n", argv[1]);
+            platform::CloseWindow(opened.window);
+            return 1;
+        }
+        std::printf("engine: wrote %s: %zu bytes\n", argv[1], sizeof payload);
+
         platform::FileData file = platform::ReadFile(argv[1]);
         if (file.error != platform::FILE_OK) {
-            std::printf("engine: %s: %s\n", argv[1],
-                        file.error == platform::FILE_NOT_FOUND
-                            ? "file not found"
-                            : "unreadable");
+            std::printf("engine: %s: could not read back\n", argv[1]);
             platform::CloseWindow(opened.window);
             return 1;
         }
-        long lines = 0;
-        for (size_t i = 0; i < file.size; ++i)
-            if (file.data[i] == '\n')
-                ++lines;
-        std::printf("engine: read %s: %zu bytes, %ld lines\n", argv[1],
-                    file.size, lines);
+
+        long mismatch = -1;
+        size_t checked = file.size < sizeof payload ? file.size : sizeof payload;
+        for (size_t i = 0; i < checked; ++i)
+            if (file.data[i] != payload[i]) {
+                mismatch = (long)i;
+                break;
+            }
+        if (file.size == sizeof payload && mismatch < 0) {
+            std::printf("engine: round-trip ok: %zu bytes match\n", file.size);
+        } else {
+            std::printf("engine: round-trip FAILED: %zu bytes back (wanted %zu), first mismatch %ld\n",
+                        file.size, sizeof payload, mismatch);
+        }
         platform::ReleaseFile(file);
     }
 
diff --git a/src/platform.h b/src/platform.h
index 73ec7e5..ae9ddca 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -68,8 +68,9 @@ double Now(void);
    presented as success. */
 enum FileError {
     FILE_OK = 0,
-    FILE_NOT_FOUND,  /* nothing is there */
-    FILE_UNREADABLE, /* something is there and the OS says no */
+    FILE_NOT_FOUND,   /* nothing is there */
+    FILE_UNREADABLE,  /* something is there and the OS says no */
+    FILE_UNWRITABLE,  /* the OS refused to take the bytes */
 };
 
 struct FileData {
@@ -86,6 +87,11 @@ FileData ReadFile(const char *path);
 /* Gives the file's bytes back to the OS. */
 void ReleaseFile(FileData &file);
 
+/* Writes a whole file to the OS: the file is created, or replaced if it
+   already exists, with exactly the bytes given — or the write is a typed
+   failure. */
+FileError WriteFile(const char *path, const unsigned char *data, size_t size);
+
 /* Reads whatever news the OS has about this window and folds it into the
    platform layer's state. The engine never sees an event object — it polls
    state afterwards. Blocks until there is news or the run is interrupted. */
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index 3205c28..a569e4d 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -286,6 +286,31 @@ void ReleaseFile(FileData &file)
     file.size = 0;
 }
 
+FileError WriteFile(const char *path, const unsigned char *data, size_t size)
+{
+    /* Created, or replaced if it exists — exactly these bytes or nothing
+       the caller can mistake for success. */
+    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
+    if (fd < 0)
+        return FILE_UNWRITABLE;
+
+    /* write() may take fewer bytes than offered — a short write is not a
+       finished file. Keep going until they are all in. */
+    size_t total = 0;
+    while (total < size) {
+        ssize_t n = write(fd, data + total, size - total);
+        if (n < 0) {
+            close(fd);
+            return FILE_UNWRITABLE;
+        }
+        total += (size_t)n;
+    }
+
+    if (close(fd) < 0) /* the OS can still refuse at the very end */
+        return FILE_UNWRITABLE;
+    return FILE_OK;
+}
+
 void PumpEvents(Window *window)
 {
     if (!window || !window->display)
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The screenshot *(extend-the-code)*

Lesson 017 wrote an image file by hand in `paint`; the engine can now save
its own pixels the same way. Write a `SaveScreenshot` that builds a PPM
file in memory — the three-line header, then one RGB triple per pixel
(mind the byte order: the framebuffer is blue-green-red-x, the file is
red-green-blue) — and hands it to `WriteFile`. Take the output path as a
second command-line argument, render the scene first, and verify the file
by reading its pixels back: is the marker's color at the marker's reported
position?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-038/ex1.md)

### Exercise 2 — The device that is always full *(predict-the-output)*

Three paths, three predictions — write them down before running anything:
what does `WriteFile` answer for **`/dev/full`**, for a path in a
**directory that does not exist**, and for a path in a **directory that
refuses you** (`/usr/out.bin` works for that)? Then make the
implementation narrate — one instrumenting line at each refusal, naming
the OS's own error. Reconcile the predictions, and decide: should the seam
grow typed failures for "disk full" versus "permission denied", or is one
`FILE_UNWRITABLE` the right contract? Defend the answer.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-038/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 037 — whole-file reads](lesson-037-file-read.md) ·
**Next:** [Lesson 039 — the virtual-memory deep dive](lesson-039-virtual-memory.md) ·
**Code tag:** [`lesson-038`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-038)
