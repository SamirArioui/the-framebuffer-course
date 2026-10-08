# Solution: exercise 1 — the 60 fps line on your machine

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 101 — pass 3: the frame-budget report](../../lessons/part-5/lesson-101-frame-budget.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The account grows a histogram — 65 longs of its own memory, buckets
of `0.25 ms` out to the budget's neighborhood, the last bucket an
overflow pail — and `AccountFrame` bins every frame as it is recorded.
The percentiles are read off it by walking the cumulative count: the
bucket where the median frame sits, where the 95th sits, where the
99th sits. No allocation (the language law's habit), no sort, no
retained frames.

This machine's line, from a real six-leg run of the finished game
(1,551 frames):

```
engine:   by state    1415 play frames at 1.256 ms, 136 screen frames at 0.718 ms
engine:   budget      60 fps is 16.667 ms a frame — 0 of 1551 frames over it, worst 2.659 ms (16% of it)
engine:   percentiles p50 1.250 ms, p95 1.750 ms, p99 2.000 ms (0.25 ms buckets)
engine:   machine     WSL2, Xvfb :99, no sound hardware (the course's authoring machine)
```

Read the tail, not the average: the *median* frame is `1.250 ms`, the
worst one percent of frames still under `2.000 ms`, against a
`16.667 ms` budget. The average (`1.225–1.3 ms`) sat between the
median and the p95 — which is exactly why the average is not the
instrument for a frame-rate claim: **60 fps is a statement about the
tail.** A machine averaging 2 ms with a p99 of 40 ms drops frames
visibly; a machine averaging 4 ms with a p99 of 6 never does.

Now the port — and what your report must contain to answer the
checklist's line honestly:

1. **Your machine's name where ours is** — the `RUN_MACHINE` string in
   `main.cpp` is yours to edit. A number that lost its machine is a
   rumor.
2. **Your run's shape, named** — a window on your desktop changes two
   of our numbers structurally: the `present` row (a local display
   costs the *process* CPU where our X server charged it as wait) and
   the frame *rate* (with a sound device, the audio horizon paces your
   loop at ~60 feeds a second instead of our jiggles' 25).
3. **The percentiles beside the budget line** — and the sentence the
   MVD's perf line needs: *"my machine holds 60 fps"* only if the
   budget line says `0 of N frames over it` **and** the p99 sits under
   the budget with the margin your game's ambitions require. If frames
   miss, count them, name the phase that ate them (exercise 2's
   attribution), and say honestly which of the two it is: work the
   engine can shed, or wait the seam owns.

When your card says `0 of N over` on real hardware at real frame
rates, the checklist's perf line is checked **all the way** — and that
was always going to be your measurement to make, not this rig's (D12).
