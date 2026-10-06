# Lesson 049 — the SIMD lens

{{#include ../../stability-horizon.md}}

## Prose

Same C++, one flag apart. Lesson 048 read the compiler's faithful `-O0`
answer to `BlitSprite`; today reads its *transformed* answer — the code
that runs in every build of this engine from `-O2` up — and through it,
**vector registers**: the machine's way of moving sixteen bytes where you
asked for one. The discipline of the dive is right in the task name: the
SIMD *lens*. This course never writes SIMD (no intrinsics, no inline
assembly in engine code); it reads what the compiler reached for, and
prices it. Writing vector code is Part 5's business, if it is anyone's —
and even there the first step is this lesson's: read the answer first.

### What a vector register is

A general-purpose register on x86-64 holds 8 bytes. Alongside them sit
the **vector registers**: `xmm0`–`xmm15`, each **16 bytes** wide; `ymm`
registers at **32 bytes** on CPUs with AVX; `zmm` at **64** with AVX-512.
The instruction set has moves and arithmetic that operate on the whole
register at once: `movdqu` ("move double quadword, unaligned") moves 16
bytes in one instruction. Lesson 048's copy needed one load and one
store *per byte*; the vectorized copy needs one load and one store *per
sixteen*.

Which width the compiler may use is a build decision, not a code one: the
baseline x86-64 target includes SSE2 (`xmm`, 16 bytes); AVX (`ymm`) is
unlocked with flags like `-march=native`. The compiler reaches for what
you have allowed it.

### The census

The code step grows `tools/disasm.sh` with a mode that counts what each
function's compiled code actually uses:

```
$ CXXFLAGS="-std=c++17 -O3 -g -Wall -Wextra" ./build.sh
$ ./tools/disasm.sh --census
  188  engine::Run()
   14  engine::AccountFrame(engine::FrameStats&, engine::FrameRecord const&)
    6  platform::Now()
    5  engine::LoadSprite(engine::Arena&, char const*)
    2  engine::CopyStrided(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  engine::CopySequential(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  engine::ArenaInit(engine::Arena&)
```

Read it with one caution the numbers themselves teach: these are
instructions that *touch vector registers* — and `xmm` is also where
ordinary floating-point lives. `AccountFrame`'s 14 are `movsd`/`addsd`
on the frame's doubles (lesson 036's sums), not data parallelism. The
census is a lens; the listing is the fact. Two names on the list are
missing entirely: **`BlitSprite` and `ClearBuffer` use zero vector
instructions.** Hold that thought; it is the honest core of this lesson.

### The vectorized copy, read

`CopySequential` — the lesson-047 walk, `dst[i] = src[i]`, nothing else —
compiled at `-O3`. Its listing is small enough to read whole; the parts
that matter:

```
    195c:	lea    -0x1(%rdx),%rdi
    1960:	cmp    $0x6,%rdi
    1964:	jbe    1a78 <engine::CopySequential(...)+0x128>
    196a:	lea    0x1(%rsi),%rax
    196e:	mov    %rcx,%r8
    1971:	sub    %rax,%r8
    1974:	xor    %eax,%eax
    1976:	cmp    $0xe,%r8
    197a:	ja     1998 <engine::CopySequential(...)+0x48>
```

First, **permission**: the compiler will not vectorize a copy it cannot
prove safe. `dst` and `src` are pointers that could overlap; a 16-byte
wide move is wrong when they do. So it *asks at runtime* — the distance
between the buffers is compared against 14, and if they are too close the
code takes the safe road:

```
    1980:	movzbl (%rsi,%rax,1),%edi
    1984:	mov    %dil,(%rcx,%rax,1)
    1988:	add    $0x1,%rax
    198c:	cmp    %rax,%rdx
    198f:	jne    1980 <engine::CopySequential(...)+0x30>
```

— the byte-at-a-time loop, one pixel of lesson 048's anatomy, kept for
the copies the wide path cannot take. Then the road it prefers:

```
    19a5:	and    $0xfffffffffffffff0,%rdi
    ...
    19b0:	movdqu (%rsi,%rax,1),%xmm0
    19b5:	movups %xmm0,(%rcx,%rax,1)
    19b9:	add    $0x10,%rax
    19bd:	cmp    %rdi,%rax
    19c0:	jne    19b0 <engine::CopySequential(...)+0x60>
```

Read those six lines against lesson 048's seventy-six. The `and` rounds
the byte count down to a multiple of 16 — the loop below moves exactly
16 bytes per turn. `movdqu (%rsi,%rax,1),%xmm0` loads sixteen bytes into
one register; `movups` stores them on the other side; `add $0x10` steps
by sixteen. **Two memory instructions do what thirty-two did in the
`-O0` listing.** What the round-down left over (0–15 bytes) is handled by
the tails after it — an 8-byte `mov` pair, then bytes — because the
vector path only covers complete registers.

That is the SIMD lens in one function: a guard, a scalar fallback, a wide
loop, and tails. Every auto-vectorized loop has this shape, and once you
can name the four parts you can read any compiler's answer to "copy this
memory".

### What the compiler did *not* vectorize

Now the missing names. `BlitSprite`'s inner loop is this, at `-O3`:

```
    1772:	movzbl 0x2(%rax),%r13d
    1777:	cmp    %r14b,(%rax)
    177a:	jne    1720 <engine::BlitSprite(...)+0x80>
    177c:	movzbl 0x11(%rsi),%r14d
    1781:	cmp    %r14b,0x1(%rax)
    1785:	jne    1720 <engine::BlitSprite(...)+0x80>
```

Registers instead of stack slots (lesson 048's spill storm is gone — the
`× 3` folded into `lea (%rax,%rax,2)`, the `× 4` into a scaled index),
but the loop is still **scalar**: compare, branch, per pixel. The key
check is the reason, and it is a *good* reason: what would "vectorized
transparency" even mean? The decision is per-pixel and data-dependent —
each pixel either writes or does not. Compilers can in principle emit
masked moves for such loops, and GCC declines here. The census says `0`;
the listing says why.

`ClearBuffer` fares the same: four byte stores per pixel even at `-O3`,
because a pixel is four *different* bytes (blue, green, red, zero) and
the loop is a pattern fill, not a copy. The compiler has no wide move for
"this exact four-byte pattern, repeated" at these settings.

The full map of outcomes a loop can meet — keep it next to the census:

| Loop | Outcome | Evidence |
| ---- | ------- | -------- |
| `CopySequential` | **vectorized** — `movdqu`, 16 B/instruction | 2 census hits, the wide listing |
| `CopyStrided` | barely vectorized | 2 hits — the stride costs the width |
| `LoadSprite` | partially vectorized | 5 hits — wide copies plus scalar tails |
| `BlitSprite` | scalar, register-allocated | 0 hits; branches per pixel |
| `ClearBuffer` | scalar | 0 hits; four byte stores per pixel |

### What the code generation is worth

Everything above changed *no source line*, and the numbers moved
accordingly (all measured on this machine, the sprite bench from lesson
048's exercise 1 still in the tree):

| Measurement | `-O0` | `-O3` | Change |
| ----------- | ----- | ----- | ------ |
| sprite draw, per sprite | 881.9 ns | 187.7 ns | **4.7×** |
| frame `render` phase (avg) | 0.435 ms | 0.173 ms | 2.5× |
| copy walk, 4 KB row | 1.5 GB/s | 102.5 GB/s | **68×** |
| copy walk, 12 MB row | 1.5 GB/s | 16.9 GB/s | 11× |

The 68× is the vectorized copy's width plus the instruction count
falling away; the 4.7× of the sprite draw is *not* SIMD (the blit stayed
scalar) — it is register allocation and folded arithmetic, lesson 048's
stack traffic gone. Two different causes, one table: that is why the
frame record's named phases (lesson 046) and these dives belong together.
The `sprites` phase still reads `0.001 ms` in both builds — the honest
floor of a 16×16 sprite against a microsecond-resolution clock; the bench
is what prices it.

And the flag that widens the registers: with
`CXXFLAGS="-std=c++17 -O3 -march=native …"`, the copy's loop becomes
`vmovdqu (%rsi,%rdi,1),%ymm0` — **32 bytes per instruction** — and the
4 KB row measures 169.6 GB/s against 102.5. Same C++; the compiler was
allowed more of the machine's registers and used them.

### The lens, and where it points

Reading SIMD is a Part 2 skill; *writing* it is not a Part 2 job. The
engine's code keeps its one-copy-loop honesty, and the deep dives keep
their promise: measure, read, name the revisit point. Part 5's revisit is
concrete — the profiler's top-2 hotspots will very likely include
something in this lesson's table, and the three levers are already named:
copy less, copy closer together, copy wider. The third lever is the one
you can now read before anyone writes it.

## Code step

One change for this lesson: the cache probe's copy walkers lose their
`static` — the compiler must emit each as a named function so the lens
has a listing to read — and `tools/disasm.sh` grows the `--census` mode
(vector-register instructions per function). No engine behavior changes.
Its end state is tagged `lesson-049`.

```diff
diff --git a/src/main.cpp b/src/main.cpp
index 18747c7..887d551 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -25,17 +25,18 @@ constexpr double SPRITE_SPEED = 240.0; /* pixels per second */
    memory at two strides, timed at working-set sizes that cross this
    machine's caches. The walk is the blit's inner copy with the
    bookkeeping removed: source bytes into destination bytes, nothing
-   else — so what it costs is what the blitter's copy costs. */
+   else — so what it costs is what the blitter's copy costs.
+   Lesson 049: the walkers lose their `static` so the compiler must emit
+   each one as a named function — the SIMD lens needs a listing to read. */
 
-static void CopySequential(unsigned char *dst, const unsigned char *src,
-                           size_t n)
+void CopySequential(unsigned char *dst, const unsigned char *src, size_t n)
 {
     for (size_t i = 0; i < n; ++i)
         dst[i] = src[i];
 }
 
-static void CopyStrided(unsigned char *dst, const unsigned char *src,
-                        size_t n, size_t stride)
+void CopyStrided(unsigned char *dst, const unsigned char *src, size_t n,
+                 size_t stride)
 {
     for (size_t i = 0; i < n; i += stride)
         dst[i] = src[i];
diff --git a/tools/disasm.sh b/tools/disasm.sh
index f54a09c..794c8b5 100755
--- a/tools/disasm.sh
+++ b/tools/disasm.sh
@@ -10,6 +10,7 @@
 # Usage:
 #   ./tools/disasm.sh                 # the blitter (BlitSprite)
 #   ./tools/disasm.sh ClearBuffer     # any other symbol by substring
+#   ./tools/disasm.sh --census        # vector-register instructions per function
 #
 # build/ must be current: run ./build.sh first. To read the optimized
 # build's instructions (lesson 049), build with the flags first:
@@ -28,5 +29,23 @@ fi
 
 # Only definition lines start at column 0 with an address and end in ':' —
 # call sites name the symbol too, and those are not what we are reading.
-objdump -d -C --no-show-raw-insn build/game |
-    sed -n "/^[0-9a-f]* <.*${SYMBOL}.*>:/,/^\$/p"
+listing() {
+    objdump -d -C --no-show-raw-insn build/game |
+        sed -n "/^[0-9a-f]* <.*${1}.*>:/,/^\$/p"
+}
+
+if [ "$SYMBOL" = "--census" ]; then
+    # The SIMD lens: how many vector-register instructions (xmm/ymm/zmm)
+    # each function's compiled code actually uses.
+    objdump -d -C --no-show-raw-insn build/game |
+        awk '/^[0-9a-f]+ <.*>:$/ { name = $0
+                                  sub(/^[0-9a-f]+ </, "", name)
+                                  sub(/>:$/, "", name)
+                                  next }
+             /%[xyz]mm/ { count[name]++ }
+             END { for (n in count) printf "%5d  %s\n", count[n], n }' |
+        sort -rn
+    exit 0
+fi
+
+listing "$SYMBOL"
```

## Exercises

Two "make it yours" extensions. Each ends with its solution — a diff against
this lesson's end state plus a walkthrough — after the prompt.

### Exercise 1 — Two fills, predicted *(predict-the-output)*

The lesson's table has a fifth outcome it only teased. Add two named
functions to the probe file: `FillPattern`, a loop that writes four
*different* bytes per pixel (like `ClearBuffer`'s), and `FillUniform`, a
loop that writes one byte value everywhere. Before you run anything,
write down what the compiler will do with each — vectorized, scalar, or
something else — and why. Then run the census and read both listings, and
reconcile: one of your two functions does not come back as a loop at all.

> **Solution:** [ex1 — diff + walkthrough](../../solutions/lesson-049/ex1.md)

### Exercise 2 — Your compiler's register width *(port-to-your-own-machine)*

The census counts vector instructions; make it also report the **width**
of the widest register each function touches — 16 bytes for `xmm`, 32
for `ymm`, 64 for `zmm` — so a listing's reach is visible at a glance.
Then run it twice on your machine: the default `-O3`, and
`CXXFLAGS="-std=c++17 -O3 -march=native -g -Wall -Wextra" ./build.sh`.
Predict first which functions will widen and what the 4 KB probe row will
do; then measure. Which numbers moved with the register width, and which
did not?

> **Solution:** [ex2 — diff + walkthrough](../../solutions/lesson-049/ex2.md)

---

**Part:** [Part 2 — software rendering](../../index.md) ·
**Previous:** [Lesson 048 — the blitter's compiled assembly](lesson-048-assembly.md) ·
**Next:** [Lesson 050 — the bitmap font as an asset](lesson-050-font.md) ·
**Code tag:** [`lesson-049`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-049)
