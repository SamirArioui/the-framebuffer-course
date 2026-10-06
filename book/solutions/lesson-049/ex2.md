# Solution: exercise 2 — Your compiler's register width

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 049 — the SIMD lens](../../lessons/part-2/lesson-049-simd.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch adds the width column: for every function the census lists, the
widest vector register its code touches — 16 bytes for `xmm`, 32 for
`ymm`, 64 for `zmm`. The prediction before running anything: at the
baseline x86-64 target every function should read **16 B** (SSE2 is
guaranteed; AVX is not), and with `-march=native` on this machine's CPU
the copy walkers and the arithmetic-heavy functions should widen to
**32 B** — while anything that barely uses vector registers (the clock,
the arena) has no reason to widen at all.

The default `-O3` build:

```
$ ./tools/disasm.sh --census
  188  16 B  engine::Run()
   14  16 B  engine::AccountFrame(engine::FrameStats&, engine::FrameRecord const&)
    6  16 B  platform::Now()
    5  16 B  engine::LoadSprite(engine::Arena&, char const*)
    2  16 B  engine::CopyStrided(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  16 B  engine::CopySequential(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  16 B  engine::ArenaInit(engine::Arena&)
```

Every width 16 B, as predicted: the portable target may assume SSE2 and
nothing more. With `-march=native` — one flag, telling the compiler this
binary may use everything *this* CPU has:

```
  207  32 B  engine::Run()
   11  32 B  engine::AccountFrame(engine::FrameStats&, engine::FrameRecord const&)
    6  32 B  engine::LoadSprite(engine::Arena&, char const*)
    5  16 B  platform::Now()
    4  32 B  engine::CopyStrided(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    4  32 B  engine::CopySequential(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  16 B  engine::ArenaInit(engine::Arena&)
```

The copies widened (their listing now shows
`vmovdqu (%rsi,%rdi,1),%ymm0` — 32 bytes per move — with `xmm` tails
for the 16-byte remainder), the counts moved (AVX lowering makes
different choices — `Run`'s 188 became 207), and `platform::Now` and
`ArenaInit` stayed at 16 B because their handful of scalar double
operations gain nothing from wider registers.

Now the measurements — the 4 KB row of the cache probe, the working set
that fits in L1d and is therefore bounded by instruction throughput
rather than memory:

| Build | 4 KB row | 12 MB row |
| ----- | -------- | --------- |
| `-O3` | 102.5 GB/s | 16.9 GB/s |
| `-O3 -march=native` | **169.6 GB/s** | 16.2 GB/s |

The L1-resident row moved by two-thirds — the wide registers halve the
instruction count of the copy's inner loop and the core retires more
bytes per cycle. The 12 MB row barely moved: that row is main memory's
speed (lesson 047's curve), and no register width defeats physics. This
is the shape of every "SIMD made it faster" claim you will ever read:
**vector width pays where the work is compute- or instruction-bound, and
not where it is memory-bound** — which is why lesson 047's locality
lessons come first.

One warning the exercise earns the hard way: `-march=native` bakes *this*
CPU's instruction set into the binary. It is the right flag for a probe
you run on your own machine and the wrong flag for a game you ship to
someone else's — the portable `-O3` column is the one that runs
everywhere. Record both columns with the machine that produced them;
they are different products, not a better and a worse one.
