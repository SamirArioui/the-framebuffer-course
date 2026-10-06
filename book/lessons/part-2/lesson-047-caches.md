# Lesson 047 — the caches deep dive

{{#include ../../stability-horizon.md}}

## Prose

The frame account said `render 0.435 ms` and, inside it, `sprites 0.001
ms`. One microsecond to draw a sprite; four hundred to paint the
background. This lesson answers the question that arithmetic asks:
**what does copying a byte actually cost?** The answer is not in the C++
standard and not in the x86-64 manual either — it is in the small, fast
memories sitting between the CPU and the slow, large one. This is the
*caches deep dive*: concept first, x86-64 as the worked example (the
machine this course targets; every concept here is the machine's, not the
instruction set's), measured on our own copy loop, and ending where every
deep dive in this course ends — with what Part 5 will do about it.

### Memory is not bytes: it is cache lines

The CPU never fetches one byte. It fetches a **cache line** — on this
machine, 64 bytes, and you can ask the machine directly:

```
$ for i in /sys/devices/system/cpu/cpu0/cache/index*; do
>   echo "$i: L$(cat $i/level) $(cat $i/type) $(cat $i/size) line=$(cat $i/coherency_line_size)"
> done
/sys/devices/system/cpu/cpu0/cache/index0: L1 Data 48K line=64
/sys/devices/system/cpu/cpu0/cache/index1: L1 Instruction 64K line=64
/sys/devices/system/cpu/cpu0/cache/index2: L2 Unified 3072K line=64
/sys/devices/system/cpu/cpu0/cache/index3: L3 Unified 20480K line=64
```

Four facts of this machine, straight from the kernel: a 48 KB data cache
closest to the core (**L1d**), a 3 MB shared-but-close cache (**L2**), a
20 MB last-level cache (**L3**), and 64-byte lines everywhere. Between
those and main memory is a factor of fifty in latency; between the tiers
it is two to four times each. The caches are not an optimization detail —
they are *the memory system*, and RAM is the slow backing store they hide.

A line fetch is the unit of every memory cost. Read one byte and the
machine fetches 64; touch the next byte and it is already there. Touch a
byte 64 bytes away and the machine fetches a *second* line. The difference
between those two sentences is the difference between the columns of every
table in this lesson.

Put the frame's own numbers in lines: one framebuffer row is `640 × 4 =
2560` bytes = 40 lines; one row of the sprite is `16 × 3 = 48` bytes —
inside a single line. The sprite's whole 768 bytes are twelve lines. The
framebuffer is 19,200 lines and cannot fit in any cache on this machine.
Those two sentences predict almost everything the measurements below show.

### The probe: the blit's copy, measured

The code step adds a copy walk — source bytes into destination bytes,
nothing else. It is `BlitSprite`'s inner copy with the key check and the
byte swap removed, in two shapes: **sequential** (`dst[i] = src[i]` for
every byte) and **stride 64** (`dst[i] = src[i]` for every 64th byte —
one useful byte per cache line). Each is timed over working sets that
cross the caches above, and the table reports *useful* GB/s — bytes the
walk actually consumed, not bytes the machine fetched.

Built at `-O3` (one flag changed; the next two lessons are about why that
matters), pinned to one core (`taskset -c 3`), on the machine whose caches
are printed above:

```
engine: cache probe — copy walk, useful GB/s per stride
engine: working set   sequential    stride 64
engine:       4 KB        102.5          3.0
engine:      64 KB         59.0          1.1
engine:    512 KB         52.1          0.9
engine:   4096 KB         14.7          0.4
engine:   8192 KB         16.3          0.3
engine:  12288 KB         16.9          0.3
```

Read the sequential column top to bottom and you are watching the
hierarchy (one detail the table wears openly: the row label is the bytes
*copied per buffer* — the walk touches source and destination, so the
memory it stands on is twice the label; and a run's digits move a few
percent between runs, while the drops are the claim):

- **4 KB: 102 GB/s** — the whole working set lives in L1d. The copy runs
  at core speed; memory is not involved after the first line.
- **64 KB – 512 KB: ~55 GB/s** — past L1d (48 KB), still inside L2.
  Two-and-a-half times slower, and stable: every line is a short hop away.
- **4 MB – 12 MB: ~15 GB/s** — the working set (two buffers, so 2× the
  row's size) has outgrown L2 (3 MB) and then L3 (20 MB). This is main
  memory's speed: seven times slower than L1d, and where every big scene
  eventually lives.

The stride 64 column is the same machine asked the wrong way: 3.0 GB/s in
L1d down to 0.3 GB/s in memory — **thirty times slower** at the same
working set. Not because the strides are slow, but because each touch
brings in 64 bytes and uses 1. The table reports useful bytes, so the
waste is exactly visible: a 64-byte stride in the memory regime moves
0.3 useful GB/s while the machine fetches 64 × 0.3 ≈ 19 GB/s of cache
lines doing it. Sequential and strided touch the same lines; sequential
gets 64 useful bytes per fetch.

That is the whole concept: **cost is per line fetched, and locality is
how many useful bytes you get per line.** Everything else is detail.

### The loop is also a cost

The same probe, built at the default `-O0` — the build this course ships
— shows something the `-O3` table hides:

```
engine: cache probe — copy walk, useful GB/s per stride
engine: working set   sequential    stride 64
engine:       4 KB          1.3          0.5
engine:      64 KB          1.4          0.6
engine:    512 KB          1.5          0.6
engine:   4096 KB          1.5          0.4
engine:   8192 KB          1.6          0.3
engine:  12288 KB          1.5          0.2
```

Flat at ~1.5 GB/s. Not because the memory system changed — because at
`-O0` the copy loop is one byte per *instruction sequence*: load a byte,
store a byte, bump the index, compare, jump. The CPU spends its time on
the loop, not on memory; the caches are never asked hard enough to show a
knee. A copy has two layers of cost — the code that runs and the memory it
touches — and at `-O0` the first layer is the whole story.

This is not a flaw to fix here (no optimization lands in Part 2 — the
fixing is Part 5's three-pass menu). It is the thread the next two lessons
pull: lesson 048 reads what the compiler actually made of the loop, and
lesson 049 reads the vectorized form it reaches for when allowed. The
`-O0`/`-O3` pair above is the same code — the difference is entirely in
what the compiler emitted.

### What the blitter's copy actually costs

Back to the frame. Per frame the engine moves:

| Path | Bytes | Where |
| ---- | ----- | ----- |
| `ClearBuffer` | 1,228,800 written | framebuffer — 19,200 lines, cold every frame |
| `BlitSprite` | 768 read + ≤1,024 written | sprite (12 lines, always hot) into the framebuffer |
| `Present` | 1,228,800 read | framebuffer into the window |

About 2.4 MB of traffic per frame — at 60 frames per second, roughly
150 MB/s against a machine whose memory system moves 15,000 MB/s and whose
L1d moves 100,000 MB/s. At 640×480 with one sprite, **the caches are not
this frame's problem**, and the account's `sprites 0.001 ms` says so: the
sprite's twelve lines are resident after the clear touches them.

But watch where the pressure would come from. The clear is one pass over a
working set that fits L2 but not L1d — it runs at L2 speed (the 512 KB
row's neighborhood: tens of GB/s) minus the loop's `-O0` overhead, which
is exactly why the account reads 0.4 ms and not 0.04. A scene with fifty
sprites would multiply the blit path's bytes by fifty — still small. A
scene with a *map* being redrawn every frame, or a framebuffer four times
the size, moves the working set down the curve — and Part 5's frame-budget
report is where that shows up as numbers instead of fears.

### And here is what Part 5 will do about it

The dive stops at measurement, as dives here do. The named hooks waiting
in Part 5:

- **The profiler pass re-runs these numbers** on the finished game's real
  scene — same probe, same columns — and names the top-2 hotspots before
  anything is fixed (the three-pass menu's step 1).
- **The frame record's named phases** (lesson 046's `sprites`, and the
  text and tilemap phases to come) attribute the cost to subsystems; the
  cache curve explains the shape of each row.
- **The fixes in step 2 come from this table's rows**: copy less (dirty
  regions instead of full clears), copy closer together (locality), copy
  wider (vectorization — next lesson). Each is one of the three levers
  this dive named; Part 5 pulls them with measurements attached.

## Code step

One change for this lesson: `main.cpp` grows the cache probe — two copy
loops (`CopySequential`, `CopyStrided`), the sweep over working sets and
strides, and the arena grown to 32 MB so the probe can out-size every
cache the machine has. The probe runs once at startup, before the frame
loop. Its end state is tagged `lesson-047`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 1f8ad5c..18747c7 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -21,6 +21,72 @@ namespace engine {
    per-frame step. */
 constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
 
+/* Lesson 047: the caches deep dive's evidence — a copy walk over arena
+   memory at two strides, timed at working-set sizes that cross this
+   machine's caches. The walk is the blit's inner copy with the
+   bookkeeping removed: source bytes into destination bytes, nothing
+   else — so what it costs is what the blitter's copy costs. */
+
+static void CopySequential(unsigned char *dst, const unsigned char *src,
+                           size_t n)
+{
+    for (size_t i = 0; i < n; ++i)
+        dst[i] = src[i];
+}
+
+static void CopyStrided(unsigned char *dst, const unsigned char *src,
+                        size_t n, size_t stride)
+{
+    for (size_t i = 0; i < n; i += stride)
+        dst[i] = src[i];
+}
+
+static void CacheProbe(Arena &arena)
+{
+    const size_t sizes[] = { 4096, 65536, 524288, 4194304, 8388608,
+                             12582912 };
+    const size_t biggest = 12582912;
+
+    /* Two allocations the compiler cannot connect: source and destination
+       are separate arena blocks — 2 × size bytes of working set per walk,
+       and the copy loop is free to run wide. */
+    unsigned char *src = (unsigned char *)ArenaAlloc(arena, biggest, 64);
+    unsigned char *dst = (unsigned char *)ArenaAlloc(arena, biggest, 64);
+    if (!src || !dst) {
+        std::printf("engine: cache probe: no room in the arena\n");
+        return;
+    }
+    for (size_t i = 0; i < biggest; i += 4096) {
+        src[i] = (unsigned char)(i * 7); /* touch every page first */
+        dst[i] = 0;
+    }
+
+    std::printf("engine: cache probe — copy walk, useful GB/s per stride\n");
+    std::printf("engine: %10s %12s %12s\n", "working set", "sequential",
+                "stride 64");
+    for (unsigned s = 0; s < sizeof sizes / sizeof sizes[0]; ++s) {
+        double gbs[2];
+        for (unsigned mode = 0; mode < 2; ++mode) {
+            size_t stride = mode == 0 ? 1 : 64;
+            size_t per_rep = sizes[s] / stride;
+            long reps = (long)(8000000 / per_rep);
+            if (reps < 1)
+                reps = 1;
+            double t0 = platform::Now();
+            for (long r = 0; r < reps; ++r) {
+                if (mode == 0)
+                    CopySequential(dst, src, sizes[s]);
+                else
+                    CopyStrided(dst, src, sizes[s], 64);
+            }
+            double seconds = platform::Now() - t0;
+            gbs[mode] = (double)per_rep * reps / seconds / 1e9;
+        }
+        std::printf("engine: %7zu KB %12.1f %12.1f\n", sizes[s] / 1024,
+                    gbs[0], gbs[1]);
+    }
+}
+
 int Run(void)
 {
     platform::WindowResult opened =
@@ -44,9 +110,11 @@ int Run(void)
     }
 
     /* The engine's memory: one arena over one reservation. Everything the
-       engine allocates lives in here and is released together. */
+       engine allocates lives in here and is released together. Lesson 047
+       grows it past every cache this machine has, so the cache probe can
+       walk working sets bigger than all of them. */
     Arena arena;
-    ArenaInit(arena, 4 * 1024 * 1024);
+    ArenaInit(arena, 32 * 1024 * 1024);
     Framebuffer *fb = GetFramebuffer(arena);
 
     /* Lesson 044: the sprite is a file's bytes. It is loaded once, at
@@ -152,6 +220,9 @@ int Run(void)
     std::printf("engine: blit check: clip at -4,-4 landed %d pixels, %d wrong, %d touched outside\n",
                 landed, wrong, wrapped);
 
+    /* Lesson 047's evidence, measured before the loop starts. */
+    CacheProbe(arena);
+
     double sprite_x = (FRAME_WIDTH - sprite.width) / 2.0;
     double sprite_y = (FRAME_HEIGHT - sprite.height) / 2.0;
     double started = platform::Now();
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Your machine's knees *(measure-the-performance)*

Six rows show the shape; a machine deserves its own map. Extend the sweep
between the lesson's sizes — add 16 KB, 48 KB, 128 KB, 1 MB, 2 MB — and
run at `-O3` on your own machine. Before you run, predict where the
bandwidth should drop, using your CPU's cache sizes (the `/sys` command
from the lesson works on any Linux). Then compare: where are the knees you
measured, where were the ones you predicted, and what does the *distance*
between a knee and your prediction say about what else touches your
memory?

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-047/ex1.md)

### Exercise 2 — The stride that doesn't fit the line *(predict-the-output)*

Add a third column to the probe: **stride 60**. Before you run anything,
write down where you expect it to land between stride 64 and sequential at
each working set — the line is 64 bytes and 60 does not divide into it
cleanly, so think about how many touches land in each line before you
commit to a number. Then run at `-O3` and reconcile the column with your
prediction. The question the numbers ask: is it the *stride value* that
costs, or the *useful bytes per line*?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-047/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 046 — the sprite moves](lesson-046-movable-sprite.md) ·
**Next:** [Lesson 048 — the blitter's compiled assembly](lesson-048-assembly.md) ·
**Code tag:** [`lesson-047`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-047)
