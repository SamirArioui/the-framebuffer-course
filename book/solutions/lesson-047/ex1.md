# Solution: exercise 1 — Your machine's knees

{{#include ../../stability-horizon.md}}

*Solution for [Lesson 047 — the caches deep dive](../../lessons/part-2/lesson-047-caches.md).*

## The diff

```diff
{{#include ex1.patch}}
```

## Walkthrough

The prediction, from the machine's own description. This CPU reports L1d
48 KB, L2 3 MB, L3 20 MB, 64-byte lines. The walk touches two buffers, so
a row labeled *n* KB stands on *2n* KB of memory — the knees should fall
where `2n` crosses a cache:

- **L1d (48 KB)** — between the 16 KB row (32 KB touched) and the 48 KB
  row (96 KB touched).
- **L2 (3 MB)** — between 1 MB (2 MB touched) and 2 MB (4 MB touched).
- **L3 (20 MB)** — at the last row: 12 MB copied touches 24 MB, past it.

The extended sweep at `-O3`, pinned to one core:

```
engine: cache probe — copy walk, useful GB/s per stride
engine: working set   sequential    stride 64
engine:       4 KB        108.3          3.0
engine:      16 KB        116.5          4.6
engine:      48 KB         53.5          1.2
engine:      64 KB         56.3          1.2
engine:     128 KB         42.8          0.9
engine:     512 KB         49.7          0.9
engine:    1024 KB         40.4          0.9
engine:    2048 KB         23.7          0.5
engine:    4096 KB         19.2          0.4
engine:    8192 KB         16.9          0.3
engine:   12288 KB         11.5          0.3
```

All three knees land where predicted — and the shape between them is
worth as much as the drops:

- **16 KB → 48 KB: 116 → 53 GB/s.** The L1d knee, exactly on schedule:
  32 KB touched fits, 96 KB does not.
- **1 MB → 2 MB: 40 → 24 GB/s.** The L2 knee — gentle, because L3
  absorbs the difference before main memory does.
- **8 MB → 12 MB: 17 → 11.5 GB/s.** Past L3, main memory's speed.

Now the part the prompt asks about — the *distance* between prediction and
measurement. It is not zero, and the rows wobble: 16 KB reads *faster*
than 4 KB (116 vs 108), and 512 KB reads faster than 128 KB (50 vs 43).
Nothing about the cache hierarchy says it should. What that noise is:

- **The caches are shared.** L2 and L3 on this chip are unified and
  shared with the other cores; anything else the machine runs during the
  probe (and something always runs) displaces lines.
- **Frequency is not constant.** Boost clocks, thermal state, and power
  limits move the core's speed under sustained load — a 12 MB sweep
  running for seconds is exactly the workload that provokes it.
- **The probe itself has fixed costs** — loop setup, the clock reads —
  amortized over the measurement; small rows feel them more.

The lesson's rule from Part 0 restated for caches: *a measurement without
its machine is a rumor, and a measurement without its noise is a lie.*
Pin the core (`taskset`), take the shape, and treat single-digit
differences between adjacent rows as weather. The drops by factors of
two and seven are the climate.
