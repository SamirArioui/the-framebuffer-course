# Solution: exercise 1 — The instruction budget

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 048 — the blitter's compiled assembly](../../lessons/part-2/lesson-048-assembly.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The count first, straight off the listing the lesson walks. The inner
loop's copy path, section by section:

| Section | From | To | Instructions |
| ------- | ---- | -- | ------------ |
| source address (`((j−y)*width + (i−x)) * 3`) | `17e6` | `181a` | 18 |
| key check (three loads, three compares) | `181e` | `185d` | 20 |
| the copy (destination `× 4`, four stores) | `185f` | `18ce` | 34 |
| increment and test (`++i`, `i < right`) | `18d1` | `18db` | 4 |
| **per opaque pixel** | | | **76** |

The skip path takes the address math and key check, then the `nop` at
`18d0` and the same increment and test: about **43**. Our sprite is 130
opaque and 126 key-colored, so one draw retires roughly
`130 × 76 + 126 × 43 ≈ 15,300` instructions.

The naive prediction: one instruction per cycle at 3.26 GHz (0.31 ns)
gives `76 × 0.31 ≈ 23 ns` per opaque pixel — and a sprite around 6 µs.
The patch measures it with ten thousand timed draws:

```
engine: blit bench: 10000 draws in 8.819 ms — 881.9 ns per sprite, 3.45 ns per pixel
```

**3.45 ns per pixel**, not 23. The prediction is off by a factor of
almost seven, and where the factor lives is the whole lesson: the
retirement core of a modern CPU executes *several* instructions per
cycle when they are independent and simple. Per sprite the arithmetic is
blunt — 15,300 instructions in 882 ns is 2,875 cycles at 3.26 GHz, so
this code retires about **5.3 instructions per cycle**. Not bad for code
the compiler was told not to think about; the loads and stores all hit
L1 (the sprite is 12 cache lines and the destination is hot from the
clear), so nothing in the loop waits on memory.

What the prediction got right and wrong is worth keeping:

- **Right:** instruction count predicts *ordering*. The copy path is
  ~1.8× the skip path's instructions, and opaque pixels really do cost
  about that much more than key pixels.
- **Wrong:** instructions are not cycles. At `-O0` the compiler's
  schedule is arbitrary (one statement, one run of instructions), but the
  *hardware* still runs ahead — loads are issued early, the adds overlap.
  A back-of-envelope instruction count is a floor on work, not a stopwatch.

The second half of the answer is what `-O0` costs in *kind*, not count:
every variable round-trips through `-0xNN(%rbp)`, so the loop is full of
loads and stores that an optimizing build would keep in registers. That
is exactly the axis lesson 049 turns — same C++, `-O3`, and a listing
where one instruction does what twenty did here.
