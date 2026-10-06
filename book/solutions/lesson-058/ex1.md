# Solution: exercise 1 — The row that isn't there

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 058 — the frame-budget table](../../lessons/part-2/lesson-058-budget.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The patch computes the gap — `render − (sprites + text + tilemap)` — and
prints it as a row inside render, where the other subsystem rows live:

```
engine: frame budget — 26 frames, avg 2.252 ms, worst 3.392 ms (frame 19)
engine:   subsystem   avg ms    share
engine:   update       0.023       1%
engine:   render       1.530      68%
engine:     sprites    0.001       0%
engine:     text       0.007       0%
engine:     tilemap    1.033      46%
engine:     (rest)     0.488      22%
engine:   present      0.699      31%
engine:   total        2.252     100%
```

The arithmetic closes now: `0.001 + 0.007 + 1.033 + 0.488 = 1.529`
against the render row's 1.530 (the last microsecond is printf rounding
of the averages). The `0.488 ms` — **22% of the frame** — is real time
the render phase spends on things no subsystem row names.

Where does it go? Two things, and the lesson's earlier exercises already
met both:

- **The clear.** `ClearBuffer` paints all 307,200 pixels of background
  before anything draws on it — lesson 046's exercise 1 isolated it at
  ~0.4 ms, and it is the same 0.488 ms here (the demo draws more over
  the clear, and the cache probe is gone, but the cost is the clear's).
- **The measurement itself.** Two `platform::Now()` reads per named
  phase, the loop's setup — a few microseconds of bookkeeping (lesson
  046's exercise 1 found the same residue at smaller scale).

The question the row asks — should the clear be a named subsystem? — has
a real answer on both sides:

- **Yes:** it is the biggest single per-frame pixel cost in the engine,
  it is the row Part 5's "copy less" lever would move, and a budget that
  hides 22% of the frame inside an unnamed "(rest)" is not a budget. The
  final report wants a `clear` row.
- **No, not as a *subsystem*:** the clear is not a thing the game does —
  it is a property of the drawing strategy (redraw everything, every
  frame). Naming it as a subsystem invites treating it as one. The
  honest row might be named `background` or `frame setup`, and the deep
  truth is that a dirty-rectangle renderer would make the row *disappear*
  rather than shrink it.

The course's answer for the final report (Part 5's finale): the row gets
named — `clear` — because budgets name what they spend, and the *text*
around the row is where "this is a strategy, not a feature" gets said.
An unnamed 22% is how budgets grow fiction.
