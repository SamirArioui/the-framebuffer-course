# Lesson 041 — arenas

{{#include ../../stability-horizon.md}}

## Prose

The reservation gives the engine memory; today the engine builds
**allocation** on top of it — and it is almost embarrassingly small. An
arena is one reservation and a pointer that moves forward. No free lists,
no per-allocation bookkeeping, no `free`. What it costs is a discipline,
and what it hides from the tools is the subject of the lesson's last
section.

### The bump pointer

```c++
struct Arena {
    platform::Reservation memory;
    size_t used;   /* the bump pointer: bytes handed out */
};

void *ArenaAlloc(Arena &arena, size_t bytes, size_t align);
```

Allocation is one arithmetic step: round the bump pointer up to the
alignment, hand out the bytes at that offset, move the pointer past them.
That is the entire mechanism. `align` is a power of two, so the rounding is
lesson 007's padding arithmetic done with a mask — the alignment a struct
needs, computed at allocation time.

The failure mode is the model's other half: when the request does not fit,
`ArenaAlloc` returns **0**. Not a smaller block, not a retry, not a hidden
call to some other allocator. The arena owns exactly one reservation and it
never lies about having more.

### Marks and rollback

The bump pointer only moves forward — except when the engine says "this
much of the past is over":

```c++
size_t mark = ArenaMark(arena);
... ArenaAlloc, ArenaAlloc, ArenaAlloc ...
ArenaRollback(arena, mark);
```

Rollback is not a free; it is a *forget*. Nothing was destroyed, no
metadata was updated — the cursor moved back, and the next allocation lands
where the marked one did. This is the right shape for memory with a scope:
a frame's scratch data (roll back at frame end), a level's data (roll back
at level end), an experiment (roll back when done). The cost of an
allocation is a comparison; the cost of freeing thousands of them is also
a comparison.

### The arena, measured

The engine's startup exercises the allocator the way the future will use
it — allocations with different alignments, a mark and rollback, and the
honest failure:

```
engine: arena over 65536 bytes (16 pages)
engine: alloc 100 (align 16) -> offset 0, used 100
engine: alloc 50 (align 32) -> offset 128, used 178
engine: mark at 178
engine: alloc 3 (align 1) -> offset 178, used 181
engine: rollback to 178 — used 178
engine: alloc 3 (align 1) -> offset 178, used 181
engine: exhausted: alloc 999999 -> 0 (as it should be)
```

Read the numbers: the second allocation lands at offset 128, not 100 —
`align 32` rounded the bump pointer up from 100 to the next multiple of
32. After the rollback, the same request lands at offset **178** again:
the same memory, handed out twice, at zero cost. And the ninth-of-a-megabyte
request gets the only failure the arena has.

### The limitation, taught honestly

Now the part the design insisted on teaching here rather than letting it
be discovered later: **AddressSanitizer cannot see inside an arena.**

Since lesson 005 the sanitizer has been the course's honesty machine — it
catches leaks, use-after-free, and out-of-bounds on the *heap*, because it
instruments every `malloc` and `free` and marks what is live in a shadow
memory. The arena takes part in none of that: its memory is one big
anonymous mapping (lesson 040), and an "allocation" is arithmetic the
sanitizer is never told about.

Exercise 2 is the demonstration — a use-after-rollback, run under a real
ASan build, that corrupts one allocation through another and reports
**nothing**. The bug is real; the tool is blind, on purpose, because the
model it knows is not this model.

What covers the gap:

- **The contract.** A rolled-back pointer is dead, like a `free`d one.
  The arena's discipline *is* the safety — this is the flip side of "no
  per-allocation bookkeeping".
- **Poisoning.** A debug arena can mark its free space inaccessible
  (lesson 040's exercise 2 shows the mechanism) or stamp it with a
  sentinel — the way real engines make their arenas checkable.
- **The hooks.** Sanitizers have custom-allocator interfaces for exactly
  this. Part 4's arena service can grow a debug mode that speaks to them.

Teaching the limitation here, where the arena is small enough to hold in
your head, is the difference between a tool you trust blindly and a tool
you trust *knowingly*.

### Why an arena at all

Lesson 004's pain — every `malloc` needs a `free`, and the question "who
owns the bytes" never goes away — gets a structural answer for engine
data. Game data has *scopes*: a frame, a level, a session. An arena per
scope means allocations cost nothing, frees cost nothing, and ownership is
settled by construction: the arena owns everything in it, and the scope's
end is a rollback. The framebuffer's reservation (lesson 040) and this
allocator on top (this lesson) are the two halves of "the engine owns its
memory" — Part 4 will formalize both as services.

## Code step

One change for this lesson: `arena.h` and `arena.cpp` grow the bump
allocator over a reservation — aligned allocation, marks, rollback, and
the honest 0 on exhaustion — and `main.cpp` exercises it at startup and
releases it with the rest of the run's takes. Its end state is tagged
`lesson-041`.

```diff
diff --git a/src/arena.cpp b/src/arena.cpp
new file mode 100644
index 0000000..400e5d0
--- /dev/null
+++ b/src/arena.cpp
@@ -0,0 +1,51 @@
+// arena.cpp — the arena: the bump pointer, the alignment, the marks.
+//
+// Lesson 041: allocation this simple is three pieces of arithmetic and one
+// rule — the arena never hands out memory it does not have.
+
+#include "arena.h"
+
+namespace engine {
+
+void ArenaInit(Arena &arena, size_t bytes)
+{
+    arena.memory = platform::ReserveMemory(bytes);
+    arena.used = 0;
+}
+
+void ArenaRelease(Arena &arena)
+{
+    platform::ReleaseMemory(arena.memory);
+    arena.used = 0;
+}
+
+void *ArenaAlloc(Arena &arena, size_t bytes, size_t align)
+{
+    if (!arena.memory.bytes)
+        return 0;
+
+    /* The bump pointer, rounded up to the alignment (a power of two, so
+       the mask does what lesson 007's padding did by hand). */
+    size_t base = (size_t)(arena.memory.bytes + arena.used);
+    size_t aligned = (base + align - 1) & ~(align - 1);
+    size_t start = aligned - (size_t)arena.memory.bytes;
+
+    if (start + bytes > arena.memory.size)
+        return 0; /* out of room: the honest answer */
+
+    arena.used = start + bytes;
+    return arena.memory.bytes + start;
+}
+
+size_t ArenaMark(const Arena &arena)
+{
+    return arena.used;
+}
+
+void ArenaRollback(Arena &arena, size_t mark)
+{
+    if (mark <= arena.used)
+        arena.used = mark; /* nothing is freed; the cursor just moves back */
+}
+
+} /* namespace engine */
diff --git a/src/arena.h b/src/arena.h
new file mode 100644
index 0000000..f9dff98
--- /dev/null
+++ b/src/arena.h
@@ -0,0 +1,41 @@
+// arena.h — the arena: a bump allocator over a reservation.
+//
+// Lesson 041: allocation is a pointer that moves forward (and comes back
+// on demand). No free lists, no bookkeeping per allocation — the arena is
+// one reservation and a cursor in it. Memory comes from the OS reservation
+// of lesson 040; the allocator is ours.
+#ifndef ARENA_H
+#define ARENA_H
+
+#include <stddef.h>
+
+#include "platform.h"
+
+namespace engine {
+
+struct Arena {
+    platform::Reservation memory; /* the whole reserve, from the OS */
+    size_t used;                  /* the bump pointer: bytes handed out */
+};
+
+/* An arena over an OS reservation of at least this many bytes. */
+void ArenaInit(Arena &arena, size_t bytes);
+
+/* Gives the reservation back to the OS. */
+void ArenaRelease(Arena &arena);
+
+/* Allocates from the bump pointer, aligned to align (a power of two).
+   Returns 0 when the arena has no room — the honest answer, never a
+   smaller allocation. */
+void *ArenaAlloc(Arena &arena, size_t bytes, size_t align);
+
+/* A rollback mark: where the bump pointer is now. */
+size_t ArenaMark(const Arena &arena);
+
+/* Everything allocated since the mark is gone — and the next allocation
+   lands where the marked one did. */
+void ArenaRollback(Arena &arena, size_t mark);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index c345bb4..06eba6e 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -7,6 +7,7 @@
 
 #include <cstdio>
 
+#include "arena.h"
 #include "framebuffer.h"
 #include "frame.h"
 #include "platform.h"
@@ -127,6 +128,37 @@ int Run(int argc, char **argv)
     std::printf("engine: arrow keys move the marker; close the window to stop\n");
     std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
 
+    /* The arena: bump allocation over a reservation. This report is the
+       allocator's behavior — the same numbers Part 4's services will rely
+       on. */
+    Arena arena;
+    ArenaInit(arena, 65536);
+    std::printf("engine: arena over %zu bytes (%zu pages)\n",
+                arena.memory.size, arena.memory.size / page);
+
+    void *a = ArenaAlloc(arena, 100, 16);
+    std::printf("engine: alloc 100 (align 16) -> offset %ld, used %zu\n",
+                (long)((unsigned char *)a - arena.memory.bytes), arena.used);
+    void *b = ArenaAlloc(arena, 50, 32);
+    std::printf("engine: alloc 50 (align 32) -> offset %ld, used %zu\n",
+                (long)((unsigned char *)b - arena.memory.bytes), arena.used);
+
+    size_t mark = ArenaMark(arena);
+    std::printf("engine: mark at %zu\n", mark);
+    void *c = ArenaAlloc(arena, 3, 1);
+    std::printf("engine: alloc 3 (align 1) -> offset %ld, used %zu\n",
+                (long)((unsigned char *)c - arena.memory.bytes), arena.used);
+    ArenaRollback(arena, mark);
+    std::printf("engine: rollback to %zu — used %zu\n", mark, arena.used);
+    void *d = ArenaAlloc(arena, 3, 1);
+    std::printf("engine: alloc 3 (align 1) -> offset %ld, used %zu\n",
+                (long)((unsigned char *)d - arena.memory.bytes), arena.used);
+
+    void *e = ArenaAlloc(arena, 999999, 16);
+    std::printf("engine: exhausted: alloc 999999 -> %s\n",
+                e ? "handed out (!)" : "0 (as it should be)");
+    ArenaRelease(arena);
+
     /* The frame step: read news, update from polled state, draw, present.
        This is the shape every later part fills in — Part 2 draws into it,
        Part 5 measures it. Every phase is now measured: the frame record is
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — The marker's trail *(extend-the-code)*

Give the engine a reason to allocate: the marker remembers where it has
been. Allocate a trail of eight positions from the arena at startup
(after rolling the startup experiment back to zero), record each move into
the next slot — a ring of eight, wrapping — and draw the trail behind the
marker as fading dots, oldest dimmest. Report where the trail's memory
came from (its arena offset) and what the arena's `used` is after it. In
your write-up: why is a ring *not* a rollback, and when would you use
each?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-041/ex1.md)

### Exercise 2 — The bug ASan cannot see *(explain-in-prose)*

**This exercise corrupts memory on purpose** — a deliberate teaching state,
flagged as one. Take a stale pointer: allocate, roll back, allocate again,
and write through the stale pointer onto the new block. Build with
`-fsanitize=address` the way lesson 005 taught, run it, and then write down
what the sanitizer reported and *why*: what does ASan actually track, why
is the arena's memory invisible to it, and what would catch this bug in a
real engine (name at least two defenses and their costs)?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-041/ex2.md)

---

**Part:** [Part 1 — the platform layer](../../index.md) ·
**Previous:** [Lesson 040 — reservation-backed buffers](lesson-040-reservations.md) ·
**Next:** [Lesson 042 — the interface as a contract](lesson-042-contract.md) ·
**Code tag:** [`lesson-041`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-041)
