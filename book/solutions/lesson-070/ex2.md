# Solution: exercise 2 — The row's two populations

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 070 — the mix's cost in the frame budget](../../lessons/part-3/lesson-070-audio-row.md).*

## The diff

```diff
{{#include ex2.patch}}
```

## Walkthrough

The patch keeps the two populations apart where the phase is measured:
a frame that fed a buffer mixed a whole buffer; a frame that fed none
mixed nothing. The account counts them and averages each — and the run
prints the two beside the demo's counts.

An undriven run has exactly one population, as the paced wait predicts:

```
engine: demo: 467 frames measured, 467 buffers fed, 21 effects fired, 2 music wraps
engine: audio: 467 feeding frames avg 0.035 ms, 0 quiet frames avg 0.000 ms
```

The wait wakes the loop at the buffer horizon, so every woken frame has
a buffer due: 467 frames, 467 feeds. The second population appears the
moment something else wakes the loop early — here, scripted input on
the arrow keys:

```
engine: demo: 609 frames measured, 584 buffers fed, 25 effects fired, 3 music wraps
engine: audio: 584 feeding frames avg 0.035 ms, 25 quiet frames avg 0.000 ms
engine: frame budget — 609 frames, avg 1.953 ms, worst 4.060 ms (frame 278)
engine:   audio        0.033       2%
```

Now reconcile three things. **The feeding frames against the work:**
one buffer is 735 output frames across all `AUDIO_MIXER_CHANNELS` = 16
channels — 11,760 channel pulls — and 0.035 ms over that is about 3 ns
a pull, the same figure lesson 068's harness measured for the mix.
**The quiet frames:** 0.000 ms at the printed precision — the two clock
reads around a test that says no buffer is due. **The row against the
populations:** the table's 0.033 ms is the two populations weighted by
their sizes — (584 × 0.035 + 25 × 0.000) / 609 = 0.034 ms from the
printed averages, 0.0334 ms from the log's own frame lines, the row's
0.033. The row and the populations cannot disagree: the row is *only*
their sum divided by their count. What the split adds is the reason —
96% of the frames carry the whole cost, so the row is essentially the
feeding frames' number.

The budget's question, from the same run's log: the worst frame is
frame 278 at 4.060 ms, and its `audio` reads 0.220 ms — about 5% of
that frame, against the phase's 2% share of the average frame. The
phase's share of the worst is bigger because the worst frame is a
whole-frame hiccup (its render is up too), and on a frame that happens
to feed, the mix's cost rides along. What would make this row the first
one to grow: more mixer channels, more frames per buffer, a higher
sample rate — each multiplies the pulls the mix makes — or a device
whose submit waits, which would land in this row exactly the way the
presentation's sync lands in `present`.
