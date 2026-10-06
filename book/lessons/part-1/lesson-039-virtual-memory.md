# Lesson 039 — the virtual-memory deep dive

{{#include ../../stability-horizon.md}}

## Prose

The framebuffer is 1,228,800 bytes of the engine's, and lesson 038's file
reads took bytes from the OS into memory the OS gave us. Both of those
statements hide a machine: *where do bytes live, and who decides?* This is
the course's virtual-memory deep dive — pages, mappings, protection —
taught concept-first and then measured on x86-64, the reference machine.
It sits here on purpose: lesson 040's reservations and lesson 041's arenas
are built out of exactly these parts.

### Pages

Physical memory is a pile of bytes; a running program does not see it. What
the program sees is a **virtual address space**: another, larger, tidier
pile of addresses that belongs to it alone. Between the two sits the
**page table**, and its unit is the **page** — one fixed-size chunk of
address space mapped to one fixed-size chunk of physical memory.

The page size is a machine fact, not a convention. On this machine:

```
$ getconf PAGESIZE
4096
```

and the engine measures the same number through the seam
(`platform::PageSize()`):

```
engine: page size 4096 bytes
```

4 KiB pages on x86-64: the virtual address is 48 bits wide (128 TiB per
process), and the low 12 bits of any address are the offset *within* its
page. The upper bits are what the page table translates. x86-64 does that
translation through four levels of table — the address is cut into four
9-bit indices plus the 12-bit offset — and the CPU keeps the recent results
in a cache called the **TLB**, because four memory lookups per access would
otherwise be the price of every byte touched. (x86-64 also has 2 MiB and
1 GiB *huge pages*: the same mechanism with the bottom levels skipped. The
engine will not need them; knowing they exist explains why "page size" is
a question and not always "4096".)

### Mappings

A process's address space is not one blob — it is a **table of mappings**,
each a range of virtual pages with a source and a set of permissions. The
kernel keeps that table, and on Linux it prints it: `/proc/self/maps` is
the map of the process reading it. Here is the engine's, verbatim, from a
real run (the top of it):

```
5923fe127000-5923fe128000 r--p 00000000 08:30 397590   .../build/game
5923fe128000-5923fe12b000 r-xp 00001000 08:30 397590   .../build/game
5923fe12b000-5923fe12c000 r--p 00004000 08:30 397590   .../build/game
5923fe12c000-5923fe12d000 r--p 00004000 08:30 397590   .../build/game
5923fe12d000-5923fe12e000 rw-p 00005000 08:30 397590   .../build/game
5923fe12e000-5923fe25a000 rw-p 00000000 00:00 0
59242e643000-59242e664000 rw-p 00000000 00:00 0        [heap]
```

Each line is one mapping: the **range** of virtual addresses (the end
exclusive), the **permissions**, the **offset** into the file behind it,
the file's **device and inode**, and its **path**. The five lines naming
`build/game` are the executable's own segments — one mapping per part of
one file. The lines with no path are **anonymous** mappings: memory that
backs nothing but itself. Two kinds of mapping, one table.

The engine's framebuffer lives in this table like everything else. Its
address in the run above was `0x5923fe12d040` — inside the fifth line, and
spilling into the sixth. The engine's own report says why the spilling
happens:

```
engine: page size 4096 bytes
engine: framebuffer at 0x5923fe12d040 — 1228800 bytes = 300.00 pages (page-aligned: no)
```

Exactly 300.00 pages of bytes — `640 × 480 × 4` divides evenly into 4 KiB
pages — but the buffer starts 64 bytes into its first page, so it *touches*
301 pages (exercise 1 prints the range). Mappings count pages; bytes do not
care. When lesson 040 sizes buffers in whole pages, this is why.

### Protection

The permissions on each mapping — `r--`, `r-x`, `rw-p` — are not advisory.
They are bits in the page-table entries, checked by the hardware on every
access: **read**, **write**, **execute**. Read the executable's middle line
again: `r-xp` is its code, readable and executable but never writable —
and a mapping that tried to be both writable and executable is the classic
weak point hardened systems refuse.

Touch a page in a way its permissions forbid — write to the code segment,
read a `---` page — and the hardware stops the instruction and the OS
delivers **SIGSEGV**. This is not a language-level error and no check in
C++ catches it; lesson 006 showed undefined behavior from the language's
side, this is the machine's own refusal. (The crash is worth seeing with
your own eyes — exercise 2 of this batch has a safe place to do it.)

The `p` in `rw-p` is the other half of protection: **private**, meaning
copy-on-write. Two processes can map the same file's pages read-only and
share the physical frames; the first to write gets its own copy. It is how
one `libc.so.6` can sit in memory once for every process on the machine.

### Why this is not an interlude

The engine is about to stop taking whatever memory the loader happened to
give it:

- **Lesson 040** asks the OS for memory the way this lesson says memory is
  actually gotten: a *reservation* — an anonymous mapping of a size the
  engine chooses, in whole pages, with permissions chosen on purpose. The
  framebuffer will move into one.
- **Lesson 041** builds an arena on top: a bump pointer over a reservation,
  with alignment and rollback. The map will show one large anonymous
  mapping where before there were statics and library allocations — and
  now you can read what that means.

Both are vocabulary from this lesson. That is why the deep dive sits here
and not in an appendix.

## Code step

One change for this lesson: the seam grows `PageSize`, and the engine
prints its virtual-memory report — the page size and the framebuffer's
address, size in pages, and alignment — measured from the process's real
address space. Its end state is tagged `lesson-039`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index fafec04..0eedfe1 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -111,6 +111,18 @@ int Run(int argc, char **argv)
     double marker_y = (FRAME_HEIGHT - MARKER_SIZE) / 2.0;
     double started = platform::Now();
     double last = started;
+
+    /* The virtual-memory report: every byte the engine owns lives in a
+       mapping the OS keeps, counted in pages. This is what the deep dive
+       explains — the numbers here are measured, not illustrative. */
+    size_t page = platform::PageSize();
+    size_t fb_bytes = (size_t)fb->width * fb->height * 4;
+    std::printf("engine: page size %zu bytes\n", page);
+    std::printf("engine: framebuffer at %p — %zu bytes = %.2f pages (page-aligned: %s)\n",
+                (void *)fb->pixels, fb_bytes,
+                (double)fb_bytes / (double)page,
+                (size_t)fb->pixels % page == 0 ? "yes" : "no");
+
     std::printf("engine: arrow keys move the marker; close the window to stop\n");
     std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
 
diff --git a/src/platform.h b/src/platform.h
index ae9ddca..1c9bf6c 100644
--- a/src/platform.h
+++ b/src/platform.h
@@ -64,6 +64,9 @@ bool HasFocus(const Window *window);
    clock Part 2's frame timing and Part 5's frame-budget report stand on. */
 double Now(void);
 
+/* The OS's memory page: the unit every mapping is counted in. */
+size_t PageSize(void);
+
 /* A file's complete bytes — or a typed failure. Never partial data
    presented as success. */
 enum FileError {
diff --git a/src/platform_x11.cpp b/src/platform_x11.cpp
index a569e4d..58c6339 100644
--- a/src/platform_x11.cpp
+++ b/src/platform_x11.cpp
@@ -221,6 +221,11 @@ double Now(void)
     return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
 }
 
+size_t PageSize(void)
+{
+    return (size_t)sysconf(_SC_PAGESIZE);
+}
+
 /* File I/O is the OS side too — POSIX here, Win32's own calls in a second
    implementation. The bytes the OS reads for us live in memory the OS
    gives us (its allocator) and leave through ReleaseFile. */
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Find your mapping *(explain-in-prose)*

The report gives you the framebuffer's address; the machine gives you its
map. Grow the report with the *page range* the buffer touches — the
address rounded down to a page boundary and the last byte's page likewise —
and then, while the engine runs, find that range in `cat /proc/<pid>/maps`.
Explain, in prose, every column of the line (or lines) that hold it: the
range, the permissions, the offset, the device and inode, the path. Why
does 300.00 pages of data touch 301 pages of address space? And why do the
map's addresses differ from run to run?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-039/ex1.md)

### Exercise 2 — The file that lies about its size *(fix-the-crash)*

Lesson 037's whole-file reader trusts the size the OS reports — and some
files lie. `/proc/self/maps` reports **0 bytes** and produces kilobytes
when read; our reader returns 0 bytes *presented as success*, the exact
outcome the contract forbids. Prove it (point the engine at the map file),
then fix the reader so whole-file means whole file even when the size is a
hint: read to the end of the file, growing the buffer as needed — lesson
008's `realloc` has been waiting for this job. Show the fixed reader
returning the map's real bytes, and write down what "complete bytes" now
means for a file whose size changes as you read it.

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-039/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 038 — whole-file writes and round-trips](lesson-038-file-write.md) ·
**Next:** [Lesson 040 — reservation-backed buffers](lesson-040-reservations.md) ·
**Code tag:** [`lesson-039`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-039)
