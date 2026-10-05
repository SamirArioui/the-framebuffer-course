# Solution: exercise 3 — What the diff saves

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 022 — the double-buffered character grid](../../lessons/part-0/lesson-022-double-buffer.md).*

## The diff

```diff
{{#include ex3.patch}}
```

## Walkthrough

`SNEK_FULL=1` makes the flush treat every cell as changed — the same grid,
redrawn wholesale each frame. On one machine, `./snek 60 2>/dev/null | wc -c`
reported 6 949 bytes through the diff and 396 007 bytes with the full
redraw: 57× more traffic for the same picture. At 120 frames the gap grows —
7 289 versus 792 007 bytes, 109× — because the diff's cost is nearly
constant (one full draw, then a trickle of changed cells) while the full
redraw's cost is linear in frames. Your ratios will differ with the frame
count; the shape will not.

Two costs are hiding in those numbers. Bandwidth is the obvious one: the
full redraw pushes roughly 6 600 bytes of escape sequences per frame, which
is fine over a pty and painful over SSH. The subtler one is *flicker*: a
whole-screen rewrite briefly shows half-drawn frames, and the eye reads that
as shimmer — the same reason film-era terminals and game consoles double
buffer. The diff flush never erases what is already correct, so only real
changes are ever visible moving. `getenv` is the smallest possible switch:
one comparison per flush, no command-line plumbing.
