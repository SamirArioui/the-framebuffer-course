# Solution: exercise 1 — The twenty-first press

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 065 — channel allocation](../../lessons/part-3/lesson-065-allocation.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The diff is one probe: the sequence as a script of steps — a start or
an ending — walked on a scratch mixer through the engine's own
`MixerPlay`, with every landing channel, every ending, and every
dropped sound printed. The `occupant` array is the probe's own memory
of which effect holds which channel; a call is reported as a drop
exactly when the channel it landed on still held one. An ending is the
state the mix leaves at a sample's end — the channel goes inactive — so
the probe sets the same flag `ChannelFrame` sets at the sample's end,
and no mixing is needed to make a channel free.

The prediction, before any run. Fifteen presses on a fresh mixer take
the first free channel and find it at 1, 2, 3, and on to 15, stamped
`started` 1 through 15. Ending effects 4 and 9 frees channels 4 and 9,
and the walk takes the lowest free channel first: effect 16 lands on 4
and effect 17 on 9, stamped 16 and 17. Effect 18 meets a full pool —
the smallest `started` is 1, on channel 1 — so effect 1 is dropped and
channel 1 restarts at `started` 18. Effect 7 ends, effect 19 takes
channel 7 at 19. Effect 20 meets the full pool again: channel 1 now
holds the newest stamp in the pool, so the smallest `started` is 2 and
effect 2 drops from channel 2 at 20. The sequence leaves `mixer.order`
at 20 — one per sound that ever started.

| prediction | answer |
| --- | --- |
| effects 1–15 | channels 1–15, `started` 1–15 |
| effect 16 | channel 4 — first free, `started` 16 |
| effect 17 | channel 9 — first free, `started` 17 |
| effect 18 | channel 1, effect 1 dropped — `started` 18 |
| effect 19 | channel 7 — first free, `started` 19 |
| effect 20 | channel 2, effect 2 dropped — `started` 20 |
| `mixer.order` when the sequence is done | 20 |
| effect 21 (the boundary) | channel 3, effect 3 dropped — `started` 21 |

The run's log:

```
engine: alloc: effect  1 -> channel  1, order 1
engine: alloc: effect  2 -> channel  2, order 2
engine: alloc: effect  3 -> channel  3, order 3
...
engine: alloc: effect 15 -> channel 15, order 15
engine: alloc: effect  4 ends, channel  4 free
engine: alloc: effect  9 ends, channel  9 free
engine: alloc: effect 16 -> channel  4, order 16
engine: alloc: effect 17 -> channel  9, order 17
engine: alloc: effect 18 -> channel  1 (dropped effect  1), order 18
engine: alloc: effect  7 ends, channel  7 free
engine: alloc: effect 19 -> channel  7, order 19
engine: alloc: effect 20 -> channel  2 (dropped effect  2), order 20
engine: alloc: effect 21 -> channel  3 (dropped effect  3), order 21
```

Every line reconciles with the walk and the counter, and three details
are where this sequence earns its keep.

**The walk's order, not the ending order.** Effects 4 and 9 ended in
that order, but effect 16 takes channel 4 because the walk goes by
channel number and stops at the first free one — the order in which
channels were freed is never consulted. First free is a fact about the
pool, not about time.

**A stolen channel is stamped anew.** After effect 18 takes channel 1,
that channel holds the *newest* sound in the pool — `started` 18 — so
the next steal cannot land on it again: effect 20 goes to channel 2,
the next-oldest. The counter moves in one direction and every start
takes the next number, which is what keeps a busy pool cutting roughly
in start order instead of hammering one channel.

**Endings do not make a channel old.** Channels 4, 9, and 7 were freed
and refilled during the sequence, and they hold `started` 16, 17, and
19 — as new as the sounds that just arrived. Only starts stamp;
endings only free. "Oldest" is a fact about the sound playing now, and
nothing about whatever played on that channel before.

The boundary is the same relation one press later. Effect 21 meets a
full pool and drops effect 3 — the third sound of the run, the one that
has been heard through the whole sequence — landing on channel 3 at
`started` 21. And channel 0 appears nowhere in the log: it is never
offered to an effect and never a steal candidate, even though it holds
no sound at all. With music on it (lesson 066) every line above would
be identical.

What the probe cannot hide about itself: it marks endings directly
rather than running a sample out — the same state, reached by hand —
and every effect is the same tone, so the log says nothing about what
any of it sounds like. This question is answered in channels and
counters, and those the probe shows exactly.
