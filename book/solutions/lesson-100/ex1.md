# Solution: exercise 1 — the price of a pixel

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 100 — pass 2b: fix the clear](../../lessons/part-5/lesson-100-clear.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The bench is lesson 048's sprite bench turned on the clear: three
sizes over 200 clears each, timed on the real buffer at startup, so
the price per pixel shows up apart from everything else the frame
does. And because the files it touches (`report.*`, `main.cpp`) are
untouched by this lesson's code step, the same patch applies at
`lesson-099` — the before number is measured by the *same*
instrument, not borrowed from the lesson's table.

At `lesson-099` (the byte-store loop):

```
engine: bench: clear 640x480 — 1.2 ns/pixel over 200 clears
engine: bench: clear 320x240 — 1.2 ns/pixel over 200 clears
engine: bench: clear 160x120 — 1.3 ns/pixel over 200 clears
```

At `lesson-100` (the word fill):

```
engine: bench: clear 640x480 — 0.5 ns/pixel over 200 clears
engine: bench: clear 320x240 — 0.5 ns/pixel over 200 clears
engine: bench: clear 160x120 — 0.5 ns/pixel over 200 clears
```

**1.2 → 0.5 ns per pixel: a 2.4× fall**, flat across sizes (the
per-pixel price is the loop's, not the buffer's).

Now the decomposition the exercise asks for. The instruction-count
prediction was: four byte stores plus an address computation per pixel
become one word store — a factor of about 2.5–3 at `-O0` once the
remaining loop bookkeeping is counted. Measured: 2.4×. So:

- **The store count owns the fall** — four stores and the per-pixel
  address math gone is the bulk of the 0.7 ns/pixel saved.
- **The `-O0` loop bookkeeping owns what remains** — the index
  compare, the increment, the stack traffic the unoptimized compiler
  keeps: the `0.5 ns/pixel` floor here. The lesson's census shows what
  a real build does with the same loop — eight pixels per wide-store
  turn — so at `-O3` this number falls again, and the *shape* of the
  fix is what survives the flag, not the millisecond.

One nuance worth carrying into every micro-bench you ever write: these
numbers are **hot-cache** — 200 consecutive clears leave the buffer in
the cache between runs — while the frame account's `clear` row
measures the clear against a buffer the frame just drew over and the
seam just read (cold-ish). That is why the bench reads `0.5` where the
frame's row implies `0.78 ns/pixel` after the fix (and `1.2` vs
`1.48` before). Neither number is wrong; a bench prices the loop, the
frame prices the game. When you quote one, name the other.
